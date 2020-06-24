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
#include <codecvt>
#include <functional>
#include <locale>
#include <map>
#include <string>
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/imediaserver.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/iparameterclient.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/miditovst.h"
//#include "public.sdk/source/vst/basewrapper/basewrapper.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/optional.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/hosting/stringconvert.h"

/**
 * (1) VST3 Modules may implement any number of VST3 plugins.
 * (2) The vst3init opcode uses a singleton vst3_host_t instance to load a 
 *     VST3 Module, and obtains from the Module a VST3 PluginFactory.
 * (3) All of the VST3 ClassInfo objects exposed by the PluginFactory 
 *     are iterated.
 * (4) Either the first PlugProvider, or the PlugProviders that is named in 
 *     the vst3init call, is used to create a vst3_plugin_t instance, which 
 *     actually obtains the interfaces called by Csound to use the plugin.
 * (5) The vst3_plugin initializes its IComponent and IEditController interfaces 
 *     for communication with Csound.
 * (6) A handle to the vst3_plugin_t instance is returned by vst3init to the 
 *     user, who must pass it to all other vst3 opcodes.
 * (7) When Csound calls csoundModuleDestroy, the vst3_host_t instance 
 *     terminates all plugins and deallocates all state.
 *
 * The life cycle of the plugin is:
 * (1) 
 */
 
#define CSOUND_LIFECYCLE_DEBUG 1

namespace Steinberg {
    FUnknown* gStandardPluginContext;
};

namespace csound {
    
