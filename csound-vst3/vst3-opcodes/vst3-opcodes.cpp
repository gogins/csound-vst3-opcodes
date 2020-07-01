/**
 * V S T 3   H O S T   O P C O D E S   F O R   C S O U N D
 * 
 * Author: Michael Gogins
 * http://michaelgogins.tumblr.com
 * michael dot gogins at gmail dot com
 * 
 * This code is licensed under the terms of the 
 * GNU General Public License, Version 3.
 *
 * For project maintainers: This code is derived from the hosting samples in  
 * Steinberg's VST3 SDK. However, I have removed all namespace aliases in 
 * favor of spelling out all namespaces. 
 */
 
// This one must come first to avoid conflict with Csound #defines.
#include <thread>

#include <csound/csdl.h>
#include <csound/csound.h>
#include <csound/OpcodeBase.hpp>
#include <cstring>
#include <codecvt>
#include <fstream>
#include <functional>
#include <locale>
#include <map>
#include <string>
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/ivstunits.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/imediaserver.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/iparameterclient.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/miditovst.h"
#include "public.sdk/samples/vst-hosting/editorhost/source/editorhost.h"
#include "public.sdk/samples/vst-hosting/editorhost/source/platform/iapplication.h"
#include "public.sdk/samples/vst-hosting/editorhost/source/platform/iplatform.h"
#include "public.sdk/samples/vst-hosting/editorhost/source/platform/iwindow.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/optional.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/hosting/stringconvert.h"
#include "public.sdk/source/vst/vstpresetfile.h"

/**
 *(1) VST3 Modules may implement any number of VST3 plugins.
 *(2) The vst3init opcode uses a singleton vst3_host_t instance to load a 
 *     VST3 Module, and obtains from the Module a VST3 PluginFactory.
 *(3) All of the VST3 ClassInfo objects exposed by the PluginFactory 
 *     are iterated.
 *(4) Either the first PlugProvider, or the PlugProviders that is named in 
 *     the vst3init call, is used to create a vst3_plugin_t instance, which 
 *     actually obtains the interfaces called by Csound to use the plugin.
 *(5) The vst3_plugin initializes its IComponent and IEditController interfaces 
 *     for communication with Csound.
 *(6) A handle to the vst3_plugin_t instance is returned by vst3init to the 
 *     user, who must pass it to all other vst3 opcodes.
 *(7) When Csound calls csoundModuleDestroy, the vst3_host_t instance 
 *     terminates all plugins and deallocates all state.
 */
 
#define DEBUGGING 1

namespace Steinberg {
    FUnknown* gStandardPluginContext;
};

namespace csound {
    
    /**
     * Persistent state:
     *(1) There is one and only one vst3_host_t instance in a process.
     *(2) There are zero or more vst3_plugin_t instances for each CSOUND 
     *     instance, and these plugins are deleted when csoundModuleDestroy 
     *     is called.
     */
    
    enum
    {
        kMaxMidiMappingBusses = 4,
        kMaxMidiChannels = 16
    };
    
    using Controllers = std::vector<int32>;
    using Channels = std::array<Controllers, kMaxMidiChannels>;
    using Busses = std::array<Channels, kMaxMidiMappingBusses>;
    using MidiCCMapping = Busses;
        
    static inline MidiCCMapping initMidiCtrlerAssignment(Steinberg::Vst::IComponent* component, Steinberg::Vst::IMidiMapping* midiMapping) {
        MidiCCMapping midiCCMapping {};
        if (!midiMapping || !component) {
            return midiCCMapping;
        }
        int32 busses = std::min<int32>(component->getBusCount(Steinberg::Vst::kEvent, Steinberg::Vst::kInput), kMaxMidiMappingBusses);
        if (midiCCMapping[0][0].empty()) {
            for (int32 b = 0; b < busses; b++) {
                for (int32 i = 0; i < kMaxMidiChannels; i++) {
                    midiCCMapping[b][i].resize(Steinberg::Vst::kCountCtrlNumber);
                }
            }
        }
        Steinberg::Vst::ParamID paramID;
        for (int32 b = 0; b < busses; b++) {
            for (int16 ch = 0; ch < kMaxMidiChannels; ch++) {
                for (int32 i = 0; i < Steinberg::Vst::kCountCtrlNumber; i++) {
                    paramID = Steinberg::Vst::kNoParamId;
                    if (midiMapping->getMidiControllerAssignment(b, ch,(Steinberg::Vst::CtrlNumber)i, paramID) ==
                        Steinberg::kResultTrue) {
                        midiCCMapping[b][ch][i] = paramID;
                    } else {
                        midiCCMapping[b][ch][i] = Steinberg::Vst::kNoParamId;
                    }
                }
            }
        }
        return midiCCMapping;
    }
    
