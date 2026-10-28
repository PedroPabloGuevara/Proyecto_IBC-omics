#include "../include/clientes.h"
#include "../include/cierreTienda.h"
#include "../include/entrada.h"
#include "../include/entradaInterna.h"
#include "../include/libros.h"
#include "../include/pases.h"
#include "../include/reservas.h"
#include "../include/salas.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static struct Pase *crearPasePrueba(int tipo, int codigo, int estado,
                                    const char *fecha, const char *hora) {
    return crearPase(tipo, codigo, 0, estado, (char *)fecha, (char *)hora);
}

static struct Libro *agregarLibroPrueba(struct SalaTematica *sala, int codigo,
                                        const char *nombre, int precio,
                                        int stock, int disponibilidad) {
    struct Libro *libro;
    struct NodoLibro *nodo;

    libro = crearLibro(codigo, (char *)nombre, precio, stock, "Edicion", 0,
                       disponibilidad, "Prueba", "01/01/2020", 0);
    assert(libro != NULL);
    nodo = crearNodoLibro(libro);
    assert(nodo != NULL);
    assert(insertarLibro(&sala->raiz, nodo) == 1);
    return libro;
}

static void probarFechasHorasYConfiguracion(void) {
    struct Tienda tienda = {0};
    int minutos = -1;

    assert(validarFechaFila("29/02/2024") == 1);
    assert(validarFechaFila("29/02/2023") == 0);
    assert(validarFechaFila("31/04/2024") == 0);
    assert(validarFechaFila("00/01/2024") == 0);
    assert(validarFechaFila("01/13/2024") == 0);
    assert(validarFechaFila("1/01/2024") == 0);
    assert(validarFechaFila(NULL) == 0);

    assert(validarHoraFila("00:00", &minutos) == 1 && minutos == 0);
    assert(validarHoraFila("23:59", &minutos) == 1 && minutos == 1439);
    assert(validarHoraFila("24:00", &minutos) == 0);
    assert(validarHoraFila("12:60", &minutos) == 0);
    assert(validarHoraFila("9:00", &minutos) == 0);
    assert(validarHoraFila("xx:00", &minutos) == 0);
    assert(validarHoraFila("12:00", NULL) == 0);

    tienda.fecha = "31/02/2024";
    tienda.hora_inicio = "09:00";
    tienda.hora_termino = "18:00";
    assert(configurarControlAcceso(&tienda) == 0);
    tienda.fecha = "29/02/2024";
    tienda.hora_inicio = "18:00";
    tienda.hora_termino = "09:00";
    assert(configurarControlAcceso(&tienda) == 0);
    tienda.hora_inicio = "09:00";
    tienda.hora_termino = "18:00";
    assert(configurarControlAcceso(&tienda) == 1);
    assert(configurarControlAcceso(&tienda) == 0);
    assert(horaDentroHorarioTienda(&tienda, "09:00") == 1);
    assert(horaDentroHorarioTienda(&tienda, "17:59") == 1);
    assert(horaDentroHorarioTienda(&tienda, "08:59") == 0);
    assert(horaDentroHorarioTienda(&tienda, "18:00") == 0);
    assert(horaDentroHorarioTienda(&tienda, "no-es-hora") == 0);
    liberarControlAcceso(&tienda);
}

static void probarLimitePasesPorCliente(void) {
    struct Tienda tienda = {0};
    struct Pase *pase;
    int i;

    tienda.fecha = "29/02/2024";
    tienda.hora_inicio = "09:00";
    tienda.hora_termino = "18:00";
    assert(agregarCliente(&tienda.headClientes, "Capacidad", 900) == 1);
    assert(agregarCliente(&tienda.headClientes, "Segundo", 901) == 1);
    assert(modificarCliente(tienda.headClientes, 900, "Actualizado", 902) == 1);
    assert(buscarCliente(tienda.headClientes, 902) != NULL);
    assert(modificarCliente(tienda.headClientes, 902, "Conflicto", 901) == -1);
    assert(modificarCliente(tienda.headClientes, 902, NULL, 902) == 0);
    assert(modificarCliente(tienda.headClientes, 999, "No existe", 999) == 0);

    for (i = 0; i < MAXPASES; i++) {
        pase = crearPasePrueba(1, 2000 + i, 1, tienda.fecha, "hora libre");
        assert(pase != NULL);
        assert(registrarPaseCliente(&tienda, 902, pase) == 1);
    }

    pase = crearPasePrueba(1, 3000, 1, tienda.fecha, "hora libre");
    assert(pase != NULL);
    assert(registrarPaseCliente(&tienda, 902, pase) == 0);
    liberarPase(pase);
    assert(eliminarCliente(&tienda, 902) == 1);
    assert(eliminarCliente(&tienda, 901) == 1);
}

