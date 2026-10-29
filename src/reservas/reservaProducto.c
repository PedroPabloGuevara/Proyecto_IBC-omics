#include <stdio.h>
#include <stdlib.h>

#include "../../include/clientes.h"
#include "../../include/reservas.h"


/*
-crea una reserva en memoria dinamica
-guarda un puntero al cliente que reserva, no lo copia ni pasa a ser propietario de el
-guarda el tipo de pase: infantil (1), familiar (2), general (3), prioritario (4)
-si el cliente es NULL o falla la reserva de memoria retorna NULL
-retorna el puntero a la reserva creada
*/
struct Reserva *crearReserva(struct Cliente *reservador, int tipoPase) {
    struct Reserva *nuevaReserva;

    /* La reserva solo referencia al cliente; la vida del cliente se administra aparte. */
    if (reservador == NULL) {
        return NULL;
    }

    nuevaReserva = malloc(sizeof(*nuevaReserva));
    if (nuevaReserva == NULL) {
        return NULL;
    }

    nuevaReserva->reservador = reservador;
    nuevaReserva->tipoPase = tipoPase;
    return nuevaReserva;
}

/*
-envuelve una reserva en un nodo para poder meterla en la cola
-el nodo nace con sig en NULL
-si la reserva es NULL o falla la reserva de memoria retorna NULL
-retorna el puntero al nodo creado
*/
struct NodoReserva *crearNodoReserva(struct Reserva *dato) {
    struct NodoReserva *nuevoNodo;

    if (dato == NULL) {
        return NULL;
    }

    nuevoNodo = malloc(sizeof(*nuevoNodo));
    if (nuevoNodo == NULL) {
        return NULL;
    }

    /* Inicializa el enlace para que el nodo pueda insertarse en una cola vacia. */
    nuevoNodo->dato = dato;
    nuevoNodo->sig = NULL;
    return nuevoNodo;
}

/*
-registra un ejemplar reservado para un cliente en un libro en preventa
-solo permite una reserva por cliente y libro; el pase asociado determina prioridad
-retorna 1 si se agrego y 0 si la reserva no fue valida o falto memoria
*/
int reservarLibro(struct Libro *libro, struct Cliente *cliente,
                  struct Pase *pase) {
    struct Reserva *reserva;
    struct NodoReserva *nodo;
    int esPaseDelCliente = 0;
    int i;

    /* Valida el estado del libro, la credencial y que no exista otra reserva. */
    if (libro == NULL || cliente == NULL || pase == NULL ||
        libro->disponibilidad != 3 ||
        (pase->estado != 1 && pase->estado != 3) ||
        pase->tipoPase < 1 || pase->tipoPase > 4 ||
        buscarReserva(libro, cliente->rut) != NULL) {
        return 0;
    }

    /* La identidad del pase se verifica por referencia dentro del arreglo del cliente. */
    for (i = 0; i < MAXPASES; i++) {
        if (cliente->pases[i] == pase) {
            esPaseDelCliente = 1;
            break;
        }
    }
    if (!esPaseDelCliente) {
        return 0;
    }

    /* Crea los recursos por etapas; si falla una asignacion o el encolado, retorna 0. */
    reserva = crearReserva(cliente, pase->tipoPase);
    if (reserva == NULL) {
        return 0;
    }

    nodo = crearNodoReserva(reserva);
    if (nodo == NULL) {
        free(reserva);
        return 0;
    }

    if (!encolar(libro, nodo)) {
        free(nodo);
        free(reserva);
        return 0;
    }
    return 1;
}


/*
-agrega una reserva a la lista del libro segun su tipo de pase
-normal (tipo distinto de 4): va al final de la cola
-prioritario (tipo 4): va delante de los normales, pero detras de los prioritarios que ya estaban
-si el libro, el nodo o la reserva son NULL, no realiza cambios
-el nodo debe estar desligado de cualquier lista
*/
int encolar(struct Libro *libro, struct NodoReserva *nuevo) {
    struct NodoReserva *rec;

    /* No acepta nodos incompletos, ya enlazados ni reservas fuera de preventa. */
    if (libro == NULL || nuevo == NULL || nuevo->dato == NULL ||
        nuevo->dato->reservador == NULL || libro->disponibilidad != 3 ||
        nuevo->dato->tipoPase < 1 || nuevo->dato->tipoPase > 4 ||
        nuevo->sig != NULL) {
        return 0;
    }

    /* Evita insertar el mismo nodo o duplicar una reserva del mismo cliente. */
    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        if (rec == nuevo ||
            (rec->dato != NULL && rec->dato->reservador != NULL &&
            rec->dato->reservador->rut == nuevo->dato->reservador->rut)) {
            return 0;
        }
    }

    /* caso 1: lista vacia, el nuevo nodo queda como head */
    if (libro->headReserva == NULL) {
        libro->headReserva = nuevo;
    }
    /* caso 2: reserva normal, se agrega al final */
    else if (nuevo->dato->tipoPase != 4) {
        rec = libro->headReserva;
        while (rec->sig != NULL) {
            rec = rec->sig;
        }
        rec->sig = nuevo;
    }
    /* caso 3: prioritario y el primero es normal, pasa a ser el nuevo head */
    else if (libro->headReserva->dato->tipoPase != 4) {
        nuevo->sig = libro->headReserva;
        libro->headReserva = nuevo;
    }
    /* caso 4: se inserta despues del ultimo prioritario */
    else {
        rec = libro->headReserva;
        while (rec->sig != NULL && rec->sig->dato->tipoPase == 4) {
            rec = rec->sig;
        }

        nuevo->sig = rec->sig;
        rec->sig = nuevo;
    }
    return 1;
}

