#include "../../include/pases.h"
#include "../../include/clientes.h"

#include <limits.h>

/*
-elimina del arreglo del cliente los pases que no coinciden con la fecha actual
-libera los pases vencidos y actualiza la cantidad almacenada
-retorna la cantidad conservada o -1 si los argumentos no son validos
*/
int limpiarPasesVencidos(struct Cliente *cliente, char *fecha) {
    int i;
    int cantidad = 0;

    if (cliente == NULL || cliente->pases == NULL || fecha == NULL) {
        return -1;
    }

    /* Compacta hacia el inicio los pases vigentes y libera los demas. */
    for (i = 0; i < MAXPASES; i++) {
        struct Pase *pase = cliente->pases[i];

        if (pase != NULL && paseValidoHoy(pase, fecha)) {
            cliente->pases[cantidad] = pase;
            cantidad++;
        } else {
            liberarPase(pase);
        }
    }

    /* Deja NULL las posiciones que ya no contienen pases. */
    for (i = cantidad; i < MAXPASES; i++) {
        cliente->pases[i] = NULL;
    }

    cliente->plibrepases = cantidad;
    return cantidad;
}

/*
-busca el mayor codigo que aun aparece en los pases diarios o los de clientes
-retorna cero si no hay codigos o la tienda es NULL
*/
int mayorCodigoEmitido(struct Tienda *tienda) {
    struct NodoPase *nodoPase;
    struct NodoCliente *nodoCliente;
    int mayor = 0;
    int i;

    if (tienda == NULL) {
        return mayor;
    }

    /* Revisa los pases de la lista diaria. */
    if (tienda->filaPasesDeldia != NULL) {
        for (nodoPase = tienda->filaPasesDeldia->head;
             nodoPase != NULL; nodoPase = nodoPase->sig) {
            if (nodoPase->paseVigente != NULL &&
                nodoPase->paseVigente->codigo > mayor) {
                mayor = nodoPase->paseVigente->codigo;
            }
        }
    }

    /* Revisa la lista circular y los arreglos de pases de cada cliente. */
    nodoCliente = tienda->headClientes;
    if (nodoCliente != NULL) {
        do {
            if (nodoCliente->dato != NULL &&
                nodoCliente->dato->pases != NULL) {
                for (i = 0; i < MAXPASES; i++) {
                    struct Pase *pase = nodoCliente->dato->pases[i];

                    if (pase != NULL && pase->codigo > mayor) {
                        mayor = pase->codigo;
                    }
                }
            }
            nodoCliente = nodoCliente->sig;
        } while (nodoCliente != NULL &&
                 nodoCliente != tienda->headClientes);
    }

    return mayor;
}

/*
-busca al cliente y al pase modelo y valida si la venta solicitada puede realizarse
-limpia pases vencidos, limita la compra al espacio libre y calcula los codigos nuevos
-retorna 1 si la venta puede continuar y 0 si no cumple las condiciones
*/
int prepararVentaPases(struct Tienda *tienda, int rut, int codigoTipo,
                       int cantidad, struct Cliente **cliente,
                       struct Pase **modelo, int *cantidadActual,
                       int *cantidadVenta, int *mayorCodigo) {
    struct NodoCliente *nodoCliente;
    struct NodoPase *nodoModelo;

    /* Comprueba que existan la tienda, las listas y el monto acumulado sea valido. */
    if (tienda == NULL || tienda->filaPasesDeldia == NULL ||
        tienda->headClientes == NULL || tienda->fecha == NULL ||
        cantidad <= 0 || tienda->recaudacion < 0 ||
        cliente == NULL || modelo == NULL || cantidadActual == NULL ||
        cantidadVenta == NULL || mayorCodigo == NULL) {
        return 0;
    }

    /* Busca al comprador y el pase que proporciona tipo, valor y horario. */
    nodoCliente = buscarCliente(tienda->headClientes, rut);
    nodoModelo = buscarPase(tienda->filaPasesDeldia, codigoTipo);
    if (nodoCliente == NULL || nodoModelo == NULL ||
        nodoCliente->dato == NULL || nodoModelo->paseVigente == NULL) {
        return 0;
    }

    *cliente = nodoCliente->dato;
    *modelo = nodoModelo->paseVigente;
    if ((*cliente)->pases == NULL || (*modelo)->estado != 1 ||
        (*modelo)->valor < 0 || (*modelo)->hora == NULL) {
        return 0;
    }

    /* Limpia pases vencidos antes de calcular el espacio disponible. */
    *cantidadActual = limpiarPasesVencidos(*cliente, tienda->fecha);
    if (*cantidadActual < 0) {
        return 0;
    }

    /* La cantidad vendida no puede superar la capacidad del arreglo del cliente. */
    *cantidadVenta = cantidad;
    if (*cantidadVenta > MAXPASES - *cantidadActual) {
        *cantidadVenta = MAXPASES - *cantidadActual;
    }

    /* Evita que la recaudacion o los codigos excedan el rango de int. */
    if (*cantidadVenta == 0 ||
        ((*modelo)->valor > 0 &&
         *cantidadVenta >
             (INT_MAX - tienda->recaudacion) / (*modelo)->valor)) {
        return 0;
    }

    *mayorCodigo = mayorCodigoEmitido(tienda);
    if (*mayorCodigo > INT_MAX - *cantidadVenta) {
        return 0;
    }

    return 1;
}

/*
-crea los pases de la venta y luego los enlaza al cliente
-actualiza la recaudacion de la tienda solo cuando todos los pases se crearon
-retorna la cantidad registrada o cero si falla una creacion
*/
int registrarVentaPases(struct Tienda *tienda, struct Cliente *cliente,
                        struct Pase *modelo, int cantidadActual,
                        int cantidadVenta, int mayorCodigo) {
    struct Pase *nuevos[MAXPASES];
    int ingreso;
    int i;

    /* Prepara todos los pases antes de cambiar el estado de la venta. */
    for (i = 0; i < cantidadVenta; i++) {
        nuevos[i] = crearPase(modelo->tipoPase, mayorCodigo + i + 1,
                              modelo->valor, 1, tienda->fecha, modelo->hora);
        if (nuevos[i] == NULL) {
            /* Revierte las reservas de memoria hechas antes del fallo. */
            while (i > 0) {
                i--;
                liberarPase(nuevos[i]);
            }
            return 0;
        }
    }

    /* Suma el ingreso y guarda cada pase en la primera posicion libre. */
    ingreso = cantidadVenta * modelo->valor;
    tienda->recaudacion += ingreso;
    for (i = 0; i < cantidadVenta; i++) {
        cliente->pases[cantidadActual + i] = nuevos[i];
    }
    cliente->plibrepases = cantidadActual + cantidadVenta;

    return cantidadVenta;
}

/*
-realiza el flujo completo de venta de pases para un cliente
-valida la compra, limita la cantidad al espacio del cliente y registra los pases
-retorna la cantidad vendida o cero si la venta no puede completarse
*/
int venderPasesDelDia(struct Tienda *tienda, int rut, int codigoTipo,
                      int cantidad) {
    struct Cliente *cliente;
    struct Pase *modelo;
    int cantidadActual;
    int cantidadVenta;
    int mayorCodigo;

    /* Calcula los datos de venta antes de crear o enlazar los pases. */
    if (!prepararVentaPases(tienda, rut, codigoTipo, cantidad, &cliente,
                            &modelo, &cantidadActual, &cantidadVenta,
                            &mayorCodigo)) {
        return 0;
    }

    return registrarVentaPases(tienda, cliente, modelo, cantidadActual,
                               cantidadVenta, mayorCodigo);
}
