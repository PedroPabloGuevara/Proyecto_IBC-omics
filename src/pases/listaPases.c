#include "../../include/pases.h"

#include <stdio.h>
#include <stdlib.h>

struct ListaPases *crearListaPases(void) {
    struct ListaPases *lista = malloc(sizeof(*lista));

    if (lista != NULL) {
        lista->head = NULL;
        lista->tail = NULL;
    }
    return lista;
}

struct NodoPase *buscarEnListaPases(struct ListaPases *lista, int codigo) {
    struct NodoPase *actual;

    if (lista == NULL) {
        return NULL;
    }
    for (actual = lista->head; actual != NULL; actual = actual->sig) {
        if (actual->paseVigente != NULL &&
            actual->paseVigente->codigo == codigo) {
            return actual;
        }
    }
    return NULL;
}

int agregarPaseALista(struct ListaPases *lista, struct Pase *pase) {
    struct NodoPase *nuevo;

    if (lista == NULL || pase == NULL || pase->codigo <= 0 ||
        pase->tipoPase < 1 || pase->tipoPase > 4 ||
        pase->estado < 1 || pase->estado > 4 || pase->valor < 0 ||
        pase->fecha == NULL || pase->hora == NULL ||
        buscarEnListaPases(lista, pase->codigo) != NULL) {
        return 0;
    }
    nuevo = crearNodoPase(pase);
    if (nuevo == NULL) {
        return 0;
    }
    if (lista->tail == NULL) {
        lista->head = nuevo;
    } else {
        lista->tail->sig = nuevo;
    }
    lista->tail = nuevo;
    return 1;
}

int modificarPaseEnLista(struct ListaPases *lista, int codigo,
                         int nuevoTipo, int nuevoValor) {
    struct NodoPase *nodo = buscarEnListaPases(lista, codigo);

    if (nodo == NULL) {
        return 0;
    }
    return modificarPase(nodo->paseVigente, nuevoTipo, nuevoValor);
}

int cambiarEstadoPaseEnLista(struct ListaPases *lista, int codigo,
                             int nuevoEstado) {
    struct NodoPase *nodo = buscarEnListaPases(lista, codigo);

    if (nodo == NULL) {
        return 0;
    }
    if (nuevoEstado < 1 || nuevoEstado > 4) {
        return -1;
    }
    nodo->paseVigente->estado = nuevoEstado;
    return 1;
}

int eliminarPaseDeLista(struct ListaPases *lista, int codigo) {
    struct NodoPase *actual;
    struct NodoPase *anterior = NULL;

    if (lista == NULL) {
        return 0;
    }
    actual = lista->head;
    while (actual != NULL &&
           (actual->paseVigente == NULL ||
            actual->paseVigente->codigo != codigo)) {
        anterior = actual;
        actual = actual->sig;
    }
    if (actual == NULL) {
        return 0;
    }
    if (anterior == NULL) {
        lista->head = actual->sig;
    } else {
        anterior->sig = actual->sig;
    }
    if (lista->tail == actual) {
        lista->tail = anterior;
    }
    free(actual);
    return 1;
}

int cantidadPasesEnLista(struct ListaPases *lista) {
    struct NodoPase *actual;
    int cantidad = 0;

    if (lista == NULL) {
        return 0;
    }
    for (actual = lista->head; actual != NULL; actual = actual->sig) {
        cantidad++;
    }
    return cantidad;
}

void listarPasesEnLista(struct ListaPases *lista) {
    struct NodoPase *actual;

    if (lista == NULL || lista->head == NULL) {
        printf("No hay pases registrados\n");
        return;
    }
    for (actual = lista->head; actual != NULL; actual = actual->sig) {
        if (actual->paseVigente != NULL) {
            struct Pase *pase = actual->paseVigente;

            printf("Codigo: %d, tipo: %d, valor: %d, estado: %d, fecha: %s, hora: %s\n",
                   pase->codigo, pase->tipoPase, pase->valor, pase->estado,
                   pase->fecha, pase->hora);
        }
    }
}

void vaciarListaPases(struct ListaPases *lista) {
    struct NodoPase *actual;

    if (lista == NULL) {
        return;
    }
    actual = lista->head;
    while (actual != NULL) {
        struct NodoPase *siguiente = actual->sig;

        free(actual);
        actual = siguiente;
    }
    lista->head = NULL;
    lista->tail = NULL;
}

void liberarListaPases(struct ListaPases *lista) {
    if (lista == NULL) {
        return;
    }
    vaciarListaPases(lista);
    free(lista);
}
