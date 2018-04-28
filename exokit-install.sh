#!/bin/bash
# wget https://get.webmr.io | sudo sh
# This script will download and install Exokit in /usr/lib

echo "Exokit $VERSION install script"
if [[ $EUID -ne 0 ]]; then
  echo "This script must be run as root"
  exit 1
fi
cd /tmp
wget https://github.com/webmixedreality/exokit/releases/download/$VERSION/exokit-linux-full.tar.gz
tar -zxf exokit-linux-full.tar.gz
