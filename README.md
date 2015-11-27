# tp-2015-2c-killthepony

configurar la sharedLibrary cuando se ejecuta en consola(por cada consola)
export LD_LIBRARY_PATH=/home/utnso/Escritorio/git/tp-2015-2c-killthepony/utiles/Debug


sino gedit .bashrc
al final pegar export LD_LIBRARY_PATH=/home/utnso/Escritorio/git/tp-2015-2c-killthepony/utiles/Debug

debug postmortem: 
ulimit -c unlimited

watch -n 1 cat SWAP.DATA

------------------------------
Para instalar en el server hay que hacer el mkdir de Escritorio/git
ahi dentro hacer el clone

en el repo hay una carpeta instalacion-server/script-instalacion

ahi dentro hay unos script para instalar cada proceso y la libreria

ejemplo:para instalar el planificador seria
source instalar-utiles.sh
source instalar-procesoPlanificador.sh

si aun asi no instala la libreria, escribir en consola:
export LD_LIBRARY_PATH=/home/utnso/Escritorio/git/tp-2015-2c-killthepony/utiles/Debug

NOTA: LA LIBRERIA HAY QUE HACERLA POR CADA SESION
-----------------------------------------
