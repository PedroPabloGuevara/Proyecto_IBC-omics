#include "../include/clientes.h"
#include "../include/cierreTienda.h"
#include "../include/entrada.h"
#include "../include/libros.h"
#include "../include/pases.h"
#include "../include/reservas.h"
#include "../include/salas.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define CASOS_ESTRES 1000

static struct Libro *insertarLibroPrueba(struct NodoLibro **raiz, int codigo,
                                         int disponibilidad, int stock,
                                         int precio) {
    struct Libro *libro;
    struct NodoLibro *nodo;

    libro = crearLibro(codigo, "Libro de prueba", precio, stock, "Editorial",
                       0, disponibilidad, "Prueba", "01/01/2020", 0);
    assert(libro != NULL);

    nodo = crearNodoLibro(libro);
    assert(nodo != NULL);
    assert(insertarLibro(raiz, nodo) == 1);
    return libro;
}

static void ejecutarCasoIntegrado(int caso) {
    struct Tienda tienda = {0};
    struct ResultadoFila resultado;
    struct NodoCliente *cliente;
    struct NodoSala *nodoSala;
    struct SalaTematica *sala;
    struct Libro *libroReservado;
    struct Pase *pase;
    int codigoBase = 10000 + caso * 10;
    int rut = 50000 + caso;
    int precio = caso % 900 + 1;
    int recaudacionFinal = -1;
    int reservasCanceladas = -1;
    int tipoPase = caso % 4 + 1;

    tienda.fecha = "29/10/2026";
    tienda.hora_inicio = "09:00";
    tienda.hora_termino = "18:00";
    assert(configurarControlAcceso(&tienda) == 1);
    assert(agregarCliente(&tienda.headClientes, "Cliente prueba", rut) == 1);

    cliente = buscarCliente(tienda.headClientes, rut);
    assert(cliente != NULL);
    pase = crearPase(tipoPase, 400000 + caso, precio, 1, tienda.fecha, "08:00");
    assert(pase != NULL);
    assert(registrarPaseCliente(&tienda, rut, pase) == 1);

    assert(solicitarIngreso(&tienda, cliente->dato, pase, "08:30",
                           &resultado) == 1);
    assert(resultado.esperando == 1);
    assert(procesarFila(&tienda, "09:00", &resultado) == 1);
    assert(resultado.admitidos == 1 && resultado.esperando == 0);
    assert(pase->estado == 3);

    sala = crearSala("Sala de prueba", caso + 1, 50);
    assert(sala != NULL);
    nodoSala = crearNodoSala(sala);
    assert(nodoSala != NULL);
    insertarSala(&tienda.headSalas, nodoSala);

    insertarLibroPrueba(&sala->raiz, codigoBase + 2, 3, 0, precio);
    insertarLibroPrueba(&sala->raiz, codigoBase + 1, 1, 2, precio);
    insertarLibroPrueba(&sala->raiz, codigoBase + 3, 1, 2, precio);
    insertarLibroPrueba(&sala->raiz, codigoBase + 4, 3, 0, precio);
    assert(contarLibros(sala->raiz) == 4);

    libroReservado = buscarLibro(sala->raiz, codigoBase + 2)->dato;
    assert(reservarLibro(libroReservado, cliente->dato, pase) == 1);
    assert(cantidadEnEspera(libroReservado) == 1);
    assert(recibirDespacho(sala->raiz, codigoBase + 2, 2) == 1);
    assert(venderEjemplares(&tienda, sala->raiz, codigoBase + 2, rut, 1) == 1);
    assert(libroReservado->headReserva == NULL);
    assert(tienda.recaudacion == precio);

    assert(reservarLibro(buscarLibro(sala->raiz, codigoBase + 4)->dato,
                         cliente->dato, pase) == 1);
    assert(descatalogarLibro(sala->raiz, codigoBase + 4,
                             &reservasCanceladas) == 1);
    assert(reservasCanceladas == 1);
    assert(buscarLibro(sala->raiz, codigoBase + 4)->dato->headReserva == NULL);

    /* El caso principal borra primero un nodo con dos hijos. */
    assert(eliminarLibro(&sala->raiz, codigoBase + 2) == 1);
    assert(buscarLibro(sala->raiz, codigoBase + 2) == NULL);
    assert(contarLibros(sala->raiz) == 3);
    assert(eliminarLibro(&sala->raiz, codigoBase + 1) == 1);
    assert(eliminarLibro(&sala->raiz, codigoBase + 4) == 1);
    assert(eliminarLibro(&sala->raiz, codigoBase + 3) == 1);
    assert(sala->raiz == NULL);
    assert(eliminarLibro(&sala->raiz, codigoBase + 3) == 0);
    assert(eliminarLibro(NULL, codigoBase + 3) == 0);

    assert(cerrarTienda(&tienda, &recaudacionFinal) == 1);
    assert(recaudacionFinal == precio);
    assert(tienda.filaEntrada == NULL && tienda.filaPasesDeldia == NULL);
    assert(eliminarCliente(&tienda, rut) == 1);

    sala = QuitarSala(&tienda.headSalas, caso + 1);
    assert(sala != NULL && tienda.headSalas == NULL);
    free(sala->tematica);
    free(sala);
}

int main(void) {
    int caso;

    for (caso = 0; caso < CASOS_ESTRES; caso++) {
        ejecutarCasoIntegrado(caso);
    }

    printf("%d casos integrados completados.\n", CASOS_ESTRES);
    return 0;
}
