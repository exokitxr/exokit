#!/bin/bash
# usage:
#   wget -qO- https://get.webmr.io | sudo sh
# This script will download and install Exokit in /usr/lib

echo "Exokit $VERSION install script"
[ "$(whoami)" != 'root' ] && echo "This script must be run as root" && exit 1
cd /tmp
rm -Rf /usr/local/bin/exokit /usr/local/lib/exokit
wget -O- https://get.webmr.io/linux-bin | tar -zxf - --directory /usr/local
