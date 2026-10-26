#include "../../include/filaInterno.h"

#include <stddef.h>
#include <string.h>

/*
-comprueba que la hora actual este dentro del horario de atencion de la tienda
-la apertura se considera valida y el cierre queda excluido
*/
int horaDentroHorarioTienda(struct Tienda *tienda, char *horaActual) {
    int actual;
    int apertura;
    int cierre;

    if (tienda == NULL ||
        !validarHoraFila(horaActual, &actual) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura) {
        return 0;
    }

    /* El momento de apertura admite ingresos; el de cierre ya no. */
    return actual >= apertura && actual < cierre;
}

/*
-busca si el pase ya fue utilizado y a que persona quedo asociado
-retorna el registro del pase o NULL si nunca ingreso
*/
struct UsoPase *buscarUsoPaseFila(struct Tienda *tienda, struct Pase *pase) {
    struct UsoPase *uso;

    if (tienda == NULL || pase == NULL) {
        return NULL;
    }

    /* La direccion identifica el mismo pase vendido, aunque haya sido revendido. */
    for (uso = tienda->usosPases; uso != NULL; uso = uso->sig) {
        if (uso->pase == pase) {
            return uso;
        }
    }

    return NULL;
}

/*
-comprueba si una persona ya quedo asociada a un pase por un ingreso anterior
-retorna 1 si tiene otro pase asignado y 0 si puede usar el pase recibido
*/
int personaTieneOtroPaseFila(struct Tienda *tienda, int rut, struct Pase *pase) {
    struct UsoPase *uso;

    /* No permite que un mismo RUT cambie a otro pase despues del primer ingreso. */
    for (uso = tienda->usosPases; uso != NULL; uso = uso->sig) {
        if (uso->rutUsuario == rut && uso->pase != pase) {
            return 1;
        }
    }

    return 0;
}

/*
-valida que el pase permita solicitar ingreso en la fecha y hora indicadas
-comprueba fecha, estado, horario de apertura y hora de inicio del pase
-tambien impide que una persona ya dentro de la tienda solicite otro ingreso
*/
int validarSolicitudFila(struct Tienda *tienda, struct Cliente *cliente,
                         struct Pase *pase, char *horaActual) {
    struct UsoPase *uso;
    int actual;
    int inicioPase;
    int apertura;
    int cierre;

    if (tienda == NULL || cliente == NULL || pase == NULL ||
        pase->fecha == NULL || pase->hora == NULL ||
        pase->tipoPase < 1 || pase->tipoPase > 4 || pase->estado != 1 ||
        !validarFechaFila(tienda->fecha) ||
        !horaDentroHorarioTienda(tienda, horaActual) ||
        strcmp(pase->fecha, tienda->fecha) != 0 ||
        !validarHoraFila(horaActual, &actual) ||
        !validarHoraFila(pase->hora, &inicioPase) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre)) {
        return 0;
    }

    /* El pase no puede habilitar ingreso antes de su inicio ni fuera de la tienda. */
    if (inicioPase < apertura || inicioPase >= cierre || actual < inicioPase ||
        buscarOcupacionFila(tienda, cliente->rut) != NULL) {
        return 0;
    }

    /* Un pase usado solo puede volver a presentarlo la misma persona, una vez afuera. */
    uso = buscarUsoPaseFila(tienda, pase);
    if (uso != NULL &&
        (uso->rutUsuario != cliente->rut || uso->dentroTienda)) {
        return 0;
    }

    /* Cada RUT queda asociado a un solo pase, aunque tenga otros comprados. */
    if (personaTieneOtroPaseFila(tienda, cliente->rut, pase)) {
        return 0;
    }

    return 1;
}
