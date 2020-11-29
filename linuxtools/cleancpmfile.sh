#!/usr/bin/env bash

# trim file until a CTRL+Z is found

FILE_CONTENTS=$(cat "$1")

echo "" > "$1"

while IFS="" read -r p || [ -n "$p" ]
do
  if [[ ${p:0:1} = $'\x1a' ]]; then
    break
  fi
  printf "${p}\r\n" >> "$1"
done  < <(printf '%s\n' "$FILE_CONTENTS")

