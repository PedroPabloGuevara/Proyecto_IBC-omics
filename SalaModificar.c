#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mostrarTodasSalas(struct NodoSala *headSalas){

    while (headSalas != NULL) {
        printf("nombre : %s aforo aproximado %d codigo de sala %d\n",headSalas->dato->tematica,headSalas->dato->aforoAprox,headSalas->dato->codigo);
        headSalas = headSalas->sig;
    }
}

int modificarSala(struct NodoSala **headSalas, int codigo, int nuevoCodigo,int nuevoAforo, char *nuevoNombre) {

    struct NodoSala *rec;
    char *copia;

    if (*headSalas == NULL) {
        return 0;
    }
    if (nuevoCodigo != codigo) {
        rec = buscarSala(*headSalas, nuevoCodigo);
        if (rec != NULL) {
            return 0; /* el nuevo codigo ya esta en uso */
        }
    }
    rec = buscarSala(*headSalas, codigo);
    if (rec == NULL) {
        return 0; /* la sala a modificar no existe */
    }
    copia = (char *)malloc(strlen(nuevoNombre) + 1);
    if (copia == NULL) {
        return 0; /* no se toco ningun campo */
    }
    strcpy(copia, nuevoNombre);
    rec->dato->tematica = copia;
    rec->dato->codigo = nuevoCodigo;
    rec->dato->aforoAprox = nuevoAforo;
    return 1;
}