#include "../../include/clientes.h"
#include "../../include/entrada.h"
#include "../../include/entradaInterna.h"
#include "../../include/pases.h"
#include "../../include/reservas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* === CLIENTE === */

/* #define MAXPASES 50 */

struct Cliente *crearCliente(char *nombre, int rut)
{
    struct Cliente *nuevo;
    int i;

    if (nombre == NULL) {
        return NULL;
    }

    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }

    nuevo->nombre = malloc(strlen(nombre) + 1);
    if (nuevo->nombre == NULL) {
        free(nuevo);
        return NULL;
    }
    strcpy(nuevo->nombre, nombre);
    nuevo->rut=rut;

    nuevo->pases = malloc(MAXPASES * sizeof(*nuevo->pases));
    if (nuevo->pases == NULL) {
        free(nuevo->nombre);
        free(nuevo);
        return NULL;
    }
    for(i=0; i<MAXPASES; i++)
    {
        nuevo->pases[i]=NULL;
    }
    nuevo->plibrepases=0;

    return nuevo;
}

struct NodoCliente *crearNodoCliente(struct Cliente *dato) /* para meter en lista */
{
    struct NodoCliente *nuevo;

    if (dato == NULL) {
        return NULL;
    }

    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->dato=dato;
    nuevo->sig=NULL;

    return nuevo;
}

struct NodoCliente *buscarCliente(struct NodoCliente *head, int rut) {
    struct NodoCliente *rec=head;
    if(rec!=NULL)
    {
        do
        {
            if(rec->dato->rut==rut)
            {
                return rec;
            }
            rec=rec->sig;
        }while(rec!=head);
    }
    return NULL;
}
void enlazarCliente(struct NodoCliente **head, struct NodoCliente *nuevo)
{
    struct NodoCliente *rec;

    if(*head==NULL)
    {
        *head=nuevo;
        nuevo->sig=*head;
    }
    else
    {
        rec=*head;
        while(rec->sig!=*head)
        {
            rec=rec->sig;
        }
        rec->sig=nuevo;
        nuevo->sig=*head;
    }
}

int agregarCliente(struct NodoCliente **head, char *nombre, int rut) /* retorna 1 si lo agrego, 0 si el rut ya existia */
{
    struct Cliente *cliente;
    struct NodoCliente *nodo;

    if (head == NULL || nombre == NULL) {
        return 0;
    }
    if (buscarCliente(*head, rut)!=NULL) {
        return 0;
    }

    cliente = crearCliente(nombre, rut);
    if (cliente == NULL) {
        return 0;
    }
    nodo = crearNodoCliente(cliente);
    if (nodo == NULL) {
        free(cliente->nombre);
        free(cliente->pases);
        free(cliente);
        return 0;
    }

    enlazarCliente(head, nodo);
    return 1;
}

int registrarPaseCliente(struct Tienda *tienda, int rut, struct Pase *pase) {
    struct NodoCliente *nodoCliente;
    struct NodoCliente *iterCliente;
    int apertura;
    int cierre;
    int posicionLibre = -1;
    int cantidad = 0;
    int i;

    if (tienda == NULL || pase == NULL || pase->fecha == NULL ||
        pase->codigo <= 0 || pase->tipoPase < 1 || pase->tipoPase > 4 ||
        pase->estado != 1 || !validarFechaFila(tienda->fecha) ||
        !paseValidoHoy(pase, tienda->fecha) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura) {
        return 0;
    }

    nodoCliente = buscarCliente(tienda->headClientes, rut);
    if (nodoCliente == NULL || nodoCliente->dato == NULL ||
        nodoCliente->dato->pases == NULL ||
        buscarPase(tienda->filaPasesDeldia, pase->codigo) != NULL) {
        return 0;
    }

    iterCliente = tienda->headClientes;
    if (iterCliente != NULL) {
        do {
            if (iterCliente->dato != NULL &&
                iterCliente->dato->pases != NULL) {
                for (i = 0; i < MAXPASES; i++) {
                    struct Pase *existente = iterCliente->dato->pases[i];

                    if (iterCliente == nodoCliente && existente == NULL &&
                        posicionLibre < 0) {
                        posicionLibre = i;
                    }
                    if (existente != NULL &&
                        existente->codigo == pase->codigo) {
                        return 0;
                    }
                    if (iterCliente == nodoCliente && existente != NULL) {
                        cantidad++;
                    }
                }
            }
            iterCliente = iterCliente->sig;
        } while (iterCliente != NULL && iterCliente != tienda->headClientes);
    }

    if (posicionLibre < 0) {
        return 0;
    }

    nodoCliente->dato->pases[posicionLibre] = pase;
    nodoCliente->dato->plibrepases = cantidad + 1;
    return 1;
}

