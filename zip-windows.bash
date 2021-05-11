#!/bin/bash
echo "Making a release archive for Windows..."
export ARCHIVE=csound-vst3-windows.zip
export TARGETS=build-windows
rm -f $ARCHIVE
# Possibly should be: "/c/Program Files/7-Zip/7z.exe"
7z a $ARCHIVE README.md
7z a $ARCHIVE LICENSE
7z a $ARCHIVE *.png
7z a $ARCHIVE csound-vst3/vst3-opcodes/README.md
7z a $ARCHIVE csound-vst3/vst3-opcodes/*.png
7z a $ARCHIVE csound-vst3/vst3-opcodes/*.csd
7z a $ARCHIVE $TARGETS/*.dll
7z a $ARCHIVE $TARGETS/*.pdb
7z a $ARCHIVE -r $TARGETS/*.vst3
echo "Completed the release archive for Windows."
