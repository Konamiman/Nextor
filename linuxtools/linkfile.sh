#!/usr/bin/env bash

SRC_FILE=$1
DEST_FILE=$2

ln -sf ${PWD}/"$SRC_FILE" "${DEST_FILE}"
