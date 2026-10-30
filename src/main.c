#include "../include/cierreTienda.h"
#include "../include/clientes.h"
#include "../include/entrada.h"
#include "../include/entradaInterna.h"
#include "../include/libros.h"
#include "../include/pases.h"
#include "../include/reservas.h"
#include "../include/salas.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_ENTRADA 256

int entradaTerminada;

int leerEntero(const char *mensaje, int *valor) {
    char texto[TAM_ENTRADA];
    char *fin;
    long numero;
    long maximoInt = (long)((unsigned int)-1 >> 1);
    long minimoInt = -maximoInt - 1;
    size_t largo;
    int caracter;

    printf("%s", mensaje);
    if (fgets(texto, sizeof(texto), stdin) == NULL) {
        entradaTerminada = 1;
        return 0;
    }
    largo = strlen(texto);
    if (largo > 0 && texto[largo - 1] != '\n' && !feof(stdin)) {
        while ((caracter = getchar()) != '\n' && caracter != EOF) {
        }
        printf("Numero invalido\n");
        return 0;
    }

    errno = 0;
    numero = strtol(texto, &fin, 10);
    while (isspace((unsigned char)*fin)) {
        fin++;
    }
    if (fin == texto || *fin != '\0' || errno == ERANGE ||
        numero < minimoInt || numero > maximoInt) {
        printf("Numero invalido\n");
        return 0;
    }

    *valor = (int)numero;
    return 1;
}

int leerTexto(const char *mensaje, char *texto, size_t capacidad) {
    size_t largo;
    int caracter;

    printf("%s", mensaje);
    if (fgets(texto, (int)capacidad, stdin) == NULL) {
        entradaTerminada = 1;
        return 0;
    }

    largo = strlen(texto);
    if (largo > 0 && texto[largo - 1] == '\n') {
        texto[--largo] = '\0';
    } else if (!feof(stdin)) {
        while ((caracter = getchar()) != '\n' && caracter != EOF) {
        }
        printf("Texto demasiado largo\n");
        return 0;
    }
    if (largo > 0 && texto[largo - 1] == '\r') {
        texto[--largo] = '\0';
    }
    if (largo == 0) {
        printf("El texto no puede estar vacio\n");
        return 0;
    }
    return 1;
}

struct Pase *buscarPaseCliente(struct Cliente *cliente, int codigo) {
    int i;

    if (cliente == NULL) {
        return NULL;
    }
    for (i = 0; i < MAXPASES; i++) {
        if (cliente->pases[i] != NULL &&
            cliente->pases[i]->codigo == codigo) {
            return cliente->pases[i];
        }
    }
    return NULL;
}

int registrarPase(struct Tienda *tienda, struct Cliente *cliente) {
    char horaCompra[16];
    int tipo;
    int codigo;
    int valor;
    struct Pase *pase;

    if (!leerEntero("Tipo (1 infantil, 2 familiar, 3 general, 4 prioritario): ",
                    &tipo) ||
        !leerEntero("Codigo unico del pase: ", &codigo) ||
        !leerEntero("Valor: ", &valor) ||
        !leerTexto("Hora de compra (HH:MM): ", horaCompra,
                   sizeof(horaCompra))) {
        return 0;
    }

    pase = crearPase(tipo, codigo, valor, 1, tienda->fecha, horaCompra);
    if (pase == NULL) {
        printf("No se pudo crear el pase\n");
        return 0;
    }
    if (!registrarPaseCliente(tienda, cliente->rut, pase)) {
        liberarPase(pase);
        printf("Pase invalido, codigo repetido o cliente sin capacidad\n");
        return 0;
    }

    printf("Pase asociado al cliente\n");
    return 1;
}

