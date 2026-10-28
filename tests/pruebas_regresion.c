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

/*
 * Verifica que el ingreso inicial quede registrado una sola vez y que un pase
 * utilizado pueda reingresar sin alterar la recaudacion.
 */
void probarIngresosUnicosYReingresos(void) {
    struct Tienda tienda = {0};
    struct NodoCliente *nodoCliente;
    struct Pase *ticket;
    struct Pase *ticketPrioritario;
    struct Pase *paseHoraCompra;
    struct ResultadoFila resultado;

    /* Configura la fecha y crea un cliente con pases para la misma jornada. */
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

    /* Estados anulados y vencidos deben fallar aunque el cliente los posea. */
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

/*
 * Prueba que eliminar clientes retire las referencias de fila y reservas,
 * y que el cierre preserve datos duraderos de libros y clientes.
 */
void probarEliminacionDeReferenciasYDespachos(void) {
    struct Tienda tienda = {0};
    struct SalaTematica *sala;
    struct NodoSala *nodoSala;
    struct Libro *libroUno;
    struct Libro *libroDos;
    struct NodoLibro *nodoLibro;
    struct NodoCliente *nodoCliente;
    struct Pase *ticket;
    struct Pase *ticketCarla;
    struct Pase *registro;
    struct Reserva *reservaRechazada;
    struct NodoReserva *nodoReservaRechazado;
    int recaudacionFinal = -1;

    /* Prepara la tienda, el cliente y sus estructuras diarias. */
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

    /* Inserta libros y confirma que no se permita reservar uno disponible. */
    libroUno = crearLibro(10, "A", 500, 1, "Edicion", 0, 1,
                          "Drama", "01/01/2020", 0);
    libroDos = crearLibro(20, "B", 600, 1, "Edicion", 0, 1,
                          "Drama", "01/01/2020", 0);
    assert(libroUno != NULL && libroDos != NULL);
    nodoLibro = crearNodoLibro(libroUno);
    assert(nodoLibro != NULL && insertarLibro(&sala->raiz, nodoLibro) == 1);
    nodoLibro = crearNodoLibro(libroDos);
    assert(nodoLibro != NULL && insertarLibro(&sala->raiz, nodoLibro) == 1);
    reservaRechazada = crearReserva(nodoCliente->dato, 2);
    assert(reservaRechazada != NULL);
    nodoReservaRechazado = crearNodoReserva(reservaRechazada);
    assert(nodoReservaRechazado != NULL);
    assert(encolar(libroUno, nodoReservaRechazado) == 0);
    assert(libroUno->headReserva == NULL);
    free(nodoReservaRechazado);
    free(reservaRechazada);
    assert(reservarLibro(libroUno, nodoCliente->dato, ticket) == 0);
    libroDos->ejemplaresDisponibles = 0;
    libroDos->disponibilidad = 3;
    assert(reservarLibro(libroDos, nodoCliente->dato, ticket) == 1);

    assert(recibirDespacho(sala->raiz, 10, INT_MAX) == 0);
    assert(libroUno->ejemplaresDisponibles == 1);

    /* El registro diario usa una copia independiente del pase del cliente. */
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

    /* Tras eliminar al primer cliente, otro puede reservar el libro en preventa. */
    assert(agregarCliente(&tienda.headClientes, "Carla", 789) == 1);
    nodoCliente = buscarCliente(tienda.headClientes, 789);
    assert(nodoCliente != NULL);
    ticketCarla = crearPase(1, 201, 0, 1, tienda.fecha, "11:00");
    assert(ticketCarla != NULL);
    assert(registrarPaseCliente(&tienda, 789, ticketCarla) == 1);
    assert(reservarLibro(libroDos, nodoCliente->dato, ticketCarla) == 1);

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

    /* Libera los objetos restantes creados por este escenario de regresion. */
    assert(eliminarCliente(&tienda, 789) == 1);
    while (sala->raiz != NULL) {
        assert(eliminarLibro(&sala->raiz, sala->raiz->dato->codigo) == 1);
    }
    assert(QuitarSala(&tienda.headSalas, sala->codigo) == sala);
    free(sala->tematica);
    free(sala);
}

/*
 * Comprueba argumentos invalidos, titularidad del pase, orden estable de
 * prioridades, insercion, extraccion, eliminacion y vaciado de reservas.
 */
static void probarCasosLimiteDeReservas(void) {
    struct Cliente clientes[5] = {
        {"Uno", NULL, 1001, 0},
        {"Dos", NULL, 1002, 0},
        {"Tres", NULL, 1003, 0},
        {"Cuatro", NULL, 1004, 0},
        {"Ajeno", NULL, 1005, 0}
    };
    struct Pase pases[5] = {
        {2, 1, 0, 1, NULL, NULL},
        {4, 2, 0, 1, NULL, NULL},
        {1, 3, 0, 1, NULL, NULL},
        {4, 4, 0, 1, NULL, NULL},
        {2, 5, 0, 1, NULL, NULL}
    };
    struct Pase paseAnulado = {2, 6, 0, 2, NULL, NULL};
    struct Pase paseVencido = {2, 7, 0, 4, NULL, NULL};
    struct Pase paseTipoInvalido = {5, 8, 0, 1, NULL, NULL};
    struct Pase paseTipoCero = {0, 9, 0, 1, NULL, NULL};
    struct Pase *pasesPorCliente[5][MAXPASES] = {{0}};
    struct Libro *preventa =
        crearLibro(101, "Preventa", 500, 0, "Edicion", 0, 3,
                   "Drama", "01/01/2020", 0);
    struct Libro *disponible =
        crearLibro(102, "Disponible", 500, 1, "Edicion", 0, 1,
                   "Drama", "01/01/2020", 0);
    struct Reserva reservaDuplicada = {&clientes[0], 2};
    struct Reserva reservaTipoInvalido = {&clientes[4], 0};
    struct Reserva reservaTipoAlto = {&clientes[4], 5};
    struct Reserva reservaSinCliente = {NULL, 2};
    struct Reserva reservaConEnlace = {&clientes[4], 2};
    struct NodoReserva nodoDuplicado = {&reservaDuplicada, NULL};
    struct NodoReserva nodoTipoInvalido = {&reservaTipoInvalido, NULL};
    struct NodoReserva nodoTipoAlto = {&reservaTipoAlto, NULL};
    struct NodoReserva nodoSinDatos = {NULL, NULL};
    struct NodoReserva nodoSinCliente = {&reservaSinCliente, NULL};
    struct NodoReserva nodoConEnlace = {&reservaConEnlace, &nodoDuplicado};
    struct NodoReserva *nodo;

    /* Prepara clientes y credenciales sin asignaciones dinamicas de prueba. */
    assert(preventa != NULL && disponible != NULL);

    pasesPorCliente[0][0] = &pases[0];
    pasesPorCliente[1][0] = &pases[1];
    pasesPorCliente[2][0] = &pases[2];
    pasesPorCliente[3][0] = &pases[3];
    pasesPorCliente[4][0] = &pases[4];
    clientes[0].pases = pasesPorCliente[0];
    clientes[1].pases = pasesPorCliente[1];
    clientes[2].pases = pasesPorCliente[2];
    clientes[3].pases = pasesPorCliente[3];
    clientes[4].pases = pasesPorCliente[4];

    /* Rechaza punteros ausentes, pases ajenos, estados invalidos y tipo incorrecto. */
    assert(reservarLibro(NULL, &clientes[0], &pases[0]) == 0);
    assert(reservarLibro(preventa, NULL, &pases[0]) == 0);
    assert(reservarLibro(preventa, &clientes[0], NULL) == 0);
    clientes[4].pases = NULL;
    assert(reservarLibro(preventa, &clientes[4], &pases[4]) == 0);
    clientes[4].pases = pasesPorCliente[4];

    assert(reservarLibro(preventa, &clientes[0], &pases[4]) == 0);
    assert(reservarLibro(preventa, &clientes[0], &paseAnulado) == 0);
    assert(reservarLibro(preventa, &clientes[0], &paseVencido) == 0);
    assert(reservarLibro(preventa, &clientes[0], &paseTipoInvalido) == 0);
    assert(reservarLibro(preventa, &clientes[0], &paseTipoCero) == 0);

    /* Inserta reservas validas y comprueba prioridad FIFO entre iguales. */
    assert(reservarLibro(disponible, &clientes[0], &pases[0]) == 0);
    assert(reservarLibro(preventa, &clientes[0], &pases[0]) == 1);
    assert(reservarLibro(preventa, &clientes[0], &pases[0]) == 0);
    assert(reservarLibro(preventa, &clientes[1], &pases[1]) == 1);
    assert(reservarLibro(preventa, &clientes[2], &pases[2]) == 1);
    assert(reservarLibro(preventa, &clientes[3], &pases[3]) == 1);
    assert(cantidadEnEspera(preventa) == 4);

    nodo = preventa->headReserva;
    assert(nodo->dato->reservador == &clientes[1]);
    nodo = nodo->sig;
    assert(nodo->dato->reservador == &clientes[3]);
    nodo = nodo->sig;
    assert(nodo->dato->reservador == &clientes[0]);
    nodo = nodo->sig;
    assert(nodo->dato->reservador == &clientes[2]);
    assert(nodo->sig == NULL);

    /* Nodos mal formados no deben alterar el contenido de la cola existente. */
    assert(encolar(preventa, NULL) == 0);
    assert(encolar(NULL, &nodoDuplicado) == 0);
    assert(encolar(preventa, &nodoSinDatos) == 0);
    assert(encolar(preventa, &nodoSinCliente) == 0);
    assert(encolar(preventa, &nodoDuplicado) == 0);
    assert(encolar(preventa, &nodoTipoInvalido) == 0);
    assert(encolar(preventa, &nodoTipoAlto) == 0);
    assert(encolar(preventa, &nodoConEnlace) == 0);
    assert(encolar(disponible, &nodoTipoInvalido) == 0);
    assert(cantidadEnEspera(preventa) == 4);
    assert(disponible->headReserva == NULL);

    /* Verifica que desencolar, eliminar y vaciar dejen enlaces consistentes. */
    nodo = desencolar(preventa);
    assert(nodo != NULL && nodo->dato->reservador == &clientes[1]);
    assert(nodo->sig == NULL);
    free(nodo->dato);
    free(nodo);
    assert(eliminarReserva(preventa, 1003) == 1);
    assert(eliminarReserva(preventa, 1004) == 1);
    assert(eliminarReserva(preventa, 1001) == 1);
    assert(eliminarReserva(preventa, 1001) == 0);
    assert(cantidadEnEspera(preventa) == 0);
    assert(desencolar(preventa) == NULL);

    assert(reservarLibro(preventa, &clientes[0], &pases[0]) == 1);
    assert(reservarLibro(preventa, &clientes[1], &pases[1]) == 1);
    vaciarCola(preventa);
    vaciarCola(preventa);
    assert(preventa->headReserva == NULL);
    assert(cantidadEnEspera(preventa) == 0);

    liberarLibro(preventa);
    liberarLibro(disponible);
}

/*
 * Punto de entrada de las pruebas de regresion.
 * Retorna cero cuando todos los escenarios completan sus aserciones.
 */
int main(void) {
    /* Ejecuta las pruebas regresivas de ingreso, referencias y reservas. */
    probarIngresosUnicosYReingresos();
    probarEliminacionDeReferenciasYDespachos();
    probarCasosLimiteDeReservas();
    puts("Pruebas de regresion completadas.");
    return 0;
}
