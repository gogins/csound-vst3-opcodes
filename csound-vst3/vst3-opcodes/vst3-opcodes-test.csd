<CsoundSynthesizer>
<CsOptions>
-m195 --opcode-lib="/home/mkg/csound-vst3-opcodes/build-debug/lib/Debug/libvst3_plugins.so"
</CsOptions>
<CsInstruments>
nchnls = 2
sr = 48000
ksmps = 128
instr 1
print p1, p2, p3
i_handle_mda vst3init "/home/mkg/csound-vst3-opcodes/build/VST3/Release/mda-vst3.vst3", "mda Piano", 1
// Currently the Pianoteq does not support vst3 on Linux. I asked Modartt.
// Julien said it is because Pianoteq uses JUCE which currently does not 
// support vst3 on Linux. I may go ahead and add VST2 support to this project, 
// at least privately.
// i_handle_ptq vst3init "/home/mkg/Pianoteq_6.vst3", 1
vst3info i_handle_mda
ainleft, ainright ins
aleft, aright vst3audio i_handle_mda;, ainleft, ainright
outs aleft, aright
endin
</CsInstruments>
<CsScore>
i 1 1 1
</CsScore>
</CsoundSynthesizer>