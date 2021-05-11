#!/bin/bash
echo "Making a release archive for Windows..."
export ARCHIVE=csound-vst3-windows.zip
export TARGETS=build-windows
rm -f $ARCHIVE
zip $ARCHIVE README.md
zip $ARCHIVE LICENSE
zip $ARCHIVE *.png
zip $ARCHIVE csound-vst3/vst3-opcodes/README.md
zip $ARCHIVE csound-vst3/vst3-opcodes/*.png
zip $ARCHIVE csound-vst3/vst3-opcodes/*.csd
zip $ARCHIVE $TARGETS/*.dll
zip $ARCHIVE $TARGETS/*.pdb
zip $ARCHIVE -r $TARGETS/*.vst3
echo "Completed the release archive for Windows."