void gestionarPasesCliente(struct Tienda *tienda,
                           struct Cliente *cliente) {
    int opcion;
    int codigo;
    int tipo;
    int valor;
    int i;
    int resultado;
    struct Pase *pase;

    printf("1 Agregar pase  2 Listar pases  3 Modificar pase  4 Eliminar pase\n");
    if (!leerEntero("Opcion: ", &opcion)) {
        return;
    }

    if (opcion == 1) {
        registrarPase(tienda, cliente);
        return;
    }
    if (opcion == 2) {
        for (i = 0; i < MAXPASES; i++) {
            pase = cliente->pases[i];
            if (pase != NULL) {
                printf("Codigo: %d, tipo: %d, valor: %d, estado: %d, fecha: %s, hora: %s\n",
                       pase->codigo, pase->tipoPase, pase->valor, pase->estado,
                       pase->fecha, pase->hora);
            }
        }
        return;
    }
    if (opcion < 3 || opcion > 4) {
        printf("Opcion no valida\n");
        return;
    }

    if (!leerEntero("Codigo del pase: ", &codigo)) {
        return;
    }
    pase = buscarPaseCliente(cliente, codigo);
    if (pase == NULL) {
        printf("No se encontro el pase\n");
        return;
    }
    if (opcion == 3) {
        if (!leerEntero("Nuevo tipo (1-4): ", &tipo) ||
            !leerEntero("Nuevo valor: ", &valor)) {
            return;
        }
        if (modificarPase(pase, tipo, valor) == 1) {
            printf("Pase modificado\n");
        } else {
            printf("Tipo o valor invalidos\n");
        }
        return;
    }

    resultado = eliminarPase(tienda, cliente->rut, codigo);
    if (resultado == 1) {
        printf("Pase eliminado\n");
    } else if (resultado == -1) {
        printf("No se puede eliminar: el pase esta en la fila\n");
    } else if (resultado == -2) {
        printf("No se puede eliminar: el pase ya ingreso\n");
    } else {
        printf("No se pudo eliminar el pase\n");
    }
}

void menuClientes(struct Tienda *tienda) {
    char nombre[100];
    int opcion;
    int rut;
    int nuevoRut;
    struct NodoCliente *nodo;

    printf("1 Agregar  2 Buscar  3 Modificar  4 Eliminar  5 Listar  6 Pases\n");
    if (!leerEntero("Opcion: ", &opcion)) {
        return;
    }
    if (opcion == 1) {
        if (!leerTexto("Nombre: ", nombre, sizeof(nombre)) ||
            !leerEntero("RUT: ", &rut)) {
            return;
        }
        printf(agregarCliente(&tienda->headClientes, nombre, rut)
                   ? "Cliente agregado\n"
                   : "No se pudo agregar el cliente\n");
        return;
    }
    if (opcion == 5) {
        listarClientes(tienda->headClientes);
        return;
    }
    if (opcion < 2 || opcion > 6 ||
        !leerEntero("RUT del cliente: ", &rut)) {
        if (opcion < 2 || opcion > 6) {
            printf("Opcion no valida\n");
        }
        return;
    }
    nodo = buscarCliente(tienda->headClientes, rut);
    if (nodo == NULL) {
        printf("No se encontro el cliente\n");
        return;
    }

    switch (opcion) {
    case 2:
        printf("Cliente: %s, RUT: %d\n", nodo->dato->nombre, nodo->dato->rut);
        break;
    case 3:
        if (!leerTexto("Nuevo nombre: ", nombre, sizeof(nombre)) ||
            !leerEntero("Nuevo RUT: ", &nuevoRut)) {
            return;
        }
        if (modificarCliente(tienda->headClientes, rut, nombre, nuevoRut) == 1) {
            printf("Cliente modificado\n");
        } else {
            printf("No se pudo modificar el cliente\n");
        }
        break;
    case 4:
        switch (eliminarCliente(tienda, rut)) {
        case 1:
            printf("Cliente eliminado\n");
            break;
        case -1:
            printf("No se puede eliminar: tiene un pase en la fila\n");
            break;
        case -2:
            printf("No se puede eliminar: tiene un pase que ya ingreso\n");
            break;
        case -3:
            printf("No se puede eliminar: tiene una reserva pendiente\n");
            break;
        default:
            printf("No se encontro el cliente\n");
            break;
        }
        break;
    case 6:
        gestionarPasesCliente(tienda, nodo->dato);
        break;
    default:
        printf("Opcion no valida\n");
        break;
    }
}

void liberarArbolLibros(struct NodoLibro *raiz) {
    if (raiz == NULL) {
        return;
    }
    liberarArbolLibros(raiz->izq);
    liberarArbolLibros(raiz->der);
    liberarLibro(raiz->dato);
    free(raiz);
}

void liberarSalaCompleta(struct SalaTematica *sala) {
    if (sala == NULL) {
        return;
    }
    liberarArbolLibros(sala->raiz);
    free(sala->tematica);
    free(sala);
}

