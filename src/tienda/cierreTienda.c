#include "../../include/cierreTienda.h"
#include "../../include/clientes.h"
#include "../../include/entrada.h"
#include "../../include/libros.h"
#include "../../include/pases.h"

#include <stdlib.h>

/*
-libera los datos operativos de la jornada al cerrar la tienda
-vacia la fila, libros y reservas, clientes y pases del catalogo diario
-conserva la recaudacion en Tienda y la copia al parametro de salida
-retorna 1 si se completo el cierre y 0 si algun argumento es NULL
*/
int cerrarTienda(struct Tienda *tienda, int *recaudacionFinal) {
    struct NodoSala *nodoSala;
    struct NodoSala *siguienteSala;
    int recaudacion;

    if (tienda == NULL || recaudacionFinal == NULL) {
        return 0;
    }

    /* Guarda el total antes de liberar las estructuras del dia. */
    recaudacion = tienda->recaudacion;

    /* Los nodos de la fila apuntan a pases que pertenecen a los clientes. */
    liberarControlAcceso(tienda);

    /*
     * Libera libros antes que clientes porque las reservas guardan punteros
     * a sus reservadores; liberar cada libro tambien vacia sus reservas.
     */
    for (nodoSala = tienda->headSalas; nodoSala != NULL;
        nodoSala = siguienteSala) {
        siguienteSala = nodoSala->sig;

        /* Eliminar cada libro libera tambien su cola de reservas. */
        while (nodoSala->dato != NULL && nodoSala->dato->raiz != NULL) {
            int codigo = nodoSala->dato->raiz->dato->codigo;
            eliminarLibro(&nodoSala->dato->raiz, codigo);
        }

        if (nodoSala->dato != NULL) {
            free(nodoSala->dato->tematica);
            free(nodoSala->dato);
        }
        free(nodoSala);
    }
    tienda->headSalas = NULL;

    /* Elimina clientes y los pases que cada cliente compro. */
    while (tienda->headClientes != NULL) {
        int rut = tienda->headClientes->dato->rut;
        eliminarCliente(tienda, rut);
    }

    /* Los pases registrados son copias; liberar cada lista no afecta ventas. */
    liberarPasesDelDia(tienda->pasesDelDia);
    tienda->pasesDelDia = NULL;
    liberarPasesDelDia(tienda->catalogoPases);
    tienda->catalogoPases = NULL;

    /* La fecha, el horario y la recaudacion se conservan hasta que termina. */
    *recaudacionFinal = recaudacion;
    return 1;
}



