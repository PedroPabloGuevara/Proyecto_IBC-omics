#include "../../include/libros.h"
#include "../../include/reservas.h"

#include <stddef.h>

/*
-retorna la recaudacion acumulada en la tienda
-retorna -1 si la tienda es NULL o la recaudacion es invalida
*/
int calcularRecaudacionDiaria(struct Tienda *tienda) {
    if (tienda == NULL || tienda->recaudacion < 0) {
        return -1;
    }

    /* Libros y pases comparten el acumulador definido en Tienda. */
    return tienda->recaudacion;
}

/*
-reinicia la recaudacion acumulada de la tienda
-si tienda es NULL, no realiza cambios
*/
void reiniciarRecaudacionDiaria(struct Tienda *tienda) {
    /* Una tienda inexistente no tiene un acumulador que pueda reiniciarse. */
    if (tienda != NULL) {
        tienda->recaudacion = 0;
    }
}

/*
-busca el libro por codigo y vende hasta la cantidad solicitada o el stock
-elimina la reserva de ese rut si el comprador tenia una
-retorna las unidades vendidas o cero si los datos no permiten la venta
*/
int venderEjemplares(struct Tienda *tienda, struct NodoLibro *raiz, int codigo,
                    int rut, int cantidad) {
    struct NodoLibro *nodo;
    struct Libro *libro;
    int unidadesVendidas;
    int ingreso;
    int reservasPendientes;

    if (tienda == NULL || cantidad <= 0 ||
        (nodo = buscarLibro(raiz, codigo)) == NULL) {
        return 0;
    }
    libro = nodo->dato;

    if (libro == NULL || libro->precio < 0 ||
        tienda->recaudacion < 0) {
        return 0;
    }

    reservasPendientes = libro->headReserva != NULL;
    if (reservasPendientes) {
        if ((libro->disponibilidad != 1 && libro->disponibilidad != 3) ||
            libro->headReserva->dato == NULL ||
            libro->headReserva->dato->reservador == NULL ||
            libro->headReserva->dato->reservador->rut != rut) {
            return 0;
        }
    } else if (libro->disponibilidad != 1 ||
               libro->ejemplaresDisponibles <= 0) {
        return 0;
    }

    unidadesVendidas = reservasPendientes ? 1 : cantidad;
    if (!reservasPendientes &&
        unidadesVendidas > libro->ejemplaresDisponibles) {
        unidadesVendidas = libro->ejemplaresDisponibles;
    }
    if (libro->precio > 0 &&
        (unsigned int)unidadesVendidas >
            (((unsigned int)-1 >> 1) -
             (unsigned int)tienda->recaudacion) /
                (unsigned int)libro->precio) {
        return 0;
    }

    ingreso = unidadesVendidas * libro->precio;
    if (!reservasPendientes) {
        libro->ejemplaresDisponibles -= unidadesVendidas;
    }
    tienda->recaudacion += ingreso;
    if (reservasPendientes) {
        eliminarReserva(libro, rut);
    }
    if (libro->headReserva == NULL) {
        libro->disponibilidad =
            libro->ejemplaresDisponibles == 0 ? 2 : 1;
    }
    if (libro->disponibilidad == 1 &&
        libro->ejemplaresDisponibles == 0) {
        libro->disponibilidad = 2;
    }
    return unidadesVendidas;
}

int venderEjemplar(struct Tienda *tienda, struct NodoLibro *raiz, int codigo) {
    return venderEjemplares(tienda, raiz, codigo, 0, 1);
}