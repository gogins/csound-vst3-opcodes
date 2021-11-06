#!/bin/bash 
echo "Updating all submodules for csound-vst3-opcodes..."
git submodule --init --recursive --remote
git submodule status --recursive
sudo -k
echo "Finished updating all submodules for csound-vst3-opcodes."