    /**
     * Persistent state:
     * (1) There is one and only one vst3_host_t instance in a process.
     * (2) There are zero or more vst3_plugin_t instances for each CSOUND 
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
    
    static inline MidiCCMapping initMidiCtrlerAssignment (Steinberg::Vst::IComponent* component, Steinberg::Vst::IMidiMapping* midiMapping) {
        MidiCCMapping midiCCMapping {};
        if (!midiMapping || !component) {
            return midiCCMapping;
        }
        int32 busses = std::min<int32> (component->getBusCount (Steinberg::Vst::kEvent, Steinberg::Vst::kInput), kMaxMidiMappingBusses);
        if (midiCCMapping[0][0].empty ()) {
            for (int32 b = 0; b < busses; b++) {
                for (int32 i = 0; i < kMaxMidiChannels; i++) {
                    midiCCMapping[b][i].resize (Steinberg::Vst::kCountCtrlNumber);
                }
            }
        }
        Steinberg::Vst::ParamID paramID;
        for (int32 b = 0; b < busses; b++) {
            for (int16 ch = 0; ch < kMaxMidiChannels; ch++) {
                for (int32 i = 0; i < Steinberg::Vst::kCountCtrlNumber; i++) {
                    paramID = Steinberg::Vst::kNoParamId;
                    if (midiMapping->getMidiControllerAssignment (b, ch, (Steinberg::Vst::CtrlNumber)i, paramID) ==
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

    static inline void assignBusBuffers (const Steinberg::Vst::IAudioClient::Buffers& buffers, Steinberg::Vst::HostProcessData& processData,
                                  bool unassign = false) {
        auto bufferIndex = 0;
        for (auto busIndex = 0; busIndex < processData.numOutputs; busIndex++) {
            auto channelCount = processData.outputs[busIndex].numChannels;
            for (auto chanIndex = 0; chanIndex < channelCount; chanIndex++) {
                if (bufferIndex < buffers.numOutputs) {
                    processData.setChannelBuffer (Steinberg::Vst::BusDirections::kOutput, busIndex, chanIndex,
                                                  unassign ? nullptr : buffers.outputs[bufferIndex]);
                    bufferIndex++;
                }
            }
        }
        bufferIndex = 0;
        for (auto busIndex = 0; busIndex < processData.numInputs; busIndex++) {
            auto channelCount = processData.inputs[busIndex].numChannels;
            for (auto chanIndex = 0; chanIndex < channelCount; chanIndex++) {
                if (bufferIndex < buffers.numInputs) {
                    processData.setChannelBuffer (Steinberg::Vst::BusDirections::kInput, busIndex, chanIndex,
                                                  unassign ? nullptr : buffers.inputs[bufferIndex]);
                    bufferIndex++;
                }
            }
        }
    }

    static inline void unassignBusBuffers (const Steinberg::Vst::IAudioClient::Buffers& buffers, Steinberg::Vst::HostProcessData& processData) {
        assignBusBuffers (buffers, processData, true);
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
        vst3_plugin_t () {};
        virtual ~vst3_plugin_t () override {
            // std::fprintf(stderr, "vst3_plugin_t deleting.\n");
        }
        bool process (Steinberg::Vst::IAudioClient::Buffers& buffers, int64_t continousFrames) override {
            if (!processor || !isProcessing) {
                return false;
            }
            preprocess (buffers, continousFrames);
            if (processor->process (processData) != Steinberg::kResultOk) {
                return false;
            }
            postprocess (buffers);
            return true;
        }
        bool setSamplerate (double value) override {
            if (sampleRate == value) {
                return true;
            }
            sampleRate = value;
            processContext.sampleRate = sampleRate;
            if (blockSize == 0) {
                return true;
            }
            return updateProcessSetup ();
        }
        bool setBlockSize (int32 value) override {
            if (blockSize == value) {
                return true;
            }
            blockSize = value;
            if (sampleRate == 0) {
                return true;
            }
            processData.prepare (*component, blockSize, Steinberg::Vst::kSample32);
            return updateProcessSetup ();
        }
        Steinberg::Vst::IAudioClient::IOSetup getIOSetup () const override {
            Steinberg::Vst::IAudioClient::IOSetup iosetup;
            auto count = component->getBusCount (Steinberg::Vst::kAudio, Steinberg::Vst::BusDirections::kOutput);
            for (int32_t i = 0; i < count; i++) {
                Steinberg::Vst::BusInfo info;
                if (component->getBusInfo (Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kOutput, i, info) !=
                    Steinberg::kResultOk) {
                        continue;
                    }
                for (int32_t j = 0; j < info.channelCount; j++) {
                    auto channelName = VST3::StringConvert::convert (info.name, 128); // TODO: 128???
                    iosetup.outputs.push_back (channelName + " " + std::to_string (j));
                }
            }
            count = component->getBusCount (Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kInput);
            for (int32_t i = 0; i < count; i++) {
                Steinberg::Vst::BusInfo info;
                if (component->getBusInfo (Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::BusDirections::kInput, i, info) != Steinberg::kResultOk) {
                    continue;
                }
                for (int32_t j = 0; j < info.channelCount; j++) {
                    auto channelName = VST3::StringConvert::convert (info.name, 128); // TODO: 128???
                    iosetup.inputs.push_back (channelName + " " + std::to_string (j));
                }
            }
            return iosetup;
        }
        // IMidiClient
        bool onEvent (const Steinberg::Vst::IMidiClient::Event& event, int32_t port) override {
            // Try to create Event first.
            if (processVstEvent (event, port)) {
                return true;
            }
            // In case this is no event it must be a parameter.
            if (processParamChange (event, port)) {
                return true;
            }
            return true;
        }
        Steinberg::Vst::IMidiClient::IOSetup getMidiIOSetup () const override {
            Steinberg::Vst::IMidiClient::IOSetup iosetup;
            auto count = component->getBusCount (Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput);
            for (int32_t i = 0; i < count; i++) {
                Steinberg::Vst::BusInfo info;
                if (component->getBusInfo (Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kInput, i, info) != Steinberg::kResultOk) {
                    continue;
                }
                auto busName = VST3::StringConvert::convert (info.name, 128); // TODO: 128???
                iosetup.inputs.push_back (busName);
            }
            count = component->getBusCount (Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kOutput);
            for (int32_t i = 0; i < count; i++) {
                Steinberg::Vst::BusInfo info;
                if (component->getBusInfo (Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::BusDirections::kOutput, i, info) !=
                    Steinberg::kResultOk) {
                    continue;
                }
                auto busName = VST3::StringConvert::convert (info.name, 128); // TODO: 128???
                iosetup.outputs.push_back (busName);
            }
            return iosetup;
        }
        // IParameterClient
        void setParameter (uint32  id, double value, int32 sampleOffset) override {
            paramTransferrer.addChange (id, value, sampleOffset);
        }
        bool initialize (CSOUND *csound_, const VST3::Hosting::ClassInfo &classInfo_, Steinberg::Vst::PlugProvider *provider_) {
            csound = csound_;
            provider = provider_;
            classInfo = classInfo_;
            component = provider->getComponent ();
            controller = provider->getController ();
            processor = component.get();
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMapping (controller);
            initProcessData ();
            paramTransferrer.setMaxParameters (1000);
            midiCCMapping = initMidiCtrlerAssignment (component, midiMapping);
            csound->Message(csound, "vst3_plugin::initialize completed.\n");
            return true;
        }
        void terminate () {
            //mediaServer = nullptr;
            ///Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component.get();
            if (!processor) {
                return;
            }
            processor->setProcessing (false);
            component->setActive (false);
        }
        void updateBusBuffers (Steinberg::Vst::IAudioClient::Buffers& buffers, Steinberg::Vst::HostProcessData& processData) {
            // Doesn't actually seem to be defined in the VST3 SDK or examples.
        }
        void initProcessData () {
            // processData.prepare is done in setBlockSize.
            processData.inputEvents = &eventList;
            processData.inputParameterChanges = &inputParameterChanges;
            processData.processContext = &processContext;
            initProcessContext ();
        }
        void initProcessContext () {
            processContext = {};
            processContext.tempo = 120;
        }
        bool updateProcessSetup () {
            ///Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component.get();
            if (!processor) {
                return false;
            }
            if (isProcessing) {
                if (processor->setProcessing (false) != Steinberg::kResultOk) {
                    return false;
                }
                if (component->setActive (false) != Steinberg::kResultOk) {
                    return false;
                }
            }
            Steinberg::Vst::ProcessSetup setup {Steinberg::Vst::kRealtime, Steinberg::Vst::kSample32, blockSize, sampleRate};
            if (processor->setupProcessing (setup) != Steinberg::kResultOk) {
                return false;
            }
            if (component->setActive (true) != Steinberg::kResultOk) {
                return false;
            }
            processor->setProcessing (true);
            isProcessing = true;
            return isProcessing;
        }
        void preprocess (Steinberg::Vst::IAudioClient::Buffers& buffers, int64_t continousFrames) {
            processData.numSamples = buffers.numSamples;
            processContext.continousTimeSamples = continousFrames;
            assignBusBuffers (buffers, processData);
            paramTransferrer.transferChangesTo (inputParameterChanges);
        }
        void postprocess (Steinberg::Vst::IAudioClient::Buffers& buffers) {
            eventList.clear ();
            inputParameterChanges.clearQueue ();
            unassignBusBuffers (buffers, processData);
        }
        bool isPortInRange (int32 port, int32 channel) const {
            return port < kMaxMidiMappingBusses && !midiCCMapping[port][channel].empty ();
        }
        bool processVstEvent (const Steinberg::Vst::IMidiClient::Event& event, int32 port) {
            auto vstEvent = Steinberg::Vst::midiToEvent (event.type, event.channel, event.data0, event.data1);
            if (vstEvent) {
                vstEvent->busIndex = port;
                if (eventList.addEvent (*vstEvent) != Steinberg::kResultOk) {
                    assert (false && "Event was not added to EventList!");
                }
                return true;
            }
            return false;
        }
        bool processParamChange (const Steinberg::Vst::IMidiClient::Event& event, int32 port) {
            auto paramMapping = [port, this] (int32 channel, Steinberg::Vst::MidiData data1) -> Steinberg::Vst::ParamID {
                if (!isPortInRange (port, channel)) {
                    return Steinberg::Vst::
                    kNoParamId;
                }
                return midiCCMapping[port][channel][data1];
            };
            auto paramChange =
                Steinberg::Vst::midiToParameter (event.type, event.channel, event.data0, event.data1, paramMapping);
            if (paramChange) {
                int32 index = 0;
                Steinberg::Vst::IParamValueQueue* queue =
                    inputParameterChanges.addParameterData ((*paramChange).first, index);
                if (queue) {
                    if (queue->addPoint (event.timestamp, (*paramChange).second, index) != Steinberg::kResultOk) {
                        assert (false && "Parameter point was not added to ParamValueQueue!");
                    }
                }
                return true;
            }
            return false;
        }
        void print_information() {
            Steinberg::TUID controllerClassTUID;
            if (component->getControllerClassId (controllerClassTUID) != Steinberg::kResultOk) {
                csound->Message(csound, "vst3_plugin: This component does not export an edit controller class ID!");
            }
            Steinberg::FUID controllerClassUID;
            controllerClassUID = Steinberg::FUID::fromTUID (controllerClassTUID);
            if (controllerClassUID.isValid () == false) {
                csound->Message(csound, "vst3_plugin: The edit controller class has no valid UID!");
            }
            char cidString[50];
            controllerClassUID.toRegistryString (cidString);
            // Class information.
            csound->Message(csound, "vst3_plugin: plugin controller class id: %s\n", cidString);
            csound->Message(csound, "vst3_plugin: UID:                    %s\n", classInfo.ID().toString().c_str());
            csound->Message(csound, "vst3_plugin: cardinality:            %i\n", classInfo.cardinality());
            csound->Message(csound, "vst3_plugin: category:               %s\n", classInfo.category().c_str());
            csound->Message(csound, "vst3_plugin: name:                   %s\n", classInfo.name().c_str());
            csound->Message(csound, "vst3_plugin: vendor:                 %s\n", classInfo.vendor().c_str());
            csound->Message(csound, "vst3_plugin: version:                %s\n", classInfo.version().c_str());
            csound->Message(csound, "vst3_plugin: sdkVersion:             %s\n", classInfo.sdkVersion().c_str());
            csound->Message(csound, "vst3_plugin: subCategoriesString:    %s\n", classInfo.subCategoriesString().c_str());
            csound->Message(csound, "vst3_plugin: classFlags:             %i\n", classInfo.classFlags());
            // Input busses.
            int32 n = component->getBusCount( Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::kInput);
            for (int32 i = 0; i < n; i++) {
                Steinberg::Vst::BusInfo busInfo = {};
                auto result = component->getBusInfo (Steinberg::Vst::MediaTypes::kAudio, Steinberg::Vst::kInput, i, busInfo);
                Steinberg::String name(busInfo.name);
                name.toMultiByte(Steinberg::kCP_Utf8);
                csound->Message(csound, "Buss: media type %d: direction %d: channels %d: name: %s bus type: %d flags: %d\n", 
                    busInfo.mediaType, 
                    busInfo.direction,
                    busInfo.channelCount,
                    name,
                    busInfo.busType,
                    busInfo.flags);
            }
            n = component->getBusCount( Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::kInput);
            for (int32 i = 0; i < n; i++) {
                Steinberg::Vst::BusInfo busInfo = {};
                auto result = component->getBusInfo (Steinberg::Vst::MediaTypes::kEvent, Steinberg::Vst::kInput, i, busInfo);
                Steinberg::String name(busInfo.name);
                name.toMultiByte(Steinberg::kCP_Utf8);
                csound->Message(csound, "Buss: media type %d: direction %d: channels %d: name: %s bus type: %d flags: %d\n", 
                    busInfo.mediaType, 
                    busInfo.direction,
                    busInfo.channelCount,
                    name,
                    busInfo.busType,
                    busInfo.flags);
            }
            // Output busses.
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
                    csound->Message(csound, "vst3_plugin: parameter: %4d: name: %-64s units: %-16s default: %9.4f\n", i, title.text8(), units.text8(), parameterInfo.defaultNormalizedValue);
                }
            }
        }
        CSOUND* csound = nullptr;
        Steinberg::IPtr<Steinberg::Vst::PlugProvider> provider;
        VST3::Hosting::ClassInfo classInfo;
        Steinberg::IPtr<Steinberg::Vst::IComponent> component;
        Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor;
        Steinberg::TUID controller_class_id;
        Steinberg::IPtr<Steinberg::Vst::IEditController> controller;
        Steinberg::Vst::HostProcessData processData;
        Steinberg::Vst::ProcessContext processContext;
        Steinberg::Vst::EventList eventList;
        Steinberg::Vst::ParameterChanges inputParameterChanges;
        Steinberg::Vst::ParameterChangeTransfer paramTransferrer;
        MidiCCMapping midiCCMapping;
        bool isProcessing = false;
        double sampleRate = 0;
        int32 blockSize = 0;
        std::string name;
    };

    static vst3_plugin_t *static_plugin = nullptr;
        
    /**
     * Singleton class for managing all persistent VST3 state.
     */
    class vst3_host_t : public Steinberg::Vst::HostApplication {
    public:
        vst3_host_t(){};
        static vst3_host_t &get_instance();
        vst3_host_t(vst3_host_t const&) = delete;
        void operator=(vst3_host_t const&) = delete;
        ~vst3_host_t () noexcept override {
            std::fprintf(stderr, "vst3_host_t deleting.\n");
        }
        /**
         * Loads a VST3 Module and obtains all plugins in it.
         */
        int load_module (CSOUND *csound, const std::string& module_pathname, const std::string &plugin_name, bool verbose) {
            int handle = -1;
            if (verbose == true) {
                csound->Message(csound, "vst3init: loading module: %s\n", module_pathname.c_str());
            }
            std::string error;
            auto module = VST3::Hosting::Module::create(module_pathname, error);
            if (!module) {
                std::string reason = "Could not create Module for file:";
                reason += module_pathname;
                reason += "\nError: ";
                reason += error;
                csound->Message(csound, "vst3init: error: %s\n", reason.c_str());
                return handle;
            }
            modules_for_pathnames[module_pathname] = module;
            auto factory = module->getFactory ();
            int count = 0;
            // Loop over all class infos from the module, but create only the requested plugin.
            // This gives the user a list of all plugins available from the module.
            VST3::Hosting::ClassInfo classInfo_; 
            for (auto& classInfo : factory.classInfos ()) {
                count = count + 1;
                if (verbose == true) {
                    csound->Message(csound, "vst3init: found classInfo number: %d\n", count);
                    csound->Message(csound, "vst3init: UID:                    %s\n", classInfo.ID().toString().c_str());
                    csound->Message(csound, "vst3init: cardinality:            %i\n", classInfo.cardinality());
                    csound->Message(csound, "vst3init: category:               %s\n", classInfo.category().c_str());
                    csound->Message(csound, "vst3init: name:                   %s\n", classInfo.name().c_str());
                    csound->Message(csound, "vst3init: vendor:                 %s\n", classInfo.vendor().c_str());
                    csound->Message(csound, "vst3init: version:                %s\n", classInfo.version().c_str());
                    csound->Message(csound, "vst3init: sdkVersion:             %s\n", classInfo.sdkVersion().c_str());
                    csound->Message(csound, "vst3init: subCategoriesString:    %s\n", classInfo.subCategoriesString().c_str());
                    csound->Message(csound, "vst3init: classFlags:             %i\n\n", classInfo.classFlags());
                }
                if ((classInfo.category () == kVstAudioEffectClass) &&  (plugin_name == classInfo.name())) {
                    classInfo_ = classInfo;
                }
            }
            auto plugProvider = owned (NEW Steinberg::Vst::PlugProvider (factory, classInfo_, true));
            if (!plugProvider) {
                std::string error = "No VST3 Audio Module class found in file ";
                error += module_pathname;
                csound->Message(csound, "vst3init: error: %s\n", error.c_str());
                return -1;;
            } 
            auto vst3_plugin = std::make_shared<vst3_plugin_t>();
            static_plugin = vst3_plugin.get();
            vst3_plugin->initialize(csound, classInfo_, plugProvider);
            Steinberg::TUID controllerClassTUID;
            if (vst3_plugin->component->getControllerClassId (controllerClassTUID) != Steinberg::kResultOk) {
                csound->Message(csound, "vst3init: This component does not export an edit controller class ID!");
            }
            Steinberg::FUID controllerClassUID;
            controllerClassUID = Steinberg::FUID::fromTUID (controllerClassTUID);
            if (controllerClassUID.isValid () == false) {
                csound->Message(csound, "vst3init: The edit controller class has no valid UID!");
            }
            char cidString[50];
            controllerClassUID.toRegistryString (cidString);
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
    
    inline vst3_host_t &vst3_host_t::get_instance() {
        static vst3_host_t vst3_host;
        if (Steinberg::gStandardPluginContext == nullptr) {
            Steinberg::gStandardPluginContext = &vst3_host;
        }
        return vst3_host;
    };
    
    inline static vst3_plugin_t *get_plugin(MYFLT *handle) {
        auto plugin = vst3_host_t::get_instance().plugin_for_handle(handle);
        return plugin;
    }
    
    struct VST3INIT : public csound::OpcodeBase<VST3INIT> {
        // Outputs.
        MYFLT *i_vst3_handle;
        // Inputs.
        MYFLT *i_module_pathname;
        MYFLT *i_plugin_name;
        MYFLT *i_verbose;
        int init(CSOUND *csound) {
            int result = OK;
            auto &host = csound::vst3_host_t::get_instance();
            std::string module_pathname = ((STRINGDAT *)i_module_pathname)->data;
            std::string plugin_name = ((STRINGDAT *)i_plugin_name)->data;
            *i_vst3_handle = host.load_module(csound, module_pathname, plugin_name, (bool)*i_verbose);
            auto vst3_plugin = get_plugin(i_vst3_handle);
            log(csound, "\nvst3init: created plugin: \"%s\": address: %p handle: %d\n", plugin_name.c_str(), vst3_plugin, (int) *i_vst3_handle);
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
            vst3_plugin->print_information();
            return result;
        };
    };

    struct VST3AUDIO : public csound::OpcodeBase<VST3AUDIO> {
        // Outputs.
        MYFLT *aouts[32];
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *ains[32];
        // State.
        MYFLT   zerodbfs;
        size_t  ksmps;
        size_t  pluginInChannels;
        size_t  pluginOutChannels;
        size_t  opcodeInChannels;
        size_t  opcodeOutChannels;
        size_t  inputChannels;
        size_t  outputChannels;
        vst3_plugin_t vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            return result;
        };
        int audio(CSOUND *csound) {
            int result = OK;
            return result;
        };
    };

