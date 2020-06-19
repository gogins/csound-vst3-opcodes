/**
 * V S T 3   H O S T   O P C O D E S   F O R   C S O U N D
 * 
 * Author: Michael Gogins
 * http://michaelgogins.tumblr.com
 * michael dot gogins at gmail dot com
 * 
 * This code is licensed under the terms of the 
 * GNU General Public License, Version 3.
 */
 #include <thread>


#include <OpcodeBase.hpp>
#include <csound/csdl.h>
#include <functional>
#include <string>
#include <map>
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/optional.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"

/**
 * (1) VST3 Modules may implement any number of VST3 plugins.
 * (2) The vst3init opcode uses a singleton vst3_host_t instance to load a 
 *     VST3 Module, and obtains from it a VST3 PluginFactory.
 * (3) All of the VST3 ClassInfo objects exposed by the PluginFactory 
 *     are iterated, and all of the kVstAudioEffectClass PlugProviders are 
 *     saved by name and by UID.
 * (4) Either the first PlugProvider, or the PlugProviders that is named in 
 *     the vst3init call, is used to create a vst3_plugin_t instance, which 
 *     actually obtains the interfaces called by Csound to use the plugin.
 * (5) The vst3_plugin initializes its IComponent and IEditController interfaces 
 *     for communication with Csound.
 * (6) A handle to the vst3_plugin_t instance is returned by vst3init to the 
 *     user, who passes it to all other vst3 opcodes.
 * (7) When Csound calls csoundModuleDestroy, the vst3_host_t instance 
 *     terminates all plugins and deallocates all state.
 */
 

#define CSOUND_LIFECYCLE_DEBUG 1

namespace Steinberg {
    FUnknown* gStandardPluginContext;
};

namespace csound {
    
    /**
     * This class manages one instance of a plugin and all of its 
     * communications with Csound, including audio input and output, 
     * MIDI input and output, and parameter input and output.
     */    
    struct vst3_plugin_t {
        vst3_plugin_t ();
        virtual ~vst3_plugin_t ();
        static std::shared_ptr<vst3_plugin_t> create (const std::string& name, Steinberg::Vst::IComponent* component,
                                      Steinberg::Vst::IMidiMapping* midiMapping) {
        }
/*         // IAudioClient
 *         bool process (Buffers& buffers, int64_t continousFrames) override (
 *         };
 *         bool setSamplerate (SampleRate value) override;
 *         bool setBlockSize (int32 value) override;
 *         IAudioClient::IOSetup getIOSetup () const override;
 *         // IMidiClient
 *         bool onEvent (const Event& event, int32_t port) override;
 *         IMidiClient::IOSetup getMidiIOSetup () const override;
 *         // IParameterClient
 *         void setParameter (ParamID id, ParamValue value, int32 sampleOffset) override;
 *         bool initialize (const Name& name, IComponent* component, IMidiMapping* midiMapping);
 *         void createLocalMediaServer (const Name& name);
 *         void terminate ();
 *         void updateBusBuffers (Buffers& buffers, HostProcessData& processData);
 *         void initProcessData ();
 *         void initProcessContext ();
 *         bool updateProcessSetup ();
 *         void preprocess (Buffers& buffers, int64_t continousFrames);
 *         void postprocess (Buffers& buffers);
 *         bool isPortInRange (int32 port, int32 channel) const;
 *         bool processVstEvent (const Steinberg::Vst::IMidiClient::Event& event, int32 port);
 *         bool processParamChange (const Steinberg::Vst::IMidiClient::Event& event, int32 port);
 *         Csound csound;
 *         SampleRate sampleRate = 0;
 *         int32 blockSize = 0;
 *         HostProcessData processData;
 *         ProcessContext processContext;
 *         EventList eventList;
 *         ParameterChanges inputParameterChanges;
 *         Steinberg::Vst::IComponent* component = nullptr;
 *         ParameterChangeTransfer paramTransferrer;
 *         MidiCCMapping midiCCMapping;
 */
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
         * Loads a VST3 Module and obtains all PlugProviders in it.
         */
        void load_module (CSOUND *csound, const std::string& path, bool verbose) {
            if (verbose == true) {
                csound->Message(csound, "vst3init: loading module: %s\n", path.c_str());
            }
            std::string error;
            module = VST3::Hosting::Module::create(path, error);
            if (!module) {
                std::string reason = "Could not create Module for file:";
                reason += path;
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
                        error += path;
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
        Steinberg::IPtr<Steinberg::Vst::PlugProvider> plugProvider {nullptr};
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
        MYFLT *i_verbose;
        int init(CSOUND *csound) {
            int result = OK;
            auto &host = csound::vst3_host_t::get_instance();
            std::string module_pathname = ((STRINGDAT *)i_module_pathname)->data;
            host.load_module(csound, module_pathname, (bool)*i_verbose);
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
 