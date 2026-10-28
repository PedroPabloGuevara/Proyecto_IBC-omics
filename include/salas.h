#ifndef SALAS_H
#define SALAS_H

#include "estructuras.h"

struct SalaTematica *crearSala(char *tematica, int codigo, int aforoAprox);
struct NodoSala *crearNodoSala(struct SalaTematica *nuevaSala);
void insertarSala(struct NodoSala **headSalas, struct NodoSala *nuevaSala);
struct NodoSala *buscarSala(struct NodoSala *headSalas, int codigo);
int EliminarSala(struct NodoSala **headSalas, int codigo);
struct SalaTematica *QuitarSala(struct NodoSala **headSalas, int codigo);
void mostrarTodasSalas(struct NodoSala *headSalas);
int modificarSala(struct NodoSala **headSalas, int codigo, int nuevoCodigo,
                  int nuevoAforo, char *nuevoNombre);

#endif
