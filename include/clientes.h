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
/* El llamador debe retirar reservas que aun referencien al cliente. */
int eliminarCliente(struct NodoCliente **head, int rut);
/* Retorna -2 si no hay memoria suficiente para copiar el nombre. */
int modificarCliente(struct NodoCliente *head, int rut, char *nuevoNombre,
                     int nuevoRut);
void listarClientes(struct NodoCliente *head);

#endif