void liberarPasesClientes(struct NodoCliente *head) {
    struct NodoCliente *actual = head;
    int i;

    if (head == NULL) {
        return;
    }

    do {
        if (actual->dato != NULL && actual->dato->pases != NULL) {
            for (i = 0; i < MAXPASES; i++) {
                liberarPase(actual->dato->pases[i]);
                actual->dato->pases[i] = NULL;
            }
            actual->dato->plibrepases = 0;
        }
        actual = actual->sig;
    } while (actual != NULL && actual != head);
}

/* busca recursivamente si el cliente tiene una reserva en algun libro del arbol */
/* retorna 1 si encuentra una reserva con ese rut, 0 si no */

static int tieneReservaArbol(struct NodoLibro *raiz, int rut) {

    if (raiz == NULL) {
        return 0;
    }

    if (buscarReserva(raiz->dato, rut) != NULL) {
        return 1;
    }

    return tieneReservaArbol(raiz->izq, rut) || tieneReservaArbol(raiz->der, rut);
}

int eliminarReservasClienteArbol(struct NodoLibro *raiz, int rut) {
    if (raiz == NULL) {
        return 0;
    }

    return eliminarReservasClienteArbol(raiz->izq, rut) +
           eliminarReserva(raiz->dato, rut) +
           eliminarReservasClienteArbol(raiz->der, rut);
}

/* === ELIMIMNAR PASE === */

/* elimina un pase del arreglo de pases de un cliente y compacta el arreglo */
/* no se elimina si el pase esta esperando en la fila o ya ingreso (esta en el registro del dia) */
/* retorna 1 si se elimino, 0 si el cliente o el pase no existen, */
/* -1 si el pase esta en la fila, -2 si el pase ya ingreso */

int eliminarPase(struct Tienda *tienda, int rut, int codigo) {
    struct NodoCliente *nodo;
    struct Cliente *c;
    int i, pos = -1;

    if (tienda == NULL) {
        return 0;
    }

    nodo = buscarCliente(tienda->headClientes, rut);

    if (nodo == NULL) {
        return 0;
    }

    c = nodo->dato;
    if (c == NULL || c->pases == NULL) {
        return 0;
    }

    /* buscar la posicion del pase en el arreglo del cliente */
    for (i = 0; i < MAXPASES && pos < 0; i++) {

        if (c->pases[i] != NULL && c->pases[i]->codigo == codigo) {
            pos = i;
        }
    }

    if (pos < 0) {

        return 0; /* el cliente no tiene ese pase */
    }

    if (buscarEnFila(tienda->filaEntrada, codigo) != NULL) {

        return -1; /* esta esperando en la fila */
    }

    if (buscarPase(tienda->filaPasesDeldia, codigo) != NULL) {
        return -2; /* ya ingreso, el cliente esta dentro */
    }

    /* no se si esto esta correcto */
    /* el pase tiene 3 bloques de memoria (fecha, hora y el struct), cada uno con su free */
    /* primero los textos y al final el struct, porque el struct es quien los apunta */
    free(c->pases[pos]->fecha);
    free(c->pases[pos]->hora);
    free(c->pases[pos]);

    /* cada pase siguiente retrocede una posicion y tapa el eliminado */
    for (i = pos; i < MAXPASES - 1; i++) {
        c->pases[i] = c->pases[i + 1];
    }

    /* el for no toca la ultima posicion, queda con una copia repetida, se vacia con NULL */
    c->pases[MAXPASES - 1] = NULL;
    if (c->plibrepases > 0) {
        c->plibrepases--;
    }

    return 1;
}

/* elimina un cliente solo si no esta en la fila, dentro de la tienda, no tiene reservas ni pases activos */
/* al eliminar un cliente (nodo), el nodo anterior debe saltarse el eliminado. */
/* retorna 1 si se elimino, 0 si el cliente no existe, */
/* -1 si tiene un pase en la fila, -2 si esta dentro de la tienda (un pase ya ingreso), */
/* -3 si tiene una reserva, -4 si tiene un pase activo disponible */

