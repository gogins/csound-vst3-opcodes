<CsoundSynthesizer>
<CsOptions>
--m-amps=1 --m-range=1 --m-dB=1 --m-benchmarks=1 --m-warnings=0 -+msg_color=0 -d -oambisonic.wav
</CsOptions>
<CsInstruments>

sr      = 48000
; NOTE: ksmps = 128 messes up timing!
ksmps   = 100
nchnls  = 2
0dbfs   = 2

connect "Piano_Output", "outleft", "Master_Output", "inleft"
connect "Piano_Output", "outright", "Master_Output", "inright"

alwayson "Piano_Output"
alwayson "Master_Output"

gi_vst3_handle_piano vst3init "/Library/Audio/Plug-Ins/VST3/Pianoteq 7.vst3", "Pianoteq 7", 1
vst3info gi_vst3_handle_piano

// Score generating instrument.

gi_iterations init 100
gi_duration init 2
gi_time_step init .25
gi_loudness init 70
instr Score_Generator
i_time = p2
i_instrument = 2
i_c = p5
i_y = p6
i_bass = p7
i_range = p8
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

instr Piano
i_note_id vst3note gi_vst3_handle_piano, 0, p4, p5, p3
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Piano_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_piano 
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Master_Output
a_in_left inleta "inleft"
a_in_right inleta "inright"
out a_in_left, a_in_right
prints "%-24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

</CsInstruments>
<CsScore>
f 0 35
i "Score_Generator" 1 1 3 .989 .5 [36 +  0] 60
i "Score_Generator" 2 1 4 .989 .5 [36 +  9] 60
i "Score_Generator" 3 1 4 .989 .5 [36 + 16] 60
</CsScore>
</CsoundSynthesizer>
