#!/bin/bash

# Add required Ubuntu repos
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update

# Install target g++ version
sudo apt install g++-11

# Check installed version
g++-11 --version

# This script does only work for Ubuntu OS.
# Check the generic (from source) method so that it's possible to make it work in any linux-based OS.