static void probarFlujoCompletoDeJornada(void) {
    struct Tienda tienda = {0};
    struct SalaTematica *sala;
    struct NodoSala *nodoSala;
    struct NodoCliente *anaNodo;
    struct NodoCliente *betoNodo;
    struct NodoCliente *carlaNodo;
    struct Pase *anaPase;
    struct Pase *betoPase;
    struct Pase *segundoBetoPase;
    struct Pase *pasePendiente;
    struct Pase *paseExtraAna;
    struct Pase *paseInvalido;
    struct Pase *paseNuevoDia;
    struct Libro *disponible;
    struct Libro *preventa;
    struct Libro *preventaDescatalogar;
    struct Libro *descatalogado;
    struct Libro *precioMaximo;
    struct Libro *duplicado;
    struct ResultadoFila resultado;
    struct NodoLibro *nodoDuplicado;
    int recaudacionFinal = -1;

    tienda.fecha = "29/02/2024";
    tienda.hora_inicio = "09:00";
    tienda.hora_termino = "18:00";
    assert(configurarControlAcceso(&tienda) == 1);
    assert(tienda.recaudacion == 0);

    assert(agregarCliente(&tienda.headClientes, "Ana", 101) == 1);
    assert(agregarCliente(&tienda.headClientes, "Beto", 102) == 1);
    assert(agregarCliente(&tienda.headClientes, "Carla", 103) == 1);
    assert(agregarCliente(&tienda.headClientes, "Duplicada", 101) == 0);
    anaNodo = buscarCliente(tienda.headClientes, 101);
    betoNodo = buscarCliente(tienda.headClientes, 102);
    carlaNodo = buscarCliente(tienda.headClientes, 103);
    assert(anaNodo != NULL && betoNodo != NULL && carlaNodo != NULL);
    assert(buscarCliente(tienda.headClientes, 999) == NULL);

    anaPase = crearPasePrueba(2, 501, 1, tienda.fecha, "25:99");
    betoPase = crearPasePrueba(4, 502, 1, tienda.fecha, "06:00");
    segundoBetoPase = crearPasePrueba(4, 503, 1, tienda.fecha, "23:59");
    assert(anaPase != NULL && betoPase != NULL && segundoBetoPase != NULL);
    assert(registrarPaseCliente(&tienda, 101, anaPase) == 1);
    assert(registrarPaseCliente(&tienda, 102, betoPase) == 1);
    assert(registrarPaseCliente(&tienda, 102, segundoBetoPase) == 1);
    assert(anaNodo->dato->plibrepases == 1);
    assert(betoNodo->dato->plibrepases == 2);

    paseInvalido = crearPasePrueba(1, 504, 1, "28/02/2024", "10:00");
    assert(paseInvalido != NULL);
    assert(registrarPaseCliente(&tienda, 103, paseInvalido) == 0);
    liberarPase(paseInvalido);
    paseInvalido = crearPasePrueba(5, 505, 1, tienda.fecha, "10:00");
    assert(paseInvalido != NULL);
    assert(registrarPaseCliente(&tienda, 103, paseInvalido) == 0);
    liberarPase(paseInvalido);
    paseInvalido = crearPasePrueba(1, 506, 2, tienda.fecha, "10:00");
    assert(paseInvalido != NULL);
    assert(registrarPaseCliente(&tienda, 103, paseInvalido) == 0);
    liberarPase(paseInvalido);
    paseInvalido = crearPasePrueba(1, 507, 1, tienda.fecha, "10:00");
    assert(paseInvalido != NULL);
    assert(registrarPaseCliente(&tienda, 999, paseInvalido) == 0);
    liberarPase(paseInvalido);
    paseInvalido = crearPasePrueba(1, 501, 1, tienda.fecha, "10:00");
    assert(paseInvalido != NULL);
    assert(registrarPaseCliente(&tienda, 103, paseInvalido) == 0);
    liberarPase(paseInvalido);
    assert(tienda.recaudacion == 0);

    assert(solicitarIngreso(&tienda, anaNodo->dato, anaPase, "08:30",
                            &resultado) == 1);
    assert(resultado.esperando == 1 && resultado.admitidos == 0);
    assert(solicitarIngreso(&tienda, anaNodo->dato, anaPase, "08:30",
                            &resultado) == 0);
    assert(cantidadEnFila(tienda.filaEntrada) == 1);
    assert(solicitarIngreso(&tienda, betoNodo->dato, betoPase, "08:30",
                            &resultado) == 1);
    assert(solicitarIngreso(&tienda, betoNodo->dato, segundoBetoPase, "08:30",
                            &resultado) == 1);
    assert(tienda.filaEntrada->head->paseVigente == betoPase);
    assert(tienda.filaEntrada->head->sig->paseVigente == segundoBetoPase);
    assert(tienda.filaEntrada->tail->paseVigente == anaPase);
    assert(procesarFila(&tienda, "08:59", &resultado) == 1);
    assert(resultado.esperando == 3 && resultado.admitidos == 0);
    assert(procesarFila(&tienda, "09:00", &resultado) == 1);
    assert(resultado.admitidos == 3 && resultado.esperando == 0);
    assert(anaPase->estado == 3 && betoPase->estado == 3 &&
           segundoBetoPase->estado == 3);
    assert(contarPasesPorEstado(tienda.filaPasesDeldia, 3) == 3);

    assert(solicitarIngreso(&tienda, anaNodo->dato, anaPase, "09:05",
                            &resultado) == 1);
    assert(resultado.admitidos == 1);
    assert(contarPasesPorEstado(tienda.filaPasesDeldia, 3) == 3);
    assert(solicitarIngreso(&tienda, betoNodo->dato, anaPase, "09:06",
                            &resultado) == 0);
    paseInvalido = crearPasePrueba(1, 508, 1, tienda.fecha, "10:00");
    assert(paseInvalido != NULL);
    assert(registrarPaseCliente(&tienda, 103, paseInvalido) == 1);
    assert(solicitarIngreso(&tienda, carlaNodo->dato, paseInvalido, "18:00",
                            &resultado) == 0);
    assert(solicitarIngreso(&tienda, carlaNodo->dato, paseInvalido,
                            "no-es-hora", &resultado) == 0);

    sala = crearSala("Narrativa", 7, 30);
    assert(sala != NULL);
    nodoSala = crearNodoSala(sala);
    assert(nodoSala != NULL);
    insertarSala(&tienda.headSalas, nodoSala);
    assert(buscarSala(tienda.headSalas, 7) == nodoSala);
    assert(modificarSala(&tienda.headSalas, 7, 8, 32, "Narrativa nueva") == 1);
    assert(sala->codigo == 8 && sala->aforoAprox == 32);
    assert(buscarSala(tienda.headSalas, 7) == NULL);
    assert(buscarSala(tienda.headSalas, 8) == nodoSala);
    assert(modificarSala(&tienda.headSalas, 999, 9, 20, "No existe") == 0);
    disponible = agregarLibroPrueba(sala, 10, "Disponible", 1000, 3, 1);
    preventa = agregarLibroPrueba(sala, 20, "Preventa", 500, 0, 3);
    descatalogado =
        agregarLibroPrueba(sala, 30, "Descatalogado", 700, 2, 4);
    agregarLibroPrueba(sala, 40, "Costo", 100, 2, 1);
    precioMaximo =
        agregarLibroPrueba(sala, 45, "Precio maximo", INT_MAX, 2, 1);
    preventaDescatalogar =
        agregarLibroPrueba(sala, 50, "Preventa descatalogada", 400, 0, 3);
    assert(contarLibros(sala->raiz) == 6);
    assert(crearLibro(0, "Codigo invalido", 1, 1, "Edicion", 0, 1,
                      "Prueba", "01/01/2020", 0) == NULL);
    assert(crearLibro(60, "Precio invalido", -1, 1, "Edicion", 0, 1,
                      "Prueba", "01/01/2020", 0) == NULL);
    assert(crearLibro(60, "Stock invalido", 1, -1, "Edicion", 0, 1,
                      "Prueba", "01/01/2020", 0) == NULL);
    assert(crearLibro(60, "Estado invalido", 1, 1, "Edicion", 0, 5,
                      "Prueba", "01/01/2020", 0) == NULL);
    assert(crearLibro(60, NULL, 1, 1, "Edicion", 0, 1,
                      "Prueba", "01/01/2020", 0) == NULL);
    assert(existeLibro(sala->raiz, 20) == 1);
    assert(existeLibro(sala->raiz, 999) == 0);
    duplicado = crearLibro(20, "Duplicado", 1, 1, "Edicion", 0, 1,
                           "Prueba", "01/01/2020", 0);
    assert(duplicado != NULL);
    nodoDuplicado = crearNodoLibro(duplicado);
    assert(nodoDuplicado != NULL);
    assert(insertarLibro(&sala->raiz, nodoDuplicado) == 0);
    free(nodoDuplicado);
    liberarLibro(duplicado);

    assert(reservarLibro(disponible, anaNodo->dato, 2) == 0);
    assert(reservarLibro(descatalogado, anaNodo->dato, 2) == 0);
    assert(reservarLibro(preventa, anaNodo->dato, 2) == 1);
    assert(reservarLibro(preventa, betoNodo->dato, 3) == 1);
    assert(reservarLibro(preventa, carlaNodo->dato, 4) == 1);
    assert(reservarLibro(preventa, carlaNodo->dato, 4) == 0);
    assert(cantidadEnEspera(preventa) == 3);
    assert(preventa->headReserva->dato->reservador == carlaNodo->dato);
    assert(preventa->headReserva->sig->dato->reservador == anaNodo->dato);
    assert(preventa->headReserva->sig->sig->dato->reservador == betoNodo->dato);

    assert(recibirDespacho(sala->raiz, 20, 2) == 1);
    assert(preventa->disponibilidad == 1);
    assert(preventa->ejemplaresDisponibles == 2);
    assert(venderEjemplares(&tienda, sala->raiz, 20, 101, 2) == 0);
    assert(venderEjemplar(&tienda, sala->raiz, 20) == 0);
    assert(venderEjemplares(&tienda, sala->raiz, 20, 103, 2) == 1);
    assert(preventa->ejemplaresDisponibles == 1);
    assert(buscarReserva(preventa, 103) == NULL);
    assert(buscarReserva(preventa, 101) != NULL);
    assert(venderEjemplares(&tienda, sala->raiz, 20, 101, 2) == 1);
    assert(preventa->ejemplaresDisponibles == 0);
    assert(buscarReserva(preventa, 101) == NULL);
    assert(buscarReserva(preventa, 102) != NULL);
    assert(tienda.recaudacion == 1000);

    assert(modificarPrecio(sala->raiz, 10, 1200) == 1);
    assert(modificarPrecio(sala->raiz, 999, 1200) == 0);
    assert(modificarPrecio(sala->raiz, 10, -1) == 0);
    assert(venderEjemplares(&tienda, sala->raiz, 10, 101, 2) == 2);
    assert(disponible->ejemplaresDisponibles == 1);
    assert(tienda.recaudacion == 3400);
    assert(venderEjemplares(&tienda, sala->raiz, 10, 102, 5) == 1);
    assert(disponible->ejemplaresDisponibles == 0);
    assert(disponible->disponibilidad == 2);
    assert(tienda.recaudacion == 4600);
    assert(venderEjemplar(&tienda, sala->raiz, 10) == 0);
    assert(venderEjemplar(&tienda, sala->raiz, 30) == 0);
    assert(venderEjemplar(&tienda, sala->raiz, 999) == 0);
    assert(venderEjemplares(&tienda, sala->raiz, 20, 101, 1) == 0);
    assert(venderEjemplar(&tienda, sala->raiz, 40) == 1);
    assert(tienda.recaudacion == 4700);
    assert(venderEjemplares(&tienda, sala->raiz, 10, 101, 0) == 0);
    assert(tienda.recaudacion == 4700);
    assert(calcularRecaudacionDiaria(&tienda) == 4700);
    assert(venderEjemplar(&tienda, sala->raiz, 45) == 0);
    assert(precioMaximo->ejemplaresDisponibles == 2);
    assert(tienda.recaudacion == 4700);

    assert(recibirDespacho(sala->raiz, 10, 2) == 1);
    assert(disponible->ejemplaresDisponibles == 2);
    assert(disponible->disponibilidad == 1);
    assert(recibirDespacho(sala->raiz, 10, 0) == 0);
    assert(recibirDespacho(sala->raiz, 10, -1) == 0);
    assert(recibirDespacho(sala->raiz, 30, 1) == 0);
    assert(recibirDespacho(sala->raiz, 999, 1) == 0);

    assert(descatalogarLibro(sala->raiz, 50) == 1);
    assert(preventaDescatalogar->disponibilidad == 4);
    assert(preventaDescatalogar->headReserva == NULL);
    assert(reservarLibro(preventaDescatalogar, anaNodo->dato, 1) == 0);
    assert(descatalogarLibro(sala->raiz, 999) == 0);
    assert(tienda.recaudacion == 4700);

    paseExtraAna = crearPasePrueba(1, 509, 1, tienda.fecha, "12:00");
    assert(paseExtraAna != NULL);
    assert(registrarPaseCliente(&tienda, 101, paseExtraAna) == 1);
    assert(encolarFila(tienda.filaEntrada, crearNodoPase(paseExtraAna)) == 1);
    assert(buscarEnFila(tienda.filaEntrada, 509) != NULL);
    assert(eliminarCliente(&tienda, 101) == 1);
    assert(buscarCliente(tienda.headClientes, 101) == NULL);
    assert(buscarEnFila(tienda.filaEntrada, 509) == NULL);
    assert(buscarReserva(preventa, 101) == NULL);
    assert(contarPasesPorEstado(tienda.filaPasesDeldia, 3) == 3);
    assert(eliminarCliente(&tienda, 999) == 0);

    pasePendiente = crearPasePrueba(1, 510, 1, tienda.fecha, "12:00");
    assert(pasePendiente != NULL);
    assert(registrarPaseCliente(&tienda, 102, pasePendiente) == 1);
    assert(encolarFila(tienda.filaEntrada, crearNodoPase(pasePendiente)) == 1);
    assert(procesarFila(&tienda, "18:00", &resultado) == 1);
    assert(resultado.rechazados == 1 && resultado.esperando == 0);
    assert(pasePendiente->estado == 1);

    assert(cerrarTienda(NULL, &recaudacionFinal) == 0);
    assert(cerrarTienda(&tienda, NULL) == 0);
    assert(cerrarTienda(&tienda, &recaudacionFinal) == 1);
    assert(recaudacionFinal == 4700);
    assert(tienda.recaudacion == 4700);
    assert(tienda.filaEntrada == NULL && tienda.filaPasesDeldia == NULL);
    assert(buscarCliente(tienda.headClientes, 102) == betoNodo);
    assert(buscarCliente(tienda.headClientes, 103) == carlaNodo);
    assert(tienda.headSalas == nodoSala);
    assert(buscarLibro(sala->raiz, 20)->dato == preventa);
    assert(buscarReserva(preventa, 102) != NULL);
    assert(buscarReserva(preventa, 103) == NULL);
    assert(betoNodo->dato->plibrepases == 0);
    assert(carlaNodo->dato->plibrepases == 0);
    assert(betoNodo->dato->pases[0] == NULL);
    assert(carlaNodo->dato->pases[0] == NULL);
    assert(cerrarTienda(&tienda, &recaudacionFinal) == 1);
    assert(recaudacionFinal == 4700);
    assert(buscarCliente(tienda.headClientes, 102) == betoNodo);

    puts("Flujo integrado: ingreso, prioridad, reingreso, libros, reservas y cierre OK.");

    tienda.fecha = "01/03/2024";
    assert(configurarControlAcceso(&tienda) == 1);
    paseNuevoDia = crearPasePrueba(1, 502, 1, tienda.fecha, "08:00");
    assert(paseNuevoDia != NULL);
    assert(registrarPaseCliente(&tienda, 102, paseNuevoDia) == 1);
    assert(betoNodo->dato->plibrepases == 1);
    assert(cerrarTienda(&tienda, &recaudacionFinal) == 1);
    assert(recaudacionFinal == 4700);
    assert(betoNodo->dato->plibrepases == 0);
    assert(betoNodo->dato->pases[0] == NULL);

    assert(eliminarCliente(&tienda, 102) == 1);
    assert(eliminarCliente(&tienda, 103) == 1);
    while (sala->raiz != NULL) {
        assert(eliminarLibro(&sala->raiz, sala->raiz->dato->codigo) == 1);
    }
    assert(QuitarSala(&tienda.headSalas, sala->codigo) == sala);
    free(sala->tematica);
    free(sala);
}

int main(void) {
    probarFechasHorasYConfiguracion();
    probarLimitePasesPorCliente();
    probarFlujoCompletoDeJornada();
    puts("Todas las pruebas del main de flujo pasaron.");
    return 0;
}
