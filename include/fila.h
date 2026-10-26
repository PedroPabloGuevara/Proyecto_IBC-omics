#ifndef FILA_H
#define FILA_H

#include "estructuras.h"

#define FILA_SALA_PRINCIPAL (-1)

struct ResultadoFila {
    int admitidos;
    int rechazados;
    int esperando;
};

/* Debe llamarse una vez con una Tienda inicializada y sin control de acceso previo. */
int configurarControlAcceso(struct Tienda *tienda, int aforoSalaPrincipal);
/* Libera la fila y los registros internos sin liberar clientes, pases ni salas. */
void liberarControlAcceso(struct Tienda *tienda);
struct Fila *crearFila(void);
struct NodoFila *crearNodoFila(struct Cliente *cliente, struct Pase *pase);
/* En éxito, la fila toma posesión del nodo, no del cliente ni del pase. */
int encolarFila(struct Fila *fila, struct NodoFila *nuevo);
/* Valida, encola y procesa inmediatamente las solicitudes que tengan espacio. */
int solicitarIngreso(struct Tienda *tienda, struct Cliente *cliente,
                     struct Pase *pase, int codigoSalaPreferida,
                     char *horaActual, struct ResultadoFila *resultado);
/* Atiende la fila hasta llenar la capacidad; actualiza admitidos/rechazados/esperando. */
int procesarFila(struct Tienda *tienda, char *horaActual,
                 struct ResultadoFila *resultado);
/* Registra la salida y procesa de inmediato la fila con el espacio liberado. */
int registrarSalidaTienda(struct Tienda *tienda, int rut, char *horaActual,
                          struct ResultadoFila *resultado);
/* Cambia de sala si el destino tiene espacio, sin variar el aforo total ocupado. */
int cambiarSalaTienda(struct Tienda *tienda, int rut, int codigoSalaDestino);
/* Retorna FILA_SALA_PRINCIPAL o el codigo tematico; -2 si la persona esta afuera. */
int obtenerSalaActual(struct Tienda *tienda, int rut);
int contarPersonasEnTienda(struct Tienda *tienda);
int calcularAforoTotal(struct Tienda *tienda);
/* Desencolar devuelve el nodo separado; quien llama debe liberarlo. */
struct NodoFila *desencolarFila(struct Fila *fila);
struct NodoFila *buscarEnFila(struct Fila *fila, int rut);
/* Quitar y vaciar liberan nodos, pero mantienen intactos cliente y pase. */
int quitarDeFila(struct Fila *fila, int rut);
void vaciarFila(struct Fila *fila);
void liberarFila(struct Fila *fila);

#endif
