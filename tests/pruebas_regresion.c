#include "../include/clientes.h"
#include "../include/cierreTienda.h"
#include "../include/entrada.h"
#include "../include/entradaInterna.h"
#include "../include/libros.h"
#include "../include/pases.h"
#include "../include/reservas.h"
#include "../include/salas.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void probarIngresosUnicosYReingresos(void) {
    struct Tienda tienda = {0};
    struct NodoCliente *nodoCliente;
    struct Pase *ticket;
    struct Pase *ticketPrioritario;
    struct Pase *paseHoraCompra;
    struct ResultadoFila resultado;

    tienda.fecha = "27/10/2026";
    tienda.hora_inicio = "09:00";
    tienda.hora_termino = "18:00";
    assert(agregarCliente(&tienda.headClientes, "Ana", 123) == 1);
    assert(configurarControlAcceso(&tienda) == 1);

    nodoCliente = buscarCliente(tienda.headClientes, 123);
    assert(nodoCliente != NULL);
    ticket = crearPase(1, 100, 0, 1, tienda.fecha, "15:00");
    assert(ticket != NULL);
    assert(registrarPaseCliente(&tienda, 123, ticket) == 1);
    assert(tienda.recaudacion == 0);

    ticketPrioritario = crearPase(4, 104, 0, 1, tienda.fecha, "17:00");
    assert(ticketPrioritario != NULL);
    assert(registrarPaseCliente(&tienda, 123, ticketPrioritario) == 1);

    /* La prioridad de fila no depende de la hora de compra de cada pase. */
    assert(solicitarIngreso(&tienda, nodoCliente->dato, ticket, "08:30",
                            &resultado) == 1);
    assert(resultado.esperando == 1);
    assert(tienda.filaPasesDeldia == NULL);

    assert(solicitarIngreso(&tienda, nodoCliente->dato, ticketPrioritario,
                            "08:30", &resultado) == 1);
    assert(resultado.esperando == 2);
    assert(tienda.filaEntrada->head->paseVigente == ticketPrioritario);

    assert(procesarFila(&tienda, "09:00", &resultado) == 1);
    assert(resultado.admitidos == 2);
    assert(ticket->estado == 3);
    assert(ticketPrioritario->estado == 3);
    assert(contarPasesPorEstado(tienda.filaPasesDeldia, 3) == 2);
    assert(tienda.recaudacion == 0);

    /* El reingreso se admite sin duplicar la entrada ni sumar dinero por pase. */
    assert(solicitarIngreso(&tienda, nodoCliente->dato, ticket, "10:30",
                            &resultado) == 1);
    assert(resultado.admitidos == 1);
    assert(contarPasesPorEstado(tienda.filaPasesDeldia, 3) == 2);
    assert(tienda.recaudacion == 0);

    nodoCliente->dato->pases[2] =
        crearPase(1, 101, 0, 2, tienda.fecha, "10:00");
    nodoCliente->dato->pases[3] =
        crearPase(1, 102, 0, 4, tienda.fecha, "10:00");
    assert(nodoCliente->dato->pases[2] != NULL);
    assert(nodoCliente->dato->pases[3] != NULL);
    assert(solicitarIngreso(&tienda, nodoCliente->dato,
                            nodoCliente->dato->pases[2], "10:30",
                            &resultado) == 0);
    assert(solicitarIngreso(&tienda, nodoCliente->dato,
                            nodoCliente->dato->pases[3], "10:30",
                            &resultado) == 0);

    paseHoraCompra = crearPase(1, 103, 0, 1, tienda.fecha, "20:00");
    assert(paseHoraCompra != NULL);
    assert(registrarPaseCliente(&tienda, 123, paseHoraCompra) == 1);
    assert(cerrarTienda(&tienda, &resultado.admitidos) == 1);
    assert(resultado.admitidos == 0);
    assert(buscarCliente(tienda.headClientes, 123) != NULL);
    assert(tienda.filaEntrada == NULL);
    assert(tienda.filaPasesDeldia == NULL);
    assert(eliminarCliente(&tienda, 123) == 1);
}

