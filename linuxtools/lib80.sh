#!/usr/bin/env bash

outfile=$(tempfile)

rm -f "${1}"

function finish {
  rm "${outfile}"
}

trap finish EXIT

echo -e "LIB80\r\n${@:2}\r\nbye"  | cpm 2>&1 | tee ${outfile} | grep --color=always -E "^\?File not found|$" | grep -v "Sorry, terminal not found, using cooked mode." | grep -v "A>bye\s*\$" | grep -v "A>\s*\$"

if grep -q 'File not found' ${outfile}; then
  rm -f ${1}
  exit 1
fi
