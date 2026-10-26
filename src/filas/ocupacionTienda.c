#include "../../include/filaInterno.h"

#include <limits.h>
#include <stdlib.h>

/*
-busca una sala tematica por codigo dentro de la tienda
-retorna el nodo de sala encontrado o NULL si no existe
*/
struct NodoSala *buscarSalaFila(struct Tienda *tienda, int codigo) {
    struct NodoSala *sala;

    if (tienda == NULL) {
        return NULL;
    }

    /* Las salas se recorren sin alterar su lista enlazada. */
    for (sala = tienda->headSalas; sala != NULL; sala = sala->sig) {
        if (sala->dato != NULL && sala->dato->codigo == codigo) {
            return sala;
        }
    }

    return NULL;
}

/*
-busca el registro de ocupacion correspondiente a un RUT
-retorna NULL si la persona no figura dentro de la tienda
*/
struct NodoOcupacion *buscarOcupacionFila(struct Tienda *tienda, int rut) {
    struct NodoOcupacion *ocupacion;

    if (tienda == NULL) {
        return NULL;
    }

    /* El RUT identifica a la persona, no al pase con el que ingreso. */
    for (ocupacion = tienda->ocupacionActual; ocupacion != NULL;
         ocupacion = ocupacion->sig) {
        if (ocupacion->cliente != NULL && ocupacion->cliente->rut == rut) {
            return ocupacion;
        }
    }

    return NULL;
}

/*
-cuenta las personas que ocupan una sala tematica o la principal
-recibe FILA_SALA_PRINCIPAL para contar la sala principal
*/
int contarOcupacionSalaFila(struct Tienda *tienda, int codigoSala) {
    struct NodoOcupacion *ocupacion;
    int cantidad;

    cantidad = 0;
    /* La ocupacion se mantiene en registros separados, sin tocar las salas. */
    for (ocupacion = tienda->ocupacionActual; ocupacion != NULL;
         ocupacion = ocupacion->sig) {
        if (ocupacion->codigoSala == codigoSala) {
            cantidad++;
        }
    }

    return cantidad;
}

/*
-comprueba si una sala tiene al menos un lugar disponible
-compara la ocupacion registrada con la capacidad configurada de la sala
*/
int salaDisponibleFila(struct Tienda *tienda, int codigoSala) {
    struct NodoSala *sala;

    if (codigoSala == FILA_SALA_PRINCIPAL) {
        return tienda->aforoSalaPrincipal > 0 &&
               contarOcupacionSalaFila(tienda, codigoSala) <
                   tienda->aforoSalaPrincipal;
    }

    sala = buscarSalaFila(tienda, codigoSala);
    return sala != NULL && sala->dato->aforoAprox > 0 &&
           contarOcupacionSalaFila(tienda, codigoSala) <
               sala->dato->aforoAprox;
}

/*
-elige la sala preferida si tiene espacio; en caso contrario busca alternativas
-prueba salas tematicas en el orden de la lista y deja la principal al final
*/
int elegirSalaFila(struct Tienda *tienda, int preferida, int *elegida) {
    struct NodoSala *sala;

    if (tienda == NULL || elegida == NULL) {
        return 0;
    }

    if (salaDisponibleFila(tienda, preferida)) {
        *elegida = preferida;
        return 1;
    }

    for (sala = tienda->headSalas; sala != NULL; sala = sala->sig) {
        if (sala->dato != NULL && sala->dato->codigo != preferida &&
            salaDisponibleFila(tienda, sala->dato->codigo)) {
            *elegida = sala->dato->codigo;
            return 1;
        }
    }

    if (preferida != FILA_SALA_PRINCIPAL &&
        salaDisponibleFila(tienda, FILA_SALA_PRINCIPAL)) {
        *elegida = FILA_SALA_PRINCIPAL;
        return 1;
    }

    return 0;
}

