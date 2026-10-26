#include "../../include/estructuras.h"

#include <stdlib.h>
#include <string.h>

struct SalaTematica *crearSala(char *tematica,int codigo,int aforoAprox){

    struct SalaTematica *nuevo;

    if (tematica == NULL) {
        return NULL;
    }

    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->tematica = malloc(strlen(tematica) + 1);
    if (nuevo->tematica == NULL) {
        free(nuevo);
        return NULL;
    }
    strcpy(nuevo->tematica,tematica);
    nuevo->codigo = codigo;
    nuevo->aforoAprox = aforoAprox;
    nuevo->raiz = NULL;

    return nuevo;
}


struct NodoSala *crearNodoSala(struct SalaTematica *nuevaSala){

    struct NodoSala *nuevoNodo;

    if (nuevaSala == NULL) {
        return NULL;
    }

    nuevoNodo = malloc(sizeof(*nuevoNodo));
    if (nuevoNodo == NULL) {
        return NULL;
    }
    nuevoNodo->dato = nuevaSala;
    nuevoNodo->sig = NULL;
    nuevoNodo->ant = NULL;

    return nuevoNodo;
}

void insertarSala(struct NodoSala**headSalas,struct NodoSala *nuevaSala) {

    struct NodoSala *rec;

    if (headSalas == NULL || nuevaSala == NULL) {
        return;
    }

    nuevaSala->sig = NULL;
    nuevaSala->ant = NULL;
    if (*headSalas == NULL) {
        *headSalas = nuevaSala;
    }else {
        rec = *headSalas;
        while (rec->sig != NULL) {
            rec = rec->sig;
        }
        rec->sig = nuevaSala;
        rec->sig->ant = rec;
    }
}
