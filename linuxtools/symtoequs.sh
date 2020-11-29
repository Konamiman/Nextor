#!/usr/bin/env bash

# Extract symbols and output at asm equ statements
# symtoequs.sh source.sym output.inc pat1 pat2 pat3 patn ...

FILE_CONTENTS1=$(cat "$1" | cut -f1)
FILE_CONTENTS2=$(cat "$1" | cut -f2)
FILE_CONTENTS3=$(cat "$1" | cut -f3)
FILE_CONTENTS4=$(cat "$1" | cut -f4)
FILE_CONTENTS=$(echo "$FILE_CONTENTS1"; echo "$FILE_CONTENTS2"; echo "$FILE_CONTENTS3"; echo "$FILE_CONTENTS4")

echo "" > "$2"

for inputpat in "${@:3}"
do
  pat="\b(....)\s(${inputpat})\b"

  while IFS="" read -r p || [ -n "$p" ]
  do
    if [[ "$p" =~ $pat ]]; then
      printf "${BASH_REMATCH[2]}\tequ\t${BASH_REMATCH[1]}h\r\n" >> "$2"
      printf "\tpublic\t${BASH_REMATCH[2]}\r\n" >> "$2"
    fi

  done  < <(printf '%s\n' "$FILE_CONTENTS")

done