/*
-atiende al primero de la cola: lo desengancha y lo retorna
-si el libro es NULL o no tiene reservas, retorna NULL
-quien llama decide que hace con el nodo y la reserva cuando termine de usarlos
*/
struct NodoReserva *desencolar(struct Libro *libro) {
    struct NodoReserva *aux;

    if (libro == NULL || libro->headReserva == NULL) {
        return NULL;
    }

    /* se guarda el primero y se mueve head al nodo siguiente */
    /* Retira head y deja el nodo desligado (sig en NULL) antes de retornarlo. */
    aux = libro->headReserva;
    libro->headReserva = aux->sig;

    aux->sig = NULL;
    return aux;
}

/*
-retorna cuantas personas esperan en la cola
-si el libro es NULL retorna 0
*/
int cantidadEnEspera(struct Libro *libro) {
    struct NodoReserva *rec;
    int cantidad = 0;

    if (libro == NULL) {
        return 0;
    }

    /* Cuenta nodos sin cambiar el orden ni los enlaces de la cola. */
    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        cantidad++;
    }

    return cantidad;
}

/*
-muestra las reservas en orden de atencion
-imprime posicion, nombre del cliente y si es prioritaria
-si el libro no tiene reservas, informa que no hay clientes esperando
*/
void mostrarCola(struct Libro *libro) {
    struct NodoReserva *rec;
    int pos = 1;

    if (libro == NULL || libro->headReserva == NULL) {
        printf("No hay clientes en lista de espera\n");
        return;
    }

    /* Recorre la cola en el orden en que deben atenderse las reservas. */
    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        printf("%d) %s", pos, rec->dato->reservador->nombre);
        if (rec->dato->tipoPase == 4) {
            printf(" (prioritario)");
        }
        printf("\n");
        pos++;
    }
}

/*
-busca la reserva de un cliente en el libro por su rut
-retorna el nodo si lo encuentra, o NULL si no esta
*/
struct NodoReserva *buscarReserva(struct Libro *libro, int rut) {
    struct NodoReserva *rec;

    if (libro == NULL) {
        return NULL;
    }

    /* Busca por RUT y tolera nodos incompletos sin desreferenciar sus datos. */
    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        if (rec->dato != NULL && rec->dato->reservador != NULL &&
            rec->dato->reservador->rut == rut) {
            return rec;
        }
    }

    return NULL;
}

/*
-elimina de la lista del libro la reserva de un cliente, este donde este
-libera el nodo y la reserva, pero no libera al cliente
-retorna 1 si la elimino, 0 si no estaba o el libro es NULL
*/
int eliminarReserva(struct Libro *libro, int rut) {
    struct NodoReserva *rec;
    struct NodoReserva *ant = NULL;

    if (libro == NULL) {
        return 0;
    }

    /* Localiza la reserva y conserva el nodo anterior para reconectar la cola. */
    rec = libro->headReserva;
    while (rec != NULL &&
        (rec->dato == NULL || rec->dato->reservador == NULL || rec->dato->reservador->rut != rut)) {
        ant = rec;
        rec = rec->sig;
    }

    if (rec == NULL) {
        return 0;
    }

    /* Desenlaza el nodo desde la cabeza o desde su predecesor. */
    if (ant == NULL) {
        libro->headReserva = rec->sig;
    } else {
        ant->sig = rec->sig;
    }

    /* La cola es propietaria de la reserva y su nodo, no del cliente. */
    free(rec->dato);
    free(rec);
    return 1;
}

/*
-deja vacia la lista del libro liberando sus reservas y nodos
-al final headReserva queda en NULL y el libro puede recibir nuevas reservas
*/
void vaciarCola(struct Libro *libro) {
    struct NodoReserva *rec;
    struct NodoReserva *sig;

    if (libro == NULL) {
        return;
    }

    /* Guarda el siguiente nodo antes de liberar la reserva y el nodo actual. */
    rec = libro->headReserva;
    while (rec != NULL) {
        sig = rec->sig;
        free(rec->dato);
        free(rec);
        rec = sig;
    }

    libro->headReserva = NULL;
}

