#ifndef FILA_INTERNO_H
#define FILA_INTERNO_H

#include "fila.h"

int soloDigitos(char *texto, int inicio, int fin);
int numeroDe(char *texto, int inicio, int fin);
int esBisiesto(int anio);
int validarHoraFila(char *hora, int *minutos);
int validarFechaFila(char *fecha);
int horaDentroHorarioTienda(struct Tienda *tienda, char *horaActual);
struct NodoSala *buscarSalaFila(struct Tienda *tienda, int codigo);
struct NodoOcupacion *buscarOcupacionFila(struct Tienda *tienda, int rut);
struct UsoPase *buscarUsoPaseFila(struct Tienda *tienda, struct Pase *pase);
int personaTieneOtroPaseFila(struct Tienda *tienda, int rut, struct Pase *pase);
int contarOcupacionSalaFila(struct Tienda *tienda, int codigoSala);
int salaDisponibleFila(struct Tienda *tienda, int codigoSala);
int elegirSalaFila(struct Tienda *tienda, int preferida, int *elegida);
int cantidadEnFila(struct Fila *fila);
void inicializarResultadoFila(struct ResultadoFila *resultado);
int validarSolicitudFila(struct Tienda *tienda, struct Cliente *cliente,
                         struct Pase *pase, char *horaActual);

#endif
