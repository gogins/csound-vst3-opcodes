# csound-vst3-opcodes

<img src="VST_Compatible_Logo_Steinberg_with_TM_negative.png" width="100" height="100" />

![GitHub All Releases (total)](https://img.shields.io/github/downloads/gogins/csound-vst3-opcodes/total.svg)<br>
Michael Gogins<br>
https://github.com/gogins<br>
http://michaelgogins.tumblr.com

## Introduction

This project provides Csound plugin opcodes for hosting VST3 plugin 
instruments and effects in Csound (previously only for Csound version 6, 
currently only for Csound version 7).

These opcodes are a port of the older vst4cs opcodes, which support only the 
VST2 protocol, to the newer and more capable VST3 protocol. The opcodes have 
more or less the same behavior as the vst4cs opcodes.

These opcodes are designed to be cross-platform and to run at least on macOS 
and Linux.

csound-vst3-opcodes is non-commercial software and is licensed under GPLv3, as 
is the VST3 SDK; GPLv3, in turn, is compatible with Csound, which has a 
LGPLv2.1 license that permits relicensing to any later version of the GPL.

## Building

You should use the release binaries if you can. If you need to build this 
project for your own reasons, then...

Do not directly build the VST3 SDK. It functions as a subdirectory of the 
`csound-vst3-plugins` project. There are platform-specific shell scripts for 
building this project. The following instructions are for macOS. Just 
substitute `windows` or `linux`  for `macos` in the script filenames, 
depending on your platform.

 1. Run `update-submodules.bash`, which ensures that the Git submodules 
    used by this project have been initialized and updated.

 2. Run `clean-build-macos.bash`. It should finish with a list showing the 
    new Csound plugin shared library.

On macOS, local builds are by default not signed or notarized. If you need to 
build signed or notarized releases, export environment variables for the 
required secrets using a shell script such as:
```
# ~/csound-release-signing.env
# This file is used to set environment variables for code signing and 
# notarization when locally building Csound-based projects. It should be 
# sourced before running such builds.
export APPLE_CODESIGN_IDENTITY="Developer ID Application: My Name (9999999999)"
export APPLE_NOTARY_KEY="$HOME$/private-keys/AuthKey_9999999999.p8"
export APPLE_NOTARY_KEY_ID="9999999999"
export APPLE_NOTARY_ISSUER_ID="a9a9a9a9-a9a9-a9a9-a9a9-a9a9a9a9a9a9"
```
Then:
```
source ~/csound-release-signing.env
cmake -DCSOUND_AC_ENABLE_CODESIGN=ON -DCSOUND_AC_ENABLE_NOTARIZATION=ON ...
```
    
## Installation

Copy the vst3-opcodes shared library to your Csound plugin directory, which is 
specified in the `OPCODE7DIR64` environment variable. Or, preferably, create a 
symbolic link in the `OPCODE7DIR64` directory to the vst3-plugins shared 
library.

## Testing

To validate your build or installation on macOS, run [pianoteq9_macos_test.csd](https://github.com/gogins-dev/csound-vst3-opcodes/blob/master/examples/pianoteq9_macos_test.csd) from the terminal with Csound. If you do not have the Pianoteq plugin, you can substitute some plugin that you do have.

## User Guide

The VST3 opcodes have exactly the same names as the vst4cs opcodes, except 
that in each opcode name, "vst" is replaced by "vst3", for example "vstinit" 
becomes "vst3init".

The VST3 opcodes behave more or less the same way as the vst4cs opcodes, except 
that the VST3 protocol applies parameter changes with sample frame accuracy 
and interpolation.

For a reference to the opcodes, see their [README.md](csound-vst3/vst3-opcodes/README.md).

## Release Notes

### v2.0.0-beta

On macOS, the vst3-opcodes shared library is now built only for the amd64 
architecture.

This release is also built only for Csound version 7.

The toolchain for configuring, compiling, packaging, signing and notarizing 
(only on macOS), and releasing is now implemented completely as a GitHub 
Action.

### v1.1.0

On macOS, the vst3-opcodes shared library is now built as a universal binary 
(including both x86-64 CPU architecture and arm64 CPU architecture).

The handling of fractional pitches (i.e. fractional MIDI key numbers) in this 
project has been corrected and simplified.

Program changes are now working, at least for the sample plugins in the VST3 
SDK.

Loading presets has been implemented using the `vst3initpreset` opcode.

There was a bug in that the VST3 plugin busses were not being properly 
activated, causing some VST3 plugins not to process audio. This has been fixed.

The sample VST3 plugins build by the VST3 SDK are now included in releases.

The directory structure of this project has been simplified.

The build system has been corrected and simplified.