void probarEliminacionDeReferenciasYDespachos(void) {
    struct Tienda tienda = {0};
    struct SalaTematica *sala;
    struct NodoSala *nodoSala;
    struct Libro *libroUno;
    struct Libro *libroDos;
    struct NodoLibro *nodoLibro;
    struct NodoCliente *nodoCliente;
    struct Pase *ticket;
    struct Pase *registro;
    int recaudacionFinal = -1;

    tienda.fecha = "27/10/2026";
    tienda.hora_inicio = "09:00";
    tienda.hora_termino = "18:00";
    tienda.filaPasesDeldia = crearPasesDelDia();
    assert(tienda.filaPasesDeldia != NULL);
    assert(configurarControlAcceso(&tienda) == 1);

    assert(agregarCliente(&tienda.headClientes, "Beto", 456) == 1);
    nodoCliente = buscarCliente(tienda.headClientes, 456);
    assert(nodoCliente != NULL);
    ticket = crearPase(2, 200, 0, 1, tienda.fecha, "11:00");
    assert(ticket != NULL);
    assert(registrarPaseCliente(&tienda, 456, ticket) == 1);
    assert(encolarFila(tienda.filaEntrada, crearNodoPase(ticket)) == 1);

    sala = crearSala("Lectura", 5, 20);
    assert(sala != NULL);
    nodoSala = crearNodoSala(sala);
    assert(nodoSala != NULL);
    insertarSala(&tienda.headSalas, nodoSala);

    libroUno = crearLibro(10, "A", 500, 1, "Edicion", 0, 1,
                          "Drama", "01/01/2020", 0);
    libroDos = crearLibro(20, "B", 600, 1, "Edicion", 0, 1,
                          "Drama", "01/01/2020", 0);
    assert(libroUno != NULL && libroDos != NULL);
    nodoLibro = crearNodoLibro(libroUno);
    assert(nodoLibro != NULL && insertarLibro(&sala->raiz, nodoLibro) == 1);
    nodoLibro = crearNodoLibro(libroDos);
    assert(nodoLibro != NULL && insertarLibro(&sala->raiz, nodoLibro) == 1);
    assert(reservarLibro(libroUno, nodoCliente->dato, 4) == 0);
    libroDos->disponibilidad = 3;
    assert(reservarLibro(libroDos, nodoCliente->dato, 4) == 1);

    assert(recibirDespacho(sala->raiz, 10, INT_MAX) == 0);
    assert(libroUno->ejemplaresDisponibles == 1);

    registro = crearPase(ticket->tipoPase, ticket->codigo, ticket->valor, 3,
                         ticket->fecha, ticket->hora);
    assert(registro != NULL);
    assert(agregarPase(tienda.filaPasesDeldia, registro) == 1);

    /* Al borrar al cliente se retiran sus pases en fila y reservas. */
    assert(eliminarCliente(&tienda, 456) == 1);
    assert(tienda.headClientes == NULL);
    assert(cantidadEnFila(tienda.filaEntrada) == 0);
    assert(buscarReserva(libroDos, 456) == NULL);
    assert(contarPasesPorEstado(tienda.filaPasesDeldia, 3) == 1);

    assert(agregarCliente(&tienda.headClientes, "Carla", 789) == 1);
    nodoCliente = buscarCliente(tienda.headClientes, 789);
    assert(nodoCliente != NULL);
    assert(reservarLibro(libroDos, nodoCliente->dato, 2) == 1);

    assert(venderEjemplar(&tienda, sala->raiz, 10) == 1);
    assert(tienda.recaudacion == 500);
    assert(cerrarTienda(&tienda, &recaudacionFinal) == 1);
    assert(recaudacionFinal == 500);
    assert(buscarCliente(tienda.headClientes, 789) != NULL);
    assert(tienda.headSalas == nodoSala);
    assert(buscarLibro(sala->raiz, 10) != NULL);
    assert(buscarLibro(sala->raiz, 20) != NULL);
    assert(buscarReserva(libroDos, 789) != NULL);
    assert(tienda.filaEntrada == NULL);
    assert(tienda.filaPasesDeldia == NULL);

    assert(eliminarCliente(&tienda, 789) == 1);
    while (sala->raiz != NULL) {
        assert(eliminarLibro(&sala->raiz, sala->raiz->dato->codigo) == 1);
    }
    assert(QuitarSala(&tienda.headSalas, sala->codigo) == sala);
    free(sala->tematica);
    free(sala);
}

int main(void) {
    probarIngresosUnicosYReingresos();
    probarEliminacionDeReferenciasYDespachos();
    puts("Pruebas de regresion completadas.");
    return 0;
}
