# csound-vst3-opcodes

<img src="VST_Compatible_Logo_Steinberg_with_TM_negative.png" width="100" height="100" />

![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-vst3-opcodes/total.svg)<br>
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Introduction

This project provides plugin opcodes for hosting VST3 plugin instruments and 
effects in Csound.

These opcodes are a port of the older vst4cs opcodes, which supports only the 
VST2 protocol, to the newer and more capable VST3 protocol. The opcodes have 
the same behavior as the vst4cs opcodes,

These opcodes are designed to be cross-platform and to run at least on Linux, 
Windows, and OS X. Development is currently being done only on Linux.

csound-vst3-opcodes is non-commercial software and is licensed under GPLv3, as 
is the VST3 SDK; GPLv3, in turn, is compatible with Csound, which has a 
LGPLv2.1 license that permits relicensing to any later version of the GPL.

## Building

### Linux

In the root directory of your local repository, execute 
`bash update-submodules.bash` to initialize the Steinberg VST SDK submodules.

Change to the `vst3sdk` directory and, on Linux, prepare for building 
according to the `vst3sdk/doc/vstinterfaces/linuxSetup.html` file. Then build 
the VST SDK and examples following the instructions 
[here](https://github.com/steinbergmedia/vst3sdk#200), 

Install Csound on your system. On Linux, build Csound using the instructions 
[here](https://github.com/csound/csound/blob/develop/BUILD.md).

Change to the `vst3-plugins` directory and execute the following commands:
```
mkdir build
cd build
cmake ..
make -j4
cpack
```

## Installation

Copy the vst3-cs shared library to your Csound plugin directory, which is 
specified in the `OPCODE6DIR64` environment variable.

## User Guide

The VST3 opcodes have exactly the same names as the vst4cs opcodes, except 
that in each opcode name, "vst" is replaced by "vst3", for example "vstinit" 
becomes "vst3init".

The VST3 opcodes behave exactly the same way as the vst4cs opcodes, except 
that the VST3 protocol applies parameter changes with sample frame accuracy 
and interpolation.