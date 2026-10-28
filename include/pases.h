#ifndef PASES_H
#define PASES_H

#include "estructuras.h"

struct Pase *crearPase(int tipoPase, int codigo, int valor, int estado,
                       char *fecha, char *hora);
struct NodoPase *crearNodoPase(struct Pase *pase);
struct pasesDelDia *crearPasesDelDia(void);
struct NodoPase *buscarPase(struct pasesDelDia *lista, int codigo);
int agregarPase(struct pasesDelDia *lista, struct Pase *pase);
void liberarPase(struct Pase *pase);
void listarPases(struct pasesDelDia *lista);
int contarPasesPorEstado(struct pasesDelDia *lista, int estado);
int paseValidoHoy(struct Pase *pase, char *fecha);
/* Libera los pases y nodos que componen el registro diario de ingresos. */
void liberarPasesDelDia(struct pasesDelDia *lista);

#endif
