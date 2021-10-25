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
more or less the same behavior as the vst4cs opcodes,

These opcodes are designed to be cross-platform and to run at least on Linux, 
Windows, and OS X. Development is currently being done only on Linux.

csound-vst3-opcodes is non-commercial software and is licensed under GPLv3, as 
is the VST3 SDK; GPLv3, in turn, is compatible with Csound, which has a 
LGPLv2.1 license that permits relicensing to any later version of the GPL.

## Building

Steinberg has provided an unusual variation on the standard CMake layout that 
seems odd at first but does simplify building VST projects. The idea is that 
user projects co-opt Steinberg's CMake build system for the VST SDK. In other 
words, one builds one's VST projects as if they were parts of the VST SDK 
itself.

The build does not work from the release archive. You must clone the Git 
repository:
```
git clone https://github.com/gogins/csound-vst3-opcodes.git
```

### Linux

In the root directory of your local repository, execute 
`bash update-submodules.bash` to initialize the Steinberg VST SDK submodules. 

Then execute `bash clean-build-linux` to build a release version of the 
plugins.

You can execute `bash zip-linux` to prepare a release archive from the build.

If you need a debug version of the build, edit the build script and replace 





There are similar scripts for other platforms.

## Installation

Copy the vst3-opcodes shared library to your Csound plugin directory, which is 
specified in the `OPCODE6DIR64` environment variable.

## User Guide

The VST3 opcodes have exactly the same names as the vst4cs opcodes, except 
that in each opcode name, "vst" is replaced by "vst3", for example "vstinit" 
becomes "vst3init".

The VST3 opcodes behave more or less the same way as the vst4cs opcodes, except 
that the VST3 protocol applies parameter changes with sample frame accuracy 
and interpolation.

For a reference to the opcodes, see their [README.md](csound-vst3/vst3-opcodes/README.md).
