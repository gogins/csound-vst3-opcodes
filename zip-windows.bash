#!/bin/bash
echo "Making a release archive for Windows..."
export ARCHIVE=csound-vst3-windows.zip
export TARGETS=build-windows
rm -f $ARCHIVE
7za a $ARCHIVE README.md
7za a $ARCHIVE LICENSE
7za a $ARCHIVE *.png
7za a $ARCHIVE csound-vst3/vst3-opcodes/README.md
7za a $ARCHIVE csound-vst3/vst3-opcodes/*.png
7za a $ARCHIVE csound-vst3/vst3-opcodes/*.csd
7za a $ARCHIVE $TARGETS/*.dll
7za a $ARCHIVE $TARGETS/*.pdb
7za a $ARCHIVE -r $TARGETS/*.vst3
echo "Completed the release archive for Windows."
