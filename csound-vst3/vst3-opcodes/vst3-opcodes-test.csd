<CsoundSynthesizer>
<CsLicense>

V S T 3   O P C O D E S   T E S T

Author: Michael Gogins

The code is licensed under the terms of the GPLv3 license.

This Csound piece performs basic unit tests for the following vst3 opcodes:

  1.  vst3init
  2.  vst3info
  3.  vst3note
  4.  vst3audio
  5.  vst3paramset
  6.  vst3paramget
  7.  vst3savepreset
  8.  vst3loadpreset
  9.  vst3edit
  
These are all the opcodes that are actually needed to fully use VST3 
instruments and effects in Csound.

</CsLicense>
<CsOptions>
-m195 --opcode-lib="/home/mkg/csound-vst3-opcodes/build/lib/Debug/libvst3_plugins.so" -z1
</CsOptions>
<CsInstruments>
sr      = 48000
ksmps   = 128
nchnls  = 2
0dbfs   = 1

alwayson "Output"

//gi_vst3_handle vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Debug/mda-vst3.vst3", "mda JX10", 1
gi_vst3_handle vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/DEBUG/noteexpressionsynth.vst3", "Note Expression Synth With UI", 1
vst3info gi_vst3_handle
vst3edit gi_vst3_handle

// Currently the Pianoteq does not support vst3 on Linux. I asked Modartt about this.
// Julien said it is because Pianoteq uses JUCE, which currently does not 
// support vst3 on Linux. I may go ahead and add VST2 support to this project, 
// at least privately.
// gi_vst3_handle vst3init "/home/mkg/Pianoteq_6.vst3", 1

instr Instrument
print p1, p2, p3, p4, p5
i_note_id vst3note gi_vst3_handle, 0, p4, p5, p3
print i_note_id
endin

instr Output
ainleft, ainright ins
aleft, aright vst3audio gi_vst3_handle // Not used: , ainleft, ainright
outs aleft, aright
endin

</CsInstruments>
<CsScore>
i 1 1 4.5 60 60
i 1 .5 5.20384 72 60
</CsScore>
</CsoundSynthesizer>