    struct VST3MIDIOUT : public csound::OpcodeBase<VST3MIDIOUT> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *kstatus;
        MYFLT *kchan;
        MYFLT *kdata1;
        MYFLT *kdata2;
        // State.
        size_t  vstHandle;
        int     prvMidiData;
        vst3_plugin_t vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            return result;
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            return result;
        };
    };

    struct VST3NOTE : public csound::OpcodeBase<VST3NOTE> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *kchan;
        MYFLT *knote;
        MYFLT *kveloc;
        MYFLT *kdur;
        // State.
        size_t  vstHandle;
        int     chn;
        int     note;
        size_t  framesRemaining;
        vst3_plugin_t vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
            return result;
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            return result;
        };
    };

    struct VST3PARAMGET : public csound::OpcodeBase<VST3PARAMGET> {

        // Outputs.
        MYFLT *kvalue;
        // Intputs.
        MYFLT *i_vst3_handle;
        MYFLT *kparam;
        // State.
        vst3_plugin_t vst3_plugin;
    };

    struct VST3PARAMSET : public csound::OpcodeBase<VST3PARAMSET> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *kparam;
        MYFLT *kvalue;
        // State.
        MYFLT   oldkparam;
        MYFLT   oldkvalue;
        vst3_plugin_t vst3_plugin;
    };

    struct VST3BANKLOAD : public csound::OpcodeBase<VST3BANKLOAD> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *ibank;
        // State.
        vst3_plugin_t vst3_plugin;
    };

    struct VST3PROGSET : public csound::OpcodeBase<VST3PROGSET> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *iprogram;
        // State.
        vst3_plugin_t vst3_plugin;
    };

    struct VST3EDIT : public csound::OpcodeBase<VST3EDIT> {
        // Inputs.
        MYFLT *i_vst3_handle;
        // State.
        vst3_plugin_t vst3_plugin;
    };

    struct VST3TEMPO : public csound::OpcodeBase<VST3TEMPO> { 
        // Inputs.
        MYFLT *tempo;
        MYFLT *i_vst3_handle;
        // State.
        vst3_plugin_t vst3_plugin;
    };

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
    static OENTRY localops[] = {
        {"vst3init", sizeof(VST3INIT), 0, 1, "i", "TTo", &VST3INIT::init_, 0, 0},
        {"vst3info", sizeof(VST3INFO), 0, 1, "", "i", &VST3INFO::init_, 0, 0},
        {   "vst3audio", sizeof(VST3AUDIO), 0, 3, "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm",
            "iy", &VST3AUDIO::init_, &VST3AUDIO::audio_, 0
        },
        {   "vst3midiout", sizeof(VST3MIDIOUT), 0, 3, "", "ikkkk", &VST3MIDIOUT::init_,
            &VST3MIDIOUT::kontrol_, 0
        },
        {   "vst3paramget", sizeof(VST3PARAMGET), 0, 3, "k", "ik", &VST3PARAMGET::init_,
            &VST3PARAMGET::kontrol_, 0
        },
        {   "vst3paramset", sizeof(VST3PARAMSET), 0, 3, "", "ikk", &VST3PARAMSET::init_,
            &VST3PARAMSET::kontrol_, 0
        },
        {"vst3bankload", sizeof(VST3BANKLOAD), 0, 1, "", "iT", &VST3BANKLOAD::init_, 0, 0},
        {"vst3progset", sizeof(VST3PROGSET), 0, 1, "", "ii", &VST3PROGSET::init_, 0, 0},
        {"vst3edit", sizeof(VST3EDIT), 0, 1, "", "i", &VST3EDIT::init_, 0, 0},
        {
            "vst3tempo", sizeof(VST3TEMPO), 0, 2, "", "ki", 0, &VST3TEMPO::init_,
            0 /*, &vstedit_deinit*/
        },
        {   "vst3note", sizeof(VST3NOTE), 0, 3, "", "iiiii", &VST3NOTE::init_,
            &VST3NOTE::kontrol_, 0
        },
        {"vst3banksave", sizeof(VST3BANKLOAD), 0, 1, "", "iT", &VST3BANKLOAD::init_, 0, 0},
        {0, 0, 0, 0, 0, 0, (SUBR)0, (SUBR)0, (SUBR)0}
    };
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern "C" {
#if defined(CSOUND_LIFECYCLE_DEBUG)
    std::hash<std::thread::id> thread_hasher;
#endif
    
    PUBLIC int csoundModuleCreate(CSOUND *csound) {
#if defined(CSOUND_LIFECYCLE_DEBUG)
        csound->Message(csound, "csoundModuleCreate: csound: %p thread: %ld\n", csound,
                        thread_hasher(std::this_thread::get_id()));
#endif
        int result = 0;
        return result;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound) {
#if defined(CSOUND_LIFECYCLE_DEBUG)
        csound->Message(csound, "csoundModuleInit: csound: %p thread: %ld\n", csound,
                        thread_hasher(std::this_thread::get_id()));
#endif
        OENTRY *ep = (OENTRY *)&(csound::localops[0]);
        int err = 0;
        while (ep->opname != NULL) {
            err |= csound->AppendOpcode(csound, ep->opname, ep->dsblksiz, ep->flags,
                                        ep->thread, ep->outypes, ep->intypes,
                                        (int (*)(CSOUND *, void *))ep->iopadr,
                                        (int (*)(CSOUND *, void *))ep->kopadr,
                                        (int (*)(CSOUND *, void *))ep->aopadr);
            ep++;
        }
        return err;
    }

    PUBLIC int csoundModuleDestroy(CSOUND *csound) {
#if defined(CSOUND_LIFECYCLE_DEBUG)
        csound->Message(csound, "csoundModuleDestroy: csound: %p thread: %ld\n",
                        csound,
                        thread_hasher(std::this_thread::get_id()));
#endif
        auto &vst3_host = csound::vst3_host_t::get_instance();
        auto range = vst3_host.vst3_plugins_for_csounds.equal_range(csound);
        for (auto it = range.first; it != range.second; ++it) {
            it->second = nullptr;
        }
        vst3_host.vst3_plugins_for_csounds.erase(csound);
        return 0;
    }
} // extern "C"
 