int eliminarCliente(struct Tienda *tienda, int rut) {
    struct NodoCliente *act, *rec; /* rec esta justo antes de act */
    struct NodoSala *sala;
    struct Cliente *c;
    int i;

    if (tienda == NULL || tienda->headClientes == NULL) {
        return 0;
    }

    act = buscarCliente(tienda->headClientes, rut);

    if (act == NULL) {
        return 0; /* no se encontro */
    }

    c = act->dato;

    /* revisar sus pases: en fila o ya ingresados */
    for (i = 0; i < MAXPASES; i++) {
        if (c->pases[i] != NULL) {
            if (buscarEnFila(tienda->filaEntrada, c->pases[i]->codigo) != NULL) {
                return -1;
            }
            if (buscarPase(tienda->filaPasesDeldia, c->pases[i]->codigo) != NULL) {
                return -2;
            }
        }
    }

    /* revisar sus reservas en todos los libros de todas las salas */
    for (sala = tienda->headSalas; sala != NULL; sala = sala->sig) {
        if (tieneReservaArbol(sala->dato->raiz, rut)) {
            return -3;
        }
    }

    /* revisar que no tenga pases activos disponibles (estado 1) */
    for (i = 0; i < MAXPASES; i++) {
        if (c->pases[i] != NULL && c->pases[i]->estado == 1) {
            return -4;
        }
    }

    /* desenlazar de la lista circular */
    if (act->sig == act) { /* unico nodo -> lista queda vacia */
        tienda->headClientes = NULL;
    }
    else {
        rec = tienda->headClientes;
        while (rec->sig != act) { /* rec queda justo antes del nodo a eliminar */
            rec = rec->sig;
        }
        rec->sig = act->sig; /* el anterior apunta a lo que apuntaba el eliminado */

        if (act == tienda->headClientes) { /* si era head, el head pasa al siguiente */
            tienda->headClientes = act->sig;
        }
    }

    /* no se si esta bien */
    /* liberar memoria: pases restantes (anulados o vencidos), nombre, arreglo, cliente y nodo */
    for (i = 0; i < MAXPASES; i++) {
        if (c->pases[i] != NULL) {
            free(c->pases[i]->fecha);
            free(c->pases[i]->hora);
            free(c->pases[i]);
        }
    }

    free(c->nombre);
    free(c->pases);
    free(c);
    free(act);

    return 1;
}

/* retorna 1 si se modifico, 0 si el cliente no existe o el nombre es NULL, */
/* -1 si el rut nuevo ya pertenece a otro cliente, -2 si no hay memoria */

int modificarCliente(struct NodoCliente *head, int rut, char *nuevoNombre, int nuevoRut) {

    struct NodoCliente *nodo;
    char *copia;

    if(nuevoNombre==NULL) {
        return 0;
    }

    nodo=buscarCliente(head, rut);

    if(nodo==NULL) {
        return 0;
    }

    if(nuevoRut!=rut && buscarCliente(head, nuevoRut)!=NULL) { /* rut no puede pertenecer a otro cliente */
        return -1;
    }

    /* reservar espacio para el nombre nuevo */
    copia=((char *)malloc((strlen(nuevoNombre)+1)*sizeof(char)));

    /* revisar si hubo memoria, si no hay el cliente conserva su nombre */
    if(copia==NULL) {
        return -2;
    }

    strcpy(copia, nuevoNombre); /*  copia el nuevo nombre al espacio neuvo */

    free(nodo->dato->nombre); /* se libera el nombre viejo */

    nodo->dato->nombre=copia; /* el cliente pasa a apuntar al nombre nuevo */
    nodo->dato->rut=nuevoRut;

    return 1;
}

void listarClientes(struct NodoCliente *head) { /* mostrar clientes */

    struct NodoCliente *rec=head;
    int cantidad=0;

    if(rec==NULL) {
        printf("No hay clientes registrados.\n");
    }
    else {
        do
        {
            cantidad++;
            printf("%d) Nombre: %s, RUT: %d, Pases: %d\n",
                   cantidad, rec->dato->nombre, rec->dato->rut,
                   rec->dato->plibrepases);
            rec=rec->sig;
        } while(rec!=head);
    }
}
