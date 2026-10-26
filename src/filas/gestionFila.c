#include "../../include/filaInterno.h"

#include <stdlib.h>

/*
-cuenta cuantos nodos esperan actualmente en la fila exterior
*/
int cantidadEnFila(struct Fila *fila) {
    struct NodoFila *nodo;
    int cantidad;

    cantidad = 0;
    if (fila == NULL) {
        return cantidad;
    }
    for (nodo = fila->head; nodo != NULL; nodo = nodo->sig) {
        cantidad++;
    }
    return cantidad;
}

/*
-reinicia los contadores de resultado antes de ejecutar una operacion de fila
*/
void inicializarResultadoFila(struct ResultadoFila *resultado) {
    if (resultado != NULL) {
        resultado->admitidos = 0;
        resultado->rechazados = 0;
        resultado->esperando = 0;
    }
}

/*
-agrega una solicitud de ingreso y procesa la fila con la hora actual
-valida el pase antes de encolar y guarda la sala preferida para respetar la eleccion
-retorna 1 si la solicitud queda registrada, 0 si los datos no son validos,
 o -1 si quedo encolada pero no se pudo completar su procesamiento
*/
int solicitarIngreso(struct Tienda *tienda, struct Cliente *cliente,
                     struct Pase *pase, int codigoSalaPreferida,
                     char *horaActual, struct ResultadoFila *resultado) {
    struct NodoFila *nuevo;
    int procesado;

    inicializarResultadoFila(resultado);
    if (tienda == NULL || tienda->filaEntrada == NULL ||
        !validarSolicitudFila(tienda, cliente, pase, horaActual) ||
        (codigoSalaPreferida != FILA_SALA_PRINCIPAL &&
         buscarSalaFila(tienda, codigoSalaPreferida) == NULL)) {
        return 0;
    }

    nuevo = crearNodoFila(cliente, pase);
    if (nuevo == NULL) {
        return 0;
    }
    nuevo->codigoSalaPreferida = codigoSalaPreferida;

    if (!encolarFila(tienda->filaEntrada, nuevo)) {
        free(nuevo);
        return 0;
    }

    procesado = procesarFila(tienda, horaActual, resultado);
    if (!procesado) {
        if (resultado != NULL) {
            resultado->esperando = cantidadEnFila(tienda->filaEntrada);
        }
        return -1;
    }
    return 1;
}

/*
-procesa solicitudes en orden de prioridad hasta que no haya espacio o solicitudes
-descarta solicitudes con pases vencidos, invalidos o asociados a otra persona
-intenta la sala elegida, luego las tematicas disponibles y finalmente la principal
-si la tienda o todas las salas estan llenas, mantiene las solicitudes esperando
-retorna 1 si el proceso se ejecuto y 0 si faltan datos o hubo un error de memoria
*/
int procesarFila(struct Tienda *tienda, char *horaActual,
                 struct ResultadoFila *resultado) {
    struct NodoFila *nodoFila;
    struct NodoOcupacion *nuevaOcupacion;
    struct UsoPase *uso;
    struct UsoPase *nuevoUso;
    int horaActualMinutos;
    int apertura;
    int cierre;
    int aforoTotal;
    int ocupacionTotal;
    int salaElegida;

    inicializarResultadoFila(resultado);
    if (tienda == NULL || tienda->filaEntrada == NULL ||
        !validarFechaFila(tienda->fecha) ||
        !validarHoraFila(horaActual, &horaActualMinutos) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura) {
        return 0;
    }

    if (horaActualMinutos < apertura) {
        /* Mantiene la cola intacta si el procesamiento se invoca antes de abrir. */
        if (resultado != NULL) {
            resultado->esperando = cantidadEnFila(tienda->filaEntrada);
        }
        return 1;
    }

    if (horaActualMinutos >= cierre) {
        /* Ninguna solicitud pendiente puede entrar despues del cierre. */
        while (tienda->filaEntrada->head != NULL) {
            nodoFila = desencolarFila(tienda->filaEntrada);
            free(nodoFila);
            if (resultado != NULL) {
                resultado->rechazados++;
            }
        }
        return 1;
    }

    aforoTotal = calcularAforoTotal(tienda);
    ocupacionTotal = contarPersonasEnTienda(tienda);
    if (aforoTotal < 0 || ocupacionTotal < 0) {
        return 0;
    }

    while (tienda->filaEntrada->head != NULL) {
        nodoFila = tienda->filaEntrada->head;
        if (!validarSolicitudFila(tienda, nodoFila->cliente, nodoFila->pase,
                                  horaActual)) {
            /* Una solicitud que vencio o no coincide con su titular no bloquea la fila. */
            nodoFila = desencolarFila(tienda->filaEntrada);
            free(nodoFila);
            if (resultado != NULL) {
                resultado->rechazados++;
            }
            continue;
        }

        if (ocupacionTotal >= aforoTotal ||
            !elegirSalaFila(tienda, nodoFila->codigoSalaPreferida,
                            &salaElegida)) {
            /* La primera solicitud valida espera hasta que se libere capacidad. */
            break;
        }

        nuevaOcupacion = malloc(sizeof(*nuevaOcupacion));
        if (nuevaOcupacion == NULL) {
            if (resultado != NULL) {
                resultado->esperando =
                    cantidadEnFila(tienda->filaEntrada);
            }
            return 0;
        }

        uso = buscarUsoPaseFila(tienda, nodoFila->pase);
        nuevoUso = NULL;
        if (uso == NULL) {
            nuevoUso = malloc(sizeof(*nuevoUso));
            if (nuevoUso == NULL) {
                free(nuevaOcupacion);
                if (resultado != NULL) {
                    resultado->esperando =
                        cantidadEnFila(tienda->filaEntrada);
                }
                return 0;
            }
            nuevoUso->pase = nodoFila->pase;
            nuevoUso->rutUsuario = nodoFila->cliente->rut;
            nuevoUso->dentroTienda = 1;
            nuevoUso->sig = tienda->usosPases;
            tienda->usosPases = nuevoUso;
        } else {
            /* La persona vuelve a entrar con el mismo pase previamente utilizado. */
            uso->dentroTienda = 1;
        }

        /* Registra quien entro y en que sala para controlar el aforo y su salida. */
        nuevaOcupacion->cliente = nodoFila->cliente;
        nuevaOcupacion->pase = nodoFila->pase;
        nuevaOcupacion->codigoSala = salaElegida;
        nuevaOcupacion->sig = tienda->ocupacionActual;
        tienda->ocupacionActual = nuevaOcupacion;

        nodoFila = desencolarFila(tienda->filaEntrada);
        free(nodoFila);
        ocupacionTotal++;
        if (resultado != NULL) {
            resultado->admitidos++;
        }
    }

    if (resultado != NULL) {
        resultado->esperando = cantidadEnFila(tienda->filaEntrada);
    }
    return 1;
}
