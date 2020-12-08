#!/usr/bin/env bash

set -e

outfile=$(tempfile)

rm -f "${1}"

function finish {
  rm "${outfile}"
}

trap finish EXIT

echo -e "L80\r\n${@:2}\r\nbye"  | \
    cpm 2>&1 | \
    tee ${outfile} | \
    grep --color=always -E "\?Out of memory|$" | \
    grep --color=always -E "%Overlaying Data area|$" | \
    grep --color=always -E "\?.*+Not Found|$" | \
    grep --color=always -E "\?Loading Error|$" | \
    grep -v "Sorry, terminal not found, using cooked mode." | \
    grep -v "A>bye\s*\$" | \
    grep -v "A>\s*\$" | \
    grep --color -E "[0-9]+\s+Undefined Global\(s\)|$"



if grep -q 'Undefined Global(s)' ${outfile}; then
  rm -f ${1}
  exit 1
fi

if grep -q ' Not Found' ${outfile}; then
  rm -f ${1}
  exit 1
fi


if grep -q '?Out of memory' ${outfile}; then
  rm -f ${1}
  exit 1
fi

cleancpmfile.sh "${1}"
