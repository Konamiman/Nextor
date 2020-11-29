#!/usr/bin/env bash

set -x

wget https://sourceforge.net/projects/sdcc/files/sdcc-linux-amd64/4.0.0/sdcc-4.0.0-amd64-unknown-linux2.5.tar.bz2/download -O "sdcc-4.0.0-amd64-unknown-linux2.5.tar.bz2"

tar -xjf  sdcc-4.0.0-amd64-unknown-linux2.5.tar.bz2

rm sdcc-4.0.0-amd64-unknown-linux2.5.tar.bz2