/*
-calcula la capacidad maxima total de la tienda
-suma el aforo principal y el aforo de cada sala tematica
-retorna -1 si encuentra capacidades invalidas o una suma fuera de rango
*/
int calcularAforoTotal(struct Tienda *tienda) {
    struct NodoSala *sala;
    struct NodoSala *otraSala;
    int total;

    if (tienda == NULL || tienda->aforoSalaPrincipal < 0) {
        return -1;
    }

    /* El aforo total es el de la principal mas la capacidad de cada tematica. */
    total = tienda->aforoSalaPrincipal;
    for (sala = tienda->headSalas; sala != NULL; sala = sala->sig) {
        if (sala->dato == NULL || sala->dato->aforoAprox < 0 ||
            sala->dato->codigo == FILA_SALA_PRINCIPAL ||
            total > INT_MAX - sala->dato->aforoAprox) {
            return -1;
        }
        for (otraSala = sala->sig; otraSala != NULL;
             otraSala = otraSala->sig) {
            if (otraSala->dato == NULL ||
                otraSala->dato->codigo == sala->dato->codigo) {
                return -1;
            }
        }
        total += sala->dato->aforoAprox;
    }

    return total;
}

/*
-cuenta las personas actualmente admitidas en todas las salas
-retorna -1 si la tienda es NULL o si el conteo excede el rango de int
*/
int contarPersonasEnTienda(struct Tienda *tienda) {
    struct NodoOcupacion *ocupacion;
    int cantidad;

    if (tienda == NULL) {
        return -1;
    }

    cantidad = 0;
    /* Cada nodo de ocupacion representa a una persona actualmente admitida. */
    for (ocupacion = tienda->ocupacionActual; ocupacion != NULL;
         ocupacion = ocupacion->sig) {
        if (cantidad == INT_MAX) {
            return -1;
        }
        cantidad++;
    }
    return cantidad;
}

/*
-registra la salida de una persona y libera el aforo de la sala que ocupaba
-marca el pase como usado pero conserva el RUT que lo utilizo para permitir reingreso
-despues procesa la fila para ocupar el espacio que se libero
-retorna 1 si la persona salio y 0 si no estaba dentro de la tienda
*/
int registrarSalidaTienda(struct Tienda *tienda, int rut, char *horaActual,
                          struct ResultadoFila *resultado) {
    struct NodoOcupacion *ocupacion;
    struct NodoOcupacion *anterior;
    struct UsoPase *uso;
    int procesado;

    inicializarResultadoFila(resultado);
    if (tienda == NULL) {
        return 0;
    }

    anterior = NULL;
    ocupacion = tienda->ocupacionActual;
    while (ocupacion != NULL &&
           (ocupacion->cliente == NULL || ocupacion->cliente->rut != rut)) {
        anterior = ocupacion;
        ocupacion = ocupacion->sig;
    }
    if (ocupacion == NULL) {
        return 0;
    }

    uso = buscarUsoPaseFila(tienda, ocupacion->pase);
    if (uso != NULL) {
        /* Conserva la asociacion del pase con su usuario para permitir reingreso. */
        uso->dentroTienda = 0;
    }

    if (anterior == NULL) {
        tienda->ocupacionActual = ocupacion->sig;
    } else {
        anterior->sig = ocupacion->sig;
    }
    free(ocupacion);

    procesado = procesarFila(tienda, horaActual, resultado);
    if (!procesado && resultado != NULL) {
        resultado->esperando = cantidadEnFila(tienda->filaEntrada);
    }
    return 1;
}

/*
-mueve a una persona que ya esta dentro a otra sala disponible
-no cambia el aforo total ocupado, solo actualiza la sala de destino
-retorna 1 si se movio o ya estaba en el destino y 0 si no es posible
*/
int cambiarSalaTienda(struct Tienda *tienda, int rut, int codigoSalaDestino) {
    struct NodoOcupacion *ocupacion;

    if (tienda == NULL ||
        (codigoSalaDestino != FILA_SALA_PRINCIPAL &&
         buscarSalaFila(tienda, codigoSalaDestino) == NULL)) {
        return 0;
    }

    ocupacion = buscarOcupacionFila(tienda, rut);
    if (ocupacion == NULL) {
        return 0;
    }
    if (ocupacion->codigoSala == codigoSalaDestino) {
        return 1;
    }
    /* No permite superar el limite de la sala de destino. */
    if (!salaDisponibleFila(tienda, codigoSalaDestino)) {
        return 0;
    }

    ocupacion->codigoSala = codigoSalaDestino;
    return 1;
}

/*
-obtiene el codigo de la sala donde esta actualmente una persona
-retorna FILA_SALA_PRINCIPAL para la sala principal o -2 si no esta dentro
*/
int obtenerSalaActual(struct Tienda *tienda, int rut) {
    struct NodoOcupacion *ocupacion;

    ocupacion = buscarOcupacionFila(tienda, rut);
    if (ocupacion == NULL) {
        return -2;
    }
    return ocupacion->codigoSala;
}
