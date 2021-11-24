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
 
#define DEBUGGING 0
#define PARAMETER_DEBUGGING 1
#define PROCESS_DEBUGGING 0
#define EDITOR_IMPLEMENTED 0 
 
// This one must come first to avoid conflict with Csound #defines.
#include <thread>

#include <csdl.h>
#include <csound.h>
#include <OpcodeBase.hpp>
#include <array>
#include <cstring>
#include <codecvt>
#include <fstream>
#include <functional>
#include <locale>
#include <map>
#include <string>
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/gui/iplugviewcontentscalesupport.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/ivstunits.h"
#if EDITOR_IMPLEMENTED
#include "public.sdk/samples/vst-hosting/audiohost/source/media/imediaserver.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/iparameterclient.h"
#endif
#include "public.sdk/samples/vst-hosting/audiohost/source/media/miditovst.h"
#if EDITOR_IMPLEMENTED
#include "public.sdk/samples/vst-hosting/editorhost/source/editorhost.h"
#include "public.sdk/samples/vst-hosting/editorhost/source/platform/appinit.h"
#include "public.sdk/samples/vst-hosting/editorhost/source/platform/iapplication.h"
#include "public.sdk/samples/vst-hosting/editorhost/source/platform/iplatform.h"
#include "public.sdk/samples/vst-hosting/editorhost/source/platform/iwindow.h"
#endif
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "base/source/fstring.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/utility/optional.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/utility/stringconvert.h"
#include "public.sdk/source/vst/vstpresetfile.h"

/**
 * (1) VST3 Modules may implement any number of VST3 plugins.
 * (2) The vst3init opcode uses a singleton vst3_host_t instance to load a 
 *     VST3 Module, and obtains from the Module a VST3 PluginFactory.
 * (3) All of the VST3 ClassInfo objects exposed by the PluginFactory 
 *     are iterated.
 * (4) The plugin that is named in the vst3init call is used to create a 
 *     vst3_plugin_t instance, which actually obtains the interfaces called 
 *     by Csound to use the plugin.
 * (5) The vst3_plugin initializes its IComponent, IAudioProcessor, and 
 *     IEditController interfaces for communication with Csound.
 * (6) A handle to the vst3_plugin_t instance is returned by vst3init to the 
 *     user, who must pass it to all other vst3 opcodes.
 * (7) When Csound calls csoundModuleDestroy, the vst3_host_t instance 
 *     terminates all plugins and deallocates all state.
 */
 
namespace csound {
    
    class vst3_host_t;
    class vst3_plugin_t;
    
    typedef csound::heap_object_manager_t<vst3_host_t> vst3hosts;

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
    
#if EDITOR_IMPLEMENTED
        