void menuSalas(struct Tienda *tienda) {
    char tematica[100];
    int opcion;
    int codigo;
    int aforo;
    int nuevoCodigo;
    int nuevoAforo;
    struct SalaTematica *sala;
    struct NodoSala *nodo;

    printf("1 Crear  2 Listar  3 Modificar  4 Eliminar\n");
    if (!leerEntero("Opcion: ", &opcion)) {
        return;
    }
    if (opcion == 2) {
        mostrarTodasSalas(tienda->headSalas);
        return;
    }
    if (opcion == 1) {
        if (!leerTexto("Tematica: ", tematica, sizeof(tematica)) ||
            !leerEntero("Codigo: ", &codigo) ||
            !leerEntero("Aforo aproximado: ", &aforo)) {
            return;
        }
        if (codigo <= 0 || aforo < 0 ||
            buscarSala(tienda->headSalas, codigo) != NULL) {
            printf("Codigo o aforo invalido, o codigo repetido\n");
            return;
        }
        sala = crearSala(tematica, codigo, aforo);
        nodo = crearNodoSala(sala);
        if (sala == NULL || nodo == NULL) {
            if (nodo != NULL) {
                free(nodo);
            }
            liberarSalaCompleta(sala);
            printf("No se pudo crear la sala\n");
            return;
        }
        insertarSala(&tienda->headSalas, nodo);
        printf("Sala creada\n");
        return;
    }
    if ((opcion != 3 && opcion != 4) ||
        !leerEntero("Codigo de la sala: ", &codigo)) {
        if (opcion != 3 && opcion != 4) {
            printf("Opcion no valida\n");
        }
        return;
    }

    if (opcion == 3) {
        if (!leerTexto("Nueva tematica: ", tematica, sizeof(tematica)) ||
            !leerEntero("Nuevo codigo: ", &nuevoCodigo) ||
            !leerEntero("Nuevo aforo: ", &nuevoAforo)) {
            return;
        }
        if (modificarSala(&tienda->headSalas, codigo, nuevoCodigo, nuevoAforo,
                          tematica)) {
            printf("Sala modificada\n");
        } else {
            printf("No se pudo modificar la sala\n");
        }
        return;
    }

    sala = QuitarSala(&tienda->headSalas, codigo);
    if (sala == NULL) {
        printf("No se encontro la sala\n");
        return;
    }
    liberarSalaCompleta(sala);
    printf("Sala y sus libros eliminados\n");
}

struct NodoSala *seleccionarSala(struct Tienda *tienda) {
    int codigo;

    if (!leerEntero("Codigo de la sala: ", &codigo)) {
        return NULL;
    }
    return buscarSala(tienda->headSalas, codigo);
}

void crearLibroEnSala(struct SalaTematica *sala) {
    char nombre[100];
    char editorial[100];
    char genero[100];
    char fechaPublicacion[32];
    int codigo;
    int precio;
    int ejemplares;
    int clasificacionEdad;
    int disponibilidad;
    int deColeccion;
    struct Libro *libro;
    struct NodoLibro *nodo;

    if (!leerEntero("Codigo del libro: ", &codigo) ||
        !leerTexto("Nombre: ", nombre, sizeof(nombre)) ||
        !leerEntero("Precio: ", &precio) ||
        !leerEntero("Ejemplares disponibles: ", &ejemplares) ||
        !leerTexto("Editorial: ", editorial, sizeof(editorial)) ||
        !leerEntero("Clasificacion de edad: ", &clasificacionEdad) ||
        !leerEntero("Disponibilidad (1 disponible, 2 agotado, 3 preventa, 4 descatalogado): ",
                    &disponibilidad) ||
        !leerTexto("Genero: ", genero, sizeof(genero)) ||
        !leerTexto("Fecha de publicacion: ", fechaPublicacion,
                   sizeof(fechaPublicacion)) ||
        !leerEntero("De coleccion (1 si, 0 no): ", &deColeccion)) {
        return;
    }

    if (existeLibro(sala->raiz, codigo)) {
        printf("Ya existe un libro con ese codigo en la sala\n");
        return;
    }
    libro = crearLibro(codigo, nombre, precio, ejemplares, editorial,
                       clasificacionEdad, disponibilidad, genero,
                       fechaPublicacion, deColeccion);
    if (libro == NULL) {
        printf("Datos invalidos o memoria insuficiente\n");
        return;
    }
    nodo = crearNodoLibro(libro);
    if (nodo == NULL || !insertarLibro(&sala->raiz, nodo)) {
        free(nodo);
        liberarLibro(libro);
        printf("No se pudo agregar el libro\n");
        return;
    }
    printf("Libro agregado\n");
}

