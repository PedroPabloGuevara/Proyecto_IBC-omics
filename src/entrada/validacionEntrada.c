Claude



















Ventaslibros · C
#include "../../include/libros.h"
#include "../../include/reservas.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

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
-registra la venta de varios ejemplares y actualiza stock y recaudacion
-si hay reservas, solo atiende al primero y vende una unidad de su reserva
-retorna las unidades vendidas o cero si no se pudo completar la venta
*/
int registrarVenta(struct Tienda *tienda, struct Libro *libro,
                int cantidad, int rut, int tieneComprador) {
    int unidadesVendidas;
    int ingreso;
    int reservasPendientes;

    /* Rechaza datos invalidos, libros descatalogados o falta de stock. */
    if (tienda == NULL || libro == NULL || cantidad <= 0 ||
        libro->disponibilidad != 1 ||
        libro->ejemplaresDisponibles <= 0 || libro->precio < 0 ||
        tienda->recaudacion < 0) {
        return 0;
    }

    reservasPendientes = libro->headReserva != NULL;
    if (reservasPendientes &&
        (!tieneComprador || libro->headReserva->dato == NULL ||
        libro->headReserva->dato->reservador == NULL ||
        libro->headReserva->dato->reservador->rut != rut)) {
        return 0;
    }

    /* Ajusta la solicitud a la cantidad de ejemplares disponibles. */
    unidadesVendidas = cantidad;
    if (unidadesVendidas > libro->ejemplaresDisponibles) {
        unidadesVendidas = libro->ejemplaresDisponibles;
    }
    if (reservasPendientes && unidadesVendidas > 1) {
        /* Cada reserva corresponde a un ejemplar; atiende la cola en orden. */
        unidadesVendidas = 1;
    }

    /* Evita que el producto o el acumulado excedan el rango de int. */
    if (libro->precio > 0 &&
        unidadesVendidas > (INT_MAX - tienda->recaudacion) / libro->precio) {
        return 0;
    }

    /* Actualiza stock e ingreso una vez validadas todas las condiciones. */
    ingreso = unidadesVendidas * libro->precio;
    libro->ejemplaresDisponibles -= unidadesVendidas;
    tienda->recaudacion += ingreso;

    /* Quien compra su libro reservado sale de la cola (solo puede ser el primero). */
    if (reservasPendientes) {
        struct NodoReserva *atendido = desencolar(libro);

        free(atendido->dato);
        free(atendido);
    }

    /* Mantiene el estado del libro sincronizado con el stock restante. */
    if (libro->ejemplaresDisponibles == 0) {
        libro->disponibilidad = 2;
    }

    return unidadesVendidas;
}

/*
-busca el libro por codigo y vende un ejemplar
-no elimina reservas porque no recibe el rut del comprador
-retorna uno si vende o cero si el libro no existe o no se puede vender
*/
int venderEjemplar(struct Tienda *tienda, struct NodoLibro *raiz, int codigo) {
    struct NodoLibro *nodo;

    /* Esta interfaz no identifica comprador, asi que no puede cumplir reservas. */
    if (tienda == NULL) {
        return 0;
    }

    /* Convierte el codigo en el libro que usara la operacion comun de venta. */
    nodo = buscarLibro(raiz, codigo);
    if (nodo == NULL) {
        return 0;
    }

    return registrarVenta(tienda, nodo->dato, 1, 0, 0);
}

/*
-busca el libro por codigo y vende hasta la cantidad solicitada o el stock
-elimina la reserva de ese rut si el comprador tenia una
-retorna las unidades vendidas o cero si los datos no permiten la venta
*/
int venderEjemplares(struct Tienda *tienda, struct NodoLibro *raiz, int codigo,
                    int rut, int cantidad) {
    struct NodoLibro *nodo;

    /* Rechaza argumentos incompletos antes de buscar el producto. */
    if (tienda == NULL || cantidad <= 0) {
        return 0;
    }

    /* Centraliza la venta para compartir validacion, stock y recaudacion. */
    nodo = buscarLibro(raiz, codigo);
    if (nodo == NULL) {
        return 0;
    }

    return registrarVenta(tienda, nodo->dato, cantidad, rut, 1);
}

/*
-entrega el libro reservado al cliente con ese rut y suma el dinero a la recaudacion
-solo puede recibirlo el primero de la cola de reservas del libro
-retorna 1 si entrego y 0 si no se pudo (no es su turno, sin stock, libro inexistente)
*/
int entregarReserva(struct Tienda *tienda, struct NodoLibro *raiz, int codigo,
                    int rut) {
    return venderEjemplares(tienda, raiz, codigo, rut, 1) == 1;
}