    struct CsoundWindowController : public Steinberg::Vst::EditorHost::IWindowController, public Steinberg::IPlugFrame
    {
    public:
        CsoundWindowController (const Steinberg::IPtr<Steinberg::IPlugView>& plugView_) : plugView(plugView_) {}
        ~CsoundWindowController () noexcept override {}
        void onShow (Steinberg::Vst::EditorHost::IWindow& w) override {
            SMTG_DBPRT1 ("onShow called (%p)\n", (void*)&w);
            window = &w;
            if (!plugView) {
                return;
            }
            auto platformWindow = window->getNativePlatformWindow ();
            if (plugView->isPlatformTypeSupported (platformWindow.type) != Steinberg::kResultTrue) {
                Steinberg::Vst::EditorHost::IPlatform::instance ().kill (-1, std::string ("PlugView does not support platform type:") +
                                                     platformWindow.type);
            }
            plugView->setFrame (this);
            if (plugView->attached (platformWindow.ptr, platformWindow.type) != Steinberg::kResultTrue) {
                Steinberg::Vst::EditorHost::IPlatform::instance ().kill (-1, "Attaching PlugView failed");
            }
        }
        void onClose (Steinberg::Vst::EditorHost::IWindow& w) override {
            SMTG_DBPRT1 ("onClose called (%p)\n", (void*)&w);
            closePlugView ();
            // TODO maybe quit only when the last window is closed
            ///Steinberg::Vst::EditorHost::IPlatform::instance ().quit ();
        }
        void onResize (Steinberg::Vst::EditorHost::IWindow& w, Steinberg::Vst::EditorHost::Size newSize) override {
            SMTG_DBPRT1 ("onResize called (%p)\n", (void*)&w);
            if (plugView) {
                Steinberg::ViewRect r {};
                r.right = newSize.width;
                r.bottom = newSize.height;
                Steinberg::ViewRect r2 {};
                if (plugView->getSize (&r2) == Steinberg::kResultTrue && std::memcmp(&r, &r2, sizeof(Steinberg::ViewRect)) != 0) {
                    plugView->onSize (&r);
                }
            }
        }
        Steinberg::Vst::EditorHost::Size constrainSize (Steinberg::Vst::EditorHost::IWindow& w, Steinberg::Vst::EditorHost::Size requestedSize) override {
            SMTG_DBPRT1 ("constrainSize called (%p)\n", (void*)&w);
            Steinberg::ViewRect r {};
            r.right = requestedSize.width;
            r.bottom = requestedSize.height;
            if (plugView && plugView->checkSizeConstraint (&r) != Steinberg::kResultTrue)
            {
                plugView->getSize (&r);
            }
            requestedSize.width = r.right - r.left;
            requestedSize.height = r.bottom - r.top;
            return requestedSize;
        }
        void onContentScaleFactorChanged (Steinberg::Vst::EditorHost::IWindow& window, float newScaleFactor) override {
            SMTG_DBPRT1 ("onContentScaleFactorChanged called (%p)\n", (void*)&window);
            Steinberg::FUnknownPtr<Steinberg::IPlugViewContentScaleSupport> css (plugView);
            if (css) {
                css->setContentScaleFactor (newScaleFactor);
            }
        }
        // IPlugFrame
        Steinberg::tresult resizeView (Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override {
            SMTG_DBPRT1 ("resizeView called (%p)\n", (void*)view);
            if (newSize == nullptr || view == nullptr || view != plugView) {
                return Steinberg::kInvalidArgument;
            }
            if (!window) {
                return Steinberg::kInternalError;
            }
            if (resizeViewRecursionGard) {
                return Steinberg::kResultFalse;
            }
            Steinberg::ViewRect r;
            if (plugView->getSize (&r) != Steinberg::kResultTrue) {
                return Steinberg::kInternalError;
            }
            if (std::memcmp(&r, newSize, sizeof(Steinberg::ViewRect)) == 0) {
                return Steinberg::kResultTrue;
            }
            resizeViewRecursionGard = true;
            Steinberg::Vst::EditorHost::Size size {newSize->right - newSize->left, newSize->bottom - newSize->top};
            window->resize (size);
            resizeViewRecursionGard = false;
            if (plugView->getSize (&r) != Steinberg::kResultTrue) {
                return Steinberg::kInternalError;
            }
            if (std::memcmp(&r, newSize, sizeof(Steinberg::ViewRect)) != 0) {
                plugView->onSize (newSize);
            }
            return Steinberg::kResultTrue;
        }
        void closePlugView () {
            if (plugView) {
                plugView->setFrame (nullptr);
                if (plugView->removed () != Steinberg::kResultTrue) {
                    Steinberg::Vst::EditorHost::IPlatform::instance ().kill (-1, "Removing PlugView failed");
                }
                plugView = nullptr;
            }
            window = nullptr;
        }
        Steinberg::tresult queryInterface (const Steinberg::TUID _iid, void** obj) override {
            if (Steinberg::FUnknownPrivate::iidEqual (_iid, Steinberg::IPlugFrame::iid) ||
                Steinberg::FUnknownPrivate::iidEqual (_iid, Steinberg::FUnknown::iid)) {
                *obj = this;
                addRef ();
                return Steinberg::kResultTrue;
            }
            if (window) {
                return window->queryInterface (_iid, obj);
            }
            return Steinberg::kNoInterface;
        }
        uint32 addRef () override { 
            return 1000; 
        }
        uint32 release () override { 
            return 1000; 
        }
        Steinberg::IPtr<Steinberg::IPlugView> plugView;
        Steinberg::Vst::EditorHost::IWindow* window {nullptr};
        bool resizeViewRecursionGard {false};
    };
    
    //~ class App : public Steinberg::VST3::Hosting::IApplication
    //~ {
    //~ public:
        //~ ~App () noexcept override {
        //~ }
        //~ void init (const std::vector<std::string>& args) override {
        //~ }
        //~ void terminate () override {
        //~ }
    //~ private:
        //~ enum OpenFlags
        //~ {
            //~ kSetComponentHandler = 1 << 0,
            //~ kSecondWindow = 1 << 1,
        //~ };
        //~ void openEditor (const std::string& path, VST3::Optional<VST3::UID> effectID, uint32 flags) {
        //~ }
        //~ void createViewAndShow (IEditController* controller) {
        //~ }
        //~ Steinberg::VST3::Hosting::Module::Ptr module {nullptr};
        //~ Steinberg::IPtr<Steinberg::Vst::PlugProvider> plugProvider {nullptr};
        //~ Vst::HostApplication pluginContext;
        //~ std::shared_ptr<WindowController> windowController;
    //~ };
    
    //~ static Steinberg::Vst::EditorHost::AppInit gInit (std::make_unique<App> ());

