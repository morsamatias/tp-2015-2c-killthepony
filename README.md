<h1>
  <br>
  <div class="row">
  <div class="column" align = "right" ><a href = "http://dds-jv.github.io"><img src = "https://www.frba.utn.edu.ar/wp-content/uploads/logo-utn.ba-horizontal-e1471367724904.jpg" alt="Logo UTN" width="300"></a></div>
  <div class="column" align= "center">TP 2015 2C KILL THE PONY</div>
</div>
  </br>
</h1>


**Configurar la sharedLibrary cuando se ejecuta en consola(por cada consola)**
```C
export LD_LIBRARY_PATH=/home/utnso/Escritorio/git/tp-2015-2c-killthepony/utiles/Debug
```

**sino gedit .bashrc
al final pegar** 
```C
export LD_LIBRARY_PATH=/home/utnso/Escritorio/git/tp-2015-2c-killthepony/utiles/Debug
```
**debug postmortem:** 
```C
ulimit -c unlimited

watch -n 1 cat SWAP.DATA
```
------------------------------
**Para instalar en el server** hacer el mkdir de Escritorio/git
ahi dentro hacer el clone

en el repo hay una carpeta instalacion-server/script-instalacion

ahi dentro hay unos script para instalar cada proceso y la libreria

ejemplo:para instalar el planificador seria
```C
source instalar-utiles.sh
source instalar-procesoPlanificador.sh
```
si aun asi no instala la libreria, escribir en consola:
```C
export LD_LIBRARY_PATH=/home/utnso/Escritorio/git/tp-2015-2c-killthepony/utiles/Debu
```

#### NOTA: LA LIBRERIA HAY QUE HACERLA POR CADA SESION
-----------------------------------------

#### [ENUNCIADO](https://github.com/dds-utn/2018-vn-group-19/blob/master/estadoActual.md)

| INTEGRANTE            |       NOTA            |
|  :----------------:   |  :----------------:   |
|    MATIAS MORSA       |        10             |
|    ANTONELLA  MIOZZO  |        10             |
|    MARTIN PEÑALOZA    |        10             |
|    PAULO DIGIANNI     |        10             |
|    MARTIN GUIXE       |        10             |