void mostrarLibro(struct NodoLibro *nodo) {
    struct Libro *libro = nodo->dato;

    printf("Codigo: %d, nombre: %s, precio: %d, stock: %d, disponibilidad: %d, editorial: %s, genero: %s, fecha: %s\n",
           libro->codigo, libro->nombre, libro->precio,
           libro->ejemplaresDisponibles, libro->disponibilidad,
           libro->publicacion, libro->genero, libro->fechaPublicacion);
}

void menuLibros(struct Tienda *tienda) {
    int opcion;
    int codigo;
    int valor;
    int cantidad;
    int rut;
    int canceladas;
    struct NodoSala *nodoSala;
    struct NodoLibro *nodoLibro;

    printf("1 Agregar  2 Buscar  3 Cambiar precio  4 Eliminar  5 Listar\n");
    printf("6 Recibir despacho  7 Vender  8 Descatalogar\n");
    if (!leerEntero("Opcion: ", &opcion)) {
        return;
    }
    if (opcion < 1 || opcion > 8) {
        printf("Opcion no valida\n");
        return;
    }
    nodoSala = seleccionarSala(tienda);
    if (nodoSala == NULL) {
        if (!entradaTerminada) {
            printf("No se encontro la sala\n");
        }
        return;
    }
    if (opcion == 1) {
        crearLibroEnSala(nodoSala->dato);
        return;
    }
    if (opcion == 5) {
        if (nodoSala->dato->raiz == NULL) {
            printf("La sala no tiene libros\n");
        } else {
            mostrarLibrosPreorden(nodoSala->dato->raiz);
            printf("Total de libros: %d\n", contarLibros(nodoSala->dato->raiz));
        }
        return;
    }

    if (!leerEntero("Codigo del libro: ", &codigo)) {
        return;
    }
    nodoLibro = buscarLibro(nodoSala->dato->raiz, codigo);
    if (nodoLibro == NULL) {
        printf("No se encontro el libro\n");
        return;
    }
    switch (opcion) {
    case 2:
        mostrarLibro(nodoLibro);
        break;
    case 3:
        if (!leerEntero("Nuevo precio: ", &valor)) {
            return;
        }
        printf(modificarPrecio(nodoSala->dato->raiz, codigo, valor)
                   ? "Precio modificado\n"
                   : "Precio invalido\n");
        break;
    case 4:
        printf(eliminarLibro(&nodoSala->dato->raiz, codigo)
                   ? "Libro eliminado\n"
                   : "No se pudo eliminar el libro\n");
        break;
    case 6:
        if (!leerEntero("Cantidad recibida: ", &cantidad)) {
            return;
        }
        printf(recibirDespacho(nodoSala->dato->raiz, codigo, cantidad)
                   ? "Despacho registrado\n"
                   : "No se pudo registrar el despacho\n");
        break;
    case 7:
        if (!leerEntero("RUT del comprador: ", &rut) ||
            !leerEntero("Cantidad solicitada: ", &cantidad)) {
            return;
        }
        valor = venderEjemplares(tienda, nodoSala->dato->raiz, codigo, rut,
                                 cantidad);
        if (valor > 0) {
            printf("Unidades vendidas: %d\n", valor);
        } else {
            printf("Venta rechazada (stock, reserva o datos invalidos)\n");
        }
        break;
    case 8:
        if (descatalogarLibro(nodoSala->dato->raiz, codigo, &canceladas)) {
            printf("Libro descatalogado; reservas canceladas: %d\n", canceladas);
        } else {
            printf("No se pudo descatalogar el libro\n");
        }
        break;
    default:
        break;
    }
}

