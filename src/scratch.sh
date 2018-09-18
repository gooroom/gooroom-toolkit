#!/bin/bash

set -xe

REPOSITORY_URL=http://packages.gooroom.kr/scratch/
RELEASE_VER=$1

[ -z $RELEASE_VER ] && RELEASE_VER=20180920

target_dir=/opt
scratchPath=$target_dir/scratch/

_gui='scratch-gui'
_vm='scratch-vm'
_blocks='scratch-blocks'

cd $target_dir
wget -P $target_dir $REPOSITORY_URL/$RELEASE_VER/scratch.tar.gz

#if [ $? -ne 0 ]; then exit; fi
[ $? -ne 0 ] && exit 1

tar xfz scratch.tar.gz -C $target_dir
rm scratch.tar.gz

cd $scratchPath

cd $scratchPath$_vm
npm install
npm link

cd $scratchPath$_blocks
npm install
npm link

cd $scratchPath$_gui

# TODO.
sudo npm install

npm start
