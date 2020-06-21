<CsoundSynthesizer>
<CsOptions>
-m195 --opcode-lib="/home/mkg/csound-vst3-opcodes/build-debug/lib/Debug/libvst3_plugins.so"
</CsOptions>
<CsInstruments>
instr 1
print p1, p2, p3
i_handle_mda vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Release/mda-vst3.vst3", "x", 1
// Currently the Pianoteq does not support vst3 on Linux. I will enquire.
// i_handle_ptq vst3init "/home/mkg/Pianoteq_6.vst3", 1
endin
</CsInstruments>
<CsScore>
i 1 1 1
</CsScore>
</CsoundSynthesizer>