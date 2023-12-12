#!/bin/bash
echo "Making a debug archive for Windows..."
export ARCHIVE=csound-vst3-windows-debug.zip
export TARGETS=build-windows-debug
rrm -f $ARCHIVE
7z a $ARCHIVE README.md
7z a $ARCHIVE LICENSE
7z a $ARCHIVE *.png
7z a $ARCHIVE vst3-opcodes/README.md
7z a $ARCHIVE vst3-opcodes/*.png
7z a $ARCHIVE vst3-opcodes/*.csd
7z a $ARCHIVE vst3-opcodes/README.md
7z a $ARCHIVE examples/*.csd
7z a $ARCHIVE examples/*.*preset
7z a $ARCHIVE -r $TARGETS/*vst3_plugins*
7z a $ARCHIVE -r $TARGETS/*.vst3
echo "Archived:"
7z l $ARCHIVE

echo "Completed the debug archive for Windows."
