#!/bin/bash
echo "Making a release archive for Linux..."
export ARCHIVE=csound-vst3-linux.zip
export TARGETS=build-linux
rm -f $ARCHIVE
7za a $ARCHIVE README.md
7za a $ARCHIVE LICENSE
7za a $ARCHIVE *.png
7za a $ARCHIVE csound-vst3/vst3-opcodes/README.md
7za a $ARCHIVE csound-vst3/vst3-opcodes/*.png
7za a $ARCHIVE csound-vst3/vst3-opcodes/*.csd
7za a $ARCHIVE -r $TARGETS/*vst3_plugins*
7za a $ARCHIVE -r $TARGETS/*.vst3
echo "Archived:"
7za l $ARCHIVE
echo "Completed the release archive for Linux."
