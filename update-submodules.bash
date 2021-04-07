#!/bin/bash 
echo "Updating all submodules for csound-vst3-opcodes..."
git submodule update --init --recursive --remote
git submodule foreach git pull origin master
git submodule status --recursive
sudo -k
echo "Finished updating all submodules for csound-vst3-opcodes."
