#!/bin/bash
echo "Making a release archive for Mac OS..."
export ARCHIVE=csound-vst3-macos.zip
export TARGETS=build-macos
rm -f $ARCHIVE
zip $ARCHIVE README.md
zip $ARCHIVE LICENSE
zip $ARCHIVE *.png
zip $ARCHIVE csound-vst3/vst3-opcodes/README.md
zip $ARCHIVE csound-vst3/vst3-opcodes/*.png
zip $ARCHIVE csound-vst3/vst3-opcodes/*.csd
zip $ARCHIVE $TARGETS/*.so
zip $ARCHIVE $TARGETS/*.dylib
zip $ARCHIVE -r $TARGETS/*.vst3
echo "Completed the release archive for Linux."
