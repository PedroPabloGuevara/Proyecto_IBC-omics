#include "../../include/clientes.h"
#include "../../include/entrada.h"
#include "../../include/entradaInterna.h"
#include "../../include/pases.h"
#include "../../include/reservas.h"

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

/*
-registra en un cliente un pase externo activo, sin venderlo ni sumar dinero
-valida su titularidad, codigo, fecha y horario antes de guardar el puntero
-si retorna 1, el cliente pasa a ser propietario del pase; si retorna 0, no
-retorna 1 si lo asocia o 0 si el pase, el cliente o los datos no son validos
*/
int registrarPaseCliente(struct Tienda *tienda, int rut, struct Pase *pase) {
    struct NodoCliente *nodoCliente;
    struct NodoCliente *iterCliente;
    struct NodoPase *nodoRegistro;
    int apertura;
    int cierre;
    int posicionLibre = -1;
    int cantidad = 0;
    int i;

    if (tienda == NULL || pase == NULL || pase->fecha == NULL ||
        pase->codigo <= 0 ||
        pase->tipoPase < 1 || pase->tipoPase > 4 || pase->estado != 1 ||
        !validarFechaFila(tienda->fecha) ||
        !paseValidoHoy(pase, tienda->fecha) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura) {
        return 0;
    }

    nodoCliente = buscarCliente(tienda->headClientes, rut);
    if (nodoCliente == NULL || nodoCliente->dato == NULL ||
        nodoCliente->dato->pases == NULL) {
        return 0;
    }

    /* No permite reutilizar un codigo ya ingresado durante la jornada. */
    nodoRegistro = buscarPase(tienda->filaPasesDeldia, pase->codigo);
    if (nodoRegistro != NULL) {
        return 0;
    }

    /* Busca una posicion libre y evita duplicar codigos entre clientes. */
    iterCliente = tienda->headClientes;
    do {
        if (iterCliente->dato != NULL && iterCliente->dato->pases != NULL) {
            for (i = 0; i < MAXPASES; i++) {
                struct Pase *existente = iterCliente->dato->pases[i];

                if (iterCliente == nodoCliente && existente == NULL &&
                    posicionLibre < 0) {
                    posicionLibre = i;
                }
                if (existente != NULL && existente->codigo == pase->codigo) {
                    return 0;
                }
                if (iterCliente == nodoCliente && existente != NULL) {
                    cantidad++;
                }
            }
        }
        iterCliente = iterCliente->sig;
    } while (iterCliente != NULL && iterCliente != tienda->headClientes);

    if (posicionLibre < 0) {
        return 0;
    }

    nodoCliente->dato->pases[posicionLibre] = pase;
    nodoCliente->dato->plibrepases = cantidad + 1;
    return 1;
}

/*
-libera las credenciales de todos los clientes y reinicia sus contadores
-conserva los clientes y sus demas datos
*/
void liberarPasesClientes(struct NodoCliente *head) {
    struct NodoCliente *actual = head;
    int i;

    if (head == NULL) {
        return;
    }

    do {
        if (actual->dato != NULL && actual->dato->pases != NULL) {
            for (i = 0; i < MAXPASES; i++) {
                liberarPase(actual->dato->pases[i]);
                actual->dato->pases[i] = NULL;
            }
            actual->dato->plibrepases = 0;
        }
        actual = actual->sig;
    } while (actual != NULL && actual != head);
}

/*
-elimina de cada libro las reservas que apuntan al rut indicado
-recorre recursivamente el arbol binario y libera solo reservas y nodos
-retorna la cantidad de reservas eliminadas
*/
int eliminarReservasClienteArbol(struct NodoLibro *raiz, int rut) {
    int eliminadas;

    if (raiz == NULL) {
        return 0;
    }

    eliminadas = eliminarReservasClienteArbol(raiz->izq, rut);
    eliminadas += eliminarReserva(raiz->dato, rut);
    eliminadas += eliminarReservasClienteArbol(raiz->der, rut);
    return eliminadas;
}

/*
-elimina un cliente de la tienda sin dejar referencias a sus datos liberados
-retira primero sus pases de la fila y sus reservas de todos los libros
-conserva los registros diarios de ingreso, que guardan copias independientes
-retorna 1 si se elimino el cliente y 0 si no existe o la tienda es NULL
*/
int eliminarCliente(struct Tienda *tienda, int rut) {
    struct NodoCliente *actual;
    struct NodoCliente *anterior;
    struct NodoSala *nodoSala;
    int i;

    if (tienda == NULL || tienda->headClientes == NULL) {
        return 0;
    }

    actual = buscarCliente(tienda->headClientes, rut);
    if (actual == NULL || actual->dato == NULL) {
        return 0;
    }

    /* Quita primero la fila, cuyos nodos referencian los pases del cliente. */
    if (tienda->filaEntrada != NULL && actual->dato->pases != NULL) {
        for (i = 0; i < MAXPASES; i++) {
            if (actual->dato->pases[i] != NULL) {
                quitarDeFila(tienda->filaEntrada, actual->dato->pases[i]->codigo);
            }
        }
    }

    /* Las reservas guardan un puntero al cliente y deben desaparecer primero. */
    for (nodoSala = tienda->headSalas; nodoSala != NULL;
         nodoSala = nodoSala->sig) {
        if (nodoSala->dato != NULL) {
            eliminarReservasClienteArbol(nodoSala->dato->raiz, rut);
        }
    }

    /* Desenlaza el cliente de la lista circular antes de liberar sus datos. */
    if (actual->sig == actual) {
        tienda->headClientes = NULL;
    } else {
        anterior = actual;
        while (anterior->sig != actual) {
            anterior = anterior->sig;
        }
        anterior->sig = actual->sig;
        if (tienda->headClientes == actual) {
            tienda->headClientes = actual->sig;
        }
    }

    /* El registro diario posee copias de los pases y permanece intacto. */
    if (actual->dato->pases != NULL) {
        for (i = 0; i < MAXPASES; i++) {
            liberarPase(actual->dato->pases[i]);
        }
    }
    free(actual->dato->nombre);
    free(actual->dato->pases);
    free(actual->dato);
    free(actual);
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
