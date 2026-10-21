#include <stdio.h>
#include <stdlib.h>
#include <string.h>





 struct NodoSala *buscarSala(struct NodoSala*headSalas,int codigo) {
    struct NodoSala *rec;
    if (headSalas == NULL)
        return NULL;
    rec = headSalas;
    while (rec != NULL) {
        if (rec->dato->codigo == codigo) {
            return rec;
        }
        rec = rec->sig;
    }
    return NULL;
}




int EliminarSala(struct NodoSala **headSalas,int codigo) {
    struct NodoSala *rec;

    if (*headSalas == NULL)
        return -1;
    rec = *headSalas;
    if (rec->dato->codigo == codigo) {
        *headSalas= rec->sig;
        (*headSalas)->ant = NULL;
        return 1;
    }else {
        while (rec->sig != NULL){
            if (rec->sig->dato->codigo == codigo) {
                if (rec->sig->sig != NULL) {
                    rec->sig->sig->ant=rec;
                }
                rec->sig = rec->sig->sig;
                return 1;
            }
            rec=rec->sig;
        }
    }
    return 0;
}

struct SalaTematica *QuitarSala(struct NodoSala **headSalas,int codigo) {
    struct NodoSala *rec;
    struct SalaTematica *quitada;

    if (*headSalas == NULL)
        return NULL;
    rec = *headSalas;
    if (rec->dato->codigo == codigo) {
        quitada =rec->dato;
        *headSalas= rec->sig;
        (*headSalas)->ant = NULL;
        return quitada;
    }else {
        while (rec->sig != NULL){
            if (rec->sig->dato->codigo == codigo) {
                quitada = rec->dato;
                if (rec->sig->sig != NULL) {
                    rec->sig->sig->ant=rec;
                }
                rec->sig = rec->sig->sig;
                return quitada;
            }
            rec=rec->sig;
        }
    }
    return NULL;

}