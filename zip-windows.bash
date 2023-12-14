#!/bin/bash
echo "Making a release archive for Windows..."
export ARCHIVE=csound-vst3-windows.zip
export TARGETS=build-windows
cp "D:\a\csound-vst3-opcodes\csound-vst3-opcodes\build-windows\lib\Release\*" $TARGETS\lib\Release
rm -f $ARCHIVE
7z a $ARCHIVE README.md
7z a $ARCHIVE LICENSE
7z a $ARCHIVE *.png
7z a $ARCHIVE vst3-opcodes/README.md
7z a $ARCHIVE vst3-opcodes/*.png
7z a $ARCHIVE vst3-opcodes/*.csd
7z a $ARCHIVE vst3-opcodes/README.md
7z a $ARCHIVE examples/*.csd
7z a $ARCHIVE examples/*.*preset
7z a $ARCHIVE -r libvst3_plugins.*
7z a $ARCHIVE -r $TARGETS/VST3/*
echo "Archived:"
7z l $ARCHIVE
echo "Completed the release archive for Windows."
