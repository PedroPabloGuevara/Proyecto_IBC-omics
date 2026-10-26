#ifndef PASES_H
#define PASES_H

#include "estructuras.h"

struct Pase *crearPase(int tipoPase, int codigo, int valor, int estado,
                       char *fecha, char *hora);
struct NodoPase *crearNodoPase(struct Pase *pase);
struct pasesDelDia *crearPasesDelDia(void);
struct NodoPase *buscarPase(struct pasesDelDia *lista, int codigo);
int agregarPase(struct pasesDelDia *lista, struct Pase *pase);
struct Pase *quitarPase(struct pasesDelDia *lista, int codigo);
int eliminarPase(struct pasesDelDia *lista, int codigo);
void liberarPase(struct Pase *pase);
int modificarPase(struct pasesDelDia *lista, int codigo, int nuevoTipo,
                  int nuevoValor);
void listarPases(struct pasesDelDia *lista);
int contarPasesPorEstado(struct pasesDelDia *lista, int estado);
int paseValidoHoy(struct Pase *pase, char *fecha);
int venderPasesDelDia(struct Tienda *tienda, int rut, int codigoTipo,
                      int cantidad);

#endif
