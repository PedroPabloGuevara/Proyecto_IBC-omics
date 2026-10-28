#include <stdio.h>
#include <stdlib.h>

#include "../../include/clientes.h"
#include "../../include/reservas.h"


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
-registra un ejemplar reservado para un cliente en un libro en preventa
-solo permite una reserva por cliente y libro; el pase asociado determina prioridad
-retorna 1 si se agrego y 0 si la reserva no fue valida o falto memoria
*/
int reservarLibro(struct Libro *libro, struct Cliente *cliente,
                  struct Pase *pase) {
    struct Reserva *reserva;
    struct NodoReserva *nodo;
    int esPaseDelCliente = 0;
    int i;

    if (libro == NULL || cliente == NULL || cliente->pases == NULL ||
        pase == NULL || libro->disponibilidad != 3 ||
        (pase->estado != 1 && pase->estado != 3) ||
        pase->tipoPase < 1 || pase->tipoPase > 4 ||
        buscarReserva(libro, cliente->rut) != NULL) {
        return 0;
    }

    for (i = 0; i < MAXPASES; i++) {
        if (cliente->pases[i] == pase) {
            esPaseDelCliente = 1;
            break;
        }
    }
    if (!esPaseDelCliente) {
        return 0;
    }

    reserva = crearReserva(cliente, pase->tipoPase);
    if (reserva == NULL) {
        return 0;
    }

    nodo = crearNodoReserva(reserva);
    if (nodo == NULL) {
        free(reserva);
        return 0;
    }

    if (!encolar(libro, nodo)) {
        free(nodo);
        free(reserva);
        return 0;
    }
    return 1;
}

/*
-agrega una reserva a la lista del libro segun su tipo de pase
-normal (tipo distinto de 4): va al final de la cola
-prioritario (tipo 4): va delante de los normales, pero detras de los prioritarios que ya estaban
-si el libro, el nodo o la reserva son NULL, no realiza cambios
-el nodo debe estar desligado de cualquier lista
*/
int encolar(struct Libro *libro, struct NodoReserva *nuevo) {
    struct NodoReserva *rec;

    if (libro == NULL || nuevo == NULL || nuevo->dato == NULL ||
        nuevo->dato->reservador == NULL || libro->disponibilidad != 3 ||
        nuevo->dato->tipoPase < 1 || nuevo->dato->tipoPase > 4 ||
        nuevo->sig != NULL) {
        return 0;
    }

    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        if (rec == nuevo ||
            (rec->dato != NULL && rec->dato->reservador != NULL &&
             rec->dato->reservador->rut == nuevo->dato->reservador->rut)) {
            return 0;
        }
    }

    /* caso 1: lista vacia, el nuevo nodo queda como head */
    if (libro->headReserva == NULL) {
        libro->headReserva = nuevo;
    }
    /* caso 2: reserva normal, se agrega al final */
    else if (nuevo->dato->tipoPase != 4) {
        rec = libro->headReserva;
        while (rec->sig != NULL) {
            rec = rec->sig;
        }
        rec->sig = nuevo;
    }
    /* caso 3: prioritario y el primero es normal, pasa a ser el nuevo head */
    else if (libro->headReserva->dato->tipoPase != 4) {
        nuevo->sig = libro->headReserva;
        libro->headReserva = nuevo;
    }
    /* caso 4: se inserta despues del ultimo prioritario */
    else {
        rec = libro->headReserva;
        while (rec->sig != NULL && rec->sig->dato->tipoPase == 4) {
            rec = rec->sig;
        }

        nuevo->sig = rec->sig;
        rec->sig = nuevo;
    }
    return 1;
}

/*
-atiende al primero de la cola: lo desengancha y lo retorna
-si el libro es NULL o no tiene reservas, retorna NULL
-quien llama debe liberar el nodo y la reserva cuando termine de usarlos
*/
struct NodoReserva *desencolar(struct Libro *libro) {
    struct NodoReserva *aux;

    if (libro == NULL || libro->headReserva == NULL) {
        return NULL;
    }

    /* se guarda el primero y se mueve head al nodo siguiente */
    aux = libro->headReserva;
    libro->headReserva = aux->sig;

    aux->sig = NULL;
    return aux;
}

/*
-retorna cuantas personas esperan en la cola
-si el libro es NULL retorna 0
*/
int cantidadEnEspera(struct Libro *libro) {
    struct NodoReserva *rec;
    int cantidad = 0;

    if (libro == NULL) {
        return 0;
    }

    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        cantidad++;
    }

    return cantidad;
}

/*
-muestra las reservas en orden de atencion
-imprime posicion, nombre del cliente y si es prioritaria
-si el libro no tiene reservas, informa que no hay clientes esperando
*/
void mostrarCola(struct Libro *libro) {
    struct NodoReserva *rec;
    int pos = 1;

    if (libro == NULL || libro->headReserva == NULL) {
        printf("No hay clientes en lista de espera\n");
        return;
    }

    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        printf("%d) %s", pos, rec->dato->reservador->nombre);
        if (rec->dato->tipoPase == 4) {
            printf(" (prioritario)");
        }
        printf("\n");
        pos++;
    }
}

/*
-busca la reserva de un cliente en el libro por su rut
-retorna el nodo si lo encuentra, o NULL si no esta
*/
struct NodoReserva *buscarReserva(struct Libro *libro, int rut) {
    struct NodoReserva *rec;

    if (libro == NULL) {
        return NULL;
    }

    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        if (rec->dato != NULL && rec->dato->reservador != NULL &&
            rec->dato->reservador->rut == rut) {
            return rec;
        }
    }

    return NULL;
}

/*
-elimina de la lista del libro la reserva de un cliente, este donde este
-libera el nodo y la reserva, pero NO al cliente
-retorna 1 si la elimino, 0 si no estaba o el libro es NULL
*/
int eliminarReserva(struct Libro *libro, int rut) {
    struct NodoReserva *rec;
    struct NodoReserva *ant = NULL;

    if (libro == NULL) {
        return 0;
    }

    rec = libro->headReserva;
    while (rec != NULL &&
        (rec->dato == NULL || rec->dato->reservador == NULL || rec->dato->reservador->rut != rut)) {
        ant = rec;
        rec = rec->sig;
    }

    if (rec == NULL) {
        return 0;
    }

    if (ant == NULL) {
        libro->headReserva = rec->sig;
    } else {
        ant->sig = rec->sig;
    }

    free(rec->dato);
    free(rec);
    return 1;
}

/*
-deja vacia la lista del libro liberando todos sus nodos y reservas
-el libro queda disponible para recibir nuevas reservas
*/
void vaciarCola(struct Libro *libro) {
    struct NodoReserva *rec;
    struct NodoReserva *sig;

    if (libro == NULL) {
        return;
    }

    rec = libro->headReserva;
    while (rec != NULL) {
        sig = rec->sig;
        free(rec->dato);
        free(rec);
        rec = sig;
    }

    libro->headReserva = NULL;
}
