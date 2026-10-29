#include "../../include/estructuras.h"

#include <stdlib.h>
#include <string.h>

/*
 * Crea una sala y copia su tematica para que la estructura sea propietaria.
 * Retorna NULL si la tematica no existe o falla una reserva de memoria.
 */
struct SalaTematica *crearSala(char *tematica,int codigo,int aforoAprox){

    struct SalaTematica *nuevo;

    if (tematica == NULL || tematica[0] == '\0' || codigo <= 0 ||
        aforoAprox < 0) {
        return NULL;
    }

    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }
    /* Reserva el texto antes de publicar la sala al llamador. */
    nuevo->tematica = malloc(strlen(tematica) + 1);
    if (nuevo->tematica == NULL) {
        free(nuevo);
        return NULL;
    }
    strcpy(nuevo->tematica,tematica);
    /* Una sala comienza sin libros asociados. */
    nuevo->codigo = codigo;
    nuevo->aforoAprox = aforoAprox;
    nuevo->raiz = NULL;

    return nuevo;
}


/*
 * Crea un nodo desligado que referencia una sala existente.
 * Retorna NULL si la sala es NULL o no se puede reservar el nodo.
 */
struct NodoSala *crearNodoSala(struct SalaTematica *nuevaSala){

    struct NodoSala *nuevoNodo;

    if (nuevaSala == NULL) {
        return NULL;
    }

    nuevoNodo = malloc(sizeof(*nuevoNodo));
    if (nuevoNodo == NULL) {
        return NULL;
    }
    /* Los enlaces se inicializan vacios; insertarSala decide su posicion. */
    nuevoNodo->dato = nuevaSala;
    nuevoNodo->sig = NULL;
    nuevoNodo->ant = NULL;

    return nuevoNodo;
}

/*
 * Agrega el nodo al final de la lista doblemente enlazada de salas.
 * Si la lista o el nodo no son validos, deja la lista sin cambios.
 */
void insertarSala(struct NodoSala**headSalas,struct NodoSala *nuevaSala) {

    struct NodoSala *rec;

    if (headSalas == NULL || nuevaSala == NULL) {
        return;
    }

    /* Reinicia los enlaces para insertar un nodo desligado como nueva cola. */
    nuevaSala->sig = NULL;
    nuevaSala->ant = NULL;
    if (*headSalas == NULL) {
        *headSalas = nuevaSala;
    }else {
        /* Avanza hasta el ultimo nodo y conecta ambos sentidos de la lista. */
        rec = *headSalas;
        while (rec->sig != NULL) {
            rec = rec->sig;
        }
        rec->sig = nuevaSala;
        rec->sig->ant = rec;
    }
}
