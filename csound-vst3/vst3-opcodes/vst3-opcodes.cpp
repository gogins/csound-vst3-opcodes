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
#include <functional>
#include <map>
#include <string>
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/imediaserver.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/iparameterclient.h"
#include "public.sdk/samples/vst-hosting/audiohost/source/media/miditovst.h"
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
 *     are iterated, and all of the kVstAudioEffectClass PlugProviders are 
 *     saved by name and by UID.
 * (4) Either the first PlugProvider, or the PlugProviders that is named in 
 *     the vst3init call, is used to create a vst3_plugin_t instance, which 
 *     actually obtains the interfaces called by Csound to use the plugin.
 * (5) The vst3_plugin initializes its IComponent and IEditController interfaces 
 *     for communication with Csound.
 * (6) A handle to the vst3_plugin_t instance is returned by vst3init to the 
 *     user, who must pass it to all other vst3 opcodes.
 * (7) When Csound calls csoundModuleDestroy, the vst3_host_t instance 
 *     terminates all plugins and deallocates all state.
 */
 
#define CSOUND_LIFECYCLE_DEBUG 1

namespace Steinberg {
    FUnknown* gStandardPluginContext;
};

namespace csound {
    
    enum
    {
        kMaxMidiMappingBusses = 4,
        kMaxMidiChannels = 16
    };
    using Controllers = std::vector<int32>;
    using Channels = std::array<Controllers, kMaxMidiChannels>;
    using Busses = std::array<Channels, kMaxMidiMappingBusses>;
    using MidiCCMapping = Busses;
    
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

