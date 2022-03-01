#!/bin/bash
echo "Making a release archive for macOS..."
export ARCHIVE=csound-vst3-macos.zip
export TARGETS=build-macos
rm -f $ARCHIVE
7zz a $ARCHIVE README.md
7zz a $ARCHIVE LICENSE
7zz a $ARCHIVE *.png
7zz a $ARCHIVE csound-vst3/vst3-opcodes/README.md
7zz a $ARCHIVE csound-vst3/vst3-opcodes/*.png
7zz a $ARCHIVE csound-vst3/vst3-opcodes/*.csd
7zz a $ARCHIVE -r $TARGETS/*vst3_plugins*
7zz a $ARCHIVE -r $TARGETS/*.vst3
echo "Archived:"
7zz l $ARCHIVE
echo "Completed the release archive for macOS."
