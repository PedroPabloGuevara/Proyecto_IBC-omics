#ifndef RESERVAS_H
#define RESERVAS_H

#include "estructuras.h"

struct Reserva *crearReserva(struct Cliente *reservador, int tipoPase);
struct NodoReserva *crearNodoReserva(struct Reserva *dato);
int reservarLibro(struct Libro *libro, struct Cliente *cliente, int tipoPase);
void encolar(struct Libro *libro, struct NodoReserva *nuevo);
struct NodoReserva *desencolar(struct Libro *libro);
int cantidadEnEspera(struct Libro *libro);
void mostrarCola(struct Libro *libro);
struct NodoReserva *buscarReserva(struct Libro *libro, int rut);
int eliminarReserva(struct Libro *libro, int rut);
void vaciarCola(struct Libro *libro);

#endif
