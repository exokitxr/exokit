#!/bin/bash
# usage:
#   wget -qO- https://get.webmr.io | sudo sh
# This script will download and install Exokit in /usr/lib

echo "Exokit $VERSION install script"
[ "$user" != 'root' ] && echo "This script must be run as root" && exit 1
cd /tmp
wget https://github.com/webmixedreality/exokit/releases/download/$VERSION/exokit-linux-full.tar.gz
tar -zxf exokit-linux-full.tar.gz
