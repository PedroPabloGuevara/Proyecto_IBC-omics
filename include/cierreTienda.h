#ifndef CIERRE_TIENDA_H
#define CIERRE_TIENDA_H

#include "estructuras.h"

/*
 * Finaliza la jornada liberando la fila de ingreso y los pases del dia.
 * Conserva clientes, salas, libros, reservas y recaudacion.
 * La recaudacion tambien permanece en tienda->recaudacion.
 * Retorna 1 si se completo el cierre y 0 si los argumentos son invalidos.
 */
int cerrarTienda(struct Tienda *tienda, int *recaudacionFinal);

#endif
