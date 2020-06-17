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

#if defined(CSOUND_LIFECYCLE_DEBUG)
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace csound {
    
class vst3plugin_t;
    
class vstHost_t : public Steinberg::Vst::HostApplication {
};

class VST3INIT : public OpcodeBase<VST3INIT> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *iplugin;
    MYFLT *iverbose;
    // State.
    vst3plugin_t vstplugin;
};

class VST3INFO : public OpcodeBase<VST3INFO> {
    // Inputs.
    MYFLT *i_vst3handle;
    // State.
    vst3plugin_t vstplugin;
};

class VST3AUDIO : public OpcodeBase<VST3AUDIO> {
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
    vst3plugin_t vstplugin;
};

class VSTNOTE : public OpcodeBase<VSTNOTE> {
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
    vst3plugin_t vstplugin;
};

class VST3MIDIOUT : public OpcodeBase<VST3MIDIOUT> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *kstatus;
    MYFLT *kchan;
    MYFLT *kdata1;
    MYFLT *kdata2;
    // State.
    size_t  vstHandle;
    int     prvMidiData;
    vst3plugin_t vstplugin;
} VSTMIDIOUT;

class VST3PARAMGET : public OpcodeBase<VST3PARAMGET> {

    // Outputs.
    MYFLT *kvalue;
    // Intputs.
    MYFLT *i_vst3handle;
    MYFLT *kparam;
    // State.
    vst3plugin_t vstplugin;
} VSTPARAMGET;

class VSTPARAMSET : public OpcodeBase<VSTPARAMSET> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *kparam;
    MYFLT *kvalue;
    // State.
    MYFLT   oldkparam;
    MYFLT   oldkvalue;
    vst3plugin_t vstplugin;
};

class VST3BANKLOAD : public OpcodeBase<VST3BANKLOAD> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *ibank;
    // State.
    vst3plugin_t vstplugin;
};

class VST3PROGSET : public OpcodeBase<VST3PROGSET> {
    // Inputs.
    MYFLT *i_vst3handle;
    MYFLT *iprogram;
    // State.
    vst3plugin_t vstplugin;
};

class VST3EDIT : public OpcodeBase<VST3EDIT> {
    // Inputs.
    MYFLT *i_vst3handle;
    // State.
    vst3plugin_t vstplugin;
};

class VSTTEMPO : public OpcodeBase<VSTTEMPO> { 
    // Inputs.
    MYFLT *tempo;
    MYFLT *i_vst3handle;
    // State.
    vst3plugin_t vstplugin;
};

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
    static OENTRY localops[] = {
        {"vstinit", sizeof(VST3INIT), 0, 1, "i", "To", &vstinit, 0, 0},
        {"vstinfo", sizeof(VST3INFO), 0, 1, "", "i", &vstinfo, 0, 0},
        {   "vstaudio", sizeof(VSTA3UDIO), 0, 3, "mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm",
            "iy", &vstaudio_init, &vstaudio
        },
        {   "vstmidiout", sizeof(VST3MIDIOUT), 0, 3, "", "ikkkk", &vstmidiout_init,
            &vstmidiout, 0
        },
        {   "vstparamget", sizeof(VST3PARAMGET), 0, 3, "k", "ik", &vstparamget_init,
            &vstparamget, 0
        },
        {   "vstparamset", sizeof(VSTPARAMSET), 0, 3, "", "ikk", &vstparamset_init,
            &vstparamset, 0
        },
        {"vstbankload", sizeof(VST3BANKLOAD), 0, 1, "", "iT", &vstbankload, 0, 0},
        {"vstprogset", sizeof(VST3PROGSET), 0, 1, "", "ii", &vstprogset, 0, 0},
        {"vstedit", sizeof(VST3EDIT), 0, 1, "", "i", &vstedit_init, 0, 0},
        {
            "vsttempo", sizeof(VST3TEMPO), 0, 2, "", "ki", 0, &vstSetTempo,
            0 /*, &vstedit_deinit*/
        },
        {   "vstnote", sizeof(VST3NOTEOUT), 0, 3, "", "iiiii", &vstnote_init,
            &vstnote_perf, 0
        },
        {"vstbanksave", sizeof(VST3BANKLOAD), 0, 1, "", "iT", &vstbanksave, 0, 0},
        {0, 0, 0, 0, 0, 0, (SUBR)0, (SUBR)0, (SUBR)0}
    };

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

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
        OENTRY *ep = (OENTRY *)&(localops[0]);
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
        auto vsts = vsts_for_csound(csound);
        for (size_t i = 0, n = vsts.size(); i < n; ++i) {
            if (vsts[i]) {
                delete vsts[i];
            }
        }
        vsts.clear();
        return 0;
    }
} // extern "C"
};
 