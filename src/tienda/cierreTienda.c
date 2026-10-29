#include "../../include/cierreTienda.h"
#include "../../include/clientes.h"
#include "../../include/entrada.h"
#include "../../include/pases.h"

#include <stddef.h>

/*
-libera la fila de ingreso y los pases diarios al cerrar la jornada
-conserva clientes, salas, libros, reservas y recaudacion en la tienda
-conserva la recaudacion en Tienda y la copia al parametro de salida
-retorna 1 si se completo el cierre y 0 si algun argumento es NULL
*/
int cerrarTienda(struct Tienda *tienda, int *recaudacionFinal) {
    int recaudacion;

    if (tienda == NULL || recaudacionFinal == NULL) {
        return 0;
    }

    /* Guarda el total antes de liberar las estructuras del dia. */
    recaudacion = tienda->recaudacion;

    /* Los nodos de la fila apuntan a pases que pertenecen a los clientes. */
    liberarControlAcceso(tienda);

    /* Los clientes permanecen, pero sus pases pertenecen a esta jornada. */
    liberarPasesClientes(tienda->headClientes);

    /* El registro diario contiene copias independientes de los pases. */
    liberarPasesDelDia(tienda->filaPasesDeldia);
    tienda->filaPasesDeldia = NULL;

    /* Los datos permanentes de la tienda y la recaudacion se conservan. */
    *recaudacionFinal = recaudacion;
    return 1;
}
