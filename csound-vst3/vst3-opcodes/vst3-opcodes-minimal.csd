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
  
These are all the opcodes actually needed to fully use VST3 instruments and 
effects in Csound. This piece also serves as basic unit tests for the VST3 
opcodes.

The piece consists of an algorithmically generated score, which is rendered 
with several of the instruments in the VST3 SDK; effects are added using 
several other plugins in the VST3 SDK. Parameters and presets also are used.

</CsLicense>
<CsOptions>
-m195 --opcode-lib="/home/mkg/csound-vst3-opcodes/build/lib/Release/libvst3_plugins.so" -z1
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 100
nchnls  = 2
0dbfs   = 20

connect "JX10_Output", "outleft", "Master_Output", "inleft"
connect "JX10_Output", "outright", "Master_Output", "inright"
connect "Piano_Output", "outleft", "Master_Output", "inleft"
connect "Piano_Output", "outright", "Master_Output", "inright"

alwayson "JX10_Output"
alwayson "Piano_Output"
alwayson "Master_Output"

gi_vst3_handle_jx10 vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Release/mda-vst3.vst3", "mda JX10", 1
vst3info gi_vst3_handle_jx10

gi_vst3_handle_piano vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Release/mda-vst3.vst3", "mda Piano", 1
vst3info gi_vst3_handle_piano

// Array of instrument plugins indexed by instrument number, for sending 
// parameter changes.

gi_plugins[] init 5
gi_plugins[3] init gi_vst3_handle_piano
gi_plugins[4] init gi_vst3_handle_jx10

// Score generating instrument.

gi_iterations init 500
gi_duration init 2
gi_time_step init .125
gi_loudness init 70
instr Score_Generator
i_time = p2
i_instrument = p4
i_c = p5
i_y = p6
i_bass = p7
i_range = p8
i_time_step = 1 / 8
i_iteration = 0
while i_iteration < gi_iterations do
    i_iteration = i_iteration + 1
    i_time = p2 + (i_iteration * gi_time_step)
    // Normalized logistic equation:
    i_y1 = i_c * i_y * (1 - i_y) * 4
    i_y = i_y1
    i_pitch = floor(i_bass + (i_y * i_range)) 
    event_i "i", i_instrument, i_time, gi_duration, i_pitch, gi_loudness
    prints "   %f => i %f %f %f %f %f\n", i_y, i_instrument, i_time, gi_duration, i_pitch, gi_loudness
od
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Param_Change
i_target_plugin = p4
i_vst3_plugin init gi_plugins[p4]
k_parameter_id init p5
k_parameter_value init p6
vst3paramset i_vst3_plugin, k_parameter_id, k_parameter_value
prints "%-24.24s i %9.4f t %9.4f d %9.4f target: %3d  id: %3d  value: %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p6, active(p1)
endin

instr Piano
i_note_id vst3note gi_vst3_handle_piano, 0, p4, p5, p3
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr JX10
i_note_id vst3note gi_vst3_handle_jx10, 0, p4, p5, p3
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Piano_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_piano 
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
i_target_plugin = p4
i_vst3_plugin init gi_plugins[p4]
vst3info i_vst3_plugin
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Save_Preset
i_target_plugin = p4
S_preset_name init p5
i_vst3_plugin init gi_plugins[p4]
vst3presetsave i_vst3_plugin, S_preset_name
prints "%-24.24s i %9.4f t %9.4f d %9.4f target: %3d  preset: %s #%3d\n", nstrstr(p1), p1, p2, p3, i_target_plugin, S_preset_name, active(p1)
endin

instr Load_Preset
i_target_plugin = p4
S_preset_name init p5
i_vst3_plugin init gi_plugins[p4]
vst3presetload i_vst3_plugin, S_preset_name
prints "%-24.24s i %9.4f t %9.4f d %9.4f target: %3d  preset: %s #%3d\n", nstrstr(p1), p1, p2, p3, i_target_plugin, S_preset_name, active(p1)
endin

instr Program_Change
i_target_plugin = p4
i_vst3_plugin init gi_plugins[p4]
p6 = 1886548852
k_parameter_id init p5
k_parameter_value init p6
vst3paramset i_vst3_plugin, k_parameter_id, k_parameter_value
prints "%-24.24s i %9.4f t %9.4f d %9.4f target: %3d  id: %3d  value: %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p6, active(p1)
endin

instr Master_Output
a_in_left inleta "inleft"
a_in_right inleta "inright"
outs a_in_left, a_in_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

</CsInstruments>
<CsScore>
f 0 72
i "Score_Generator" 1 1 3 .989 .5 36 60
i "Score_Generator" 2 1 4 .989 .5 78 6
; Stores original filter state...
i "Save_Preset" 1 1 4 "jx10.preset"
; Changes filter state...
i "Param_Change" 10 1 4 6 .1
i "Print_Info" 10.5 1 4
; Restores original filter state.
i "Load_Preset" 12 1 4 "jx10.preset"
i "Program_Change" 15 1 4 0 .5
i "Print_Info" 12.5 1 4
</CsScore>
</CsoundSynthesizer>