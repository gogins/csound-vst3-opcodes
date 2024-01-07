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
  7.  vst3presetsave
  8.  vst3presetload
  
These are all the opcodes actually needed to fully use VST3 instruments and 
effects in Csound. This piece also serves as basic unit tests for the VST3 
opcodes.

The piece consists of an algorithmically generated score, which is rendered 
with several of the instruments in the VST3 SDK; effects are added using 
several other plugins in the VST3 SDK. Parameters and presets also are used.

Assuming you are on the Mac OS, have installed Csound, ahd have unzipped the 
csound-vst3-macos.zip file into a new empty directory, then things are set up
to run this piece from the `csound-vst3/vst3-opcodes` subdirectory.

</CsLicense>
<CsOptions>
-m162 -odac
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

gi_vst3_handle_jx10 vst3init "../build-macos/VST3/Debug/mda-vst3.vst3", "mda JX10", "", 1
vst3info gi_vst3_handle_jx10

gi_vst3_handle_piano vst3init "../build-macos/VST3/Debug/mda-vst3.vst3", "mda Piano", "", 1
vst3info gi_vst3_handle_piano

gi_vst3_handle_delay vst3init "../build-macos/VST3/Debug/mda-vst3.vst3", "mda Delay", "", 1
vst3info gi_vst3_handle_delay

gi_vst3_handle_ambience vst3init "../build-macos/VST3/Debug/mda-vst3.vst3", "mda Ambience", "", 1
vst3info gi_vst3_handle_ambience

// Array of instrument plugins indexed by instrument number, for sending 
// parameter changes.

gi_plugins[] init 5
gi_plugins[3] init gi_vst3_handle_piano
gi_plugins[4] init gi_vst3_handle_jx10

// Score generating instrument.

gi_iterations init 50
gi_duration init 1.8
gi_time_step init .6666667
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
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Param_Change
i_target_plugin = p4
i_vst3_plugin init gi_plugins[p4]
k_parameter_id init p5
k_parameter_value init p6
vst3paramset i_vst3_plugin, k_parameter_id, k_parameter_value
prints "%-24s i %9.4f t %9.4f d %9.4f target: %3d  id: %3d  value: %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p6, active(p1)
endin

instr Piano
i_note_id vst3note gi_vst3_handle_piano, 0, p4, p5, p3
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr JX10
i_note_id vst3note gi_vst3_handle_jx10, 0, p4, p5, p3
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Piano_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_piano 
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr JX10_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_jx10 
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Print_Info
i_target_plugin = p4
i_vst3_plugin init gi_plugins[p4]
vst3info i_vst3_plugin
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Save_Preset
i_target_plugin = p4
S_preset_name init p5
i_vst3_plugin init gi_plugins[i_target_plugin]
vst3presetsave i_vst3_plugin, S_preset_name
prints "%-24s i %9.4f t %9.4f d %9.4f target: %3d  preset: %s #%3d\n", nstrstr(p1), p1, p2, p3, i_target_plugin, S_preset_name, active(p1)
endin

instr Program_Change
i_target_plugin = p4
i_vst3_plugin init gi_plugins[i_target_plugin]
; May only be relevant to the MDA example plugins.
k_parameter_id init p5
k_parameter_value init p6
vst3paramset i_vst3_plugin, k_parameter_id, k_parameter_value
prints "Don't expect this one to work yet!\n"
prints "%-24s i %9.4f t %9.4f d %9.4f target: %3d  id: %3d  value: %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, i_target_plugin, k_parameter_id, k_parameter_value, active(p1)
endin

instr Load_Preset
i_target_plugin = p4
S_preset_name init p5
i_vst3_plugin init gi_plugins[i_target_plugin]
vst3presetload i_vst3_plugin, S_preset_name
prints "%-24s i %9.4f t %9.4f d %9.4f target: %3d  preset: %s #%3d\n", nstrstr(p1), p1, p2, p3, i_target_plugin, S_preset_name, active(p1)
endin

instr Delay
a_in_left inleta "inleft"
a_in_right inleta "inright"
a_out_left, a_out_right vst3audio gi_vst3_handle_delay, a_in_left, a_in_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Reverb
a_in_left inleta "inleft"
a_in_right inleta "inright"
k_old_size vst3paramget gi_vst3_handle_ambience, 0
vst3paramset gi_vst3_handle_ambience, 0, .95
vst3paramset gi_vst3_handle_ambience, 3, .9
vst3presetsave gi_vst3_handle_ambience, "ambience.preset"
a_out_left, a_out_right vst3audio gi_vst3_handle_ambience, a_in_left, a_in_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Master_Output
a_in_left inleta "inleft"
a_in_right inleta "inright"
outs a_in_left, a_in_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

</CsInstruments>
<CsScore>
f 0 40
i "Score_Generator" 1 1 3 .989 .5 36 60
i "Score_Generator" 1 1 4 .989 .5 78 6
; Stores original parameter state...
i "Print_Info" 1.1 1 4
i "Save_Preset" 1.2 1 4 "jx10.vstpreset"
i "Print_Info" 1.3 1 4
; Changes filter state...
i "Param_Change" 10 .1 4 1 .15
i "Param_Change" 11 .1 4 7 .8
i "Param_Change" 12 .1 4 12 .1
i "Print_Info" 13 .1 4
; Restores original parameter state.
i "Load_Preset" 25 1 4 "jx10.vstpreset"
i "Print_Info" 25.5 1 4
;i "Program_Change" 25 1 4 0 12
;i "Print_Info" 30.0 1 4
;i "Program_Change" 30.1 4 0 5
;i "Print_Info" 30.2 1 3

</CsScore>
</CsoundSynthesizer>
