#ifndef RESERVAS_H
#define RESERVAS_H

#include "estructuras.h"

struct Reserva *crearReserva(struct Cliente *reservador, int tipoPase);
struct NodoReserva *crearNodoReserva(struct Reserva *dato);
/*
 * El pase debe pertenecer al cliente y estar activo o utilizado; su tipo
 * determina prioridad. Cada cliente puede reservar una unidad por libro.
 */
int reservarLibro(struct Libro *libro, struct Cliente *cliente,
                  struct Pase *pase);
/* Retorna 1 si enlaza el nodo; si falla, el llamador conserva su propiedad. */
int encolar(struct Libro *libro, struct NodoReserva *nuevo);
/* Devuelve el nodo separado; quien llama debe liberar su reserva y el nodo. */
struct NodoReserva *desencolar(struct Libro *libro);
int cantidadEnEspera(struct Libro *libro);
void mostrarCola(struct Libro *libro);
struct NodoReserva *buscarReserva(struct Libro *libro, int rut);
/* Quita y libera el nodo y su reserva, pero no el cliente. */
int eliminarReserva(struct Libro *libro, int rut);
/* Cancela una reserva y devuelve al stock la unidad que habia apartado. */
int cancelarReserva(struct Libro *libro, int rut);
/* Cambia el pase de una reserva existente y la reubica segun su nueva prioridad. */
int modificarReserva(struct Libro *libro, struct Cliente *cliente,
                     struct Pase *nuevoPase);
/* Libera los nodos y reservas de la cola, sin liberar los clientes. */
void vaciarCola(struct Libro *libro);


#endif
