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
#include <csound/csdl.h>
#include <csound/OpcodeBase.hpp>

#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/optional.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"

#if defined(CSOUND_LIFECYCLE_DEBUG)
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#endif


namespace csound {
    
    struct vst3plugin_t {
        std::shared_ptr<class AudioClient> vst3Processor;
        std::shared_ptr<class AudioClient> vst3Controller;
    };
        
    class vst_host_t : public Steinberg::Vst::HostApplication {
    public:
        ~vst_host_t () noexcept override {
        }
        /**
         * Loads a VST3 module and obtains the interfaces required to run it.
         */
        void startAudioClient (const std::string& path, VST3::Optional<VST3::UID> effectID,
                               uint32 flags) {
            std::string error;
            module = VST3::Hosting::Module::create(path, error);
            if (!module) {
                std::string reason = "Could not create Module for file:";
                reason += path;
                reason += "\nError: ";
                reason += error;
                return;
            }
            auto factory = module->getFactory ();
            for (auto& classInfo : factory.classInfos ()) {
                if (classInfo.category () == kVstAudioEffectClass) {
                    if (effectID) {
                        if (*effectID != classInfo.ID ()) {
                            continue;
                        }
                    }
                    plugProvider = owned (NEW Steinberg::Vst::PlugProvider (factory, classInfo, true));
                    break;
                }
            }
            if (!plugProvider) {
                std::string error;
                if (effectID) {
                    error =
                        "No VST3 Audio Module Class with UID " + effectID->toString () + " found in file ";
                } else {
                    error = "No VST3 Audio Module Class found in file ";
                }
                error += path;
                return;
            }
            Steinberg::OPtr<Steinberg::Vst::IComponent> component = plugProvider->getComponent ();
            Steinberg::OPtr<Steinberg::Vst::IEditController> controller = plugProvider->getController ();
            Steinberg::FUnknownPtr<Steinberg::Vst::IMidiMapping> midiMapping (controller);
            vst3Processor = AudioClient::create ("VST 3 SDK", component, midiMapping);
        }
        VST3::Hosting::Module::Ptr module {nullptr};
        Steinberg::IPtr<Steinberg::Vst::PlugProvider> plugProvider {nullptr};
        std::shared_ptr<class AudioClient> vst3Processor;
    };

    struct std::vector<vst3plugin_t*> &vsts_for_csound(CSOUND *csound) {
        static std::map<CSOUND *, std::vector<vst3plugin_t*> > vsts_for_csounds;
        return vsts_for_csounds[csound];
    }

    static vst3plugin_t *vst_for_csound(CSOUND *csound, size_t index) {
        std::vector<vst3plugin_t*> &vsts = vsts_for_csound(csound);
        return vsts[index];
    }
};

namespace Steinberg {
    FUnknown* gStandardPluginContext = NEW csound::vstHost_t();
};

namespace csound {

struct VST3INIT : public OpcodeBase<VST3INIT> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *iplugin;
    MYFLT *iverbose;
    // State.
    vst3plugin_t *vstplugin;
    int init(CSOUND *csound) {
        int result = OK;
        
        return result;
    };
};

struct VST3INFO : public OpcodeBase<VST3INFO> {
    // Inputs.
    MYFLT *i_vst3handle;
    // State.
    vst3plugin_t *vstplugin;
    int init(CSOUND *csound) {
        int result = OK;
        return result;
    };
};

struct VST3AUDIO : public OpcodeBase<VST3AUDIO> {
    // Outputs.
    MYFLT *aouts[32];
    // Inputs.
    MYFLT *i_vst3handle;
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
    vst3plugin_t *vstplugin;
    int init(CSOUND *csound) {
        int result = OK;
        return result;
    };
    int audio(CSOUND *csound) {
        int result = OK;
        return result;
    };
};

struct VST3MIDIOUT : public OpcodeBase<VST3MIDIOUT> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *kstatus;
    MYFLT *kchan;
    MYFLT *kdata1;
    MYFLT *kdata2;
    // State.
    size_t  vstHandle;
    int     prvMidiData;
    vst3plugin_t *vstplugin;
    int init(CSOUND *csound) {
        int result = OK;
        return result;
    };
    int kontrol(CSOUND *csound) {
        int result = OK;
        return result;
    };
} VSTMIDIOUT;

