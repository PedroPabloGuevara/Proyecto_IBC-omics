#include "../../include/entrada.h"

#include <stdio.h>
#include <stdlib.h>

/*
-crea una fila vacia reservando memoria dinamica
-inicializa head y tail en NULL
-retorna la fila creada o NULL si no hay memoria disponible
*/
struct Fila *crearFila(void) {
    struct Fila *fila = malloc(sizeof(*fila));

    if (fila == NULL) {
        return NULL;
    }

    /* Una fila recien creada no tiene nodos enlazados. */
    fila->head = NULL;
    fila->tail = NULL;
    return fila;
}

/*
-agrega un nodo de pase a la fila
-rechaza pases invalidos, repetidos o nodos que ya esten enlazados
-inserta los pases de tipo 4 antes de los demas, conservando el orden entre prioritarios;
los otros tipos se agregan al final
-retorna 1 si el nodo se agrego y 0 si no se pudo agregar
-si tiene exito, la fila pasa a ser propietaria del nodo; el pase sigue siendo externo
*/
int encolarFila(struct Fila *fila, struct NodoPase *nuevo) {
    struct NodoPase *rec;
    struct NodoPase *anterior;

    if (fila == NULL || nuevo == NULL || nuevo->paseVigente == NULL ||
        nuevo->sig != NULL || nuevo->paseVigente->tipoPase < 1 ||
        nuevo->paseVigente->tipoPase > 4) {
        return 0;
    }

    /* Rechaza referencias o codigos de pase que ya esten esperando. */
    anterior = NULL;
    for (rec = fila->head; rec != NULL; rec = rec->sig) {
        if (rec == nuevo || rec->paseVigente == NULL ||
            rec->paseVigente == nuevo->paseVigente ||
            rec->paseVigente->codigo == nuevo->paseVigente->codigo) {
            return 0;
        }
        anterior = rec;
    }

    /* Comprueba que head y tail coincidan con los enlaces existentes. */
    if ((fila->head == NULL && fila->tail != NULL) ||
        (fila->head != NULL && anterior != fila->tail)) {
        return 0;
    }

    /* En una fila vacia, el nodo nuevo ocupa ambos extremos. */
    if (fila->head == NULL) {
        fila->head = nuevo;
        fila->tail = nuevo;
    } else if (nuevo->paseVigente->tipoPase != 4) {
        /* Los pases no prioritarios se agregan al final en orden de llegada. */
        fila->tail->sig = nuevo;
        fila->tail = nuevo;
    } else {
        /* Busca el final del grupo prioritario para mantener su orden FIFO. */
        rec = fila->head;
        anterior = NULL;
        while (rec != NULL && rec->paseVigente->tipoPase == 4) {
            anterior = rec;
            rec = rec->sig;
        }

        nuevo->sig = rec;
        if (anterior == NULL) {
            fila->head = nuevo;
        } else {
            anterior->sig = nuevo;
        }
        if (nuevo->sig == NULL) {
            fila->tail = nuevo;
        }
    }

    return 1;
}

/*
-retira el primer nodo de la fila y lo separa de la lista
-actualiza head y tail; ambos quedan en NULL si se retira el ultimo nodo
-retorna el nodo retirado, o NULL si la fila es NULL o esta vacia
-quien llama decide que hace con el nodo retirado; el pase guardado no se toca
*/
struct NodoPase *desencolarFila(struct Fila *fila) {
    struct NodoPase *primero;

    if (fila == NULL || fila->head == NULL) {
        return NULL;
    }

    /* Avanza head y conserva el nodo retirado para devolverlo al llamador. */
    primero = fila->head;
    fila->head = primero->sig;
    if (fila->head == NULL) {
        fila->tail = NULL;
    }
    primero->sig = NULL;
    return primero;
}

/*
-busca en la fila un pase por su codigo
-recorre desde head hasta encontrar el codigo o llegar al final
-retorna el nodo encontrado o NULL si la fila es NULL o no existe ese codigo
-no modifica la fila ni libera memoria
*/
struct NodoPase *buscarEnFila(struct Fila *fila, int codigoPase) {
    struct NodoPase *rec;

    if (fila == NULL) {
        return NULL;
    }

    /* Examina cada pase enlazado y compara su codigo. */
    for (rec = fila->head; rec != NULL; rec = rec->sig) {
        if (rec->paseVigente != NULL &&
            rec->paseVigente->codigo == codigoPase) {
            return rec;
        }
    }
    return NULL;
}

/*
-busca y quita de la fila el pase con el codigo indicado
-actualiza head o tail cuando elimina un extremo
-libera el nodo, pero no el pase al que apunta
-retorna 1 si lo quito y 0 si la fila es NULL o no encontro el codigo
*/
int quitarDeFila(struct Fila *fila, int codigoPase) {
    struct NodoPase *rec;
    struct NodoPase *anterior = NULL;

    if (fila == NULL) {
        return 0;
    }

    /* Conserva anterior para reconectar la lista al encontrar el pase. */
    rec = fila->head;
    while (rec != NULL && (rec->paseVigente == NULL || rec->paseVigente->codigo != codigoPase)) {
        anterior = rec;
        rec = rec->sig;
    }
    if (rec == NULL) {
        return 0;
    }

    /* Actualiza el extremo inicial o el enlace del nodo anterior. */
    if (anterior == NULL) {
        fila->head = rec->sig;
    } else {
        anterior->sig = rec->sig;
    }
    if (fila->tail == rec) {
        fila->tail = anterior;
    }
    /* La fila es propietaria del nodo, pero no del pase referenciado. */
    free(rec);
    return 1;
}

/*
-libera todos los nodos de la fila
-deja head y tail en NULL para que la fila pueda reutilizarse
-no libera los pases apuntados por los nodos
-si fila es NULL, no realiza cambios
*/
void vaciarFila(struct Fila *fila) {
    struct NodoPase *rec;
    struct NodoPase *siguiente;

    if (fila == NULL) {
        return;
    }

    /* Guarda el siguiente enlace antes de liberar cada nodo. */
    for (rec = fila->head; rec != NULL; rec = siguiente) {
        siguiente = rec->sig;
        free(rec);
    }
    fila->head = NULL;
    fila->tail = NULL;
}

/*
-libera los nodos y la estructura de la fila
-no toca los pases apuntados por los nodos
-si fila es NULL, no realiza ninguna accion
*/
void liberarFila(struct Fila *fila) {
    if (fila != NULL) {
        vaciarFila(fila);
        free(fila);
    }
}

/*
-muestra los pases de la fila en el orden de atencion
-no modifica la fila ni sus nodos
*/
void mostrarFila(struct Fila *fila) {
    struct NodoPase *rec;

    if (fila == NULL || fila->head == NULL) {
        printf("La fila esta vacia.\n");
        return;
    }

    for (rec = fila->head; rec != NULL; rec = rec->sig) {
        if (rec->paseVigente != NULL) {
            printf("Codigo: %d, Tipo: %d, Estado: %d\n",
                   rec->paseVigente->codigo, rec->paseVigente->tipoPase,
                   rec->paseVigente->estado);
        }
    }
}