#include "../../include/filaInterno.h"

#include <stdlib.h>

/*
-crea la fila y configura las estructuras que registran ocupacion y uso de pases
-requiere fecha DD/MM/AAAA y horario HH:MM validos previamente en la tienda
-retorna 1 si configura el control, o 0 si los datos o la reserva fallan
-debe ejecutarse una sola vez sobre una tienda inicializada
*/
int configurarControlAcceso(struct Tienda *tienda, int aforoSalaPrincipal) {
    struct Fila *fila;
    int apertura;
    int cierre;

    if (tienda == NULL || aforoSalaPrincipal <= 0 ||
        tienda->hora_inicio == NULL || tienda->hora_termino == NULL ||
        !validarFechaFila(tienda->fecha) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura) {
        return 0;
    }

    /* La fila se reserva antes de modificar la tienda para evitar medias configuraciones. */
    fila = crearFila();
    if (fila == NULL) {
        return 0;
    }

    tienda->filaEntrada = fila;
    tienda->ocupacionActual = NULL;
    tienda->usosPases = NULL;
    tienda->aforoSalaPrincipal = aforoSalaPrincipal;
    return 1;
}

/*
-libera las estructuras propias del control de acceso de la tienda
-vacía la fila y libera registros de ocupación y uso de pases
-no libera clientes, pases, salas ni textos que pertenecen a la tienda
-debe llamarse cuando el control de acceso ya no se vaya a utilizar
*/
void liberarControlAcceso(struct Tienda *tienda) {
    struct NodoOcupacion *ocupacion;
    struct NodoOcupacion *siguienteOcupacion;
    struct UsoPase *uso;
    struct UsoPase *siguienteUso;

    if (tienda == NULL) {
        return;
    }

    /* Libera los nodos propios del control, nunca los objetos referenciados. */
    liberarFila(tienda->filaEntrada);
    tienda->filaEntrada = NULL;

    /* Destruye el registro de quienes estaban dentro. */
    ocupacion = tienda->ocupacionActual;
    while (ocupacion != NULL) {
        siguienteOcupacion = ocupacion->sig;
        free(ocupacion);
        ocupacion = siguienteOcupacion;
    }
    tienda->ocupacionActual = NULL;

    /* Destruye la asociacion diaria entre RUT y pase. */
    uso = tienda->usosPases;
    while (uso != NULL) {
        siguienteUso = uso->sig;
        free(uso);
        uso = siguienteUso;
    }
    tienda->usosPases = NULL;
}
