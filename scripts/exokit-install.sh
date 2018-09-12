#!/bin/bash
# usage:
#   wget -qO- https://get.webmr.io | sudo sh
# This script will download and install Exokit in /usr/lib

echo "Exokit $VERSION install script"
[ "$(whoami)" != 'root' ] && echo "This script must be run as root" && exit 1
if [ -x "$(command -v apt-get)" ]; then
  apt-get install -y \
    build-essential wget python libglfw3-dev libglew-dev libfreetype6-dev libfontconfig1-dev uuid-dev libxcursor-dev libxinerama-dev libxi-dev libasound2-dev libexpat1-dev
fi
cd /tmp
rm -Rf /usr/local/bin/exokit /usr/local/lib/exokit
wget -O- https://get.webmr.io/linux-bin | tar -zxf - --directory /usr/local
