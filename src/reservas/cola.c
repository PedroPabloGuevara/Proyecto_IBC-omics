#include <stdio.h>
#include <stdlib.h>

#include "../../include/estructuras.h"

/*
-crea una cola de reservas vacia en memoria dinamica
-head y tail quedan en NULL y cantidad en 0
-si falla la reserva de memoria retorna NULL
-retorna el puntero a la cola creada
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
-crea una reserva en memoria dinamica
-guarda un puntero al cliente que reserva, no lo copia ni pasa a ser propietario de el
-guarda el tipo de pase: infantil (1), familiar (2), general (3), prioritario (4)
-si el cliente es NULL o falla la reserva de memoria retorna NULL
-retorna el puntero a la reserva creada
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
-envuelve una reserva en un nodo para poder meterla en la cola
-el nodo nace con sig en NULL
-si la reserva es NULL o falla la reserva de memoria retorna NULL
-retorna el puntero al nodo creado
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
-agrega una reserva a la cola segun su tipo de pase
-normal (tipo distinto de 4): va al final de la cola
-prioritario (tipo 4): va delante de los normales, pero detras de los prioritarios que ya estaban
-si la cola, el nodo o la reserva son NULL, no realiza cambios
-si el nodo queda como ultimo, se actualiza tail
*/
void encolar(struct ColaReservas *cola, struct NodoReserva *nuevo) {
    struct NodoReserva *rec;

    if (cola == NULL || nuevo == NULL || nuevo->dato == NULL) {
        return;
    }

    nuevo->sig = NULL;

    /* caso 1: cola vacia, el nuevo nodo queda como head y tail */
    if (cola->head == NULL) {
        cola->head = nuevo;
        cola->tail = nuevo;
    }
    /* caso 2: reserva normal, se agrega al final */
    else if (nuevo->dato->tipoPase != 4) {
        cola->tail->sig = nuevo;
        cola->tail = nuevo;
    }
    /* caso 3: prioritario y el primero es normal, pasa a ser el nuevo head */
    else if (cola->head->dato->tipoPase != 4) {
        nuevo->sig = cola->head;
        cola->head = nuevo;
    }
    /* caso 4: se inserta despues del ultimo prioritario */
    else {
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
-atiende al primero de la cola: lo desengancha y lo retorna
-si la cola esta vacia o es NULL, retorna NULL
-si la cola queda vacia, tail tambien pasa a NULL
-quien llama debe liberar el nodo y la reserva cuando termine de usarlos
*/
struct NodoReserva *desencolar(struct ColaReservas *cola) {
    struct NodoReserva *aux;

    if (cola == NULL || cola->head == NULL) {
        return NULL;
    }

    /* se guarda el primero y se mueve head al nodo siguiente */
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
-retorna cuantas personas esperan en la cola
-si la cola es NULL retorna 0
*/
int cantidadEnEspera(struct ColaReservas *cola) {
    return cola == NULL ? 0 : cola->cantidad;
}

/*
-muestra las reservas en orden de atencion
-imprime posicion, nombre del cliente y si es prioritaria
-si la cola esta vacia o es NULL, informa que no hay clientes esperando
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
-libera todos los nodos de la cola y sus reservas
-NO libera a los clientes, pertenecen a la lista de clientes
-si la cola es NULL, no realiza cambios
-al final libera la cola misma
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
