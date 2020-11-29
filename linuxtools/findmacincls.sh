#!/usr/bin/env bash

# FILE_CONTENTS=$(cat "$1")

# echo "" > "$1"

# while IFS="" read -r p || [ -n "$p" ]
# do
#   if [[ ${p:0:1} = $'\x1a' ]]; then
#     break
#   fi
#   printf "${p}\r\n" >> "$1"
# done  < <(printf '%s\n' "$FILE_CONTENTS")

shopt -s nocasematch

pat="^\sinclude\s*([\"a-zA-Z0-9\.]+)"
while IFS= read -r -d '' -u 9
do

FILENAME=$REPLY
FILE_CONTENTS=$(cat "$REPLY")

  printf "$(basename $FILENAME): "

  while IFS="" read -r p || [ -n "$p" ]
  do
    if [[ $p =~ $pat ]]; then
      printf "${BASH_REMATCH[1],,} "
    fi;
  done  < <(printf '%s\n' "$FILE_CONTENTS")

  printf "\r\n"

done 9< <( find -iname *.mac -type f -exec printf '%s\0' {} + )

shopt -u nocasematch
