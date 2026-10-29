#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
-crea un pase en memoria dinamica con los datos recibidos
-los datos son: tipoPase, codigo, valor, estado, fecha y hora
-retorna el puntero al pase creado
*/

struct Pase *crearPase(int tipoPase, int codigo, int valor, int estado, char *fecha, char *hora){
    struct Pase *nuevo;

    nuevo=((struct Pase*)malloc(sizeof(struct Pase)));

    nuevo->tipoPase=tipoPase;
    nuevo->codigo=codigo;
    nuevo->valor=valor;
    nuevo->estado=estado;

    nuevo->fecha=((char *)malloc((strlen(fecha)+1)*sizeof(char)));
    strcpy(nuevo->fecha, fecha);
    nuevo->hora=((char *)malloc((strlen(hora)+1)*sizeof(char)));
    strcpy(nuevo->hora, hora);

    return nuevo;
}

/*
-hace que un pase se convierta en un nodo de lista
-recibe un puntero a pase y retorna un puntero a nodo de pase
-lo usan tanto la lista de pases del dia como la fila
*/

struct NodoPase *crearNodoPase(struct Pase *pase){
    struct NodoPase *nuevo;

    nuevo=((struct NodoPase*)malloc(sizeof(struct NodoPase)));
    nuevo->paseVigente=pase;
    nuevo->sig=NULL;

    return nuevo;
}

/*
-busca la posicion de un pase por codigo en un arreglo de pases
-recibe el arreglo y su plibre (cantidad de pases guardados)
-retorna la posicion si lo encuentra, o -1 si no existe
-sirve para que quien haga eliminarPase sepa desde donde compactar el arreglo
*/

int buscarPosicionPase(struct Pase **pases, int plibre, int codigo){
    int i;

    for(i=0; i<plibre; i++)
    {
        if(pases[i]->codigo==codigo)
        {
            return i;
        }
    }
    return -1;
}

/*
-busca un pase por codigo en un arreglo de pases
-usa buscarPosicionPase
-retorna el pase si lo encuentra, o NULL si no existe
*/

struct Pase *buscarPase(struct Pase **pases, int plibre, int codigo){
    int pos;

    pos=buscarPosicionPase(pases, plibre, codigo);
    if(pos==-1)
    {
        return NULL;
    }
    return pases[pos];
}

/*
-agrega un pase al arreglo, en la primera posicion libre (plibre)
-recibe el arreglo, su plibre por puntero (porque aumenta), su tamano total y el pase
-no permite codigos repetidos, datos invalidos (tipo y estado entre 1 y 4, valor no negativo) ni arreglo lleno
-retorna 1 si se agrego, 0 si no se pudo
*/

int agregarPase(struct Pase **pases, int *plibre, int tam, struct Pase *pase){

    if(pase==NULL || *plibre>=tam)
    {
        return 0;
    }

    if(pase->tipoPase<1 || pase->tipoPase>4 || pase->estado<1 || pase->estado>4 || pase->valor<0)
    {
        return 0;
    }

    if(buscarPosicionPase(pases, *plibre, pase->codigo)!=-1)
    {
        return 0;
    }

    pases[*plibre]=pase;
    (*plibre)++;
    return 1;
}

/*
-modifica el tipo y el valor de un pase
-el codigo y el estado no se modifican
-como el pase es compartido, el cambio lo ven el cliente, la fila y los pases del dia
-retorna 1 si se modifico, 0 si el pase es NULL, -1 si el tipo o el valor no son validos
*/

int modificarPase(struct Pase *pase, int nuevoTipo, int nuevoValor){

    if(pase==NULL)
    {
        return 0;
    }

    if(nuevoTipo<1 || nuevoTipo>4 || nuevoValor<0)
    {
        return -1;
    }

    pase->tipoPase=nuevoTipo;
    pase->valor=nuevoValor;
    return 1;
}

/*
-muestra todos los pases de un arreglo, en el orden en que estan
-si no hay pases avisa con un mensaje
*/

void listarPases(struct Pase **pases, int plibre){
    int i;

    if(plibre==0)
    {
        printf("No hay pases registrados.\n");
    }
    else
    {
        for(i=0; i<plibre; i++)
        {
            printf("Codigo: %d, Tipo: %d, Valor: %d, Estado: %d, Fecha: %s, Hora: %s\n",
                   pases[i]->codigo, pases[i]->tipoPase, pases[i]->valor,
                   pases[i]->estado, pases[i]->fecha, pases[i]->hora);
        }
    }
}

/*
-libera un pase y sus textos (fecha y hora)
-usar solo cuando el pase se elimina de todos lados, una sola vez por pase
*/

void liberarPase(struct Pase *pase){

    if(pase==NULL)
    {
        return;
    }

    free(pase->fecha);
    free(pase->hora);
    free(pase);
}