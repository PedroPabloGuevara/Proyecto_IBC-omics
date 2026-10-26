#include <stdio.h>
#include <stdlib.h>

#include "cola.h"

/*
 * Crea una cola vacia e inicializa sus extremos y contador.
 * Retorna NULL si no se puede reservar memoria.
 */
struct ColaReservas *crearCola(void) {
    struct ColaReservas *nuevaCola = malloc(sizeof(*nuevaCola));
    if (nuevaCola == NULL) {
        return NULL;
    }

    nuevaCola->head = NULL;
    nuevaCola->tail = NULL;
    nuevaCola->cantidad = 0;
    return nuevaCola;
}

/*
 * Crea una reserva asociada a un cliente existente; el cliente no se copia
 * ni pasa a ser propiedad de la reserva.
 */
struct Reserva *crearReserva(struct Cliente *reservador, int tipoPase) {
    struct Reserva *nuevaReserva;

    if (reservador == NULL) {
        return NULL;
    }

    nuevaReserva = malloc(sizeof(*nuevaReserva));
    if (nuevaReserva == NULL) {
        return NULL;
    }

    nuevaReserva->reservador = reservador;
    nuevaReserva->tipoPase = tipoPase;
    return nuevaReserva;
}

/*
 * Crea un nodo que contiene la reserva y comienza sin un nodo siguiente.
 */
struct NodoReserva *crearNodoReserva(struct Reserva *dato) {
    struct NodoReserva *nuevoNodo;

    if (dato == NULL) {
        return NULL;
    }

    nuevoNodo = malloc(sizeof(*nuevoNodo));
    if (nuevoNodo == NULL) {
        return NULL;
    }

    nuevoNodo->dato = dato;
    nuevoNodo->sig = NULL;
    return nuevoNodo;
}

/*
 * Inserta la reserva normal al final. Las prioritarias (tipo 4) se insertan
 * despues de las prioritarias existentes y antes de las normales.
 */
void encolar(struct ColaReservas *cola, struct NodoReserva *nuevo) {
    struct NodoReserva *rec;

    if (cola == NULL || nuevo == NULL || nuevo->dato == NULL) {
        return;
    }

    nuevo->sig = NULL;
    if (cola->head == NULL) {
        cola->head = nuevo;
        cola->tail = nuevo;
    } else if (nuevo->dato->tipoPase != 4) {
        cola->tail->sig = nuevo;
        cola->tail = nuevo;
    } else if (cola->head->dato->tipoPase != 4) {
        nuevo->sig = cola->head;
        cola->head = nuevo;
    } else {
        rec = cola->head;
        while (rec->sig != NULL && rec->sig->dato->tipoPase == 4) {
            rec = rec->sig;
        }

        nuevo->sig = rec->sig;
        rec->sig = nuevo;
        if (nuevo->sig == NULL) {
            cola->tail = nuevo;
        }
    }

    cola->cantidad++;
}

/*
 * Quita y retorna el primer nodo, actualizando head, tail y cantidad.
 * El nodo queda separado de la cola y debe ser liberado por quien lo recibe.
 */
struct NodoReserva *desencolar(struct ColaReservas *cola) {
    struct NodoReserva *aux;

    if (cola == NULL || cola->head == NULL) {
        return NULL;
    }

    aux = cola->head;
    cola->head = aux->sig;
    if (cola->head == NULL) {
        cola->tail = NULL;
    }

    aux->sig = NULL;
    cola->cantidad--;
    return aux;
}

/*
 * Retorna el numero de reservas en espera; para una cola NULL retorna cero.
 */
int cantidadEnEspera(struct ColaReservas *cola) {
    return cola == NULL ? 0 : cola->cantidad;
}

/*
 * Imprime las reservas en orden de atencion y marca las prioritarias.
 */
void mostrarCola(struct ColaReservas *cola) {
    struct NodoReserva *rec;
    int pos = 1;

    if (cola == NULL || cola->head == NULL) {
        printf("No hay clientes en lista de espera\n");
        return;
    }

    for (rec = cola->head; rec != NULL; rec = rec->sig) {
        printf("%d) %s", pos, rec->dato->reservador->nombre);
        if (rec->dato->tipoPase == 4) {
            printf(" (prioritario)");
        }
        printf("\n");
        pos++;
    }
}

/*
 * Libera cada nodo y su reserva, y al final la cola. No libera a los clientes,
 * que son propiedad de la lista de clientes.
 */
void liberarCola(struct ColaReservas *cola) {
    struct NodoReserva *rec;

    if (cola == NULL) {
        return;
    }

    rec = cola->head;
    while (rec != NULL) {
        struct NodoReserva *sig = rec->sig;
        free(rec->dato);
        free(rec);
        rec = sig;
    }

    free(cola);
}
