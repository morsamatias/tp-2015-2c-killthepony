#!/bin/bash

cp -a /home/utnso/Escritorio/git/tp-2015-2c-killthepony/instalacion-server/procesoSwap/.  /home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoSwap/

cd /home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoSwap/Debug

make clean
make

echo "*******************************Fin ok"
