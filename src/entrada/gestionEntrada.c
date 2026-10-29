#include "../../include/entradaInterna.h"
#include "../../include/pases.h"

#include <stdlib.h>

/*
-cuenta los nodos que esperan en la fila
-retorna cero si la fila es NULL o no contiene nodos
*/
int cantidadEnFila(struct Fila *fila) {
    struct NodoPase *nodo;
    int cantidad = 0;

    if (fila == NULL) {
        return 0;
    }
    /* Recorre los enlaces sin modificar el orden de la fila. */
    for (nodo = fila->head; nodo != NULL; nodo = nodo->sig) {
        cantidad++;
    }
    return cantidad;
}

/*
-inicializa en cero los resultados de una operacion de fila
-si resultado es NULL, no realiza cambios
*/
void inicializarResultadoFila(struct ResultadoFila *resultado) {
    if (resultado != NULL) {
        /* Evita que una llamada reutilizada conserve contadores anteriores. */
        resultado->admitidos = 0;
        resultado->rechazados = 0;
        resultado->esperando = 0;
    }
}

/*
-valida un pase y su titular, crea un nodo de espera y solicita procesar la fila
-retorna 1 si la solicitud se registro, 0 si no es valida, o -1 si fallo el procesamiento
-resultado recibe los conteos de admitidos, rechazados y personas que esperan
*/
int solicitarIngreso(struct Tienda *tienda, struct Cliente *cliente,
                     struct Pase *pase, char *horaActual,
                     struct ResultadoFila *resultado) {
    struct NodoPase *nuevo;

    inicializarResultadoFila(resultado);
    if (tienda == NULL || tienda->filaEntrada == NULL ||
        !validarSolicitudFila(tienda, cliente, pase, horaActual)) {
        return 0;
    }

    /* El nodo enlaza el pase existente; no crea ni copia otro pase. */
    nuevo = crearNodoPase(pase);
    if (nuevo == NULL) {
        return 0;
    }
    if (!encolarFila(tienda->filaEntrada, nuevo)) {
        free(nuevo);
        return 0;
    }

    /* Atiende de inmediato; el pase puede quedar esperando si aun no inicia. */
    if (!procesarFila(tienda, horaActual, resultado)) {
        if (resultado != NULL) {
            resultado->esperando = cantidadEnFila(tienda->filaEntrada);
        }
        return -1;
    }
    return 1;
}

/*
-procesa los pases en orden de prioridad y llegada dentro de la fila
-rechaza solicitudes invalidas y marca como utilizado cada pase admitido
-antes de abrir conserva la fila; al cierre rechaza y retira lo pendiente
-retorna 1 si pudo procesar la fila y 0 si la tienda o la hora no son validas
*/
int procesarFila(struct Tienda *tienda, char *horaActual,
                 struct ResultadoFila *resultado) {
    struct NodoPase *nodo;
    struct Cliente *cliente;
    int actual;
    int apertura;
    int cierre;

    inicializarResultadoFila(resultado);
    if (tienda == NULL || tienda->filaEntrada == NULL ||
        !validarFechaFila(tienda->fecha) ||
        !validarHoraFila(horaActual, &actual) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura) {
        return 0;
    }

    if (actual < apertura) {
        /* Aun no se procesan pases, pero se informa cuantos siguen esperando. */
        if (resultado != NULL) {
            resultado->esperando = cantidadEnFila(tienda->filaEntrada);
        }
        return 1;
    }

    if (actual >= cierre) {
        /* Al cerrar, retira y libera todos los nodos pendientes. */
        while (tienda->filaEntrada->head != NULL) {
            nodo = desencolarFila(tienda->filaEntrada);
            free(nodo);
            if (resultado != NULL) {
                resultado->rechazados++;
            }
        }
        return 1;
    }

    /* Atiende desde head para respetar prioridad y orden dentro de cada tipo. */
    while (tienda->filaEntrada->head != NULL) {
        nodo = tienda->filaEntrada->head;
        cliente = buscarClientePorPaseFila(tienda, nodo->paseVigente);
        if (cliente == NULL ||
            !validarSolicitudFila(tienda, cliente, nodo->paseVigente, horaActual)) {
            nodo = desencolarFila(tienda->filaEntrada);
            free(nodo);
            if (resultado != NULL) {
                resultado->rechazados++;
            }
            continue;
        }

        /*
         * Solo el primer ingreso crea el registro diario. La venta ya sumo
         * el dinero; los reingresos tampoco vuelven a contar esta entrada.
         */
        if (nodo->paseVigente->estado == 1) {
            struct Pase *registro;

            if (tienda->filaPasesDeldia == NULL) {
                tienda->filaPasesDeldia = crearPasesDelDia();
                if (tienda->filaPasesDeldia == NULL) {
                    if (resultado != NULL) {
                        resultado->esperando =
                            cantidadEnFila(tienda->filaEntrada);
                    }
                    return 0;
                }
            }

            /* El registro es una copia: sobrevive si se elimina al cliente. */
            registro = crearPase(nodo->paseVigente->tipoPase,
                                nodo->paseVigente->codigo,
                                nodo->paseVigente->valor, 3,
                                nodo->paseVigente->fecha,
                                nodo->paseVigente->hora);
            if (registro == NULL ||
                !agregarPase(tienda->filaPasesDeldia, registro)) {
                liberarPase(registro);
                if (resultado != NULL) {
                    resultado->esperando =
                        cantidadEnFila(tienda->filaEntrada);
                }
                return 0;
            }

            /* El pase se marca usado solo despues de guardarlo correctamente. */
            nodo->paseVigente->estado = 3;
        }

        nodo = desencolarFila(tienda->filaEntrada);
        free(nodo);
        if (resultado != NULL) {
            resultado->admitidos++;
        }
    }

    /* Registra cuantas solicitudes quedaron pendientes al terminar el recorrido. */
    if (resultado != NULL) {
        resultado->esperando = cantidadEnFila(tienda->filaEntrada);
    }
    return 1;
}