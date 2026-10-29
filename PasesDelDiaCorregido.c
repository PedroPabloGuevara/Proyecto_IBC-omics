#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
-crea la lista de pases del dia, vacia
-retorna el puntero a la lista, que se guarda en tienda->filaPasesDeldia
*/

struct pasesDelDia *crearPasesDelDia(){
    struct pasesDelDia *nueva;

    nueva=((struct pasesDelDia*)malloc(sizeof(struct pasesDelDia)));
    nueva->head=NULL;

    return nueva;
}

/*
-busca un pase por codigo recorriendo la lista de pases del dia de principio a fin
-recibe la lista y el codigo a buscar
-retorna el nodo si lo encuentra, o NULL si no existe
*/

struct NodoPase *buscarPaseDelDia(struct pasesDelDia *lista, int codigo){
    struct NodoPase *rec;

    if(lista==NULL)
    {
        return NULL;
    }

    rec=lista->head;
    while(rec!=NULL)
    {
        if(rec->paseVigente->codigo==codigo)
        {
            return rec;
        }
        rec=rec->sig;
    }
    return NULL;
}

/*
-agrega un pase al final de la lista de pases del dia
-el pase ya debe existir (creado con crearPase y guardado en el arreglo de pases)
-la lista solo guarda la referencia, no copia el pase
-no permite codigos repetidos
-retorna 1 si se agrego, 0 si el codigo ya existia o los datos no son validos
*/

int agregarPaseDelDia(struct pasesDelDia *lista, struct Pase *pase){
    struct NodoPase *rec, *nuevo;

    if(lista==NULL || pase==NULL)
    {
        return 0;
    }

    if(buscarPaseDelDia(lista, pase->codigo)!=NULL)
    {
        return 0;
    }

    nuevo=crearNodoPase(pase);

    if(lista->head==NULL)
    {
        lista->head=nuevo;
    }
    else
    {
        rec=lista->head;
        while(rec->sig!=NULL)
        {
            rec=rec->sig;
        }
        rec->sig=nuevo;
    }
    return 1;
}

/*
-elimina un pase de la lista de pases del dia segun su codigo
-retorna 1 si se elimino, 0 si no existe, -1 si la lista esta vacia
*/

int eliminarPaseDelDia(struct pasesDelDia *lista, int codigo){
    struct NodoPase *rec, *borrar;

    if(lista==NULL)
    {
        return 0;
    }

    if(lista->head==NULL)
    {
        return -1;
    }

    rec=lista->head;
    if(rec->paseVigente->codigo==codigo)
    {
        borrar=rec;
        lista->head=rec->sig;
        free(borrar);
        return 1;
    }
    else
    {
        while(rec->sig!=NULL)
        {
            if(rec->sig->paseVigente->codigo==codigo)
            {
                borrar=rec->sig;
                rec->sig=borrar->sig;
                free(borrar);
                return 1;
            }
            rec=rec->sig;
        }
    }
    return 0;
}

/*
-modifica el tipo y el valor de un pase de la lista de pases del dia
-busca el nodo y usa modificarPase (del modulo de pases) para hacer el cambio
-retorna 1 si se modifico, 0 si no existe, -1 si el tipo o el valor no son validos
*/

int modificarPaseDelDia(struct pasesDelDia *lista, int codigo, int nuevoTipo, int nuevoValor){
    struct NodoPase *nodo;

    nodo=buscarPaseDelDia(lista, codigo);
    if(nodo==NULL)
    {
        return 0;
    }

    return modificarPase(nodo->paseVigente, nuevoTipo, nuevoValor);
}

/*
-muestra todos los pases de la lista de pases del dia, en el orden en que estan
-si no hay pases avisa con un mensaje
*/

void listarPasesDelDia(struct pasesDelDia *lista){
    struct NodoPase *rec;
    int cantidad=0;

    if(lista==NULL || lista->head==NULL)
    {
        printf("No hay pases registrados hoy.\n");
    }
    else
    {
        rec=lista->head;
        while(rec!=NULL)
        {
            cantidad++;
            printf("Cantidad: %d, Codigo: %d, Tipo: %d, Valor: %d, Estado: %d, Fecha: %s, Hora: %s\n",
                   cantidad, rec->paseVigente->codigo, rec->paseVigente->tipoPase,
                   rec->paseVigente->valor, rec->paseVigente->estado,
                   rec->paseVigente->fecha, rec->paseVigente->hora);
            rec=rec->sig;
        }
    }
}

/*
-cuenta cuantos pases de la lista del dia hay en un estado dado (activo 1, anulado 2, utilizado 3, vencido 4)
-por ejemplo, con estado 3 se sabe cuantos pases se utilizaron
-retorna la cantidad, 0 si no hay ninguno
*/

int contarPasesPorEstado(struct pasesDelDia *lista, int estado){
    struct NodoPase *rec;
    int contador=0;

    if(lista==NULL)
    {
        return 0;
    }

    rec=lista->head;
    while(rec!=NULL)
    {
        if(rec->paseVigente->estado==estado)
        {
            contador++;
        }
        rec=rec->sig;
    }
    return contador;
}

/*
-busca un pase por codigo en la fila de entrada, recorriendola desde el head hasta el final
-la fila no es circular, el ultimo nodo (tail) tiene sig en NULL
-retorna el nodo si lo encuentra, o NULL si no existe
-esta funcion corresponde al modulo de la fila, queda aqui porque recorre nodos de pase
*/

struct NodoPase *buscarPaseFila(struct Fila *fila, int codigo){
    struct NodoPase *rec;

    if(fila==NULL)
    {
        return NULL;
    }

    rec=fila->head;
    while(rec!=NULL)
    {
        if(rec->paseVigente->codigo==codigo)
        {
            return rec;
        }
        rec=rec->sig;
    }
    return NULL;
}