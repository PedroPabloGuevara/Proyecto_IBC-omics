#ifndef PASES_H
#define PASES_H

#include "estructuras.h"

struct Pase *crearPase(int tipoPase, int codigo, int valor, int estado,
                       char *fecha, char *hora);
struct NodoPase *crearNodoPase(struct Pase *pase);
/* La lista enlaza referencias; las operaciones sobre nodos no liberan pases. */
struct ListaPases *crearListaPases(void);
struct NodoPase *buscarEnListaPases(struct ListaPases *lista, int codigo);
int agregarPaseALista(struct ListaPases *lista, struct Pase *pase);
int modificarPaseEnLista(struct ListaPases *lista, int codigo,
                         int nuevoTipo, int nuevoValor);
int cambiarEstadoPaseEnLista(struct ListaPases *lista, int codigo,
                             int nuevoEstado);
int eliminarPaseDeLista(struct ListaPases *lista, int codigo);
int cantidadPasesEnLista(struct ListaPases *lista);
void listarPasesEnLista(struct ListaPases *lista);
void vaciarListaPases(struct ListaPases *lista);
void liberarListaPases(struct ListaPases *lista);
struct pasesDelDia *crearPasesDelDia(void);
struct NodoPase *buscarPase(struct pasesDelDia *lista, int codigo);
struct NodoPase *buscarPaseDelDia(struct pasesDelDia *lista, int codigo);
int agregarPase(struct pasesDelDia *lista, struct Pase *pase);
int agregarPaseDelDia(struct pasesDelDia *lista, struct Pase *pase);
int modificarPase(struct Pase *pase, int nuevoTipo, int nuevoValor);
int modificarPaseDelDia(struct pasesDelDia *lista, int codigo,
                         int nuevoTipo, int nuevoValor);
int eliminarPaseDelDia(struct pasesDelDia *lista, int codigo);
void liberarPase(struct Pase *pase);
void listarPases(struct pasesDelDia *lista);
void listarPasesDelDia(struct pasesDelDia *lista);
int contarPasesPorEstado(struct pasesDelDia *lista, int estado);
int paseValidoHoy(struct Pase *pase, char *fecha);
/* Libera los pases y nodos que componen el registro diario de ingresos. */
void liberarPasesDelDia(struct pasesDelDia *lista);

#endif
