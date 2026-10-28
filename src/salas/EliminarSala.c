#include "../../include/salas.h"

#include <stdlib.h>

/*
 * Busca una sala por codigo en la lista y retorna su nodo.
 * Retorna NULL si no existe coincidencia.
 */
struct NodoSala *buscarSala(struct NodoSala *headSalas, int codigo) {
    struct NodoSala *rec = headSalas;

    /* Recorre los enlaces hacia adelante sin cambiar la lista. */
    while (rec != NULL) {
        if (rec->dato->codigo == codigo) {
            return rec;
        }
        rec = rec->sig;
    }

    return NULL;
}

/*
 * Desenlaza una sala por codigo y devuelve el nodo separado de la lista.
 * No libera ni el nodo ni la sala; la funcion llamadora define su propiedad.
 */
struct NodoSala *desenlazarSala(struct NodoSala **headSalas, int codigo) {
    struct NodoSala *sala;

    if (headSalas == NULL) {
        return NULL;
    }

    /* Busca primero para no modificar la lista si el codigo no existe. */
    sala = buscarSala(*headSalas, codigo);
    if (sala == NULL) {
        return NULL;
    }

    /* Repara el enlace anterior y el siguiente, incluyendo los extremos. */
    if (sala->ant == NULL) {
        *headSalas = sala->sig;
    } else {
        sala->ant->sig = sala->sig;
    }

    if (sala->sig != NULL) {
        sala->sig->ant = sala->ant;
    }

    /* Deja el nodo aislado para que pueda liberarse o transferirse con seguridad. */
    sala->sig = NULL;
    sala->ant = NULL;
    return sala;
}

/*
-quita la sala de la lista y retorna 1 si se encontro
-retorna -1 si la lista o su puntero son NULL, y 0 si no existe el codigo
-libera el nodo de la lista, pero no la sala: quien la conserva debe liberarla
*/
int EliminarSala(struct NodoSala **headSalas, int codigo) {
    struct NodoSala *sala;

    if (headSalas == NULL || *headSalas == NULL) {
        return -1;
    }

    /* Separa el nodo y libera solo el envoltorio, no la sala que contiene. */
    sala = desenlazarSala(headSalas, codigo);
    if (sala == NULL) {
        return 0;
    }

    free(sala);
    return 1;
}

/*
-quita la sala de la lista y retorna la sala encontrada
-libera el nodo de la lista; quien recibe la sala queda a cargo de ella
-retorna NULL si la lista es NULL o no existe el codigo
*/
struct SalaTematica *QuitarSala(struct NodoSala **headSalas, int codigo) {
    struct NodoSala *nodoSala;
    struct SalaTematica *sala;

    /* Transfiere la propiedad de la sala al llamador y libera solo su nodo. */
    nodoSala = desenlazarSala(headSalas, codigo);
    if (nodoSala == NULL) {
        return NULL;
    }

    sala = nodoSala->dato;
    free(nodoSala);
    return sala;
}
