#ifndef CLIENTES_H
#define CLIENTES_H

#include "estructuras.h"

/* Maximo de pases diarios almacenados por cliente. */
#define MAXPASES 50

struct Cliente *crearCliente(char *nombre, int rut);
struct NodoCliente *crearNodoCliente(struct Cliente *dato);
struct NodoCliente *buscarCliente(struct NodoCliente *head, int rut);
void enlazarCliente(struct NodoCliente **head, struct NodoCliente *nuevo);
int agregarCliente(struct NodoCliente **head, char *nombre, int rut);
/* Asocia un pase externo activo; solo transfiere su propiedad si retorna 1. */
int registrarPaseCliente(struct Tienda *tienda, int rut, struct Pase *pase);
/* Libera los pases diarios de todos los clientes sin eliminar a los clientes. */
void liberarPasesClientes(struct NodoCliente *head);
/* Elimina primero las referencias de fila y reservas, y luego sus pases. */
int eliminarCliente(struct Tienda *tienda, int rut);
int eliminarReservasClienteArbol(struct NodoLibro *raiz, int rut);
/* Retorna -2 si no hay memoria suficiente para copiar el nombre. */
int modificarCliente(struct NodoCliente *head, int rut, char *nuevoNombre,
                     int nuevoRut);
void listarClientes(struct NodoCliente *head);

#endif
