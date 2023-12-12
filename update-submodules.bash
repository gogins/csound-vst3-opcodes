#!/bin/bash 
echo "Updating all submodules for csound-vst3-opcodes..."
git submodule update --init --recursive
git submodule status --recursive
sudo -k
cd csound
git pull
git checkout csound6
git branch
cd ..
echo "Finished updating all submodules for csound-vst3-opcodes."