void menuReservas(struct Tienda *tienda) {
    int opcion;
    int codigoLibro;
    int rut;
    int codigoPase;
    struct NodoSala *nodoSala;
    struct NodoLibro *nodoLibro;
    struct NodoCliente *nodoCliente;
    struct NodoReserva *reserva;
    struct Pase *pase;

    printf("1 Reservar  2 Mostrar cola  3 Buscar  4 Cambiar pase\n");
    printf("5 Registrar venta reservada  6 Cancelar reserva\n");
    if (!leerEntero("Opcion: ", &opcion)) {
        return;
    }
    if (opcion < 1 || opcion > 6) {
        printf("Opcion no valida\n");
        return;
    }
    nodoSala = seleccionarSala(tienda);
    if (nodoSala == NULL) {
        if (!entradaTerminada) {
            printf("No se encontro la sala\n");
        }
        return;
    }
    if (!leerEntero("Codigo del libro: ", &codigoLibro)) {
        return;
    }
    nodoLibro = buscarLibro(nodoSala->dato->raiz, codigoLibro);
    if (nodoLibro == NULL) {
        printf("No se encontro el libro\n");
        return;
    }

    switch (opcion) {
    case 1:
        if (!leerEntero("RUT del cliente: ", &rut)) {
            return;
        }
        nodoCliente = buscarCliente(tienda->headClientes, rut);
        if (nodoCliente == NULL) {
            printf("No se encontro el cliente\n");
            return;
        }
        if (!leerEntero("Codigo del pase: ", &codigoPase)) {
            return;
        }
        pase = buscarPaseCliente(nodoCliente->dato, codigoPase);
        if (pase == NULL) {
            printf("El cliente no tiene ese pase\n");
            return;
        }
        printf(reservarLibro(nodoLibro->dato, nodoCliente->dato, pase)
                   ? "Reserva realizada\n"
                   : "No se pudo reservar (preventa, pase o duplicado invalido)\n");
        break;
    case 2:
        mostrarCola(nodoLibro->dato);
        printf("En espera: %d\n", cantidadEnEspera(nodoLibro->dato));
        break;
    case 3:
        if (!leerEntero("RUT del cliente: ", &rut)) {
            return;
        }
        printf(buscarReserva(nodoLibro->dato, rut) != NULL
                   ? "El cliente tiene una reserva\n"
                   : "El cliente no tiene una reserva\n");
        break;
    case 4:
        if (!leerEntero("RUT del cliente: ", &rut)) {
            return;
        }
        nodoCliente = buscarCliente(tienda->headClientes, rut);
        if (nodoCliente == NULL ||
            !leerEntero("Codigo del pase nuevo: ", &codigoPase)) {
            if (nodoCliente == NULL) {
                printf("No se encontro el cliente\n");
            }
            return;
        }
        pase = buscarPaseCliente(nodoCliente->dato, codigoPase);
        printf(pase != NULL &&
                       modificarReserva(nodoLibro->dato, nodoCliente->dato, pase)
                   ? "Reserva modificada\n"
                   : "No se pudo modificar la reserva\n");
        break;
    case 5:
        if (!leerEntero("RUT del cliente reservado: ", &rut)) {
            return;
        }
        if (venderEjemplares(tienda, nodoSala->dato->raiz, codigoLibro, rut,
                             1) == 1) {
            printf("Venta de la reserva registrada; recaudacion: %d\n",
                   tienda->recaudacion);
        } else {
            reserva = buscarReserva(nodoLibro->dato, rut);
            printf(reserva == NULL
                       ? "El cliente no tiene una reserva\n"
                       : "Venta rechazada: debe atenderse primero la reserva prioritaria\n");
        }
        break;
    case 6:
        if (!leerEntero("RUT del cliente reservado: ", &rut)) {
            return;
        }
        printf(cancelarReserva(nodoLibro->dato, rut)
                   ? "Reserva cancelada; el stock fue restaurado y la recaudacion no cambia\n"
                   : "El cliente no tiene una reserva\n");
        break;
    default:
        break;
    }
}

void menuIngreso(struct Tienda *tienda) {
    char horaActual[16];
    int rut;
    int codigoPase;
    struct NodoCliente *nodoCliente;
    struct Pase *pase;
    struct ResultadoFila resultado = {0, 0, 0};
    int estado;

    if (!leerEntero("RUT del cliente: ", &rut) ||
        !leerEntero("Codigo del pase: ", &codigoPase) ||
        !leerTexto("Hora actual (HH:MM): ", horaActual, sizeof(horaActual))) {
        return;
    }
    nodoCliente = buscarCliente(tienda->headClientes, rut);
    pase = buscarPaseCliente(nodoCliente != NULL ? nodoCliente->dato : NULL,
                             codigoPase);
    if (nodoCliente == NULL || pase == NULL) {
        printf("No se encontro el cliente o el pase\n");
        return;
    }

    estado = solicitarIngreso(tienda, nodoCliente->dato, pase, horaActual,
                              &resultado);
    if (estado == 1) {
        printf("Solicitud procesada: admitidos %d, rechazados %d, esperando %d\n",
               resultado.admitidos, resultado.rechazados,
               resultado.esperando);
    } else {
        printf("No se pudo procesar la solicitud\n");
    }
}

