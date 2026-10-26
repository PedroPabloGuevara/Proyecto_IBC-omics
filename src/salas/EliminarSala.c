#include "../../include/salas.h"

#include <stdlib.h>

struct NodoSala *buscarSala(struct NodoSala *headSalas, int codigo) {
    struct NodoSala *rec = headSalas;

    while (rec != NULL) {
        if (rec->dato->codigo == codigo) {
            return rec;
        }
        rec = rec->sig;
    }

    return NULL;
}

struct NodoSala *desenlazarSala(struct NodoSala **headSalas, int codigo) {
    struct NodoSala *sala;

    if (headSalas == NULL) {
        return NULL;
    }

    sala = buscarSala(*headSalas, codigo);
    if (sala == NULL) {
        return NULL;
    }

    if (sala->ant == NULL) {
        *headSalas = sala->sig;
    } else {
        sala->ant->sig = sala->sig;
    }

    if (sala->sig != NULL) {
        sala->sig->ant = sala->ant;
    }

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

    nodoSala = desenlazarSala(headSalas, codigo);
    if (nodoSala == NULL) {
        return NULL;
    }

    sala = nodoSala->dato;
    free(nodoSala);
    return sala;
}