    /**
     * This class manages one instance of one plugin and all of its 
     * communications with Csound, including audio input and output, 
     * MIDI input and output, and parameter input and output.
     */    
    struct vst3_plugin_t : 
            public Steinberg::Vst::IAudioClient, 
            public Steinberg::Vst::IMidiClient, 
            public Steinberg::Vst::IParameterClient {
        vst3_plugin_t() {};
        virtual ~vst3_plugin_t() override {
#if defined(DEBUGGING)
            std::fprintf(stderr, "vst3_plugin_t::~vst3_plugin_t.\n");
#endif
        }
        bool process(Steinberg::Vst::IAudioClient::Buffers &buffers, int64_t continuousFrames) override {
            csound->Message(csound, "vst3_plugin_t: wrong process method called!\n");
            return false;
        }
        void preprocess(int64_t continousFrames) {
#if defined(DEBUGGING2)
            csound->Message(csound, "vst3_plugin_t::preprocess: hostProcessData.numSamples: %d.\n", hostProcessData.numSamples);
#endif
            hostProcessData.numSamples = blockSize;
            processContext.continousTimeSamples = continousFrames;
            paramTransferrer.transferChangesTo(inputParameterChanges);
        }
        void postprocess() { 
            eventList.clear();
            inputParameterChanges.clearQueue();
        }
        bool process(int64_t continuous_frames) /* override */ {
#if defined(DEBUGGING2)
            csound->Message(csound, "vst3_plugin_t::process: time in frames: %ld.\n", continuous_frames);
#endif
            if (!processor || !isProcessing) {
                csound->Message(csound, "vst3_plugin_t::process: no processor or not processing!\n");
                return false;
            }
            preprocess(continuous_frames);
            auto result = processor->process(hostProcessData);
            if (result != Steinberg::kResultOk) {
                csound->Message(csound, "vst3_plugin_t::process: returned %d!\n", result);
                return false;
            }
            postprocess();
            return true;
        }
        bool setSamplerate(double value) override {
            if (sampleRate == value) {
                return true;
            }
            sampleRate = value;
            processContext.sampleRate = sampleRate;
            if (blockSize == 0) {
                return true;
            }
            return updateProcessSetup();
        }
        bool setBlockSize(int32 value) override {
            if (blockSize == value) {
                return true;
            }
            blockSize = value;
            if (sampleRate == 0) {
                return true;
            }
            hostProcessData.prepare(*component, blockSize, Steinberg::Vst::kSample32);
            auto result = updateProcessSetup();
            csound->Message(csound, "vst3_plugin::setBlockSize: sampleRate: %9.4f\n", sampleRate);
            csound->Message(csound, "vst3_plugin::setBlockSize: blockSize:  %9d\n", blockSize);
            return result;
        }
        Steinberg::Vst::IAudioClient::IOSetup getIOSetup() const override {
            Steinberg::Vst::IAudioClient::IOSetup iosetup;
            auto count = component->getBusCount(Steinberg::Vst::kAudio, Steinberg::Vst::BusDirections::kOutput);
            for (int32_t i = 0; i < count; i++) {
                Steinberg::Vst::BusInfo info;
                if (component->getBusInfo(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kOutput, i, info) !=
                    Steinberg::kResultOk) {
                        continue;
                }
                for (int32_t j = 0; j < info.channelCount; j++) {
                    auto channelName = VST3::StringConvert::convert(info.name, 128);
                    iosetup.outputs.push_back(channelName + " " + std::to_string(j));
                }
            }
            count = component->getBusCount(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kInput);
            for (int32_t i = 0; i < count; i++) {
                Steinberg::Vst::BusInfo info;
                if (component->getBusInfo(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kInput, i, info) != Steinberg::kResultOk) {
                    continue;
                }
                for (int32_t j = 0; j < info.channelCount; j++) {
                    auto channelName = VST3::StringConvert::convert(info.name, 128);
                    iosetup.inputs.push_back(channelName + " " + std::to_string(j));
                }
            }
            return iosetup;
        }
        // IMidiClient
        bool onEvent(const Steinberg::Vst::IMidiClient::Event& event, int32_t port) override {
            // Try to create Event first.
            if (processVstEvent(event, port)) {
                return true;
            }
            // In case this is no event it must be a parameter.
            if (processParamChange(event, port)) {
                return true;
            }
            return true;
        }
        Steinberg::Vst::IMidiClient::IOSetup getMidiIOSetup() const override {
            Steinberg::Vst::IMidiClient::IOSetup iosetup;
            auto count = component->getBusCount(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput);
            for (int32_t i = 0; i < count; i++) {
                Steinberg::Vst::BusInfo info;
                if (component->getBusInfo(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput, i, info) != Steinberg::kResultOk) {
                    continue;
                }
                auto busName = VST3::StringConvert::convert(info.name, 128);
                iosetup.inputs.push_back(busName);
            }
            count = component->getBusCount(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kOutput);
            for (int32_t i = 0; i < count; i++) {
                Steinberg::Vst::BusInfo info;
                if (component->getBusInfo(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kOutput, i, info) !=
                    Steinberg::kResultOk) {
                    continue;
                }
                auto busName = VST3::StringConvert::convert(info.name, 128);
                iosetup.outputs.push_back(busName);
            }
            return iosetup;
        }
        // IParameterClient
        void setParameter(uint32  id, double value, int32 sampleOffset) override {
            paramTransferrer.addChange(id, value, sampleOffset);
        }
        bool initialize(CSOUND *csound_, const VST3::Hosting::ClassInfo &classInfo_, Steinberg::Vst::PlugProvider *provider_) {
            csound = csound_;
            provider = provider_;
            classInfo = classInfo_;
            component = provider->getComponent();
            controller = provider->getController();
            processor = component.get();
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMapping(controller);
            initProcessData();
            paramTransferrer.setMaxParameters(1000);
            midiCCMapping = initMidiCtrlerAssignment(component, midiMapping);
            csound->Message(csound, "vst3_plugin_t::initialize completed.\n");
            return true;
        }
        void terminate() {
            if (!processor) {
                return;
            }
            processor->setProcessing(false);
            component->setActive(false);
        }
        void initProcessData() {
            hostProcessData.inputEvents = &eventList;
            hostProcessData.inputParameterChanges = &inputParameterChanges;
            hostProcessData.processContext = &processContext;
            initProcessContext();
        }
        void initProcessContext() {
            processContext = {};
            // Csound's default tempo is one beat per second.
            processContext.tempo = 60;
        }
        bool updateProcessSetup() {
            if (!processor) {
                csound->Message(csound, "vst3_plugin_t::updateProcessSetup: null IProcessor.\n");
                return false;
            }
            if (isProcessing) {
                if (processor->setProcessing(false) != Steinberg::kResultOk) {
                    csound->Message(csound, "vst3_plugin_t::updateProcessSetup: Could not stop processing.\n");
                    return false;
                }
                if (component->setActive(false) != Steinberg::kResultOk) {
                    csound->Message(csound, "vst3_plugin_t::setActive: Could not deactivate component.\n");
                    return false;
                }
            }
            Steinberg::Vst::ProcessSetup setup {Steinberg::Vst::kRealtime, Steinberg::Vst::kSample32, blockSize, sampleRate};
            if (processor->setupProcessing(setup) != Steinberg::kResultOk) {
                csound->Message(csound, "vst3_plugin_t::updateProcessSetup: setupProcessing returned 'false'.\n");
                return false;
            }
            if (component->setActive(true) != Steinberg::kResultOk) {
                csound->Message(csound, "vst3_plugin_t::updateProcessSetup: setActive returned 'false'.\n");
                return false;
            }
            processor->setProcessing(true);
            isProcessing = true;
            return isProcessing;
        }
        bool isPortInRange(int32 port, int32 channel) const {
            return port < kMaxMidiMappingBusses && !midiCCMapping[port][channel].empty();
        }
        bool processVstEvent(const Steinberg::Vst::IMidiClient::Event& event, int32 port) {
            auto vstEvent = Steinberg::Vst::midiToEvent(event.type, event.channel, event.data0, event.data1);
            if (vstEvent) {
                vstEvent->busIndex = port;
                if (eventList.addEvent(*vstEvent) != Steinberg::kResultOk) {
                    assert(false && "Event was not added to EventList!");
                }
                return true;
            }
            return false;
        }
        bool processParamChange(const Steinberg::Vst::IMidiClient::Event& event, int32 port) {
            auto paramMapping = [port, this](int32 channel, Steinberg::Vst::MidiData data1) -> Steinberg::Vst::ParamID {
                if (!isPortInRange(port, channel)) {
                    return Steinberg::Vst::kNoParamId;
                }
                return midiCCMapping[port][channel][data1];
            };
            auto paramChange =
                Steinberg::Vst::midiToParameter(event.type, event.channel, event.data0, event.data1, paramMapping);
            if (paramChange) {
                int32 index = 0;
                Steinberg::Vst::IParamValueQueue* queue =
                    inputParameterChanges.addParameterData((*paramChange).first, index);
                if (queue) {
                    if (queue->addPoint(event.timestamp,(*paramChange).second, index) != Steinberg::kResultOk) {
                        assert(false && "Parameter point was not added to ParamValueQueue!");
                    }
                }
                return true;
            }
            return false;
        }
        void print_information() {
            Steinberg::TUID controllerClassTUID;
            if (component->getControllerClassId(controllerClassTUID) != Steinberg::kResultOk) {
                csound->Message(csound, "vst3_plugin_t: This component does not export an edit controller class ID!");
            }
            Steinberg::FUID controllerClassUID;
            controllerClassUID = Steinberg::FUID::fromTUID(controllerClassTUID);
            if (controllerClassUID.isValid() == false) {
                csound->Message(csound, "vst3_plugin_t: The edit controller class has no valid UID!");
            }
            char cidString[50];
            controllerClassUID.toRegistryString(cidString);
            // Class information.
            csound->Message(csound, "vst3_plugin_t: class:      controller class id: %s\n", cidString);
            csound->Message(csound, "               class:      UID:                 %s\n", classInfo.ID().toString().c_str());
            csound->Message(csound, "               class:      cardinality:         %i\n", classInfo.cardinality());
            csound->Message(csound, "               class:      category:            %s\n", classInfo.category().c_str());
            csound->Message(csound, "               class:      name:                %s\n", classInfo.name().c_str());
            csound->Message(csound, "               class:      vendor:              %s\n", classInfo.vendor().c_str());
            csound->Message(csound, "               class:      version:             %s\n", classInfo.version().c_str());
            csound->Message(csound, "               class:      sdkVersion:          %s\n", classInfo.sdkVersion().c_str());
            csound->Message(csound, "               class:      subCategoriesString: %s\n", classInfo.subCategoriesString().c_str());
            csound->Message(csound, "               class:      classFlags:          %i\n", classInfo.classFlags());
            csound->Message(csound, "               can process 32 bit samples: %d\n", processor->canProcessSampleSize(Steinberg::Vst::kSample32));
            csound->Message(csound, "               can process 64 bit samples: %d\n", processor->canProcessSampleSize(Steinberg::Vst::kSample64));
            csound->Message(csound, "               Csound samples: %d bits\n", int((sizeof(MYFLT) * 8)));
            // Input and output busses.
            // There is no ID in a BusInfo.
            int32 n = component->getBusCount( Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::kInput);
            for (int32 i = 0; i < n; i++) {
                Steinberg::Vst::BusInfo busInfo = {};
                auto result = component->getBusInfo(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::kInput, i, busInfo);
                auto name = VST3::StringConvert::convert(busInfo.name);
                csound->Message(csound, "               buss:       direction: %s  media: %s  channels: %3d  bus type: %s  flags: %d  name: %-32s \n", 
                    busInfo.direction == 0 ? "Input " : "Output",
                    busInfo.mediaType == 0 ? "Audio" : "Event",
                    busInfo.channelCount,
                    busInfo.busType == 0 ? "Main" : "Aux ",
                    busInfo.flags,
                    name.data());
            }
            n = component->getBusCount( Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::kInput);
            for (int32 i = 0; i < n; i++) {
                Steinberg::Vst::BusInfo busInfo = {};
                auto result = component->getBusInfo(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::kInput, i, busInfo);
                auto name = VST3::StringConvert::convert(busInfo.name);
                csound->Message(csound, "               buss:       direction: %s  media: %s  channels: %3d  bus type: %s  flags: %d  name: %-32s \n", 
                    busInfo.direction == 0 ? "Input " : "Output",
                    busInfo.mediaType == 0 ? "Audio" : "Event",
                    busInfo.channelCount,
                    busInfo.busType == 0 ? "Main" : "Aux ",
                    busInfo.flags,
                    name.data());
            }
            n = component->getBusCount( Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::kOutput);
            for (int32 i = 0; i < n; i++) {
                Steinberg::Vst::BusInfo busInfo = {};
                auto result = component->getBusInfo(Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::kOutput, i, busInfo);
                auto name = VST3::StringConvert::convert(busInfo.name);
                csound->Message(csound, "               buss:       direction: %s  media: %s  channels: %3d  bus type: %s  flags: %d  name: %-32s \n", 
                    busInfo.direction == 0 ? "Input " : "Output",
                    busInfo.mediaType == 0 ? "Audio" : "Event",
                    busInfo.channelCount,
                    busInfo.busType == 0 ? "Main" : "Aux ",
                    busInfo.flags,
                    name.data());
            }
            n = component->getBusCount( Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::kOutput);
            for (int32 i = 0; i < n; i++) {
                Steinberg::Vst::BusInfo busInfo = {};
                auto result = component->getBusInfo(Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::kOutput, i, busInfo);
                auto name = VST3::StringConvert::convert(busInfo.name);
                 csound->Message(csound, "               buss:       direction: %s  media: %s  channels: %3d  bus type: %s  flags: %d  name: %-32s \n", 
                    busInfo.direction == 0 ? "Input " : "Output",
                    busInfo.mediaType == 0 ? "Audio" : "Event",
                    busInfo.channelCount,
                    busInfo.busType == 0 ? "Main" : "Aux ",
                    busInfo.flags,
                    name.data());
            }
            // Parameters.
            if (controller) {
                int32 n = controller->getParameterCount();
                Steinberg::Vst::ParameterInfo parameterInfo;
                 for (int i = 0; i < n; ++i) {
                    controller->getParameterInfo(i, parameterInfo);
                    Steinberg::String title(parameterInfo.title);
                    title.toMultiByte(Steinberg::kCP_Utf8);
                    Steinberg::String units(parameterInfo.units);
                    units.toMultiByte(Steinberg::kCP_Utf8);
                    csound->Message(csound, "               parameter:  index: %4d: id: %12d name: %-64s units: %-16s default: %9.4f\n", i, parameterInfo.id, title.text8(), units.text8(), parameterInfo.defaultNormalizedValue);
                }
            }
            // Units, program lists, and programs, in a flat list.
            Steinberg::FUnknownPtr<Steinberg::Vst::IUnitInfo> i_unit_info(controller);
            if (i_unit_info) {
                auto unit_count = i_unit_info->getUnitCount();
                for (auto unit_index = 0; unit_index < unit_count; ++unit_index) {
                    Steinberg::Vst::UnitInfo unit_info;
                    i_unit_info->getUnitInfo(unit_index, unit_info);
                    auto program_list_count = i_unit_info->getProgramListCount();
                    for (auto program_list_index = 0; program_list_index < program_list_count; ++program_list_index) {
                        Steinberg::Vst::ProgramListInfo program_list_info;
                        if (i_unit_info->getProgramListInfo(program_list_index, program_list_info) == Steinberg::kResultOk) {
                             for (auto program_index = 0; program_index < program_list_info.programCount; ++program_index) {
                                Steinberg::Vst::TChar program_name[256];
                                i_unit_info->getProgramName(unit_info.programListId, program_index, program_name);
                                csound->Message(csound, "               unit:       id: %4d(parent id: %4d) name: %-32s program list: id: %12d(index: %4d) program: id: %4d name: %s\n", 
                                    unit_info.id, 
                                    unit_info.parentUnitId, 
                                    VST3::StringConvert::convert(unit_info.name).c_str(), 
                                    unit_info.programListId, 
                                    program_list_index, 
                                    program_index, 
                                    VST3::StringConvert::convert(program_name).data());
                             }
                        }
                    }
                }
            }
        }
        void showPluginEditorWindow() {
            auto view = owned(controller->createView(Steinberg::Vst::ViewType::kEditor));
            if (!view) {
                //Steinberg::Vst::EditorHost::IPlatform::instance().kill(-1, "EditController does not provide its own editor");
            }
            Steinberg::ViewRect plugViewSize {};
            auto result = view->getSize(&plugViewSize);
            if (result != Steinberg::kResultTrue) {
                //Steinberg::Vst::EditorHost::IPlatform::instance().kill(-1, "Could not get editor view size");
            }
            //~ auto viewRect = Steinberg::Vst::EditorHost::ViewRectToRect(plugViewSize);
            //~ auto windowController = std::make_shared<Steinberg::WindowController>(view);
            //~ auto window = Steinberg::Vst::EditorHost::IPlatform::instance().createWindow(
            //~ "Editor", viewRect.size, view->canResize() == kResultTrue, windowController);
            //~ if (!window) {
                //~ Steinberg::Vst::EditorHost::IPlatform::instance().kill(-1, "Could not create window");
            //~ }
            //~ window->show();
        }
        void setTempo(double new_tempo) {
            processContext.tempo = new_tempo;
        }
        CSOUND* csound = nullptr;
        Steinberg::IPtr<Steinberg::Vst::PlugProvider> provider;
        VST3::Hosting::ClassInfo classInfo;
        Steinberg::IPtr<Steinberg::Vst::IComponent> component;
        Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor;
        Steinberg::TUID controller_class_id;
        Steinberg::IPtr<Steinberg::Vst::IEditController> controller;
        Steinberg::Vst::HostProcessData hostProcessData;
        Steinberg::Vst::ProcessContext processContext;
        Steinberg::Vst::EventList eventList;
        Steinberg::Vst::ParameterChanges inputParameterChanges;
        Steinberg::Vst::ParameterChangeTransfer paramTransferrer;
        //std::shared_ptr<Steinberg::Vst::EditorHost::WindowController> windowController;
        MidiCCMapping midiCCMapping;
        bool isProcessing = false;
        double sampleRate = 0;
        int32 blockSize = 0;
        // Incremented for every MIDI Note On message created, 
        // and paired with the corresponding Note Off message, 
        // for the lifetime of this plugin instance.
        int note_id = 0;
        std::string name;
    };