    // Does this do any good? I dunno.

    class ComponentHandler : public Steinberg::Vst::IComponentHandler
    {
    public:
        Steinberg::tresult PLUGIN_API beginEdit (Steinberg::Vst::ParamID id) override {
            SMTG_DBPRT1 ("beginEdit called (%d)\n", id);
            return Steinberg::kNotImplemented;
        }
        Steinberg::tresult PLUGIN_API performEdit (Steinberg::Vst::ParamID id, Steinberg::Vst::ParamValue valueNormalized) override {
            SMTG_DBPRT2 ("performEdit called (%d, %f)\n", id, valueNormalized);
            return Steinberg::kNotImplemented;
        }
        Steinberg::tresult PLUGIN_API endEdit (Steinberg::Vst::ParamID id) override {
            SMTG_DBPRT1 ("endEdit called (%d)\n", id);
            return Steinberg::kNotImplemented;
        }
        Steinberg::tresult PLUGIN_API restartComponent (Steinberg::int32 flags) override {
            SMTG_DBPRT1 ("restartComponent called (%d)\n", flags);
            return Steinberg::kNotImplemented;
        }
    private:
        Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID /*_iid*/, void** /*obj*/) override {
            return Steinberg::kNoInterface;
        }
        Steinberg::uint32 PLUGIN_API addRef () override { return 1000; }
        Steinberg::uint32 PLUGIN_API release () override { return 1000; }
    };

#endif