/*
-modifica la reserva de un cliente cambiando el pase con que reserva
-el nuevo pase debe pertenecer al cliente y estar activo o utilizado, igual que en reservarLibro
-el tipo del pase determina la prioridad, por eso la reserva se vuelve a ubicar en la lista
-si el tipo de pase no cambia, la reserva conserva su posicion
-no reserva ni libera memoria: reutiliza el mismo nodo y la misma reserva
-recibe el libro donde esta la reserva, el cliente que reservo y el pase nuevo
-ejemplo: lista 1,2,3,4 (todos con pase general); el cliente 3 cambia a un pase
prioritario y la lista queda 3,1,2,4
-retorna 1 si se modifico, 0 si el libro no esta en preventa, el cliente no tiene reserva o el pase no es valido
*/
int modificarReserva(struct Libro *libro, struct Cliente *cliente, struct Pase *nuevoPase) {
    struct NodoReserva *nodo;           /* nodo de la reserva que se quiere modificar */
    struct NodoReserva *ant = NULL;     /* nodo anterior, necesario para desenlazar */
    int esPaseDelCliente = 0;           /* 1 si el pase esta en el arreglo del cliente */
    int i;

    /* Primera etapa: validar los datos, sin modificar nada todavia.
    - el libro debe existir y estar en preventa (3), porque solo ahi hay reservas
    - el cliente debe existir y tener su arreglo de pases
    - el pase debe existir, estar activo (1) o utilizado (3) y tener tipo entre 1 y 4
    son las mismas reglas de reservarLibro, para que ambas funciones sean coherentes */
    if (libro == NULL || cliente == NULL || nuevoPase == NULL ||
        libro->disponibilidad != 3 ||
        (nuevoPase->estado != 1 && nuevoPase->estado != 3) ||
        nuevoPase->tipoPase < 1 || nuevoPase->tipoPase > 4) {
        return 0;
    }

    /* Segunda etapa: comprobar que el pase le pertenece a este cliente.
    se compara la direccion de memoria (no el codigo) con cada pase del arreglo,
    asi un cliente no puede usar el pase de otra persona para subir de prioridad */
    for (i = 0; i < MAXPASES; i++) {
        if (cliente->pases[i] == nuevoPase) {
            esPaseDelCliente = 1;
            break;      /* ya se encontro, no hace falta seguir revisando */
        }
    }
    if (!esPaseDelCliente) {
        return 0;
    }

    /* Tercera etapa: buscar la reserva del cliente en la lista del libro.
    se avanza nodo por nodo comparando el rut, y se va guardando el anterior (ant)
    porque para sacar un nodo de una lista enlazada hay que reconectar el anterior;
       las condiciones con NULL evitan fallar si algun nodo esta incompleto */
    nodo = libro->headReserva;
    while (nodo != NULL &&
        (nodo->dato == NULL || nodo->dato->reservador == NULL || nodo->dato->reservador->rut != cliente->rut)) {
        ant = nodo;
        nodo = nodo->sig;
    }

    /* si el ciclo llego al final sin encontrar nada, el cliente no tiene reserva */
    if (nodo == NULL) {
        return 0;
    }

    /* Cuarta etapa: si el tipo de pase es el mismo, la prioridad no cambia.
       no hay nada que reubicar, asi que la reserva conserva su posicion actual */
    if (nodo->dato->tipoPase == nuevoPase->tipoPase) {
        return 1;
    }

    /* Quinta etapa: sacar el nodo de la lista sin liberarlo.
    - si ant es NULL, el nodo era el primero: head pasa a apuntar al segundo
    - si no, el nodo anterior salta al siguiente, dejando fuera al nodo
       el nodo sigue existiendo en memoria, solo deja de estar enlazado */
    if (ant == NULL) {
        libro->headReserva = nodo->sig;
    } else {
        ant->sig = nodo->sig;
    }

    /* se corta el enlace del nodo con el resto, porque encolar solo acepta
    nodos que no estan enlazados a ninguna lista (sig en NULL) */
    nodo->sig = NULL;

    /* Sexta etapa: actualizar el tipo y volver a ubicar el nodo.
    primero se cambia el tipo de pase de la reserva, y luego encolar decide
    su lugar con la regla de siempre: los prioritarios (4) van delante de los
    normales pero detras de otros prioritarios, y los demas al final.
    encolar no puede fallar aqui: el libro esta en preventa, el tipo es valido,
    el cliente existe y el nodo ya no esta en la lista, asi que no hay duplicados */
    nodo->dato->tipoPase = nuevoPase->tipoPase;
    return encolar(libro, nodo);
}