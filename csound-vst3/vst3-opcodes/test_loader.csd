<CsoundSynthesizer>
<CsLicense>

V S T 3   O P C O D E S   T E S T

Author: Michael Gogins

The code is licensed under the terms of the GPLv3 license.

This Csound piece demonstrates how to use the following vst3 opcodes:

  1.  vst3init
  2.  vst3info
  3.  vst3note
  4.  vst3audio
  5.  vst3paramset
  6.  vst3paramget
  7.  vst3savepreset
  8.  vst3loadpreset
  
These are all the opcodes actually needed to fully use VST3 instruments and 
effects in Csound. This piece also serves as basic unit tests for the VST3 
opcodes.

The piece consists of an algorithmically generated score, which is rendered 
with several of the instruments in the VST3 SDK; effects are added using 
several other plugins in the VST3 SDK. Parameters and presets also are used.

</CsLicense>
<CsOptions>
-m195 --opcode-lib="/home/mkg/csound-vst3-opcodes/build/lib/Debug/libvst3_plugins.so" -z1
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 100
nchnls  = 2
0dbfs   = 20

connect "JX10_Output", "outleft", "Delay", "inleft"
connect "JX10_Output", "outright", "Delay", "inright"
connect "Piano_Output", "outleft", "Delay", "inleft"
connect "Piano_Output", "outright", "Delay", "inright"
connect "Delay", "outleft", "Reverb", "inleft"
connect "Delay", "outright", "Reverb", "inright"
connect "Reverb", "outleft", "Master_Output", "inleft"
connect "Reverb", "outright", "Master_Output", "inright"

alwayson "JX10_Output"
alwayson "Piano_Output"
alwayson "Delay"
alwayson "Reverb"
alwayson "Master_Output"

gi_vst3_handle_odin vst3init "/usr/lib/vst3/Odin2.vst3", "Odin2", 0
vst3info gi_vst3_handle_odin

</CsInstruments>
<CsScore>
f 0 72
i "Score_Generator" 1 1 3 .989 .5 36 60
i "Score_Generator" 1 1 4 .989 .5 78 6
; Stores original parameter state...
i "Print_Info" 1.1 1 4
i "Save_Preset" 1.2 1 4 "jx10.preset"
i "Print_Info" 1.3 1 4
; Changes filter state...
i "Param_Change" 10 10 4 0 .7
i "Param_Change" 10 1 4 4 1
i "Print_Info" 10.5 1 4
; Restores original parameter state.
i "Load_Preset" 15 1 4 "jx10.preset"
i "Print_Info" 15.5 1 4
i "Program_Change" 25 1 4 12
i "Print_Info" 30.0 1 3
i "Program_Change" 30.1 1 3 4
i "Print_Info" 30.2 1 3

</CsScore>
</CsoundSynthesizer>