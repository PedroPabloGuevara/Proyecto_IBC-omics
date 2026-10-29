#ifndef ENTRADA_H
#define ENTRADA_H

#include "estructuras.h"

struct ResultadoFila {
    int admitidos;
    int rechazados;
    int esperando;
};

/* Crea una fila de pases para una tienda con fecha y horario configurados. */
int configurarControlAcceso(struct Tienda *tienda);
/* Libera la fila y sus nodos, sin liberar los pases. */
void liberarControlAcceso(struct Tienda *tienda);
struct Fila *crearFila(void);
/* Inserta el nodo conservando prioridad de pases tipo 4 y FIFO entre iguales. */
int encolarFila(struct Fila *fila, struct NodoPase *nuevo);
/* Valida, encola y procesa solicitudes; las que aún no habilitan ingreso esperan. */
int solicitarIngreso(struct Tienda *tienda, struct Cliente *cliente,
                     struct Pase *pase, char *horaActual,
                     struct ResultadoFila *resultado);
/* Atiende los pases válidos; no lleva ocupación ni asignación de sala. */
int procesarFila(struct Tienda *tienda, char *horaActual,
                 struct ResultadoFila *resultado);
/* Desencolar devuelve el nodo separado; quien llama libera solo ese nodo. */
struct NodoPase *desencolarFila(struct Fila *fila);
/* Busca un nodo usando el codigo del pase; no modifica la fila. */
struct NodoPase *buscarEnFila(struct Fila *fila, int codigoPase);
/* Quita usando el codigo del pase y libera solo el nodo. */
int quitarDeFila(struct Fila *fila, int codigoPase);
/* Vaciar conserva la estructura; liberar tambien libera la estructura Fila. */
void vaciarFila(struct Fila *fila);
void liberarFila(struct Fila *fila);
/* Muestra la fila en orden de atencion; no la modifica. */
void mostrarFila(struct Fila *fila);

#endif
