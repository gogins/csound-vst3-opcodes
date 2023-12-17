# csound-vst3-opcodes

<img src="VST_Compatible_Logo_Steinberg_with_TM_negative.png" width="100" height="100" />

![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-vst3-opcodes/total.svg)<br>
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Introduction

This project provides Csound plugin opcodes for hosting VST3 plugin 
instruments and effects in Csound.

These opcodes are a port of the older vst4cs opcodes, which support only the 
VST2 protocol, to the newer and more capable VST3 protocol. The opcodes have 
more or less the same behavior as the vst4cs opcodes.

These opcodes are designed to be cross-platform and to run at least on Linux, 
Windows, and macOS.

csound-vst3-opcodes is non-commercial software and is licensed under GPLv3, as 
is the VST3 SDK; GPLv3, in turn, is compatible with Csound, which has a 
LGPLv2.1 license that permits relicensing to any later version of the GPL.

## Building

Do not directly build the VST3 SDK. It functions as a subdirectory of the 
`csound-vst3-plugins` project. There are platform-specific shell scripts for 
building this project. The following instructions are for macOS. Just 
substitute `windows` or `linux`  for `macos` in the script filenames, 
depending on your platform.

 1. Run `update-submodules.bash`, which ensures that the Git submodules 
    used by this project have been initialized and updated.

 2. Run `clean-build-macos.bash`. It should finish with a list showing the 
    new Csound plugin shared library.

 3. Run `zip-macos.bash` to create a Zip file containing the shared library,
    
## Installation

Copy the vst3-opcodes shared library to your Csound plugin directory, which is 
specified in the `OPCODE6DIR64` environment variable. Or, preferably, create a 
symbolic link in the `OPCODE6DIR64` directory to the vst3-plugins shared 
library.

## User Guide

The VST3 opcodes have exactly the same names as the vst4cs opcodes, except 
that in each opcode name, "vst" is replaced by "vst3", for example "vstinit" 
becomes "vst3init".

The VST3 opcodes behave more or less the same way as the vst4cs opcodes, except 
that the VST3 protocol applies parameter changes with sample frame accuracy 
and interpolation.

For a reference to the opcodes, see their [README.md](csound-vst3/vst3-opcodes/README.md).

## Release Notes

### v1.1.0

On macOS, the vst3-opcodes shared library is now built as a universal binary 
(including both x86-64 CPU architecture and arm64 CPU architecture).

The handling of fractional pitches (i.e. fractional MIDI key numbers) in this 
project has been corrected and simplified.

Program changes are now working, at least for the sample plugins in the VST3 
SDK.

There was a bug in that the VST3 plugin busses were not being properly 
activated, causing some VST3 plugins not to process audio. This has been fixed.

The sample VST3 plugins build by the VST3 SDK are now included in releases.

The directory structure of this project has been simplified.

The build system has been corrected and simplified.