struct VST3NOTE : public OpcodeBase<VST3NOTE> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *kchan;
    MYFLT *knote;
    MYFLT *kveloc;
    MYFLT *kdur;
    // State.
    size_t  vstHandle;
    int     chn;
    int     note;
    size_t  framesRemaining;
    vst3plugin_t *vstplugin;
    int init(CSOUND *csound) {
        int result = OK;
        return result;
    };
    int kontrol(CSOUND *csound) {
        int result = OK;
        return result;
    };
};

struct VST3PARAMGET : public OpcodeBase<VST3PARAMGET> {

    // Outputs.
    MYFLT *kvalue;
    // Intputs.
    MYFLT *i_vst3handle;
    MYFLT *kparam;
    // State.
    vst3plugin_t *vstplugin;
} VSTPARAMGET;

struct VSTPARAMSET : public OpcodeBase<VSTPARAMSET> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *kparam;
    MYFLT *kvalue;
    // State.
    MYFLT   oldkparam;
    MYFLT   oldkvalue;
    vst3plugin_t *vstplugin;
};

struct VST3BANKLOAD : public OpcodeBase<VST3BANKLOAD> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *ibank;
    // State.
    vst3plugin_t *vstplugin;
};

struct VST3PROGSET : public OpcodeBase<VST3PROGSET> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *iprogram;
    // State.
    vst3plugin_t *vstplugin;
};

struct VST3EDIT : public OpcodeBase<VST3EDIT> {
    // Inputs.
    MYFLT *i_vst3handle;
    // State.
    vst3plugin_t *vstplugin;
};

struct VST3TEMPO : public OpcodeBase<VST3TEMPO> { 
    // Inputs.
    MYFLT *tempo;
    MYFLT *i_vst3handle;
    // State.
    vst3plugin_t *vstplugin;
};

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
    static OENTRY localops[] = {
        {"vstinit", sizeof(VST3INIT), 0, 1, "i", "To", &VST3INIT::init_, 0, 0},
        {"vstinfo", sizeof(VST3INFO), 0, 1, "", "i", &VST3INFO::init_, 0, 0},
        {   "vstaudio", sizeof(VST3AUDIO), 0, 3, "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm",
            "iy", &VST3AUDIO::init_, &VST3AUDIO::audio_, 0
        },
        {   "vstmidiout", sizeof(VST3MIDIOUT), 0, 3, "", "ikkkk", &VST3MIDIOUT::init_,
            &VST3MIDIOUT::kontrol_, 0
        },
        {   "vstparamget", sizeof(VST3PARAMGET), 0, 3, "k", "ik", &VST3PARAMGET::init_,
            &VST3PARAMGET::kontrol_, 0
        },
        {   "vstparamset", sizeof(VSTPARAMSET), 0, 3, "", "ikk", &VSTPARAMSET::init_,
            &VSTPARAMSET::kontrol_, 0
        },
        {"vstbankload", sizeof(VST3BANKLOAD), 0, 1, "", "iT", &VST3BANKLOAD::init_, 0, 0},
        {"vstprogset", sizeof(VST3PROGSET), 0, 1, "", "ii", &VST3PROGSET::init_, 0, 0},
        {"vstedit", sizeof(VST3EDIT), 0, 1, "", "i", &VST3EDIT::init_, 0, 0},
        {
            "vsttempo", sizeof(VST3TEMPO), 0, 2, "", "ki", 0, &VST3TEMPO::init_,
            0 /*, &vstedit_deinit*/
        },
        {   "vstnote", sizeof(VST3NOTE), 0, 3, "", "iiiii", &VST3NOTE::init_,
            &VST3NOTE::kontrol_, 0
        },
        {"vstbanksave", sizeof(VST3BANKLOAD), 0, 1, "", "iT", &VST3BANKLOAD::init_, 0, 0},
        {0, 0, 0, 0, 0, 0, (SUBR)0, (SUBR)0, (SUBR)0}
    };
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

extern "C" {
    
    PUBLIC int csoundModuleCreate(CSOUND *csound) {
#if defined(CSOUND_LIFECYCLE_DEBUG)
        csound->Message(csound, "csoundModuleCreate: csound: %p thread: %d\n", csound,
                        syscall(SYS_gettid));
#endif
        int result = 0;
        return result;
    }

    PUBLIC int csoundModuleInit(CSOUND *csound) {
#if defined(CSOUND_LIFECYCLE_DEBUG)
        csound->Message(csound, "csoundModuleInit: csound: %p thread: %d\n", csound,
                        syscall(SYS_gettid));
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
        csound->Message(csound, "csoundModuleDestroy: csound: %p thread: %d\n",
                        csound, syscall(SYS_gettid));
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
 