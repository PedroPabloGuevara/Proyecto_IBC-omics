#include "../../include/clientes.h"
#include "../../include/pases.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Cliente *crearCliente(char *nombre, int rut)
{
    struct Cliente *nuevo;
    int i, tam;

    if (nombre == NULL) {
        return NULL;
    }

    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }

    tam = strlen(nombre);
    nuevo->nombre = malloc((tam + 1) * sizeof(char));
    if (nuevo->nombre == NULL) {
        free(nuevo);
        return NULL;
    }
    strcpy(nuevo->nombre, nombre);
    nuevo->rut=rut;

    nuevo->pases = malloc(MAXPASES * sizeof(*nuevo->pases));
    if (nuevo->pases == NULL) {
        free(nuevo->nombre);
        free(nuevo);
        return NULL;
    }
    for(i=0; i<MAXPASES; i++)
    {
        nuevo->pases[i]=NULL;
    }
    nuevo->plibrepases=0;

    return nuevo;
}

struct NodoCliente *crearNodoCliente(struct Cliente *dato) /* para meter en lista */
{
    struct NodoCliente *nuevo;

    if (dato == NULL) {
        return NULL;
    }

    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->dato=dato;
    nuevo->sig=NULL;

    return nuevo;
}

struct NodoCliente *buscarCliente(struct NodoCliente *head, int rut) {
    struct NodoCliente *rec=head;
    if(rec!=NULL)
    {
        do
        {
            if(rec->dato->rut==rut)
            {
                return rec;
            }
            rec=rec->sig;
        }while(rec!=head);
    }
    return NULL;
}
void enlazarCliente(struct NodoCliente **head, struct NodoCliente *nuevo)
{
    struct NodoCliente *rec;

    if (head == NULL || nuevo == NULL) {
        return;
    }

    if(*head==NULL)
    {
        *head=nuevo;
        nuevo->sig=*head;
    }
    else
    {
        rec=*head;
        while(rec->sig!=*head)
        {
            rec=rec->sig;
        }
        rec->sig=nuevo;
        nuevo->sig=*head;
    }
}

int agregarCliente(struct NodoCliente **head, char *nombre, int rut) /* retorna 1 si lo agrego, 0 si el rut ya existia */
{
    struct Cliente *cliente;
    struct NodoCliente *nodo;

    if (head == NULL || nombre == NULL) {
        return 0;
    }

    if (buscarCliente(*head, rut)!=NULL) {
        return 0;
    }

    cliente = crearCliente(nombre, rut);
    if (cliente == NULL) {
        return 0;
    }

    nodo = crearNodoCliente(cliente);
    if (nodo == NULL) {
        free(cliente->nombre);
        free(cliente->pases);
        free(cliente);
        return 0;
    }

    enlazarCliente(head, nodo);
    return 1;
}
/*tiene anterior era circular simple*/
int eliminarCliente(struct NodoCliente **head, int rut){
    struct NodoCliente *act, *ant; /* actual y anterior */
    int i;

    if(head == NULL || *head==NULL){
        return 0;
    }
    ant=*head;
    while(ant->sig!=*head){
        ant=ant->sig;
    }
    act=*head;
    do /* buscar el nodo a eliminar */{
        if(act->dato->rut==rut)
        {
            break;
        }
        ant=act;
        act=act->sig;
    }while(act!=*head);

    if(act->dato->rut!=rut)
    {
        return 0; /* no se encontro */
    }

    if(act->sig==act) /* por si es el unico nodo -> lista queda vacia */
    {
        *head=NULL;
    }
    else
    {
        ant->sig=act->sig;

        if(act==*head) /* si eliminado era head, el head pasa al siguiente */
        {
            *head=act->sig;
        }
    }

    if (act->dato->pases != NULL) {
        for (i = 0; i < MAXPASES; i++) {
            liberarPase(act->dato->pases[i]);
        }
    }

    free(act->dato->nombre);
    free(act->dato->pases);
    free(act->dato);
    free(act);
    return 1;
}

int modificarCliente(struct NodoCliente *head, int rut, char *nuevoNombre, int nuevoRut) /* 1: modificado, 0: no encontrado o nombre invalido, -1: RUT duplicado, -2: memoria insuficiente */
{
    struct NodoCliente *nodo;
    int tam;
    char *copia;

    if (nuevoNombre == NULL) {
        return 0;
    }

    nodo=buscarCliente(head, rut);
    if(nodo==NULL)
    {
        return 0;
    }

    if(nuevoRut!=rut && buscarCliente(head, nuevoRut)!=NULL) /* rut no puede pertenecer a otro cliente */
    {
        return -1;
    }

    /* reservar memoria para nombre y copiar */
    tam = strlen(nuevoNombre);
    copia = malloc((tam + 1) * sizeof(char));
    if (copia == NULL) {
        return -2;
    }
    strcpy(copia, nuevoNombre);
    free(nodo->dato->nombre);
    nodo->dato->nombre = copia;
    nodo->dato->rut=nuevoRut;

    return 1;
}

void listarClientes(struct NodoCliente *head) /* mostrar clientes */
{
    struct NodoCliente *rec=head;
    int cantidad=0;

    if(rec==NULL)
    {
        printf("No hay clientes registrados.\n");
    }
    else
    {
        do
        {
            cantidad++;
            printf("%d) Nombre: %s, RUT: %d, Pases: %d\n",
                   cantidad, rec->dato->nombre, rec->dato->rut,
                   rec->dato->plibrepases);
            rec=rec->sig;
        }while(rec!=head);
    }
}