    static inline void unassignBusBuffers (const Steinberg::Vst::IAudioClient::Buffers& buffers, Steinberg::Vst::HostProcessData& processData)
    {
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
        virtual ~vst3_plugin_t () override {};
        //~ static std::shared_ptr<vst3_plugin_t> create (CSOUND *csound,  const std::string& module_pathname, const std::string& plugin_name, Steinberg::Vst::IComponent* component,
                                      //~ Steinberg::Vst::IMidiMapping* midiMapping) {
           //~ std::shared_ptr<vst3_plugin_t>(new vst3_plugin_t);
            
        //~ };
        // IAudioClient
        bool process (Steinberg::Vst::IAudioClient::Buffers& buffers, int64_t continousFrames) override {
            Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component;
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
        //~ bool initialize (const std::string& name, Steinberg::Vst::IComponent* component, Steinberg::Vst::IMidiMapping* midiMapping);
        /**
         * Here, the vst3 opcodes themselves play the role of the local media server, in particular 
         * the note on, parameter set, and audio output opcodes.
         */
        void createLocalMediaServer (const std::string& name) {
        }
        void terminate () {
            //mediaServer = nullptr;
            Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component;
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
            // processData.prepare will be done in setBlockSize.
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
            Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor> processor = component;
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
        CSOUND* csound = nullptr;
        double sampleRate = 0;
        int32 blockSize = 0;
        Steinberg::Vst::HostProcessData processData;
        Steinberg::Vst::ProcessContext processContext;
        Steinberg::Vst::EventList eventList;
        Steinberg::Vst::ParameterChanges inputParameterChanges;
        Steinberg::Vst::IComponent* component = nullptr;
        Steinberg::Vst::ParameterChangeTransfer paramTransferrer;
        MidiCCMapping midiCCMapping;
        //IMediaServerPtr mediaServer;
        bool isProcessing = false;
        std::string name;
    };
    
    /**
     * Singleton class for managing all persistent VST3 state.
     */
    class vst3_host_t : public Steinberg::Vst::HostApplication {
    public:
        vst3_host_t(){};
        static vst3_host_t &get_instance();
        vst3_host_t(vst3_host_t const&) = delete;
        void operator=(vst3_host_t const&)  = delete;
        ~vst3_host_t () noexcept override {
        }
        /**
         * Loads a VST3 Module and obtains all plugins in it.
         */
        void load_module (CSOUND *csound, const std::string& module_pathname, const std::string &plugin_name, bool verbose) {
            if (verbose == true) {
                csound->Message(csound, "vst3init: loading module: %s\n", module_pathname.c_str());
            }
            std::string error;
            module = VST3::Hosting::Module::create(module_pathname, error);
            if (!module) {
                std::string reason = "Could not create Module for file:";
                reason += module_pathname;
                reason += "\nError: ";
                reason += error;
                csound->Message(csound, "vst3init: error: %s\n", reason.c_str());
                return;
            }
            auto factory = module->getFactory ();
            int count = 0;
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
                if (classInfo.category () == kVstAudioEffectClass) {
                    auto plugProvider = owned (NEW Steinberg::Vst::PlugProvider (factory, classInfo, true));
                    if (!plugProvider) {
                        std::string error = "No VST3 Audio Module Class found in file ";
                        error += module_pathname;
                        csound->Message(csound, "vst3init: error: %s\n", error.c_str());
                        continue;
                    }
                    Steinberg::OPtr<Steinberg::Vst::IComponent> component = plugProvider->getComponent ();
                    Steinberg::OPtr<Steinberg::Vst::IEditController> controller = plugProvider->getController ();
                }
            }
            //Steinberg::OPtr<Steinberg::Vst::IComponent> component = plugProvider->getComponent ();
            //Steinberg::OPtr<Steinberg::Vst::IEditController> controller = plugProvider->getController ();
            //Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMapping (controller);
            //vst3Processor = AudioClient::create ("VST 3 SDK", component, midiMapping);
        }
        VST3::Hosting::Module::Ptr module {nullptr};
        // Steinberg::IPtr<Steinberg::Vst::PlugProvider> plugProvider {nullptr};
//        std::shared_ptr<class AudioClient> vst3Processor;
    };
    
    inline vst3_host_t &vst3_host_t::get_instance() {
        static vst3_host_t vst3_host;
        Steinberg::gStandardPluginContext = &vst3_host;
        return vst3_host;
    };

    struct std::vector<vst3_plugin_t*> &vsts_for_csound(CSOUND *csound) {
        static std::map<CSOUND *, std::vector<vst3_plugin_t*> > vsts_for_csounds;
        return vsts_for_csounds[csound];
    }

    static vst3_plugin_t *vst_for_csound(CSOUND *csound, size_t index) {
        std::vector<vst3_plugin_t*> &vsts = vsts_for_csound(csound);
        return vsts[index];
    }

    struct VST3INIT : public csound::OpcodeBase<VST3INIT> {
        // Inputs.
        MYFLT *i_vst3_handle;
        MYFLT *i_module_pathname;
        MYFLT *i_plugin_name;
        MYFLT *i_verbose;
        int init(CSOUND *csound) {
            int result = OK;
            auto &host = csound::vst3_host_t::get_instance();
            std::string module_pathname = ((STRINGDAT *)i_module_pathname)->data;
            std::string plugin_name =  ((STRINGDAT *)i_plugin_name)->data;
            host.load_module(csound, module_pathname, plugin_name, (bool)*i_verbose);
            return result;
        };
    };

    struct VST3INFO : public csound::OpcodeBase<VST3INFO> {
        // Inputs.
        MYFLT *i_vst3_handle;
        // State.
        vst3_plugin_t vst3_plugin;
        int init(CSOUND *csound) {
            int result = OK;
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
        {"vst3init", sizeof(VST3INIT), 0, 1, "i", "To", &VST3INIT::init_, 0, 0},
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
        auto vsts = csound::vsts_for_csound(csound);
        for (size_t i = 0, n = vsts.size(); i < n; ++i) {
            if (vsts[i]) {
                delete vsts[i];
            }
        }
        vsts.clear();
        return 0;
    }
} // extern "C"
 