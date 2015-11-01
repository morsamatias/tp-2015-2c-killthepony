#!/bin/bash

cp -a /home/utnso/Escritorio/git/tp-2015-2c-killthepony/instalacion-server/procesoSwap/.  /home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoNodo/

cd /home/utnso/Escritorio/git/tp-2015-2c-killthepony/procesoNodo/Debug

make clean
make

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH=/home/utnso/Escritorio/git/tp-2015-2c-killthepony/utiles/Debug

echo "*******************************Fin"
