#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#define MAXPASES 50

struct Pase {
    int tipoPase;
    int codigo;
    int valor;
    /* Estado: 1 activo, 2 anulado, 3 utilizado, 4 vencido. */
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

struct ListaPases {
    struct NodoPase *head;
    struct NodoPase *tail;
};

struct Fila {
    struct NodoPase *head;
    struct NodoPase *tail;
};

struct Cliente {
    char *nombre;
    struct Pase *pases[MAXPASES];
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
    char *hora_inicio; /* Hora de apertura de la tienda. HH:MM*/
    char *hora_termino; /* Hora de cierre de la tienda. HH:MM*/
    char *fecha; /* Fecha del dia de operacion. DD/MM/AAAA */
};

#endif
