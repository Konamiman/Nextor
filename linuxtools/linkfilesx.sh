#!/usr/bin/env bash

SRC_FILES=$1
DEST_DIR=$2

for file in ${1}
do
  [ -e "$file" ] || continue
  target=${2}/$(basename "$file")
  ln -s ${PWD}/"$file" ${target,,}
done