    /**
     * Singleton class for managing all persistent VST3 state.
     */
    class vst3_host_t : public Steinberg::Vst::HostApplication {
    public:
        vst3_host_t(){};
        static vst3_host_t &get_singleton();
        vst3_host_t(vst3_host_t const&) = delete;
        void operator=(vst3_host_t const&) = delete;
        ~vst3_host_t() noexcept override {
            std::fprintf(stderr, "vst3_host_t deleting.\n");
        }
        /**
         * Loads a VST3 Module and obtains all plugins in it.
         */
        int load_module(CSOUND *csound, const std::string& module_pathname, const std::string &plugin_name, bool verbose) {
            int handle = -1;
            if (verbose == true) {
                csound->Message(csound, "vst3_host_t::load_module: loading: %s\n", module_pathname.c_str());
            }
            std::string error;
            auto module = VST3::Hosting::Module::create(module_pathname, error);
            if (!module) {
                std::string reason = "Could not create Module for file:";
                reason += module_pathname;
                reason += "\nError: ";
                reason += error;
                csound->Message(csound, "vst3_host_t::load_module: error: %s\n", reason.c_str());
                return handle;
            }
            modules_for_pathnames[module_pathname] = module;
            auto factory = module->getFactory();
            int count = 0;
            // Loop over all class infos from the module, but create only the requested plugin.
            // This gives the user a list of all plugins available from the module.
            VST3::Hosting::ClassInfo classInfo_; 
            for (auto& classInfo : factory.classInfos()) {
                count = count + 1;
                if (verbose == true) {
                    csound->Message(csound, "vst3_host_t::load_module: found classInfo number: %d\n", count);
                    csound->Message(csound, "                          UID:                    %s\n", classInfo.ID().toString().c_str());
                    csound->Message(csound, "                          cardinality:            %i\n", classInfo.cardinality());
                    csound->Message(csound, "                          category:               %s\n", classInfo.category().c_str());
                    csound->Message(csound, "                          name:                   %s\n", classInfo.name().c_str());
                    csound->Message(csound, "                          vendor:                 %s\n", classInfo.vendor().c_str());
                    csound->Message(csound, "                          version:                %s\n", classInfo.version().c_str());
                    csound->Message(csound, "                          sdkVersion:             %s\n", classInfo.sdkVersion().c_str());
                    csound->Message(csound, "                          subCategoriesString:    %s\n", classInfo.subCategoriesString().c_str());
                    csound->Message(csound, "                          classFlags:             %i\n\n", classInfo.classFlags());
                }
                if ((classInfo.category() == kVstAudioEffectClass) && (plugin_name == classInfo.name())) {
                    classInfo_ = classInfo;
                }
            }
            auto plugProvider = owned(NEW Steinberg::Vst::PlugProvider(factory, classInfo_, true));
            if (!plugProvider) {
                std::string error = "No VST3 Audio Module class found in file ";
                error += module_pathname;
                csound->Message(csound, "vst3_host_t::load_module: error: %s\n", error.c_str());
                return -1;;
            } 
            auto vst3_plugin = std::make_shared<vst3_plugin_t>();
            vst3_plugin->initialize(csound, classInfo_, plugProvider);
            Steinberg::TUID controllerClassTUID;
            if (vst3_plugin->component->getControllerClassId(controllerClassTUID) != Steinberg::kResultOk) {
                csound->Message(csound, "vst3_host_t::load_module: This component does not export an edit controller class ID!");
            }
            Steinberg::FUID controllerClassUID;
            controllerClassUID = Steinberg::FUID::fromTUID(controllerClassTUID);
            if (controllerClassUID.isValid() == false) {
                csound->Message(csound, "vst3_host_t::load_module: The edit controller class has no valid UID!");
            }
            char cidString[50];
            controllerClassUID.toRegistryString(cidString);
            handle = vst3_plugins_for_handles.size();
            vst3_plugins_for_handles.push_back(vst3_plugin);
            return handle;
        }
        vst3_plugin_t *plugin_for_handle(MYFLT *handle) {
            auto handle_value = *handle;
            auto index = static_cast<size_t>(handle_value);
            return vst3_plugins_for_handles[index].get();
        }
        std::map<std::string, VST3::Hosting::Module::Ptr> modules_for_pathnames;
        std::multimap<CSOUND *, std::shared_ptr<vst3_plugin_t>> vst3_plugins_for_csounds;
        // Handles for vst3_plugin_t instances are indexes into a list of 
        // plugins. It's not possible to simply store the address of a 
        // vst3_plugin_t instance in a Csound opcode parameter, because the 
        // address might be 64 bits and the MYFLT parameter might be only 32 
        // bits. 
        std::vector<std::shared_ptr<vst3_plugin_t>> vst3_plugins_for_handles;
    };
    
