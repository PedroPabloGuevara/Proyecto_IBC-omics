#include "../../include/entradaInterna.h"

#include <stdlib.h>

/*
-valida la fecha y el horario de la tienda antes de configurar la fila
-crea la fila y la asigna a tienda->filaEntrada solo si aun no existe
-retorna 1 si se configuro correctamente y 0 ante datos invalidos o falta de memoria
*/
int configurarControlAcceso(struct Tienda *tienda) {
    struct Fila *fila;
    int apertura;
    int cierre;

    if (tienda == NULL || tienda->filaEntrada != NULL ||
        !validarFechaFila(tienda->fecha) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura) {
        return 0;
    }

    /* Crea la fila antes de cambiar la tienda para evitar una configuracion parcial. */
    fila = crearFila();
    if (fila == NULL) {
        return 0;
    }
    tienda->filaEntrada = fila;
    return 1;
}

/*
-libera la fila de entrada y sus nodos
-no libera los pases, que pertenecen a los clientes o a la lista diaria
-deja tienda->filaEntrada en NULL; no hace nada si tienda es NULL
*/
void liberarControlAcceso(struct Tienda *tienda) {
    if (tienda == NULL) {
        return;
    }

    /* La fila administra los nodos, pero no es propietaria de los pases. */
    liberarFila(tienda->filaEntrada);
    tienda->filaEntrada = NULL;
}
