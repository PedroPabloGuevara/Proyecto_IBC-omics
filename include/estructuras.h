#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

struct Pase {
    int tipoPase;
    int codigo;
    int valor;
    /* Estado: 1 disponible, 2 agotado, 3 en preventa, 4 descatalogado. */
    int estado;
    char *fecha;
    char *hora;
};

struct NodoPase {
    struct Pase *paseVigente;
    struct NodoPase *sig;
};

struct Cliente;

/* Persona que espera y pase con el que solicita entrar. */
struct NodoFila {
    struct Cliente *cliente;
    struct Pase *pase;
    int codigoSalaPreferida;
    struct NodoFila *sig;
};

struct NodoOcupacion {
    struct Cliente *cliente;
    struct Pase *pase;
    /* Codigo de sala; FILA_SALA_PRINCIPAL representa la sala principal. */
    int codigoSala;
    struct NodoOcupacion *sig;
};

struct UsoPase {
    struct Pase *pase;
    /* El primer ingreso asigna el pase a un unico RUT para sus reingresos. */
    int rutUsuario;
    /* Indica si el titular del pase esta actualmente dentro de la tienda. */
    int dentroTienda;
    struct UsoPase *sig;
};

struct pasesDelDia {
    struct NodoPase *head;
    /* Evita reutilizar codigos ya asignados a pases vendidos. */
    int ultimoCodigoEmitido;
};

struct Fila {
    struct NodoFila *head;
    struct NodoFila *tail;
};

struct Cliente {
    char *nombre;
    struct Pase **pases;
    int rut;
    int plibrepases;
};

struct NodoCliente {
    struct Cliente *dato;
    struct NodoCliente *sig;
};

/* Reserva un tipo de pase para un cliente, sin copiar ni poseer al cliente. */
struct Reserva {
    struct Cliente *reservador;
    int tipoPase;
};

/* Nodo enlazado que guarda una reserva en la cola. */
struct NodoReserva {
    struct Reserva *dato;
    struct NodoReserva *sig;
};

/* Cola enlazada: head es el proximo en atenderse, tail el ultimo y cantidad su total. */
struct ColaReservas {
    struct NodoReserva *head;
    struct NodoReserva *tail;
    int cantidad;
};

struct Libro {
    int codigo;
    char *nombre;
    int precio;
    int ejemplaresDisponibles;
    char *publicacion;
    int clasificacionEdad;
    /* Disponibilidad: 1 disponible, 2 agotado, 3 en preventa, 4 descatalogado. */
    int disponibilidad;
    char *genero;
    char *fechaPublicacion;
    int deColeccion;
    struct NodoReserva *headReserva;
    int recaudacionDia;
};

struct NodoLibro {
    struct Libro *dato;
    struct NodoLibro *izq;
    struct NodoLibro *der;
};

struct SalaTematica {
    char *tematica;
    int codigo;
    int aforoAprox;
    struct NodoLibro *raiz;
};

struct NodoSala {
    struct SalaTematica *dato;
    struct NodoSala *sig;
    struct NodoSala *ant;
};

struct Tienda {
    struct NodoSala *headSalas;
    struct NodoCliente *headClientes;
    struct Fila *filaEntrada;
    struct NodoOcupacion *ocupacionActual;
    struct UsoPase *usosPases;
    /* La capacidad total se calcula sumando este aforo y los de salas tematicas. */
    int aforoSalaPrincipal;
    struct pasesDelDia *filaPasesDeldia;
    /* Acumulados del dia; recaudacion es la suma de ambos. */
    int recaudacionLibrosDia;
    int recaudacionPasesDia;
    int recaudacion;
    char *hora_inicio; /* Hora de apertura de la tienda. HH:MM*/
    char *hora_termino; /* Hora de cierre de la tienda. HH:MM*/
    char *fecha; /* Fecha del dia de operacion. DD/MM/AAAA */
};

#endif