    inline vst3_host_t &vst3_host_t::get_singleton() {
        static vst3_host_t vst3_host;
        if (Steinberg::gStandardPluginContext == nullptr) {
            Steinberg::gStandardPluginContext = &vst3_host;
        }
        return vst3_host;
    };
    
    inline static vst3_plugin_t *get_plugin(MYFLT *handle) {
        auto plugin = vst3_host_t::get_singleton().plugin_for_handle(handle);
        return plugin;
    }
    
    struct VST3AUDIO : public csound::OpcodeBase<VST3AUDIO> {
        // Outputs.
        MYFLT *a_output_channels[32];
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *a_input_channels[32];
        // State.
        MYFLT zerodbfs;
        int opcode_input_channel_count;
        int plugin_input_channel_count;
        int input_channel_count;
        int opcode_output_channel_count;
        int plugin_output_channel_count;
        int output_channel_count;
        vst3_plugin_t *vst3_plugin;
        float **plugin_input_channels_32;
        double **plugin_input_channels_64;
        float **plugin_output_channels_32;
        double **plugin_output_channels_64;
        Steinberg::Vst::SymbolicSampleSizes plugin_sample_size;
        int frame_count;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            auto sr = csound->GetSr(csound);
            vst3_plugin->setSamplerate(sr);
            // This also creates the host buffers.
            frame_count = ksmps();
            vst3_plugin->setBlockSize(frame_count);
            // Because Csound and the plugin may not use the same sample word 
            // size, allowance must be made for different buffer shapes and 
            // sample word sizes, and the audio streams must be copied in and 
            // out word for word. 
            // TODO: find 'Main' buffers instead of assuming they are at 
            // buss index 0?
            opcode_input_channel_count = input_arg_count();
            auto &process_data = vst3_plugin->hostProcessData;
            if (process_data.numInputs > 0) {
                plugin_input_channel_count = process_data.inputs[0].numChannels;
                plugin_input_channels_32 = process_data.inputs[0].channelBuffers32;
            } else {
                plugin_input_channel_count = 0;
            }
            input_channel_count = std::min(opcode_input_channel_count, plugin_input_channel_count);
            opcode_output_channel_count = output_arg_count();
            if (process_data.numOutputs > 0) {
                plugin_output_channel_count = process_data.outputs[0].numChannels;
                plugin_output_channels_32 = process_data.outputs[0].channelBuffers32;
            } else {
                plugin_output_channel_count = 0;
            }
            output_channel_count = std::min(opcode_output_channel_count, plugin_output_channel_count);
            log(csound, "vst3audio: input channels: %3d  output channels: %3d processing: %d\n", input_channel_count, output_channel_count, vst3_plugin->isProcessing);
            return result;
        };
        int audio(CSOUND *csound) {
            int result = OK;
            size_t current_time_in_frames = csound->GetCurrentTimeSamples(csound);
            // Because this opcode should always be on, and because the plugins 
            // _themselves_ are actually responsible for scheduling by sample 
            // frame, ksmps_offset and ksmps_no_end have no effect and are not 
            // used.
            if (plugin_sample_size == Steinberg::Vst::kSample32) {
                for (int channel_index = 0; channel_index < input_channel_count; ++channel_index) {
                    for (int frame_index = 0; frame_index < frame_count; ++frame_index) {
                        plugin_input_channels_32[channel_index][frame_index] = a_input_channels[channel_index][frame_index];
                    }
                }
                vst3_plugin->process(current_time_in_frames);
                for (int channel_index = 0; channel_index < output_channel_count; ++channel_index) {
                    for (int frame_index = 0; frame_index < frame_count; ++frame_index) {
#if defined(DEBUGGING2)
                        log(csound, "vst3audio::audio for 32 bits: sample[%4d][%4d]: opcode: %f plugin: %f\n", 
                            channel_index, frame_index, a_output_channels[channel_index][frame_index], plugin_output_channels_32[channel_index][frame_index]);
#endif
                        a_output_channels[channel_index][frame_index] = plugin_output_channels_32[channel_index][frame_index];
                    }
                }
            } else {
                for (int channel_index = 0; channel_index < input_channel_count; ++channel_index) {
                    for (int frame_index = 0; frame_index < frame_count; ++frame_index) {
                        plugin_input_channels_64[channel_index][frame_index] = a_input_channels[channel_index][frame_index];
                    }
                }
                vst3_plugin->process(current_time_in_frames);
                for (int channel_index = 0; channel_index < output_channel_count; ++channel_index) {
                    for (int frame_index = 0; frame_index < frame_count; ++frame_index) {
                        a_output_channels[channel_index][frame_index] = plugin_output_channels_64[channel_index][frame_index];
#if defined(DEBUGGING2)
                        log(csound, "vst3audio::audio for 64 bits: sample[%4d][%4d]: opcode: %f plugin: %f\n", 
                            channel_index, frame_index, a_output_channels[channel_index][frame_index], plugin_output_channels_64[channel_index][frame_index]);
#endif
                    }
                }
            }
            return result;
        };
    };

    struct VST3INFO : public csound::OpcodeBase<VST3INFO> {
        // Inputs.
        MYFLT *i_vst3_handle;
        // State.
        vst3_plugin_t *vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            log(csound, "vst3info::init: printing plugin information...\n");
            vst3_plugin->print_information();
            return result;
        };
    };

    struct VST3INIT : public csound::OpcodeBase<VST3INIT> {
        // Outputs.
        MYFLT *i_vst3_handle;
        // Inputs.
        MYFLT *i_module_pathname;
        MYFLT *i_plugin_name;
        MYFLT *i_verbose;
        int init(CSOUND *csound) {
            int result = OK;
            auto &host = csound::vst3_host_t::get_singleton();
            std::string module_pathname =((STRINGDAT *)i_module_pathname)->data;
            std::string plugin_name =((STRINGDAT *)i_plugin_name)->data;
            *i_vst3_handle = host.load_module(csound, module_pathname, plugin_name,(bool)*i_verbose);
            auto vst3_plugin = get_plugin(i_vst3_handle);
            log(csound, "\nvst3init::init: created plugin: \"%s\": address: %p handle: %d\n", plugin_name.c_str(), vst3_plugin,(int) *i_vst3_handle);
            return result;
        };
    };

    struct VST3EDIT : public csound::OpcodeBase<VST3EDIT> {
        // Inputs.
        MYFLT *i_vst3_handle;
        // State.
        vst3_plugin_t *vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            vst3_plugin->showPluginEditorWindow();
            return result;
        };
    };

    struct VST3MIDIOUT : public csound::OpcodeBase<VST3MIDIOUT> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *k_status;
        MYFLT *k_channel;
        MYFLT *k_data1;
        MYFLT *k_data2;
        // State.
        // MIDI channel message parts.
        uint8_t status;
        uint8_t channel;
        uint8_t data1;
        uint8_t data2;
        Steinberg::Vst::Event midi_channel_message;
        Steinberg::Vst::Event prior_midi_channel_message;
        vst3_plugin_t *vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            prior_midi_channel_message = midi_channel_message;
            vst3_plugin = get_plugin(i_vst3_handle);
            return result;
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            status = static_cast<uint8_t>(*k_status) & 0xF0;
            channel = static_cast<uint8_t>(*k_channel) & 0x0F;
            data1 = static_cast<uint8_t>(*k_data1);
            data2 = static_cast<uint8_t>(*k_data2);
            auto event = Steinberg::Vst::midiToEvent(status, channel, data1, data2);
            if (event) {
                midi_channel_message = *event;
                if (std::memcmp(&prior_midi_channel_message, &midi_channel_message, sizeof(Steinberg::Vst::Event)) != 0) {
                    if (vst3_plugin->eventList.addEvent(midi_channel_message) != Steinberg::kResultOk) {
                        log(csound, "vst3midiout: addEvent error.\n");
                    }
                    std::memcpy(&prior_midi_channel_message, &midi_channel_message, sizeof(Steinberg::Vst::Event));
                }
            }
            return result;
        };
    };

    struct VST3NOTE : public csound::OpcodeNoteoffBase<VST3NOTE> {
        // Outputs.
        MYFLT *i_note_id;
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *i_channel;
        MYFLT *i_key;
        MYFLT *i_velocity;
        MYFLT *i_duration;
        // State.
        int16 channel;		// channel index in event bus
        MYFLT key;
        int16 pitch;		// range [0, 127] = [C-2, G8] with A3=440Hz(12-TET)
        float tuning;		// 1.f = +1 cent, -1.f = -1 cent
        float velocity;		// range [0.0, 1.0]
        int32 length;		// in sample frames(optional, Note Off has to follow in any case!)
        int32 noteId;		// note identifier (if not available then -1)
        Steinberg::Vst::Event note_on_event;
        Steinberg::Vst::Event note_off_event;
        size_t framesRemaining;
        vst3_plugin_t *vst3_plugin;
        MYFLT start_time;
        MYFLT onTime;
        MYFLT duration;
        MYFLT offTime;
        MYFLT deltaTime;
        int delta_frames;
        bool on = false;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            start_time = csound->GetCurrentTimeSamples(csound) / csound->GetSr(csound);
            onTime = opds.insdshead->p2.value;
            duration = *i_duration;
            deltaTime = onTime - start_time;
            int delta_frames = 0;
            if (deltaTime > 0) {
                delta_frames = int(deltaTime * csound->GetSr(csound));
            }
            // Use the warped p3 to schedule the note off message.
            if (duration > FL(0.0)) {
                offTime = start_time + MYFLT(opds.insdshead->p3.value);
                // In case of real-time performance with indefinite p3...
            } else if (duration == FL(0.0)) {
#if defined(DEBUGGING)
                csound->Message(csound,
                                    Str("vstnote::init: not scheduling 0 duration note.\n"));
#endif
                return OK;
            } else {
                offTime = start_time + FL(1000000.0);
            }
            channel = static_cast<int16>(*i_channel) & 0xf;
            // Split the real-valued MIDI key number
            // into an integer key number and an integer number of cents(plus or
            // minus 50 cents).
            key = *i_key;
            pitch = static_cast<int16>(key + 0.5);
            tuning =(double(*i_key) - double(key)) * double(100.0);
            velocity = *i_velocity;
            velocity = velocity / 127.;
            // Ensure that the opcode instance is still active when we are scheduled
            // to turn the note off!
            opds.insdshead->xtratim = opds.insdshead->xtratim + 2;
            on = true;
            vst3_plugin->note_id++;
            note_on_event.type = Steinberg::Vst::Event::EventTypes::kNoteOnEvent;
            note_on_event.sampleOffset = delta_frames;
            note_on_event.noteOn.channel = static_cast<int16_t>(*i_channel);
            note_on_event.noteOn.pitch = pitch;
            note_on_event.noteOn.tuning = tuning;
            note_on_event.noteOn.velocity = velocity;
            note_on_event.noteOn.length = duration * csound->GetSr(csound);
            note_on_event.noteOn.noteId = vst3_plugin->note_id;
            note_off_event.type = Steinberg::Vst::Event::EventTypes::kNoteOffEvent;
            note_off_event.noteOff.channel = note_on_event.noteOn.channel;
            note_off_event.noteOff.pitch = note_on_event.noteOn.pitch;
            note_off_event.noteOff.tuning = note_on_event.noteOn.tuning;
            note_off_event.noteOff.velocity = 0;
            note_off_event.noteOff.noteId = note_on_event.noteOn.noteId;
#if defined(DEBUGGING)
            log(csound, "vst3note::init:    note_on_event time:          %f\n", start_time);
            log(csound, "                   note_on_event.type:          %d\n", note_on_event.type);
            log(csound, "                   note_on_event.sampleOffset:  %d\n", note_on_event.sampleOffset);
            log(csound, "                   note_on_event.channel:       %d\n", note_on_event.noteOn.channel);
            log(csound, "                   note_on_event.pitch:         %d\n", note_on_event.noteOn.pitch);
            log(csound, "                   note_on_event.tuning:        %f\n", note_on_event.noteOn.tuning);
            log(csound, "                   note_on_event.velocity:      %f\n", note_on_event.noteOn.velocity);
            log(csound, "                   note_on_event.length:        %d\n", note_on_event.noteOn.length);
            log(csound, "                   note_on_event.noteId:        %d\n", note_on_event.noteOn.noteId);
#endif
            if (vst3_plugin->eventList.addEvent(note_on_event) != Steinberg::kResultOk) {
                log(csound, "vst3note::init: addEvent error for Note On.\n");
            }
            return result;
        }
        int noteoff(CSOUND *csound) {
            int result = OK;
            // TODO: delta_frames?
#if defined(DEBUGGING)
            log(csound, "vst3note::noteoff: note_off_event time:         %f\n", csound->GetCurrentTimeSamples(csound) / csound->GetSr(csound));
            log(csound, "                   note_off_event.type:         %d\n", note_off_event.type);
            log(csound, "                   note_off_event.sampleOffset: %d\n", note_off_event.sampleOffset);
            log(csound, "                   note_off_event.channel:      %d\n", note_off_event.noteOff.channel);
            log(csound, "                   note_off_event.pitch:        %d\n", note_off_event.noteOff.pitch);
            log(csound, "                   note_off_event.tuning:       %f\n", note_off_event.noteOff.tuning);
            log(csound, "                   note_off_event.velocity:     %f\n", note_off_event.noteOff.velocity);
            log(csound, "                   note_off_event.noteId:       %d\n", note_off_event.noteOff.noteId);
#endif
            if (vst3_plugin->eventList.addEvent(note_off_event) != Steinberg::kResultOk) {
                log(csound, "vst3note: addEvent error for Note Off.\n");
            }
            return result;
            
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            return result;
        };
    };

    struct VST3PARAMGET : public csound::OpcodeBase<VST3PARAMGET> {
        // Outputs.
        MYFLT *k_parameter_value;
        // Intputs.
        MYFLT *i_vst3_handle;
        MYFLT *k_parameter_id;
        // State.
        vst3_plugin_t *vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            return result;
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            *k_parameter_value = vst3_plugin->controller->getParamNormalized(*k_parameter_id);
            return result;
        };
    };

    struct VST3PARAMSET : public csound::OpcodeBase<VST3PARAMSET> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *k_parameter_id;
        MYFLT *k_parameter_value;
        // State.
        vst3_plugin_t *vst3_plugin;
        Steinberg::int32 parameter_id;
        double parameter_value;
        Steinberg::int32 prior_parameter_id;
        double prior_parameter_value;
        double start_time;
        double on_time;
        double delta_time;
        size_t delta_frames;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            return result;
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            parameter_id = static_cast<Steinberg::int32>(*k_parameter_id);
            parameter_value = static_cast<double>(*k_parameter_value);
            if (parameter_id != prior_parameter_id || parameter_value != prior_parameter_value) {
                on_time = opds.insdshead->p2.value;
                delta_time = on_time - start_time;
                int delta_frames = 0;
                if (delta_time > 0) {
                    delta_frames = int(delta_time * csound->GetSr(csound));
                }
                vst3_plugin->setParameter(parameter_id, parameter_value, delta_frames);
                prior_parameter_id = parameter_id;
                prior_parameter_value = parameter_value;
            }
            return result;
        };
    };

    struct VST3PRESETLOAD : public csound::OpcodeBase<VST3PRESETLOAD> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *S_preset_filepath;
        // State.
        vst3_plugin_t *vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            std::string preset_filepath = ((STRINGDAT *)S_preset_filepath)->data;
            std::fstream input_stream(preset_filepath.c_str(), std::fstream::in | std::fstream::binary);
            Steinberg::MemoryStream memory_stream;
            char c;
            int32_t bytes_to_write = 1;
            int32_t bytes_written = 0;
            while (input_stream.get(c)) {
                memory_stream.write(static_cast<void *>(&c), bytes_to_write, &bytes_written);
            }
            input_stream.close();
            auto class_id = Steinberg::FUID::fromTUID(vst3_plugin->classInfo.ID().data());
            auto loaded = Steinberg::Vst::PresetFile::loadPreset(&memory_stream, class_id, vst3_plugin->component,
                             vst3_plugin->controller);
            if (loaded == false) {
                warn(csound, "vst3presetload: failed to load: %s\n", preset_filepath.c_str());
                result = NOTOK;
            } else {
                log(csound, "vst3presetload::init: loaded: %s\n", preset_filepath.c_str());
            }
            return result;
        };
    };
    
    struct VST3PRESETSAVE : public csound::OpcodeBase<VST3PRESETSAVE> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *S_preset_filepath;
        // State.
        vst3_plugin_t *vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            Steinberg::MemoryStream memory_stream;
            auto class_id = Steinberg::FUID::fromTUID(vst3_plugin->classInfo.ID().data());
            auto saved = Steinberg::Vst::PresetFile::savePreset(&memory_stream, class_id, vst3_plugin->component,
                             vst3_plugin->controller);
            std::string preset_filepath =((STRINGDAT *)S_preset_filepath)->data;
            if (saved == false) {
                warn(csound, "vst3presetsave::init: failed to save preset: %s\n", preset_filepath.c_str());
                return NOTOK;
            }
            std::fstream output_stream(preset_filepath.c_str(), std::fstream::out | std::fstream::trunc);
            output_stream.write(memory_stream.getData(), memory_stream.getSize());
            output_stream.close();
                log(csound, "vst3presetsave::init: saved: %s\n", preset_filepath.c_str());
            return result;
        };
    };

    struct VST3TEMPO : public csound::OpcodeBase<VST3TEMPO> { 
        // Inputs.
        MYFLT *k_tempo;
        MYFLT *i_vst3_handle;
        // State.
        vst3_plugin_t *vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(i_vst3_handle);
            return result;
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            vst3_plugin->setTempo(*k_tempo);
            return result;
        };
    };

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
    static OENTRY localops[] = {
        {"vst3audio",       sizeof(VST3AUDIO),      0, 3, "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm", "M", &VST3AUDIO::init_, &VST3AUDIO::audio_, 0},
        {"vst3info",        sizeof(VST3INFO),       0, 1, "", "i", &VST3INFO::init_, 0, 0}, 
        {"vst3init",        sizeof(VST3INIT),       0, 1, "i", "TTo", &VST3INIT::init_, 0, 0},
        {"vst3edit",        sizeof(VST3EDIT),       0, 1, "", "i", &VST3EDIT::init_, 0, 0},
        {"vst3midiout",     sizeof(VST3MIDIOUT),    0, 3, "", "ikkkk", &VST3MIDIOUT::init_, &VST3MIDIOUT::kontrol_, 0},
        {"vst3note",        sizeof(VST3NOTE),       0, 3, "i", "iiiii", &VST3NOTE::init_, &VST3NOTE::kontrol_, 0},
        {"vst3paramget",    sizeof(VST3PARAMGET),   0, 3, "k", "ik", &VST3PARAMGET::init_, &VST3PARAMGET::kontrol_, 0},
        {"vst3paramset",    sizeof(VST3PARAMSET),   0, 3, "", "ikk", &VST3PARAMSET::init_, &VST3PARAMSET::kontrol_, 0},
        {"vst3presetload",  sizeof(VST3PRESETLOAD), 0, 1, "", "iT", &VST3PRESETLOAD::init_, 0, 0},
        {"vst3presetsave",  sizeof(VST3PRESETSAVE), 0, 1, "", "iT", &VST3PRESETSAVE::init_, 0, 0},
        {"vst3tempo",       sizeof(VST3TEMPO),      0, 2, "", "ki", 0, &VST3TEMPO::init_, 0 /*, &vstedit_deinit*/ },
        {0, 0, 0, 0, 0, 0,(SUBR)0,(SUBR)0,(SUBR)0}
    };
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern "C" {
#if defined(DEBUGGING)
    std::hash<std::thread::id> thread_hasher;
#endif
    
    PUBLIC int csoundModuleCreate(CSOUND *csound) {
#if defined(DEBUGGING)
        csound->Message(csound, "csoundModuleCreate: csound: %p thread: %ld\n", csound,
                        thread_hasher(std::this_thread::get_id()));
#endif
        int result = 0;
        return result;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound) {
#if defined(DEBUGGING)
        csound->Message(csound, "csoundModuleInit: csound: %p thread: %ld\n", csound,
                        thread_hasher(std::this_thread::get_id()));
#endif
        OENTRY *ep =(OENTRY *)&(csound::localops[0]);
        int err = 0;
        while (ep->opname != NULL) {
            err |= csound->AppendOpcode(csound, ep->opname, ep->dsblksiz, ep->flags,
                                        ep->thread, ep->outypes, ep->intypes,
                                       (int(*)(CSOUND *, void *))ep->iopadr,
                                       (int(*)(CSOUND *, void *))ep->kopadr,
                                       (int(*)(CSOUND *, void *))ep->aopadr);
            ep++;
        }
        return err;
    }

    PUBLIC int csoundModuleDestroy(CSOUND *csound) {
#if defined(DEBUGGING)
        csound->Message(csound, "csoundModuleDestroy: csound: %p thread: %ld\n",
                        csound,
                        thread_hasher(std::this_thread::get_id()));
#endif
        auto &vst3_host = csound::vst3_host_t::get_singleton();
        auto range = vst3_host.vst3_plugins_for_csounds.equal_range(csound);
        for (auto it = range.first; it != range.second; ++it) {
            it->second = nullptr;
        }
        vst3_host.vst3_plugins_for_csounds.erase(csound);
        return 0;
    }
} // extern "C"
 