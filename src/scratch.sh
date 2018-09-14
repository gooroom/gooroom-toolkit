#!/bin/sh

unzipdir=$1
installdir=$2

echo "$unzipdir"
echo "$installdir"

cd $unzipdir 

sudo unzip scratch-gui-develop.zip -d $installdir

sudo unzip scratch-vm-develop.zip -d $installdir

sudo unzip scratch-blocks-develop.zip -d $installdir

cd $installdir

cd scratch-vm-develop

sudo npm install

sudo npm link

#npm run watch

cd ../scratch-blocks-develop

sudo npm install

sudo npm link

cd ../scratch-gui-develop

sudo npm install

sudo npm link scratch-vm scratch-blocks

sudo npm install
