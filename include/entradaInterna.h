#ifndef ENTRADA_INTERNA_H
#define ENTRADA_INTERNA_H

#include "entrada.h"

/* Validan fecha/hora y determinan si el pase pertenece al cliente indicado. */
int soloDigitos(char *texto, int inicio, int fin);
int numeroDe(char *texto, int inicio, int fin);
int esBisiesto(int anio);
int validarHoraFila(char *hora, int *minutos);
int validarFechaFila(char *fecha);
int horaDentroHorarioTienda(struct Tienda *tienda, char *horaActual);
struct Cliente *buscarClientePorPaseFila(struct Tienda *tienda,
                                         struct Pase *pase);
/* Funciones auxiliares para procesar solicitudes y sus resultados. */
int cantidadEnFila(struct Fila *fila);
void inicializarResultadoFila(struct ResultadoFila *resultado);
int validarSolicitudFila(struct Tienda *tienda, struct Cliente *cliente,
                         struct Pase *pase, char *horaActual);

#endif
