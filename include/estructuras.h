#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

struct Pase {
    int tipoPase;
    int codigo;
    int valor;
    int estado;
    char *fecha;
    char *hora;
};

struct NodoPase {
    struct Pase *paseVigente;
    struct NodoPase *sig;
};

struct pasesDelDia {
    struct NodoPase *head;
};

struct Fila {
    struct NodoPase *head;
    struct NodoPase *tail;
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
    int disponibilidad;
    char *genero;
    char *fechaPublicacion;
    int deColeccion;
    struct NodoReserva *headReserva;
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
    struct pasesDelDia *filaPasesDeldia;
    int recaudacion;
    char *hora_inicio;
    char *hora_termino;
    char *fecha;
};

#endif
