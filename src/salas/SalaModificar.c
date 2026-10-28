#include "../../include/salas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
-muestra las salas con su tematica, aforo aproximado y codigo
*/
void mostrarTodasSalas(struct NodoSala *headSalas) {
    /* La lista se recorre sin alterar enlaces ni transferir propiedad. */
    while (headSalas != NULL) {
        printf("nombre : %s aforo aproximado %d codigo de sala %d\n",
               headSalas->dato->tematica, headSalas->dato->aforoAprox,
               headSalas->dato->codigo);
        headSalas = headSalas->sig;
    }
}

/*
-modifica el codigo, aforo y tematica de una sala
-retorna 1 si se modifico, o 0 si no existe, el codigo ya esta ocupado
 o no se pudo reservar memoria
*/
int modificarSala(struct NodoSala **headSalas, int codigo, int nuevoCodigo,
                  int nuevoAforo, char *nuevoNombre) {
    struct NodoSala *sala;
    struct NodoSala *salaConCodigo;
    char *copia;

    if (headSalas == NULL || nuevoNombre == NULL) {
        return 0;
    }

    /* Confirma que la sala exista y que el nuevo codigo no choque con otra. */
    sala = buscarSala(*headSalas, codigo);
    if (sala == NULL) {
        return 0;
    }

    salaConCodigo = buscarSala(*headSalas, nuevoCodigo);
    if (salaConCodigo != NULL && salaConCodigo != sala) {
        return 0;
    }

    /* Reserva y copia el nombre antes de liberar el texto vigente. */
    copia = malloc(strlen(nuevoNombre) + 1);
    if (copia == NULL) {
        return 0;
    }
    strcpy(copia, nuevoNombre);

    free(sala->dato->tematica);
    sala->dato->tematica = copia;
    sala->dato->codigo = nuevoCodigo;
    sala->dato->aforoAprox = nuevoAforo;
    return 1;
}