void liberarCliente(struct Cliente *cliente) {
    int i;

    if (cliente == NULL) {
        return;
    }
    for (i = 0; i < MAXPASES; i++) {
        liberarPase(cliente->pases[i]);
    }
    free(cliente->nombre);
    free(cliente);
}

void liberarClientes(struct NodoCliente *head) {
    struct NodoCliente *actual;

    if (head == NULL) {
        return;
    }
    actual = head->sig;
    while (actual != NULL && actual != head) {
        struct NodoCliente *siguiente = actual->sig;

        liberarCliente(actual->dato);
        free(actual);
        actual = siguiente;
    }
    liberarCliente(head->dato);
    free(head);
}

int main(void) {
    struct Tienda tienda = {0};
    char fecha[16];
    char horaInicio[16];
    char horaTermino[16];
    int opcion;
    int recaudacionFinal = 0;
    int estado;

    printf("Bienvenido a IBC-omics\n");
    for (;;) {
        if (!leerTexto("Fecha de la jornada (DD/MM/AAAA): ", fecha,
                       sizeof(fecha))) {
            if (entradaTerminada) {
                return 1;
            }
            continue;
        }
        if (validarFechaFila(fecha)) {
            break;
        }
        printf("Fecha invalida\n");
    }

    tienda.fecha = fecha;
    for (;;) {
        int apertura;
        int cierre;

        if (!leerTexto("Hora de apertura (HH:MM): ", horaInicio,
                       sizeof(horaInicio)) ||
            !leerTexto("Hora de cierre (HH:MM): ", horaTermino,
                       sizeof(horaTermino))) {
            if (entradaTerminada) {
                return 1;
            }
            continue;
        }
        if (!validarHoraFila(horaInicio, &apertura) ||
            !validarHoraFila(horaTermino, &cierre) || cierre <= apertura) {
            printf("Horario invalido\n");
            continue;
        }
        break;
    }
    tienda.hora_inicio = horaInicio;
    tienda.hora_termino = horaTermino;
    if (!configurarControlAcceso(&tienda)) {
        fprintf(stderr, "No se pudo configurar el control de acceso\n");
        return 1;
    }

    while (!entradaTerminada) {
        printf("\n1 Clientes  2 Salas  3 Libros  4 Reservas\n");
        printf("5 Registrar ingreso  6 Pases del dia  7 Recaudacion  0 Cerrar\n");
        if (!leerEntero("Opcion: ", &opcion)) {
            continue;
        }
        switch (opcion) {
        case 1:
            menuClientes(&tienda);
            break;
        case 2:
            menuSalas(&tienda);
            break;
        case 3:
            menuLibros(&tienda);
            break;
        case 4:
            menuReservas(&tienda);
            break;
        case 5:
            menuIngreso(&tienda);
            break;
        case 6:
            listarPasesDelDia(tienda.filaPasesDeldia);
            break;
        case 7:
            printf("Recaudacion: %d; ingresos unicos registrados: %d\n",
                   calcularRecaudacionDiaria(&tienda),
                   contarPasesPorEstado(tienda.filaPasesDeldia, 3));
            break;
        case 0:
            entradaTerminada = 1;
            break;
        default:
            printf("Opcion no valida\n");
            break;
        }
    }

    estado = cerrarTienda(&tienda, &recaudacionFinal);
    if (!estado) {
        fprintf(stderr, "No se pudo cerrar la jornada\n");
        return 1;
    }
    printf("\nJornada cerrada. Recaudacion final: %d\n", recaudacionFinal);
    {
        struct NodoSala *nodo = tienda.headSalas;
        while (nodo != NULL) {
            struct NodoSala *siguiente = nodo->sig;
            liberarSalaCompleta(nodo->dato);
            free(nodo);
            nodo = siguiente;
        }
    }
    liberarClientes(tienda.headClientes);
    return 0;
}