    /**
     * This class manages one instance of one plugin and all of its 
     * communications with Csound, including audio input and output, 
     * MIDI input and output, and parameter input and output.
     */    
    struct vst3_plugin_t  {
        vst3_plugin_t() {};
        virtual ~vst3_plugin_t() {
#if DEBUGGING
            std::fprintf(stderr, "vst3_plugin_t::~vst3_plugin_t.\n");
#endif
        }
        void preprocess(int64_t continousFrames) {
#if PROCESS_DEBUGGING
            csound->Message(csound, "vst3_plugin_t::preprocess: hostProcessData.numSamples: %d.\n", hostProcessData.numSamples);
#endif
            hostProcessData.numSamples = blockSize;
            processContext.continousTimeSamples = continousFrames;
            paramTransferrer.transferChangesTo(inputParameterChanges);
#if PARAMETER_DEBUGGING
            if (inputParameterChanges.getParameterCount() > 0) {
                csound->Message(csound, "vst3_plugin_t::preprocess: inputParameterChanges parameters: %d.\n", 
                    inputParameterChanges.getParameterCount());
            }
#endif
        }
        void postprocess() { 
            eventList.clear();
            inputParameterChanges.clearQueue();
        }
        bool process(int64_t continuous_frames) {
#if PROCESS_DEBUGGING
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
        bool setSamplerate(double value) {
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
        bool setBlockSize(int32 value) {
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
        void setParameter(uint32 id, double value, int32 sampleOffset) {
            paramTransferrer.addChange(id, value, sampleOffset);
#if PARAMETER_DEBUGGING
            csound->Message(csound, "vst3_plugin_t::setParameter: id: %4d  value: %9.4f  offset: %d\n", id, value, sampleOffset);
#endif
        }
        bool initialize(CSOUND *csound_, const VST3::Hosting::ClassInfo &classInfo_, Steinberg::Vst::PlugProvider *provider_) {
            csound = csound_;
            provider = provider_;
            classInfo = classInfo_;
            component = provider->getComponent();
            controller = provider->getController();
#if EDITOR_IMPLEMENTED
            if (controller) {
                controller->setComponentHandler(component_handler());
            }
#endif
            processor = component.get();
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMapping(controller);
            initProcessData();
            paramTransferrer.setMaxParameters(1000);
            // midiCCMapping = initMidiCtrlerAssignment(component, midiMapping);
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
#if EDITOR_IMPLEMENTED
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
#endif
        void print_information() {
            Steinberg::TUID controllerClassTUID;
            if (component->getControllerClassId(controllerClassTUID) != Steinberg::kResultOk) {
                csound->Message(csound, "vst3_plugin_t: This component does not export an edit controller class ID!\n");
            }
            Steinberg::FUID controllerClassUID;
            controllerClassUID = Steinberg::FUID::fromTUID(controllerClassTUID);
            if (controllerClassUID.isValid() == false) {
                csound->Message(csound, "vst3_plugin_t: The edit controller class has no valid UID!\n");
            }
            char cidString[50];
            controllerClassUID.toString(cidString);
            char iidString[50];
            component->iid.toString(iidString);
            // Class information.
            csound->Message(csound, "vst3_plugin_t: class:      module classinfo id: %s\n", classInfo.ID().toString().c_str());
            csound->Message(csound, "               class:      component class id:  %s\n", iidString);
            csound->Message(csound, "               class:      controller class id: %s\n", cidString);
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
                csound->Message(csound, "               parameter count:   %4d\n", n);
                for (int i = 0; i < n; ++i) {
                    controller->getParameterInfo(i, parameterInfo);
                    Steinberg::String title(parameterInfo.title);
                    title.toMultiByte(Steinberg::kCP_Utf8);
                    Steinberg::String units(parameterInfo.units);
                    units.toMultiByte(Steinberg::kCP_Utf8);
                    double value = controller->getParamNormalized(parameterInfo.id);
                    csound->Message(csound, "               parameter:  index: %4d: id: %12d name: %-64s units: %-16s default: %9.4f value: %9.4f\n", 
                        i, parameterInfo.id, title.text8(), units.text8(), parameterInfo.defaultNormalizedValue, value);
                }
            }            // Units, program lists, and programs, in a flat list.
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
                                csound->Message(csound, "               unit:       id: %7d (parent id: %4d) name: %-32s program list: id: %12d (index: %4d) program: id: %4d name: %s\n", 
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
        void setTempo(double new_tempo) {
            processContext.tempo = new_tempo;
        }
#if EDITOR_IMPLEMENTED
        void showPluginEditorWindow() {
            auto view = owned(controller->createView(Steinberg::Vst::ViewType::kEditor));
            if (!view) {
                csound->Message(csound, "vst3_plugin_t::showPluginEditorWindow: controller does not provide its own editor!\n");\
                return;
            }
            Steinberg::ViewRect plugViewSize {};
            auto result = view->getSize(&plugViewSize);
            if (result != Steinberg::kResultTrue) {
                  csound->Message(csound, "vst3_plugin_t::showPluginEditorWindow: could not get editor view size.\n");\
            }
            auto viewRect = Steinberg::Vst::EditorHost::ViewRectToRect(plugViewSize);
#if 1
            auto windowController = std::make_shared<CsoundWindowController>(view);
            auto window = Steinberg::Vst::EditorHost::IPlatform::instance().createWindow("Editor", 
                viewRect.size, view->canResize() == Steinberg::kResultTrue, windowController);
            if (result != Steinberg::kResultTrue) {
                  csound->Message(csound, "vst3_plugin_t::showPluginEditorWindow: could not create window for plugin editor.\n");\
            }
            if (window) {
                window->show();
            }
#endif
        }
        static ComponentHandler *component_handler() {
            static ComponentHandler component_handler_;
            return &component_handler_;
        }
#endif
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
     * Singleton class for managing all persistent VST3 state:
     * (1) There is one and only one vst3_host_t instance in a process.
     * (2) There are zero or more vst3_plugin_t instances for each CSOUND 
     *     instance, and these plugins are deleted when csoundModuleDestroy 
     *     is called.
     */
    class vst3_host_t : public Steinberg::Vst::HostApplication {
        int host_handle;
    public:
        vst3_host_t(){
        };
        vst3_host_t(vst3_host_t const&) = delete;
        void operator=(vst3_host_t const&) = delete;
        ~vst3_host_t() noexcept override {
            std::fprintf(stderr, "vst3_host_t::~vst3_host_t.\n");
        }
        /**
         * Loads a VST3 Module and obtains all plugins in it.
         */
        int load_module(CSOUND *csound, const std::string& module_pathname, const std::string &plugin_name, bool verbose) {
            int handle = -1;
            if (verbose == true) {
                csound->Message(csound, "vst3_host_t::load_module: loading: \"%s\"\n", module_pathname.c_str());
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
                    csound->Message(csound, "vst3_host_t::load_module: found module classinfo: %d\n", count);
                    csound->Message(csound, "                          module classinfo id:    %s\n", classInfo.ID().toString().c_str());
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
                csound->Message(csound, "vst3_host_t::load_module: This component does not export an edit controller class ID!\n");
            }
            Steinberg::FUID controllerClassUID;
            controllerClassUID = Steinberg::FUID::fromTUID(controllerClassTUID);
            if (controllerClassUID.isValid() == false) {
                csound->Message(csound, "vst3_host_t::load_module: The edit controller class has no valid UID!\n");
            }
            char cidString[50];
            controllerClassUID.toString(cidString);
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
        // Handles for vst3_plugin_t instances are indexes into a list of 
        // plugins. It's not possible to simply store the address of a 
        // vst3_plugin_t instance in a Csound opcode parameter, because the 
        // address might be 64 bits and the MYFLT parameter might be only 32 
        // bits. 
        std::vector<std::shared_ptr<vst3_plugin_t>> vst3_plugins_for_handles;
    };
    
    static inline vst3_host_t *vst3_host_for_csound(CSOUND *csound) {
        int handle = 0;
        auto host = vst3hosts::instance().object_for_handle(csound, handle);
        std::fprintf(stderr, "vst3_host_t::host_for_csound: csound: %p handle: %d host: %p...\n", csound, handle, host);
        if (host == nullptr) {
            host = new vst3_host_t;
            handle = vst3hosts::instance().handle_for_object(csound, host);
        }
        host = vst3hosts::instance().object_for_handle(csound, handle);
        std::fprintf(stderr, "vst3_host_t::host_for_csound: csound: %p handle: %d host: %p\n", csound, handle, host);
        return host;
    }
    
    static inline vst3_plugin_t *get_plugin(CSOUND *csound, size_t handle) {
        auto host = vst3_host_for_csound(csound);
        auto plugin = host->vst3_plugins_for_handles[handle];
        return plugin.get();        
    }

    struct VST3AUDIO : public csound::OpcodeBase<VST3AUDIO> {
        // Outputs.
        MYFLT *a_output_channels[32];
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *a_input_channels[32];
        // State.
        MYFLT zerodbfs;
        Steinberg::int32 opcode_input_channel_count;
        Steinberg::int32 plugin_input_channel_count;
        Steinberg::int32 input_channel_count;
        Steinberg::int32 opcode_output_channel_count;
        Steinberg::int32 plugin_output_channel_count;
        Steinberg::int32 output_channel_count;
        vst3_plugin_t *vst3_plugin;
        Steinberg::Vst::Sample32 **plugin_input_channels_32;
        Steinberg::Vst::Sample64 **plugin_input_channels_64;
        Steinberg::Vst::Sample32 **plugin_output_channels_32;
        Steinberg::Vst::Sample64 **plugin_output_channels_64;
        Steinberg::Vst::SymbolicSampleSizes plugin_sample_size;
        Steinberg::int32 frame_count;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
            auto sr = csound->GetSr(csound);
            vst3_plugin->setSamplerate(sr);
            frame_count = ksmps();
            // This also creates the host buffers.
            vst3_plugin->setBlockSize(frame_count);
            // Because Csound and the plugin may not use the same sample word 
            // size, allowance must be made for different buffer shapes and 
            // sample word sizes, and the audio streams must be copied in and 
            // out word for word. Csound will try to match its input and 
            // output channels with the hosted plugin, and the maximum matching 
            // set will be used. Whether the HostProcessData busses are "Main"
            // is not considered. The Csound instrument hosting this opcode 
            // may ignore or duplicate channels depending on documentation or 
            // experience.
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
            log(csound, "vst3audio::init: input channels: %3d  output channels: %3d isProcessing: %d\n", input_channel_count, output_channel_count, vst3_plugin->isProcessing);
            return result;
        };
        int audio(CSOUND *csound) {
            int result = OK;
            int64_t current_time_in_frames = csound->GetCurrentTimeSamples(csound);
            if (current_time_in_frames < 0) {
                log(csound, "vst3audio::audio: warning! current_time_in_frames is less than 0: %d\n", current_time_in_frames);
                return NOTOK;
            }
            // Because this opcode should always be on, and because the plugins 
            // _themselves_ are actually responsible for scheduling by sample 
            // frame, ksmps_offset and ksmps_no_end have no effect and are not 
            // used.
            if (plugin_sample_size == Steinberg::Vst::kSample32) {
                for (Steinberg::int32 channel_index = 0; channel_index < input_channel_count; ++channel_index) {
                    for (Steinberg::int32 frame_index = 0; frame_index < frame_count; ++frame_index) {
                        plugin_input_channels_32[channel_index][frame_index] = a_input_channels[channel_index][frame_index];
                    }
                }
                vst3_plugin->process(current_time_in_frames);
                for (Steinberg::int32 channel_index = 0; channel_index < output_channel_count; ++channel_index) {
                    for (Steinberg::int32 frame_index = 0; frame_index < frame_count; ++frame_index) {
#if PROCESS_DEBUGGING
                        log(csound, "vst3audio::audio for 32 bits: sample[%4d][%4d]: opcode: %f plugin: %f\n", 
                            channel_index, frame_index, a_output_channels[channel_index][frame_index], plugin_output_channels_32[channel_index][frame_index]);
#endif
                        a_output_channels[channel_index][frame_index] = plugin_output_channels_32[channel_index][frame_index];
                    }
                }
            } else {
                for (Steinberg::int32 channel_index = 0; channel_index < input_channel_count; ++channel_index) {
                    for (Steinberg::int32 frame_index = 0; frame_index < frame_count; ++frame_index) {
                        plugin_input_channels_64[channel_index][frame_index] = a_input_channels[channel_index][frame_index];
                    }
                }
                vst3_plugin->process(current_time_in_frames);
                for (Steinberg::int32 channel_index = 0; channel_index < output_channel_count; ++channel_index) {
                    for (Steinberg::int32 frame_index = 0; frame_index < frame_count; ++frame_index) {
                        a_output_channels[channel_index][frame_index] = plugin_output_channels_64[channel_index][frame_index];
#if PROCESS_DEBUGGING
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
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
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
            log(csound, "\nvst3init::init...\n");
            auto host = vst3_host_for_csound(csound);
            log(csound, "vst3init::init: host: %p\n", host);
            std::string module_pathname =((STRINGDAT *)i_module_pathname)->data;
            std::string plugin_name =((STRINGDAT *)i_plugin_name)->data;
            log(csound, "vst3init::init: loading module: \"%s\",  \"%s\"...\n", module_pathname.c_str(), plugin_name.c_str());
            *i_vst3_handle = host->load_module(csound, module_pathname, plugin_name,(bool)*i_verbose); 
            log(csound, "vst3init::init: loaded module: \"%s\",  \"%s\" i_vst3_handle: %ld...\n", module_pathname.c_str(), plugin_name.c_str(), (size_t)*i_vst3_handle);
            auto vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
            log(csound, "vst3init::init: created plugin: \"%s\": address: %p handle: %ld\n", plugin_name.c_str(), vst3_plugin,(size_t) *i_vst3_handle);
            return result;
        };
    };

#if EDITOR_IMPLEMENTED

    struct VST3EDIT : public csound::OpcodeBase<VST3EDIT> {
        // Inputs.
        MYFLT *i_vst3_handle;
        // State.
        vst3_plugin_t *vst3_plugin;
        /** 
         * Set up a display, a window, and an associated event loop 
         * for showing the plugin editor. These will run until the user 
         * closes the editor window,
         */
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
#if defined(LINUX)
#ifdef EDITORHOST_GTK
        app = Gtk::Application::create ("net.steinberg.vstsdk.editorhost");
        application->init (cmdArgs);
        eventLoop ();

#else
            // Connect to X server
            std::string displayName (getenv ("DISPLAY"));
            if (displayName.empty ())
                displayName = ":0.0";
            auto display = XOpenDisplay (displayName.data ());
            RunLoop::instance ().setDisplay (display);
            application->init (cmdArgs);
            eventLoop ();
            XCloseDisplay (display);
#endif
#endif
            vst3_plugin->showPluginEditorWindow();
            return result;
        };
    };
    
#endif

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
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
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
        Steinberg::Vst::Event note_on_event;
        Steinberg::Vst::Event note_off_event;
        size_t framesRemaining;
        vst3_plugin_t *vst3_plugin;
        MYFLT note_on_time;
        MYFLT note_duration;
        MYFLT note_off_time;
        MYFLT delta_time;
        int delta_frames;
        MYFLT note_off_delta_time;
        int note_off_delta_frames;
        bool on = false;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
            auto current_time = csound->GetCurrentTimeSamples(csound) / csound->GetSr(csound);
            // If scheduled after the beginning of the kperiod, will be slightly later.
            note_on_time = opds.insdshead->p2.value;
            note_duration = *i_duration;
            delta_time = note_on_time - current_time;
            int delta_frames = delta_time * csound->GetSr(csound);
            // Use the warped p3 to schedule the note off message.
            if (note_duration > FL(0.0)) {
                note_off_time = note_on_time + MYFLT(opds.insdshead->p3.value);
                // In case of real-time performance with indefinite p3...
            } else if (note_duration == FL(0.0)) {
#if DEBUGGING
                csound->Message(csound,
                                    Str("vstnote::init: not scheduling 0 duration note.\n"));
#endif
                return OK;
            } else {
                note_off_time = note_on_time + FL(1000000.0);
            }
            channel = Steinberg::int16(*i_channel) & 0xf;
            // Split the real-valued MIDI key number
            // into an integer key number and an integer number of cents(plus or
            // minus 50 cents).
            key = *i_key;
            pitch = int16(key + 0.5);
            tuning = (double(*i_key) - double(key)) * double(100.0);
            velocity = *i_velocity;
            velocity = velocity / 127.;
            // Ensure that the opcode instance is still active when we are scheduled
            // to turn the note off!
            opds.insdshead->xtratim = opds.insdshead->xtratim + 2;
            on = true;
            vst3_plugin->note_id++;
            note_on_event.type = Steinberg::Vst::Event::EventTypes::kNoteOnEvent;
            note_on_event.sampleOffset = delta_frames;
            note_on_event.noteOn.channel = Steinberg::int16(*i_channel);
            note_on_event.noteOn.pitch = pitch;
            note_on_event.noteOn.tuning = tuning;
            note_on_event.noteOn.velocity = velocity;
            note_on_event.noteOn.length = note_duration * csound->GetSr(csound);
            note_on_event.noteOn.noteId = vst3_plugin->note_id;
            note_off_event.type = Steinberg::Vst::Event::EventTypes::kNoteOffEvent;
            note_off_event.noteOff.channel = note_on_event.noteOn.channel;
            note_off_event.noteOff.pitch = note_on_event.noteOn.pitch;
            note_off_event.noteOff.tuning = note_on_event.noteOn.tuning;
            note_off_event.noteOff.velocity = 0;
            note_off_event.noteOff.noteId = note_on_event.noteOn.noteId;
#if DEBUGGING
            log(csound, "vst3note::init:    current_time:                %12.5f [%12d]\n", current_time, csound->GetCurrentTimeSamples(csound));
            log(csound, "                   note_on_event time:          %12.5f\n", note_on_time);
            log(csound, "                   delta_time:                  %12.5f\n", delta_time);
            log(csound, "                   delta_frames:                %1d\n", delta_frames);
            log(csound, "                   offset:                      %d\n", note_on_event.sampleOffset);
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
            *i_note_id = note_on_event.noteOn.noteId;
            return result;
        }
        int noteoff(CSOUND *csound) {
            int result = OK;
            // Offset does not seem to apply to the notoff callback.
            auto current_time = csound->GetCurrentTimeSamples(csound) / csound->GetSr(csound);
            note_off_event.sampleOffset = 0; 
#if DEBUGGING
            log(csound, "vst3note::noteoff: current_time:                %12.5f [%12d]\n", current_time, csound->GetCurrentTimeSamples(csound));
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
        Steinberg::int32 parameter_id;
        MYFLT old_parameter_value;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
            log(csound, "vst3paramget::kontrol: id: %4d  value: %9.4f\n", parameter_id, *k_parameter_value);
            return result;
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            parameter_id = int(*k_parameter_id);
            *k_parameter_value = vst3_plugin->controller->getParamNormalized(parameter_id);
#if PARAMETER_DEBUGGING
            if (*k_parameter_value != old_parameter_value) {
                log(csound, "vst3paramget::kontrol: id: %4d  value: %9.4f\n", parameter_id, *k_parameter_value);
                old_parameter_value = *k_parameter_value;
            }
#endif
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
        Steinberg::int32 prior_parameter_id;
        double parameter_value;
        double prior_parameter_value;
        int64_t frames_per_kperiod;
        int init(CSOUND *csound) {
            int result = OK;
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
            frames_per_kperiod = csound->GetKsmps(csound);
            return result;
        };
        int kontrol(CSOUND *csound) {
            int result = OK;
            parameter_id = int(*k_parameter_id);
            parameter_value = double(*k_parameter_value);
            // Find the frame at the beginning of the kperiod, and compute the 
            // delta frames for "now."
            if (parameter_id != prior_parameter_id || parameter_value != prior_parameter_value) {
                int64_t block_start_frames = opds.insdshead->kcounter * frames_per_kperiod;
                int64_t current_time_frames = csound->GetCurrentTimeSamples(csound);
                Steinberg::int32 delta_frames = current_time_frames - block_start_frames;
                vst3_plugin->setParameter(parameter_id, parameter_value, delta_frames);;
#if PARAMETER_DEBUGGING
                log(csound, "vst3paramset::kontrol: id: %4d  value: %9.4f  delta_frames: %4d\n", parameter_id, parameter_value, delta_frames);
#endif
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
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
            std::string preset_filepath = ((STRINGDAT *)S_preset_filepath)->data;
            log(csound, "vst3presetload: preset_filepath: %s\n", preset_filepath.c_str());
            std::fstream input_stream(preset_filepath.c_str(), std::fstream::in | std::fstream::binary);
            Steinberg::MemoryStream memory_stream;
            char c;
            Steinberg::int32 bytes_to_write = 1;
            Steinberg::int32 bytes_written = 0;
            while (input_stream.get(c)) {
                memory_stream.write(&c, sizeof(c), &bytes_written);
            }
            auto stream_size = memory_stream.getSize();
            input_stream.close();
            Steinberg::int64 position = 0;
            auto tuid = vst3_plugin->classInfo.ID().data();
            auto classid = Steinberg::FUID::fromTUID(tuid);            
            memory_stream.seek (0, Steinberg::IBStream::kIBSeekSet, &position);
            log(csound, "vst3presetload: stream_size: %d\n", stream_size);
            auto result = Steinberg::Vst::PresetFile::loadPreset(&memory_stream, classid, vst3_plugin->component);
            if (result == false) {
                log(csound, "vst3presetload: failed to load: %s\n", preset_filepath.c_str());
                return NOTOK;
            }
            log(csound, "vst3presetload::init: loaded: %s\n", preset_filepath.c_str());
            return OK;
        };
    };
    
    struct VST3PRESETSAVE : public csound::OpcodeBase<VST3PRESETSAVE> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *S_preset_filepath;
        // State.
        vst3_plugin_t *vst3_plugin;
        int init(CSOUND *csound) {
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
            std::string preset_filepath = ((STRINGDAT *)S_preset_filepath)->data;
            log(csound, "vst3presetsave: preset_filepath: %s\n", preset_filepath.c_str());
            auto tuid = vst3_plugin->classInfo.ID().data();
            auto classid = Steinberg::FUID::fromTUID(tuid);            
            Steinberg::MemoryStream memory_stream;
            auto result = Steinberg::Vst::PresetFile::savePreset(&memory_stream, classid, vst3_plugin->component,
                             vst3_plugin->controller);
            if (result == false) {
                log(csound, "vst3presetsave::init: failed to save preset: %s\n", preset_filepath.c_str());
                return NOTOK;
            }
            std::fstream output_stream(preset_filepath.c_str(), std::fstream::out | std::fstream::trunc);
            output_stream.write(memory_stream.getData(), memory_stream.getSize());
            output_stream.close();
            log(csound, "vst3presetsave::init: saved: %s\n", preset_filepath.c_str());
            return OK;
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
            vst3_plugin = get_plugin(csound, static_cast<size_t>(*i_vst3_handle));
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
        {"vst3audio",           sizeof(VST3AUDIO),      0, 3, "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm", "M", &VST3AUDIO::init_, &VST3AUDIO::audio_, 0},
        {"vst3info",            sizeof(VST3INFO),       0, 1, "", "i", &VST3INFO::init_, 0, 0}, 
        {"vst3init",            sizeof(VST3INIT),       0, 1, "i", "TTo", &VST3INIT::init_, 0, 0},
#if EDITOR_IMPLEMENTED
        {"vst3edit",            sizeof(VST3EDIT),       0, 1, "", "i", &VST3EDIT::init_, 0, 0},
#endif
        {"vst3midiout",         sizeof(VST3MIDIOUT),    0, 3, "", "ikkkk", &VST3MIDIOUT::init_, &VST3MIDIOUT::kontrol_, 0},
        {"vst3channelmessage",  sizeof(VST3MIDIOUT),    0, 3, "", "ikkkk", &VST3MIDIOUT::init_, &VST3MIDIOUT::kontrol_, 0},
        {"vst3note",            sizeof(VST3NOTE),       0, 3, "i", "iiiii", &VST3NOTE::init_, &VST3NOTE::kontrol_, 0},
        {"vst3paramget",        sizeof(VST3PARAMGET),   0, 3, "k", "ik", &VST3PARAMGET::init_, &VST3PARAMGET::kontrol_, 0},
        {"vst3paramset",        sizeof(VST3PARAMSET),   0, 3, "", "ikk", &VST3PARAMSET::init_, &VST3PARAMSET::kontrol_, 0},
        {"vst3presetload",      sizeof(VST3PRESETLOAD), 0, 1, "", "iT", &VST3PRESETLOAD::init_, 0, 0},
        {"vst3presetsave",      sizeof(VST3PRESETSAVE), 0, 1, "", "iT", &VST3PRESETSAVE::init_, 0, 0},
        {"vst3tempo",           sizeof(VST3TEMPO),      0, 2, "", "ki", 0, &VST3TEMPO::init_, 0 /*, &vstedit_deinit*/ },
        {0, 0, 0, 0, 0, 0,(SUBR)0,(SUBR)0,(SUBR)0}
    };
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern "C" {
#if DEBUGGING
    std::hash<std::thread::id> thread_hasher;
#endif
    
    PUBLIC int csoundModuleCreate(CSOUND *csound) {
#if DEBUGGING
        csound->Message(csound, "csoundModuleCreate: csound: %p thread: %ld\n", csound,
                        thread_hasher(std::this_thread::get_id()));
#endif
        int result = 0;
        return result;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound) {
#if DEBUGGING
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
//#if DEBUGGING
        csound->Message(csound, "csoundModuleDestroy (vst3_opcodes): csound: %p thread: %ld...\n",
                        csound,
                        std::this_thread::get_id());
//#endif
        csound::vst3hosts::instance().module_destroy(csound);
        csound->Message(csound, "csoundModuleDestroy (vst3_opcodes): csound: %p.\n", csound);
        return 0;
    }
} // extern "C"
 
