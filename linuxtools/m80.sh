#!/usr/bin/env bash

outfile=$(tempfile)

function finish {
  rm "${outfile}"
}

trap finish EXIT

cpm --exec M80 /Z =$1 | tee "${outfile}" | \
  grep --color=always -E "\?File not found|$" |  \
  grep --color=always -E "[0-9]+\s+Fatal\serror\(s\)|$" | \
  grep --color=always -E "^Unterminated REPT/IRP/IRPC/MACRO|$" | \
  grep --color=always -E "^%No END statement|$" | \
  GREP_COLORS='ms=01;2' grep --color=always -E "^. Size of.*bytes %|$" | \
  GREP_COLORS='ms=01;2' grep --color=always -E "No Fatal error\(s\)|$" | \
  GREP_COLORS='ms=01;2' grep --color=always -E "% Size of module.* bytes|$"

if grep -q '%No END statement'  "${outfile}"; then
  rm -f $2
  exit 1
fi

if ! grep -q 'No Fatal error(s)' "${outfile}"; then
  rm -f $2
  exit 1
fi
