#ifndef COLA_H
#define COLA_H

#include "estructuras.h"

struct ColaReservas *crearCola(void);
struct Reserva *crearReserva(struct Cliente *reservador, int tipoPase);
struct NodoReserva *crearNodoReserva(struct Reserva *dato);
void encolar(struct ColaReservas *cola, struct NodoReserva *nuevo);
struct NodoReserva *desencolar(struct ColaReservas *cola);
int cantidadEnEspera(struct ColaReservas *cola);
void mostrarCola(struct ColaReservas *cola);
void liberarCola(struct ColaReservas *cola);

#endif
