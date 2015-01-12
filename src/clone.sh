#!/bin/bash
if [ -z "$2" ]
then
    echo Usage: clone source_dir destination_dir
    exit 1
fi
DEST_DIR=$2
LAST_DIR=$PWD
export DEST_DIR LAST_DIR
mkdir -p $DEST_DIR; cd $1;tar cpf - . | ( cd $LAST_DIR; cd $DEST_DIR; tar xpf - )
