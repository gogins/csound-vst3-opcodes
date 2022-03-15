#!/bin/bash
echo "Making a release archive for macOS..."
export ARCHIVE=csound-vst3-macos.zip
export TARGETS=build-macos
rm -f $ARCHIVE
7z a $ARCHIVE README.md
7z a $ARCHIVE LICENSE
7z a $ARCHIVE *.png
7z a $ARCHIVE csound-vst3/vst3-opcodes/README.md
7z a $ARCHIVE csound-vst3/vst3-opcodes/*.png
7z a $ARCHIVE csound-vst3/vst3-opcodes/*.csd
7z a $ARCHIVE -r $TARGETS/*vst3_plugins*
7z a $ARCHIVE -r $TARGETS/*.vst3
echo "Archived:"
7z l $ARCHIVE
echo "Completed the release archive for macOS."
