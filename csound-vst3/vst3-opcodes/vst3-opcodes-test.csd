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
0dbfs   = 5

connect "JX10_Output", "outleft", "Delay", "inleft"
connect "JX10_Output", "outright", "Delay", "inright"
connect "Piano_Output", "outleft", "Delay", "inleft"
connect "Piano_Output", "outright", "Delay", "inright"
connect "Note_Expression_Output", "outleft", "Delay", "inleft"
connect "Note_Expression_Output", "outright", "Delay", "inright"
connect "Delay", "outleft", "Reverb", "inleft"
connect "Delay", "outright", "Reverb", "inright"
connect "Reverb", "outleft", "Master_Output", "inleft"
connect "Reverb", "outright", "Master_Output", "inright"

alwayson "JX10_Output"
alwayson "Piano_Output""
alwayson "Note_Expression_Output"
alwayson "Delay"
alwayson "Reverb"
alwayson "Master_Output"

gi_vst3_handle_jx10 vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Debug/mda-vst3.vst3", "mda JX10", 1
vst3info gi_vst3_handle_jx10

gi_vst3_handle_piano vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Debug/mda-vst3.vst3", "mda Piano", 1
vst3info gi_vst3_handle_piano

gi_vst3_handle_noteexpression vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Debug/noteexpressionsynth.vst3", "Note Expression Synth", 1
vst3info gi_vst3_handle_noteexpression

gi_vst3_handle_adelay vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Debug/mda-vst3.vst3", "mda Delay", 1
vst3info gi_vst3_handle_adelay

gi_vst3_handle_ambience vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Debug/mda-vst3.vst3", "mda Ambience", 1
vst3info gi_vst3_handle_ambience


// Currently the Pianoteq does not support vst3 on Linux. I asked Modartt about this.
// Julien said it is because Pianoteq uses JUCE, which currently does not 
// support vst3 on Linux. 
// gi_vst3_handle vst3init "/home/mkg/Pianoteq_6.vst3", 1

instr Piano
i_note_id vst3note gi_vst3_handle_piano, 0, p4, p5, p3
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr JX10
i_note_id vst3note gi_vst3_handle_jx10, 0, p4, p5, p3
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Piano
i_note_id vst3note gi_vst3_handle_piano, 0, p4, p5, p3
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Piano_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_piano 
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Note_Expression_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_noteexpression
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin


instr JX10_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_jx10 
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Print_Info
// Parameter values should have been changed.
vst3info gi_vst3_handle_noteexpression
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Param_Change
vst3paramset gi_vst3_handle_noteexpression, 16, .1
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Delay
a_in_left inleta "inleft"
a_in_right inleta "inright"
a_out_left, a_out_right vst3audio gi_vst3_handle_adelay, a_in_left, a_in_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Reverb
a_in_left inleta "inleft"
a_in_right inleta "inright"
k_old_size vst3paramget gi_vst3_handle_ambience, 0
vst3paramset gi_vst3_handle_ambience, 0, 20
vst3paramset gi_vst3_handle_ambience, 3, .9
vst3presetsave gi_vst3_handle_ambience, "ambience.preset"
a_out_left, a_out_right vst3audio gi_vst3_handle_ambience, a_in_left, a_in_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Master_Output
a_in_left inleta "inleft"
a_in_right inleta "inright"
outs a_in_left, a_in_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

</CsInstruments>
<CsScore>
i 1 1.382 4.5 60 60
i 1 .503 5.20384 72 60
i "Param_Change" 2 1
i "Print_Info" 3 1
</CsScore>
</CsoundSynthesizer>