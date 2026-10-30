#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXPASES 50
#define NOMBRE_BOVEDA "Boveda de Coleccionistas"


struct Pase {
    int tipoPase;   /* infantil (1), familiar(2), general(3), prioritario(4) */
    int codigo;     /* codigo unico */
    int valor;
    int estado;     /* activo (1), anulado(2), utilizado(3), vencido(4) */
    char *fecha;
    char *hora;
};

struct NodoPase {
    struct Pase *paseVigente; /* Puntero al pase almacenado en el nodo */
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
    struct Pase *pases[MAXPASES];
    int rut;
    int plibrepases;

};

struct NodoCliente {
    struct Cliente *dato;
    struct NodoCliente *sig;
};

struct Reserva {
    struct Cliente *reservador;
    int tipoPase;
};
struct NodoReserva{
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
    int disponibilidad; /*estados estilo */
    char *genero;
    char *fechaPublicacion;
    int deColeccion;
    struct NodoReserva *headReserva;/*lista FIFO*/
};

struct NodoLibro {
    struct Libro *dato;
    struct NodoLibro *izq;
    struct NodoLibro *der;
};


struct SalaTematica {
    char *tematica;
    char *nombre;
    char *horarioInicio;    /* formato hh:mm */
    char *horarioCierre;    /* formato hh:mm */
    int aforoMaximo;
    int codigo;
    int aforoActual;
    struct NodoLibro *raiz;
};

struct NodoSala {
    struct SalaTematica *dato;
    struct NodoSala *sig;
    struct NodoSala *ant;
};

struct Tienda {
    struct NodoSala *headSalas;   /* Lista doble de salas */
    struct NodoCliente *headClientes;      /* Lista circular de clientes */
    struct Fila *filaEntrada;                   /* fila de ingreso FIFO */
    struct pasesDelDia *filaPasesDeldia;            /* fila/lista de pases del dia */
    int recaudacion;
    char *hora_inicio;
    char *hora_termino;
    char *fecha;
};

void vaciarCola(struct Libro *libro);
void liberarLibro(struct Libro *libro);
int cantidadEnEspera(struct Libro *libro);
struct NodoReserva *buscarReserva(struct Libro *libro, int rut);
int eliminarReserva(struct Libro *libro, int rut);
int encolar(struct Libro *libro, struct NodoReserva *nuevo);
struct NodoPase *buscarEnFila(struct Fila *fila, int codigoPase);
struct NodoPase *buscarPaseDelDia(struct pasesDelDia *lista, int codigo);
void liberarPase(struct Pase *pase);
char *copiarTexto(char *texto);


/*funciones salas */
/*=======================================================================================================*/
/*
-crea una sala tematica en memoria dinamica, con copia propia de sus 4 textos
-los horarios van en formato hh:mm y la apertura debe ser anterior al cierre
-el aforo actual parte en 0 y el aforo maximo debe ser al menos 1
-retorna la sala, o NULL si algun dato no es valido o falta memoria
*/
struct SalaTematica *crearSala(char *tematica, char *nombre, char *horarioInicio, char *horarioCierre, int codigo, int aforoMaximo){

    struct SalaTematica *nuevo;

    if (tematica == NULL || nombre == NULL || horarioInicio == NULL ||
        horarioCierre == NULL || aforoMaximo < 1 ||
        strcmp(horarioInicio, horarioCierre) >= 0) {
        return NULL;
    }

    nuevo = (struct SalaTematica*)malloc(sizeof(struct SalaTematica));
    if (nuevo == NULL) {
        return NULL;
    }

    nuevo->tematica = copiarTexto(tematica);
    nuevo->nombre = copiarTexto(nombre);
    nuevo->horarioInicio = copiarTexto(horarioInicio);
    nuevo->horarioCierre = copiarTexto(horarioCierre);
    if (nuevo->tematica == NULL || nuevo->nombre == NULL ||
        nuevo->horarioInicio == NULL || nuevo->horarioCierre == NULL) {
        free(nuevo->tematica);          /* free(NULL) es valido */
        free(nuevo->nombre);
        free(nuevo->horarioInicio);
        free(nuevo->horarioCierre);
        free(nuevo);
        return NULL;
    }
    nuevo->codigo = codigo;
    nuevo->aforoMaximo = aforoMaximo;
    nuevo->aforoActual = 0;
    nuevo->raiz = NULL;

    return nuevo;
}


struct NodoSala *crearNodoSala(struct SalaTematica *nuevaSala){

    struct NodoSala *nuevoNodo;

    nuevoNodo = (struct NodoSala*) malloc(sizeof(struct NodoSala));
    nuevoNodo->dato = nuevaSala;
    nuevoNodo->sig = NULL;
    nuevoNodo->ant = NULL;

    return nuevoNodo;
}

void insertarSala(struct NodoSala**headSalas,struct NodoSala *nuevaSala) {

    struct NodoSala *rec;

    if (*headSalas == NULL) {
        *headSalas = nuevaSala;
    }else {
        rec = *headSalas;
        while (rec->sig != NULL) {
            rec = rec->sig;
        }
        rec->sig = nuevaSala;
        rec->sig->ant = rec;
    }
}

struct NodoSala *buscarSala(struct NodoSala*headSalas,int codigo) {

    struct NodoSala *rec;

    if (headSalas == NULL) {
        return NULL;
    }
    rec = headSalas;
    while (rec != NULL) {
        if (rec->dato->codigo == codigo) {
            return rec;
        }
        rec = rec->sig;
    }
    return NULL;
}
void vaciarCola(struct Libro *libro) {

    struct NodoReserva *rec;
    struct NodoReserva *sig;

    if (libro == NULL) {
        return;
    }

    rec = libro->headReserva;
    while (rec != NULL) {
        sig = rec->sig;          /* guardar el siguiente ANTES de liberar */
        free(rec->dato);         /* la reserva (NO el cliente) */
        free(rec);               /* el nodo */
        rec = sig;
    }
    libro->headReserva = NULL;   /* la cola queda vacia */
}
void eliminarArbolLibros(struct NodoLibro *raiz) {

    if (raiz == NULL) {
        return;
    }
    eliminarArbolLibros(raiz->izq);
    eliminarArbolLibros(raiz->der);
    vaciarCola(raiz->dato);          /* libera las reservas del libro */
    free(raiz->dato->nombre);
    free(raiz->dato->publicacion);
    free(raiz->dato->genero);
    free(raiz->dato->fechaPublicacion);
    free(raiz->dato);                /* el libro */
    free(raiz);                      /* el nodo del arbol */
}
int EliminarSala(struct NodoSala **headSalas,int codigo) {

    struct NodoSala *rec;
    struct NodoSala *elim;

    if (*headSalas == NULL) {
        return -1;
    }
    rec = *headSalas;
    if (rec->dato->codigo == codigo) {
        elim = rec;
        *headSalas= rec->sig;
        if (*headSalas != NULL) {
            (*headSalas)->ant = NULL;
        }
        eliminarArbolLibros(elim->dato->raiz);
        free(elim->dato->tematica);
        free(elim->dato->nombre);
        free(elim->dato->horarioInicio);
        free(elim->dato->horarioCierre);
        free(elim->dato);
        free(elim);
        return 1;
    }else {
        while (rec->sig != NULL){
            if (rec->sig->dato->codigo == codigo) {
                elim = rec->sig;
                if (rec->sig->sig != NULL) {
                    rec->sig->sig->ant=rec;
                }
                rec->sig = rec->sig->sig;
                eliminarArbolLibros(elim->dato->raiz);
                free(elim->dato->tematica);
                free(elim->dato->nombre);
                free(elim->dato->horarioInicio);
                free(elim->dato->horarioCierre);
                free(elim->dato);
                free(elim);
                return 1;
            }
            rec=rec->sig;
        }
    }
    return 0;
}

void mostrarTodasSalas(struct NodoSala *headSalas){

    while (headSalas != NULL) {
        printf("Sala %d: %s, tematica: %s, horario: %s a %s, aforo: %d/%d\n",
               headSalas->dato->codigo, headSalas->dato->nombre, headSalas->dato->tematica,
               headSalas->dato->horarioInicio, headSalas->dato->horarioCierre,
               headSalas->dato->aforoActual, headSalas->dato->aforoMaximo);
        headSalas = headSalas->sig;
    }
}

/*
-modifica codigo, aforo maximo, tematica, nombre y horario de una sala
-el aforo actual no se modifica aqui y el nuevo aforo maximo no puede ser menor a el
-si algo falla no se toca ningun campo
-retorna 1 si se modifico, 0 si la sala no existe, el codigo ya esta en uso o los datos no son validos
*/
int modificarSala(struct NodoSala **headSalas, int codigo, int nuevoCodigo, int nuevoAforoMaximo, char *nuevaTematica, char *nuevoNombre, char *nuevoHorarioInicio, char *nuevoHorarioCierre) {

    struct NodoSala *rec;
    char *copiaTematica;
    char *copiaNombre;
    char *copiaInicio;
    char *copiaCierre;

    if (*headSalas == NULL) {
        return 0;
    }
    if (nuevaTematica == NULL || nuevoNombre == NULL || nuevoHorarioInicio == NULL ||
        nuevoHorarioCierre == NULL || strcmp(nuevoHorarioInicio, nuevoHorarioCierre) >= 0) {
        return 0;
    }
    if (nuevoCodigo != codigo) {
        rec = buscarSala(*headSalas, nuevoCodigo);
        if (rec != NULL) {
            return 0; /* el nuevo codigo ya esta en uso */
        }
    }
    rec = buscarSala(*headSalas, codigo);
    if (rec == NULL) {
        return 0; /* la sala a modificar no existe */
    }
    if (nuevoAforoMaximo < 1 || nuevoAforoMaximo < rec->dato->aforoActual) {
        return 0; /* el aforo maximo no puede quedar bajo la gente que ya esta dentro */
    }

    copiaTematica = copiarTexto(nuevaTematica);
    copiaNombre = copiarTexto(nuevoNombre);
    copiaInicio = copiarTexto(nuevoHorarioInicio);
    copiaCierre = copiarTexto(nuevoHorarioCierre);
    if (copiaTematica == NULL || copiaNombre == NULL || copiaInicio == NULL || copiaCierre == NULL) {
        free(copiaTematica);
        free(copiaNombre);
        free(copiaInicio);
        free(copiaCierre);
        return 0; /* no se toco ningun campo */
    }

    free(rec->dato->tematica); /* se liberan los textos viejos */
    free(rec->dato->nombre);
    free(rec->dato->horarioInicio);
    free(rec->dato->horarioCierre);
    rec->dato->tematica = copiaTematica;
    rec->dato->nombre = copiaNombre;
    rec->dato->horarioInicio = copiaInicio;
    rec->dato->horarioCierre = copiaCierre;
    rec->dato->codigo = nuevoCodigo;
    rec->dato->aforoMaximo = nuevoAforoMaximo;
    return 1;
}

int cancelarReserva(struct Libro *libro, int rut) {
    if (libro == NULL || libro->disponibilidad == 4 ||
        libro->ejemplaresDisponibles < 0 ||
        (unsigned int)libro->ejemplaresDisponibles >=
            ((unsigned int)-1 >> 1) ||
        buscarReserva(libro, rut) == NULL) {
        return 0;
    }

    if (!eliminarReserva(libro, rut)) {
        return 0;
    }
    libro->ejemplaresDisponibles++;
    if (libro->disponibilidad == 2) {
        libro->disponibilidad = 1;
    }
    return 1;
}

/*=======================================================================================================*/

/*funciones cliente*/
/*=======================================================================================================*/

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
        free(cliente);
        return 0;
    }
    enlazarCliente(head, nodo);
    return 1;
}

/* busca recursivamente si el cliente tiene una reserva en algun libro del arbol */
/* retorna 1 si encuentra una reserva con ese rut, 0 si no */
int tieneReservaArbol(struct NodoLibro *raiz, int rut) {
    if (raiz == NULL) {
        return 0;
    }
    if (buscarReserva(raiz->dato, rut) != NULL) {
        return 1;
    }
    return tieneReservaArbol(raiz->izq, rut) ||
           tieneReservaArbol(raiz->der, rut);
}

/* elimina un cliente solo si no esta en la fila, dentro de la tienda y no tiene reservas */
/* al eliminar un cliente (nodo), el nodo anterior debe saltarse el eliminado. todos los nodos aqui solo tienen "sig" */
/* "ant" sirve para recordar cual es el nodo anterior mientras recorre */
/* retorna 1 si se elimino, 0 si el cliente no existe, */
/* -1 si tiene un pase en la fila, -2 si esta dentro de la tienda (un pase ya ingreso), */
/* -3 si tiene una reserva */
int eliminarCliente(struct Tienda *tienda, int rut) {
    struct NodoCliente *act;
    struct NodoCliente *ant;
    struct NodoSala *sala;
    struct Cliente *cliente;
    int i;

    if (tienda == NULL || tienda->headClientes == NULL) {
        return 0;
    }

    act = buscarCliente(tienda->headClientes, rut);
    if (act == NULL || act->dato == NULL) {
        return 0;
    }
    cliente = act->dato;

    /* revisar sus pases: en fila o ya ingresados */
    for (i = 0; i < MAXPASES; i++) {
        if (cliente->pases[i] != NULL) {
            if (buscarEnFila(tienda->filaEntrada,
                             cliente->pases[i]->codigo) != NULL) {
                return -1;
            }
            if (buscarPaseDelDia(tienda->filaPasesDeldia,
                                 cliente->pases[i]->codigo) != NULL) {
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

    /* desenlazar de la lista circular */
    if (act->sig == act) { /* unico nodo -> lista queda vacia */
        tienda->headClientes = NULL;
    } else {
        ant = tienda->headClientes;
        while (ant->sig != act) { /* ant queda justo antes del nodo a eliminar */
            ant = ant->sig;
        }
        ant->sig = act->sig; /* el anterior apunta a lo que apuntaba el eliminado */
        if (act == tienda->headClientes) { /* si era head, el head pasa al siguiente */
            tienda->headClientes = act->sig;
        }
    }

    /* el cliente es dueno de sus pases: se liberan antes de eliminarlo */
    for (i = 0; i < MAXPASES; i++) {
        liberarPase(cliente->pases[i]);
    }
    free(cliente->nombre);
    free(cliente);
    free(act);
    return 1;
}

/* retorna 1 si se modifico, 0 si el cliente no existe o el nombre es NULL, */
/* -1 si el rut nuevo ya pertenece a otro cliente, -2 si no hay memoria */
int modificarCliente(struct NodoCliente *head, int rut, char *nuevoNombre, int nuevoRut) {
    struct NodoCliente *nodo;
    char *copia;

    if (nuevoNombre == NULL) {
        return 0;
    }
    nodo=buscarCliente(head, rut);
    if(nodo==NULL) {
        return 0;
    }

    if(nuevoRut!=rut && buscarCliente(head, nuevoRut)!=NULL) { /* rut no puede pertenecer a otro cliente */
        return -1;
    }

    /* primero se copia el nombre nuevo, asi si malloc falla el cliente queda igual */
    copia = malloc(strlen(nuevoNombre) + 1);
    if (copia == NULL) {
        return -2;
    }
    strcpy(copia, nuevoNombre);
    free(nodo->dato->nombre); /* se libera el nombre viejo */
    nodo->dato->nombre = copia; /* el cliente pasa a apuntar al nombre nuevo */
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

/*=======================================================================================================*/
/* funciones pases*/

/*
-crea un pase en memoria dinamica con los datos recibidos
-los datos son: tipoPase, codigo, valor, estado, fecha y hora
-retorna el puntero al pase creado
*/

struct Pase *crearPase(int tipoPase, int codigo, int valor, int estado, char *fecha, char *hora){
    struct Pase *nuevo;

    if (fecha == NULL || hora == NULL || codigo <= 0 ||
        tipoPase < 1 || tipoPase > 4 || valor < 0 ||
        estado < 1 || estado > 4) {
        return NULL;
    }

    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }

    nuevo->tipoPase=tipoPase;
    nuevo->codigo=codigo;
    nuevo->valor=valor;
    nuevo->estado=estado;

    nuevo->fecha = malloc(strlen(fecha) + 1);
    if (nuevo->fecha == NULL) {
        free(nuevo);
        return NULL;
    }
    strcpy(nuevo->fecha, fecha);
    nuevo->hora = malloc(strlen(hora) + 1);
    if (nuevo->hora == NULL) {
        free(nuevo->fecha);
        free(nuevo);
        return NULL;
    }
    strcpy(nuevo->hora, hora);

    return nuevo;
}

/*
-hace que un pase se convierta en un nodo de lista
-recibe un puntero a pase y retorna un puntero a nodo de pase
-lo usan tanto la lista de pases del dia como la fila
*/

struct NodoPase *crearNodoPase(struct Pase *pase){
    struct NodoPase *nuevo;

    if (pase == NULL) {
        return NULL;
    }
    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->paseVigente=pase;
    nuevo->sig=NULL;

    return nuevo;
}

/*
-busca la posicion de un pase por codigo en un arreglo de pases
-recibe el arreglo y su plibre (cantidad de pases guardados)
-retorna la posicion si lo encuentra, o -1 si no existe
*/

int buscarPosicionPase(struct Pase **pases, int plibre, int codigo){
    int i;

    for(i=0; i<plibre; i++)
    {
        if(pases[i]->codigo==codigo)
        {
            return i;
        }
    }
    return -1;
}

/*
-busca un pase por codigo en un arreglo de pases
-usa buscarPosicionPase
-retorna el pase si lo encuentra, o NULL si no existe
*/

struct Pase *buscarPase(struct Pase **pases, int plibre, int codigo){
    int pos;

    pos=buscarPosicionPase(pases, plibre, codigo);
    if(pos==-1)
    {
        return NULL;
    }
    return pases[pos];
}

/*
-agrega un pase al arreglo, en la primera posicion libre (plibre)
-recibe el arreglo, su plibre por puntero (porque aumenta), su tamano total y el pase
-no permite codigos repetidos, datos invalidos (tipo y estado entre 1 y 4, valor no negativo) ni arreglo lleno
-retorna 1 si se agrego, 0 si no se pudo
*/

int agregarPase(struct Pase **pases, int *plibre, int tam, struct Pase *pase){

    if(pase==NULL || *plibre>=tam)
    {
        return 0;
    }

    if(pase->tipoPase<1 || pase->tipoPase>4 || pase->estado<1 || pase->estado>4 || pase->valor<0)
    {
        return 0;
    }

    if(buscarPosicionPase(pases, *plibre, pase->codigo)!=-1)
    {
        return 0;
    }

    pases[*plibre]=pase;
    (*plibre)++;
    return 1;
}

/*
-modifica el tipo y el valor de un pase
-el codigo y el estado no se modifican
-como el pase es compartido, el cambio lo ven el cliente, la fila y los pases del dia
-retorna 1 si se modifico, 0 si el pase es NULL, -1 si el tipo o el valor no son validos
*/

int modificarPase(struct Pase *pase, int nuevoTipo, int nuevoValor){

    if(pase==NULL)
    {
        return 0;
    }

    if(nuevoTipo<1 || nuevoTipo>4 || nuevoValor<0)
    {
        return -1;
    }

    pase->tipoPase=nuevoTipo;
    pase->valor=nuevoValor;
    return 1;
}

/*
-muestra todos los pases de un arreglo, en el orden en que estan
-si no hay pases avisa con un mensaje
*/

void listarPases(struct Pase **pases, int plibre){
    int i;

    if(plibre==0)
    {
        printf("No hay pases registrados.\n");
    }
    else
    {
        for(i=0; i<plibre; i++)
        {
            printf("Codigo: %d, Tipo: %d, Valor: %d, Estado: %d, Fecha: %s, Hora: %s\n",
                   pases[i]->codigo, pases[i]->tipoPase, pases[i]->valor,
                   pases[i]->estado, pases[i]->fecha, pases[i]->hora);
        }
    }
}

/*
-libera un pase y sus textos (fecha y hora)
-usar solo cuando el pase se elimina de todos lados, una sola vez por pase
*/

void liberarPase(struct Pase *pase){

    if(pase==NULL)
    {
        return;
    }

    free(pase->fecha);
    free(pase->hora);
    free(pase);
}
/*============================================================================*/
/*pasesDelDia*/
/*
-crea la lista de pases del dia, vacia
-retorna el puntero a la lista, que se guarda en tienda->filaPasesDeldia
*/

struct pasesDelDia *crearPasesDelDia(void){
    struct pasesDelDia *nueva;

    nueva = malloc(sizeof(*nueva));
    if (nueva == NULL) {
        return NULL;
    }
    nueva->head=NULL;

    return nueva;
}

/*
-busca un pase por codigo recorriendo la lista de pases del dia de principio a fin
-recibe la lista y el codigo a buscar
-retorna el nodo si lo encuentra, o NULL si no existe
*/

struct NodoPase *buscarPaseDelDia(struct pasesDelDia *lista, int codigo){
    struct NodoPase *rec;

    if(lista==NULL)
    {
        return NULL;
    }

    rec=lista->head;
    while(rec!=NULL)
    {
        if(rec->paseVigente->codigo==codigo)
        {
            return rec;
        }
        rec=rec->sig;
    }
    return NULL;
}

/*
-agrega un pase al final de la lista de pases del dia
-el pase ya debe existir (creado con crearPase y guardado en el arreglo de pases)
-la lista solo guarda la referencia, no copia el pase
-no permite codigos repetidos
-retorna 1 si se agrego, 0 si el codigo ya existia o los datos no son validos
*/

int agregarPaseDelDia(struct pasesDelDia *lista, struct Pase *pase){
    struct NodoPase *rec, *nuevo;

    if(lista==NULL || pase==NULL)
    {
        return 0;
    }

    if(buscarPaseDelDia(lista, pase->codigo)!=NULL)
    {
        return 0;
    }

    nuevo=crearNodoPase(pase);
    if(nuevo==NULL)
    {
        return 0;
    }

    if(lista->head==NULL)
    {
        lista->head=nuevo;
    }
    else
    {
        rec=lista->head;
        while(rec->sig!=NULL)
        {
            rec=rec->sig;
        }
        rec->sig=nuevo;
    }
    return 1;
}

/*
-elimina un pase de la lista de pases del dia segun su codigo
-retorna 1 si se elimino, 0 si no existe, -1 si la lista esta vacia
*/

int eliminarPaseDelDia(struct pasesDelDia *lista, int codigo){
    struct NodoPase *rec, *borrar;

    if(lista==NULL)
    {
        return 0;
    }

    if(lista->head==NULL)
    {
        return -1;
    }

    rec=lista->head;
    if(rec->paseVigente->codigo==codigo)
    {
        borrar=rec;
        lista->head=rec->sig;
        free(borrar);
        return 1;
    }
    else
    {
        while(rec->sig!=NULL)
        {
            if(rec->sig->paseVigente->codigo==codigo)
            {
                borrar=rec->sig;
                rec->sig=borrar->sig;
                free(borrar);
                return 1;
            }
            rec=rec->sig;
        }
    }
    return 0;
}

/*
-modifica el tipo y el valor de un pase de la lista de pases del dia
-busca el nodo y usa modificarPase (del modulo de pases) para hacer el cambio
-retorna 1 si se modifico, 0 si no existe, -1 si el tipo o el valor no son validos
*/

int modificarPaseDelDia(struct pasesDelDia *lista, int codigo, int nuevoTipo, int nuevoValor){
    struct NodoPase *nodo;

    nodo=buscarPaseDelDia(lista, codigo);
    if(nodo==NULL)
    {
        return 0;
    }

    return modificarPase(nodo->paseVigente, nuevoTipo, nuevoValor);
}

/*
-muestra todos los pases de la lista de pases del dia, en el orden en que estan
-si no hay pases avisa con un mensaje
*/

void listarPasesDelDia(struct pasesDelDia *lista){
    struct NodoPase *rec;
    int cantidad=0;

    if(lista==NULL || lista->head==NULL)
    {
        printf("No hay pases registrados hoy.\n");
    }
    else
    {
        rec=lista->head;
        while(rec!=NULL)
        {
            cantidad++;
            printf("Cantidad: %d, Codigo: %d, Tipo: %d, Valor: %d, Estado: %d, Fecha: %s, Hora: %s\n",
                   cantidad, rec->paseVigente->codigo, rec->paseVigente->tipoPase,
                   rec->paseVigente->valor, rec->paseVigente->estado,
                   rec->paseVigente->fecha, rec->paseVigente->hora);
            rec=rec->sig;
        }
    }
}

/*
-cuenta cuantos pases de la lista del dia hay en un estado dado (activo 1, anulado 2, utilizado 3, vencido 4)
-por ejemplo, con estado 3 se sabe cuantos pases se utilizaron
-retorna la cantidad, 0 si no hay ninguno
*/

int contarPasesPorEstado(struct pasesDelDia *lista, int estado){
    struct NodoPase *rec;
    int contador=0;

    if(lista==NULL)
    {
        return 0;
    }

    rec=lista->head;
    while(rec!=NULL)
    {
        if(rec->paseVigente->estado==estado)
        {
            contador++;
        }
        rec=rec->sig;
    }
    return contador;
}

/*
-busca un pase por codigo en la fila de entrada, recorriendola desde el head hasta el final
-la fila no es circular, el ultimo nodo (tail) tiene sig en NULL
-retorna el nodo si lo encuentra, o NULL si no existe
-esta funcion corresponde al modulo de la fila, queda aqui porque recorre nodos de pase
*/

struct NodoPase *buscarPaseFila(struct Fila *fila, int codigo){
    struct NodoPase *rec;

    if(fila==NULL)
    {
        return NULL;
    }

    rec=fila->head;
    while(rec!=NULL)
    {
        if(rec->paseVigente->codigo==codigo)
        {
            return rec;
        }
        rec=rec->sig;
    }
    return NULL;
}

/*=== ELIMINAR PASE ===*/

/* elimina un pase del arreglo de pases de un cliente y compacta el arreglo */
/* no se elimina si el pase esta esperando en la fila o ya ingreso (esta en el registro del dia) */
/* retorna 1 si se elimino, 0 si el cliente o el pase no existen, */
/* -1 si el pase esta en la fila, -2 si el pase ya ingreso */
int eliminarPase(struct Tienda *tienda, int rut, int codigo) {
    struct NodoCliente *nodo;
    struct Cliente *c;
    int i, pos;

    if (tienda == NULL) { /* evita fallar si no hay tienda */
        return 0;
    }

    nodo = buscarCliente(tienda->headClientes, rut);
    if (nodo == NULL) {
        return 0;
    }
    c = nodo->dato;

    /* pos es la posicion del pase en el arreglo del cliente, -1 si no lo tiene */
    pos = buscarPosicionPase(c->pases, c->plibrepases, codigo);
    if (pos < 0) {
        return 0; /* el cliente no tiene ese pase */
    }

    if (buscarEnFila(tienda->filaEntrada, codigo) != NULL) {
        return -1; /* esta esperando en la fila */
    }
    if (buscarPaseDelDia(tienda->filaPasesDeldia, codigo) != NULL) {
        return -2; /* ya ingreso, el cliente esta dentro */
    }

    /* el pase tiene 3 bloques de memoria (fecha, hora y el struct), liberarPase los libera en ese orden */
    liberarPase(c->pases[pos]);

    /* cada pase siguiente retrocede una posicion y tapa el hueco del eliminado */
    for (i = pos; i < c->plibrepases - 1; i++) {
        c->pases[i] = c->pases[i + 1];
    }
    /* el for no toca la ultima posicion ocupada: queda con una copia repetida, se vacia con NULL */
    c->plibrepases--;
    c->pases[c->plibrepases] = NULL;

    return 1;
}

/*=======================================================================================================*/
/*funciones de arbol*/


/*
 * Crea una copia dinamica de un texto para que el objeto destino sea propietario
 * de su memoria; retorna NULL si el texto no existe o malloc falla.
 */
char *copiarTexto(char *texto) {
    char *copia;

    if (texto == NULL) {
        return NULL;
    }

    copia = malloc(strlen(texto) + 1);
    if (copia != NULL) {
        strcpy(copia, texto);
    }
    return copia;
}

/*
-crea un libro en memoria dinamica con los datos recibidos
-retorna el puntero al libro creado, o NULL si los datos no son validos
*/

struct Libro *crearLibro(int codigo, char *nombre, int precio, int ejemplares,
    char *publicacion, int clasificacionEdad,
    int disponibilidad, char *genero, char *fechaPublicacion, int deColeccion) {

    struct Libro *nuevoLibro;

    if (codigo <= 0 || nombre == NULL || precio < 0 || ejemplares < 0 ||
        publicacion == NULL || genero == NULL || fechaPublicacion == NULL ||
        disponibilidad < 1 || disponibilidad > 4 ||
        (disponibilidad == 1 && ejemplares == 0) ||
        (disponibilidad == 2 && ejemplares != 0) ||
        (disponibilidad == 3 && ejemplares == 0)) {
        return NULL;
    }

    nuevoLibro = malloc(sizeof(*nuevoLibro));
    if (nuevoLibro == NULL) {
        return NULL;
    }

    nuevoLibro->codigo = codigo;
    nuevoLibro->nombre = NULL;
    nuevoLibro->precio = precio;
    nuevoLibro->ejemplaresDisponibles = ejemplares;
    nuevoLibro->publicacion = NULL;
    nuevoLibro->clasificacionEdad = clasificacionEdad;
    nuevoLibro->disponibilidad = disponibilidad;
    nuevoLibro->genero = NULL;
    nuevoLibro->fechaPublicacion = NULL;
    nuevoLibro->deColeccion = deColeccion;
    nuevoLibro->headReserva = NULL;

    nuevoLibro->nombre = copiarTexto(nombre);
    nuevoLibro->publicacion = copiarTexto(publicacion);
    nuevoLibro->genero = copiarTexto(genero);
    nuevoLibro->fechaPublicacion = copiarTexto(fechaPublicacion);
    if (nuevoLibro->nombre == NULL || nuevoLibro->publicacion == NULL ||
        nuevoLibro->genero == NULL || nuevoLibro->fechaPublicacion == NULL) {
        liberarLibro(nuevoLibro);
        free(nuevoLibro);
        return NULL;
    }

    return nuevoLibro ;
}

/*
-hace que un libro se convierta en un nodo de arbol binario
*/

struct NodoLibro *crearNodoLibro(struct Libro *dato) {

    struct NodoLibro *nuevoNodo;

    if (dato == NULL) {
        return NULL;
    }

    nuevoNodo = malloc(sizeof(*nuevoNodo));
    if (nuevoNodo == NULL) {
        return NULL;
    }
    nuevoNodo -> dato = dato ;
    nuevoNodo -> izq = NULL ;
    nuevoNodo -> der = NULL ;

    return nuevoNodo ;
}


/*
 * Busca un libro por codigo aprovechando el orden del arbol binario.
 * Retorna el nodo encontrado o NULL si el codigo no existe.
 */
struct NodoLibro *buscarLibro(struct NodoLibro *raiz, int codigo){
    if (raiz == NULL) {
        return NULL ; /*no se encontro el libro*/
    }
    if (codigo == raiz->dato->codigo) {
        return raiz ; /*se encontro el libro*/
    }

    /*si el codigo es menor se busca en el subarbol izquierdo; si es mayor, en el derecho*/
    if (codigo < raiz->dato->codigo) {
        return buscarLibro(raiz->izq, codigo) ;
    } else {
        return buscarLibro(raiz->der, codigo) ;
    }
}


/*
-retorna el nodo con el menor codigo de un subarbol (el mas a la izquierda)
-se usa para encontrar el sucesor cuando se elimina un nodo con dos hijos
*/
struct NodoLibro *menorNodo(struct NodoLibro *raiz) {
    while (raiz->izq != NULL) {
        raiz = raiz->izq;
    }
    return raiz;
}

/*
-inserta un nodo de libro en el arbol binario de busqueda, ordenado por codigo
-retorna 1 si se inserto, 0 si el codigo ya existia
*/

int insertarLibro(struct NodoLibro **raiz, struct NodoLibro *nuevo) {
    if (raiz == NULL || nuevo == NULL || nuevo->dato == NULL) {
        return 0;
    }

    if (*raiz == NULL) {
        *raiz = nuevo ;
        return 1 ; /*se engancha el nuevo nodo al arbol*/
    }

    if (nuevo->dato->codigo < (*raiz)->dato->codigo) {
        return insertarLibro(&((*raiz)->izq), nuevo) ;

    } else if (nuevo->dato->codigo > (*raiz)->dato->codigo) {
        return insertarLibro(&((*raiz)->der), nuevo) ;

    } else {
        return 0 ;
    }
}

/*
-indica si ya existe un libro con el codigo dado
*/

int existeLibro(struct NodoLibro *raiz, int codigo){
    return buscarLibro(raiz, codigo) != NULL ;
}

/*
-cuenta cuantos libros hay en el arbol
*/

int contarLibros(struct NodoLibro *raiz) {
    if (raiz == NULL) {
        return 0 ;
    }
    return 1 + contarLibros(raiz->izq) + contarLibros(raiz->der) ;
}

/*
-muestra todos los libros del arbol recorriendolo en preorden
*/

void mostrarLibrosPreorden(struct NodoLibro *raiz) {

    if (raiz == NULL) {
        return ;
    }

    printf("Codigo: %d, Nombre: %s\n", raiz->dato->codigo, raiz->dato->nombre) ;
    mostrarLibrosPreorden(raiz->izq) ;
    mostrarLibrosPreorden(raiz->der) ;
}

/*
-modifica el precio de un libro existente (el codigo es la llave del arbol y no se modifica)
-retorna 1 si se modifico, 0 si no existe el libro o el precio es invalido
*/

int modificarPrecio(struct NodoLibro *raiz, int codigo, int nuevoPrecio) {

    struct NodoLibro *nodo ;
    nodo = buscarLibro(raiz, codigo) ;

    if (nodo == NULL) {
        return 0 ;
    }

    if (nuevoPrecio < 0) {
        return 0 ;
    }

    nodo->dato->precio = nuevoPrecio ;

    return 1 ;
}

/*
-registra la llegada de un despacho: sube ejemplaresDisponibles en la cantidad recibida
-no acepta cantidades menores o iguales a 0 ni libros descatalogados (4)
-si el libro estaba agotado (2) o en preventa (3), pasa a disponible (1)
-las reservas se atienden en orden mediante venderEjemplares, que solo vende al primero de la cola
-retorna 1 si se registro el despacho, 0 si no se pudo
*/

int recibirDespacho(struct NodoLibro *raiz, int codigo, int cantidad) {

    struct NodoLibro *nodo ;
    nodo = buscarLibro(raiz, codigo) ;

    if (nodo == NULL) {
        return 0 ;
    }

    if (cantidad <= 0 || nodo->dato->ejemplaresDisponibles < 0 ||
        (unsigned int)nodo->dato->ejemplaresDisponibles >
            ((unsigned int)-1 >> 1) - (unsigned int)cantidad) {
        return 0 ; /*cantidad invalida*/
    }

    if (nodo->dato->disponibilidad == 4) {
        return 0 ; /*descatalogado (4): no recibe despachos*/
    }

    nodo->dato->ejemplaresDisponibles += cantidad ;

    if (nodo->dato->disponibilidad == 2 ||
        nodo->dato->disponibilidad == 3) {
        nodo->dato->disponibilidad = 1 ;
    }

    return 1 ;
}

int venderEjemplares(struct Tienda *tienda, struct NodoLibro *raiz,
                     int codigo, int rut, int cantidad) {
    struct NodoLibro *nodo;
    struct Libro *libro;
    int unidadesVendidas;
    int ingreso;
    int reservasPendientes;

    if (tienda == NULL || cantidad <= 0 ||
        (nodo = buscarLibro(raiz, codigo)) == NULL) {
        return 0;
    }
    libro = nodo->dato;

    if (libro == NULL || libro->precio < 0 ||
        tienda->recaudacion < 0) {
        return 0;
    }

    reservasPendientes = libro->headReserva != NULL;
    if (reservasPendientes) {
        if ((libro->disponibilidad != 1 && libro->disponibilidad != 3) ||
            libro->headReserva->dato == NULL ||
            libro->headReserva->dato->reservador == NULL ||
            libro->headReserva->dato->reservador->rut != rut) {
            return 0;
        }
    } else if (libro->disponibilidad != 1 ||
               libro->ejemplaresDisponibles <= 0) {
        return 0;
    }

    unidadesVendidas = reservasPendientes ? 1 : cantidad;
    if (!reservasPendientes &&
        unidadesVendidas > libro->ejemplaresDisponibles) {
        unidadesVendidas = libro->ejemplaresDisponibles;
    }
    if (libro->precio > 0 &&
        (unsigned int)unidadesVendidas >
            (((unsigned int)-1 >> 1) -
             (unsigned int)tienda->recaudacion) /
                (unsigned int)libro->precio) {
        return 0;
    }

    ingreso = unidadesVendidas * libro->precio;
    if (!reservasPendientes) {
        libro->ejemplaresDisponibles -= unidadesVendidas;
    }
    tienda->recaudacion += ingreso;
    if (reservasPendientes) {
        eliminarReserva(libro, rut);
    }
    if (libro->headReserva == NULL) {
        libro->disponibilidad =
            libro->ejemplaresDisponibles == 0 ? 2 : 1;
    }

    return unidadesVendidas;
}

/*
-marca un libro como descatalogado (4): la editorial no lo reimprime
-el nodo NO se borra del arbol; se cancelan sus reservas y se informa cuantas
-retorna 1 si se marco como descatalogado, 0 si no existe un libro con ese codigo
*/

int descatalogarLibro(struct NodoLibro *raiz, int codigo,
                      int *reservasCanceladas) {

    struct NodoLibro *nodo ;

    if (reservasCanceladas != NULL) {
        *reservasCanceladas = 0;
    }

    nodo = buscarLibro(raiz, codigo) ;

    if (nodo == NULL) {
        return 0 ;
    }

    if (reservasCanceladas != NULL) {
        *reservasCanceladas = cantidadEnEspera(nodo->dato);
    }
    vaciarCola(nodo->dato);
    nodo->dato->disponibilidad = 4 ;

    return 1 ;
}

/*
-libera los 4 textos de un libro y vacia su cola de reservas
-el struct Libro lo libera quien llama, despues de esta funcion
*/

void liberarLibro(struct Libro *libro) {

    if (libro == NULL) {
        return ;
    }

    vaciarCola(libro);
    free(libro->nombre);
    free(libro->publicacion);
    free(libro->genero);
    free(libro->fechaPublicacion);
    libro->nombre = NULL;
    libro->publicacion = NULL;
    libro->genero = NULL;
    libro->fechaPublicacion = NULL;
}

/*
-elimina un libro del arbol segun su codigo y libera su memoria
-retorna 1 si se elimino, 0 si no existe un libro con ese codigo
*/

int eliminarLibro(struct NodoLibro **raiz, int codigo) {

    struct NodoLibro *aux ;
    struct Libro *libro ;

    if (*raiz == NULL) {
        return 0 ;
    }

    if (codigo < (*raiz)->dato->codigo) {
        return eliminarLibro(&((*raiz)->izq), codigo) ;
    }
    if (codigo > (*raiz)->dato->codigo) {
        return eliminarLibro(&((*raiz)->der), codigo) ;
    }

    /*dos hijos: el libro a eliminar pasa al sucesor y se elimina desde ahi*/
    if ((*raiz)->izq != NULL && (*raiz)->der != NULL) {
        aux = menorNodo((*raiz)->der) ;
        libro = (*raiz)->dato ;
        (*raiz)->dato = aux->dato ;
        aux->dato = libro ;
        return eliminarLibro(&((*raiz)->der), codigo) ;
    }

    /*sin hijos o un hijo: el padre apunta al unico hijo (o a NULL si es hoja)*/
    aux = *raiz ;
    if (aux->izq != NULL) {
        *raiz = aux->izq ;
    } else {
        *raiz = aux->der ;
    }

    aux->izq = NULL ;
    aux->der = NULL ;
    liberarLibro(aux->dato);
    free(aux->dato);
    free(aux);
    return 1 ;
}

/*=======================================================================================================*/
/*funciones boveda*/

/*
-indica si la sala es la boveda, se reconoce por su nombre
-retorna 1 si lo es, 0 si no
*/
int esBoveda(struct SalaTematica *sala) {

    if (sala == NULL || sala->nombre == NULL) {
        return 0;
    }
    return strcmp(sala->nombre, NOMBRE_BOVEDA) == 0;
}

/*
-recorre la lista de salas y retorna el nodo de la boveda
-retorna NULL si no existe
*/
struct NodoSala *buscarBoveda(struct NodoSala *headSalas) {

    while (headSalas != NULL) {
        if (esBoveda(headSalas->dato)) {
            return headSalas;
        }
        headSalas = headSalas->sig;
    }
    return NULL;
}

/*
-agrega un libro a una sala, decidiendo el destino segun deColeccion
-si el libro es de coleccion (deColeccion == 1) va a la boveda, sin importar la sala pedida
-si no es de coleccion va a la sala indicada, pero la boveda no lo acepta
-retorna 1 si se inserto, 0 si la sala no existe o el codigo ya existia,
-1 si se intento meter un libro normal en la boveda, -2 si es de coleccion y no hay boveda
*/
int agregarLibroASala(struct NodoSala *headSalas, int codigoSala, struct NodoLibro *nuevo) {

    struct NodoSala *destino;

    if (nuevo == NULL || nuevo->dato == NULL) {
        return 0;
    }

    if (nuevo->dato->deColeccion == 1) {
        destino = buscarBoveda(headSalas);
        if (destino == NULL) {
            return -2;
        }
    } else {
        destino = buscarSala(headSalas, codigoSala);
        if (destino == NULL) {
            return 0;
        }
        if (esBoveda(destino->dato)) {
            return -1;
        }
    }
    return insertarLibro(&(destino->dato->raiz), nuevo);
}

/*=======================================================================================================*/
/*funciones de reserva*/
/*
-crea una reserva; guarda un puntero al cliente, no lo copia ni es su propietaria
*/
struct Reserva *crearReserva(struct Cliente *reservador, int tipoPase) {
    struct Reserva *nuevaReserva;

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

    if (libro == NULL || cliente == NULL || pase == NULL ||
        libro->disponibilidad == 1 ||  libro->disponibilidad == 4 ||
        (pase->estado != 1 && pase->estado != 3) ||
        pase->tipoPase < 1 || pase->tipoPase > 4 ||
        buscarReserva(libro, cliente->rut) != NULL) {
        return 0;
    }

    /* la identidad del pase se verifica por referencia dentro del arreglo del cliente */
    for (i = 0; i < MAXPASES; i++) {
        if (cliente->pases[i] == pase) {
            esPaseDelCliente = 1;
            break;
        }
    }
    if (!esPaseDelCliente) {
        return 0;
    }

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
    libro->ejemplaresDisponibles--;
    return 1;
}


/*
-agrega una reserva a la lista del libro segun su tipo de pase
-normal (tipo distinto de 4): va al final de la cola
-prioritario (tipo 4): va delante de los normales, pero detras de los prioritarios que ya estaban
-retorna 1 si se agrego, 0 si no
*/
int encolar(struct Libro *libro, struct NodoReserva *nuevo) {
    struct NodoReserva *rec;

    if (libro == NULL || nuevo == NULL || nuevo->dato == NULL ||
        nuevo->dato->reservador == NULL ||libro->disponibilidad == 1 ||  libro->disponibilidad == 4||
        nuevo->dato->tipoPase < 1 || nuevo->dato->tipoPase > 4 ||
        nuevo->sig != NULL) {
        return 0;
    }

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
*/
struct NodoReserva *desencolar(struct Libro *libro) {
    struct NodoReserva *aux;

    if (libro == NULL || libro->headReserva == NULL) {
        return NULL;
    }

    aux = libro->headReserva;
    libro->headReserva = aux->sig;

    aux->sig = NULL;
    return aux;
}

/*
-retorna cuantas personas esperan en la cola
*/
int cantidadEnEspera(struct Libro *libro) {
    struct NodoReserva *rec;
    int cantidad = 0;

    if (libro == NULL) {
        return 0;
    }

    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        cantidad++;
    }

    return cantidad;
}

/*
-muestra las reservas en orden de atencion
*/
void mostrarCola(struct Libro *libro) {
    struct NodoReserva *rec;
    int pos = 1;

    if (libro == NULL || libro->headReserva == NULL) {
        printf("No hay clientes en lista de espera\n");
        return;
    }

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
*/
struct NodoReserva *buscarReserva(struct Libro *libro, int rut) {
    struct NodoReserva *rec;

    if (libro == NULL) {
        return NULL;
    }

    for (rec = libro->headReserva; rec != NULL; rec = rec->sig) {
        if (rec->dato != NULL && rec->dato->reservador != NULL &&
            rec->dato->reservador->rut == rut) {
            return rec;
        }
    }

    return NULL;
}

/*
-elimina de la lista del libro la reserva de un cliente y libera el nodo y la reserva (no al cliente)
*/
int eliminarReserva(struct Libro *libro, int rut) {
    struct NodoReserva *rec;
    struct NodoReserva *ant = NULL;

    if (libro == NULL) {
        return 0;
    }

    rec = libro->headReserva;
    while (rec != NULL &&
        (rec->dato == NULL || rec->dato->reservador == NULL || rec->dato->reservador->rut != rut)) {
        ant = rec;
        rec = rec->sig;
    }

    if (rec == NULL) {
        return 0;
    }

    if (ant == NULL) {
        libro->headReserva = rec->sig;
    } else {
        ant->sig = rec->sig;
    }

    free(rec->dato);
    free(rec);
    return 1;
}

/*
-modifica la reserva de un cliente cambiando el pase con que reserva
-el tipo del pase determina la prioridad, por eso la reserva se vuelve a ubicar en la lista
-retorna 1 si se modifico, 0 si no se pudo
*/
int modificarReserva(struct Libro *libro, struct Cliente *cliente, struct Pase *nuevoPase) {
    struct NodoReserva *nodo;
    struct NodoReserva *ant = NULL;
    int esPaseDelCliente = 0;
    int i;

    if (libro == NULL || cliente == NULL || nuevoPase == NULL ||
        libro->disponibilidad == 1 ||  libro->disponibilidad == 4||
        (nuevoPase->estado != 1 && nuevoPase->estado != 3) ||
        nuevoPase->tipoPase < 1 || nuevoPase->tipoPase > 4) {
        return 0;
    }

    /* el pase debe pertenecer al cliente (se compara la direccion, no el codigo) */
    for (i = 0; i < MAXPASES; i++) {
        if (cliente->pases[i] == nuevoPase) {
            esPaseDelCliente = 1;
            break;
        }
    }
    if (!esPaseDelCliente) {
        return 0;
    }

    /* busca la reserva del cliente guardando el nodo anterior */
    nodo = libro->headReserva;
    while (nodo != NULL &&
        (nodo->dato == NULL || nodo->dato->reservador == NULL || nodo->dato->reservador->rut != cliente->rut)) {
        ant = nodo;
        nodo = nodo->sig;
    }

    if (nodo == NULL) {
        return 0;
    }

    /* si el tipo es el mismo, la prioridad no cambia */
    if (nodo->dato->tipoPase == nuevoPase->tipoPase) {
        return 1;
    }

    /* se saca el nodo de la lista sin liberarlo */
    if (ant == NULL) {
        libro->headReserva = nodo->sig;
    } else {
        ant->sig = nodo->sig;
    }
    nodo->sig = NULL;

    /* se cambia el tipo y encolar lo vuelve a ubicar segun su prioridad */
    nodo->dato->tipoPase = nuevoPase->tipoPase;
    return encolar(libro, nodo);
}

/*=======================================================================================================*/
/*funciones filaEntrada*/

struct Fila *crearFila(void) {
    struct Fila *fila = malloc(sizeof(*fila));

    if (fila == NULL) {
        return NULL;
    }

    fila->head = NULL;
    fila->tail = NULL;
    return fila;
}

/*
-agrega un nodo de pase a la fila
-los pases de tipo 4 van antes que los demas, conservando el orden entre prioritarios
-retorna 1 si el nodo se agrego y 0 si no se pudo agregar
*/
int encolarFila(struct Fila *fila, struct NodoPase *nuevo) {
    struct NodoPase *rec;
    struct NodoPase *anterior;

    if (fila == NULL || nuevo == NULL || nuevo->paseVigente == NULL ||
        nuevo->sig != NULL || nuevo->paseVigente->tipoPase < 1 ||
        nuevo->paseVigente->tipoPase > 4) {
        return 0;
    }

    anterior = NULL;
    for (rec = fila->head; rec != NULL; rec = rec->sig) {
        if (rec == nuevo || rec->paseVigente == NULL ||
            rec->paseVigente == nuevo->paseVigente ||
            rec->paseVigente->codigo == nuevo->paseVigente->codigo) {
            return 0;
        }
        anterior = rec;
    }

    if ((fila->head == NULL && fila->tail != NULL) ||
        (fila->head != NULL && anterior != fila->tail)) {
        return 0;
    }

    if (fila->head == NULL) {
        fila->head = nuevo;
        fila->tail = nuevo;
    } else if (nuevo->paseVigente->tipoPase != 4) {
        fila->tail->sig = nuevo;
        fila->tail = nuevo;
    } else {
        rec = fila->head;
        anterior = NULL;
        while (rec != NULL && rec->paseVigente->tipoPase == 4) {
            anterior = rec;
            rec = rec->sig;
        }

        nuevo->sig = rec;
        if (anterior == NULL) {
            fila->head = nuevo;
        } else {
            anterior->sig = nuevo;
        }
        if (nuevo->sig == NULL) {
            fila->tail = nuevo;
        }
    }

    return 1;
}

/*
-retira el primer nodo de la fila y lo separa de la lista
*/
struct NodoPase *desencolarFila(struct Fila *fila) {
    struct NodoPase *primero;

    if (fila == NULL || fila->head == NULL) {
        return NULL;
    }

    primero = fila->head;
    fila->head = primero->sig;
    if (fila->head == NULL) {
        fila->tail = NULL;
    }
    primero->sig = NULL;
    return primero;
}

/*
-busca en la fila un pase por su codigo
*/
struct NodoPase *buscarEnFila(struct Fila *fila, int codigoPase) {
    struct NodoPase *rec;

    if (fila == NULL) {
        return NULL;
    }

    for (rec = fila->head; rec != NULL; rec = rec->sig) {
        if (rec->paseVigente != NULL &&
            rec->paseVigente->codigo == codigoPase) {
            return rec;
        }
    }
    return NULL;
}

/*
-busca y quita de la fila el pase con el codigo indicado
-desenlaza el nodo y deja su sig en NULL; no libera memoria ni toca el pase
*/
int quitarDeFila(struct Fila *fila, int codigoPase) {
    struct NodoPase *rec;
    struct NodoPase *anterior = NULL;

    if (fila == NULL) {
        return 0;
    }

    rec = fila->head;
    while (rec != NULL && (rec->paseVigente == NULL || rec->paseVigente->codigo != codigoPase)) {
        anterior = rec;
        rec = rec->sig;
    }
    if (rec == NULL) {
        return 0;
    }

    if (anterior == NULL) {
        fila->head = rec->sig;
    } else {
        anterior->sig = rec->sig;
    }
    if (fila->tail == rec) {
        fila->tail = anterior;
    }
    rec->sig = NULL;
    return 1;
}

int limpiarLineaEntrada(void) {
    int caracter;

    while ((caracter = getchar()) != '\n' && caracter != EOF) {
    }
    return caracter != EOF;
}

/*=======================================================================================================*/
/*funciones extra (consultas)*/

/* retorna 1 si algun cliente ya tiene un pase con ese codigo */
int codigoPaseUsado(struct Tienda *tienda, int codigo) {
    struct NodoCliente *rec = tienda->headClientes;

    if (rec != NULL) {
        do {
            if (buscarPosicionPase(rec->dato->pases, rec->dato->plibrepases, codigo) != -1) {
                return 1;
            }
            rec = rec->sig;
        } while (rec != tienda->headClientes);
    }
    return 0;
}

/* recorre el arbol y deja en mejor el libro con mas clientes en espera */
void buscarMasEsperaABB(struct NodoLibro *nodo, struct Libro **mejor, int *mejorCant) {
    int cant;

    if (nodo == NULL) {
        return;
    }
    cant = cantidadEnEspera(nodo->dato);
    if (cant > *mejorCant) {
        *mejor = nodo->dato;
        *mejorCant = cant;
    }
    buscarMasEsperaABB(nodo->izq, mejor, mejorCant);
    buscarMasEsperaABB(nodo->der, mejor, mejorCant);
}

/* retorna el libro con mas espera de toda la tienda, o NULL si nadie espera */
struct Libro *libroMasEspera(struct Tienda *tienda) {
    struct NodoSala *sala;
    struct Libro *mejor = NULL;
    int mejorCant = 0;

    for (sala = tienda->headSalas; sala != NULL; sala = sala->sig) {
        buscarMasEsperaABB(sala->dato->raiz, &mejor, &mejorCant);
    }
    return mejor;
}

/* muestra los libros agotados (disponibilidad 2) del arbol */
void listarAgotadosArbol(struct NodoLibro *raiz) {
    if (raiz == NULL) {
        return;
    }
    listarAgotadosArbol(raiz->izq);
    if (raiz->dato->disponibilidad == 2) {
        printf("Agotado: %d %s\n", raiz->dato->codigo, raiz->dato->nombre);
    }
    listarAgotadosArbol(raiz->der);
}

/* dias de espera: despachos necesarios (redondeado hacia arriba) por dias entre despachos */
/* retorna -1 si los datos no son validos */
int tiempoEsperaEstimado(struct Libro *libro, int porDespacho, int diasEntre) {
    if (libro == NULL || porDespacho <= 0 || diasEntre <= 0) {
        return -1;
    }
    return ((cantidadEnEspera(libro) + porDespacho - 1) / porDespacho) * diasEntre;
}

/* ordena de menor a mayor valor los pases de un arreglo (burbuja) */
void ordenarPasesPorValor(struct Pase **pases, int plibre) {
    int i, j;
    struct Pase *aux;

    for (i = 0; i < plibre - 1; i++) {
        for (j = 0; j < plibre - 1 - i; j++) {
            if (pases[j]->valor > pases[j + 1]->valor) {
                aux = pases[j];
                pases[j] = pases[j + 1];
                pases[j + 1] = aux;
            }
        }
    }
}

/*=======================================================================================================*/

int main(void) {
    int opcion, subopcion;
    int salaCodigo, salaAforoMaximo, nuevoCodigo, nuevoAforo;
    int hIni, mIni, hFin, mFin;
    int resultado, estadoTienda;
    int rut, nuevoRut;
    int codigoPase, tipoPase, valorPase, codigoLibro;
    int precioLibro, ejemplares, clasificacionEdad, disponibilidad, deColeccion;
    int diasEntre;
    int pos;
    int ventaPases, ventaLibros, contadorPases;
    int horaCierre, minCierre, minutosCierre, minutosAhora, minutosRestantes;
    int horaApertura, minApertura, minutosApertura, anticipacion, minutosDesde;
    int avisoMinutos;
    int dia, mes, anio, hAhora, mAhora, leidos;
    char *salaTematica;
    char *salaNombre;
    char *salaHorarioInicio;
    char *salaHorarioCierre;
    char *nuevoNombre;
    char *nombre;
    char *nombreLibro;
    char *publicacion;
    char *genero;
    char *fechaPublicacion;
    char fecha[11];                /* dd/mm/aaaa */
    char hora[6];                  /* hh:mm */
    char horaInicio[6];
    char horaTermino[6];
    struct Tienda *tienda;
    struct NodoSala *nodoSala;
    struct SalaTematica *sala;
    struct NodoCliente *encontrado;
    struct Cliente *cliente;
    struct Pase *pase;
    struct NodoPase *nodoPase;
    struct NodoPase *recPase;
    struct NodoLibro *nodoLibro;
    struct Libro *libro;

    tienda = (struct Tienda *) malloc(sizeof(struct Tienda));
    if (tienda == NULL) {
        printf("No hay memoria\n");
        return 1;
    }

    /* struct tienda: conecta */
    strcpy(horaInicio, "09:00");
    strcpy(horaTermino, "20:00");
    tienda->headSalas = NULL;
    tienda->headClientes = NULL;
    tienda->filaEntrada = NULL;
    tienda->filaPasesDeldia = NULL;
    tienda->recaudacion = 0;
    tienda->hora_inicio = horaInicio;
    tienda->hora_termino = horaTermino;
    tienda->fecha = fecha;

    ventaPases = 0;
    ventaLibros = 0;
    contadorPases = 0;
    avisoMinutos = 30;   /* minutos antes del cierre para avisar */
    horaCierre = 20;     /* debe coincidir con horaTermino ("20:00") */
    minCierre = 0;
    minutosCierre = horaCierre * 60 + minCierre;
    horaApertura = 9;    /* debe coincidir con horaInicio ("09:00") */
    minApertura = 0;
    minutosApertura = horaApertura * 60 + minApertura;
    anticipacion = 60;   /* minutos antes de abrir en que ya se puede atender la fila */

    estadoTienda = 1;
    printf("Bienvenido al menu IBC-omincs\n");

    /* la fecha se pide una sola vez, al partir el programa */
    do {
        printf("Dia (1-31): ");
        leidos = scanf("%d", &dia);
        if (leidos != 1) {
            if (!limpiarLineaEntrada()) {
                free(tienda);
                return 1;
            }
            dia = 0;
        }
        printf("Mes (1-12): ");
        leidos = scanf("%d", &mes);
        if (leidos != 1) {
            if (!limpiarLineaEntrada()) {
                free(tienda);
                return 1;
            }
            mes = 0;
        }
        printf("Anio (1-9999): ");
        leidos = scanf("%d", &anio);
        if (leidos != 1) {
            if (!limpiarLineaEntrada()) {
                free(tienda);
                return 1;
            }
            anio = 0;
        }
    } while (dia < 1 || dia > 31 || mes < 1 || mes > 12 || anio < 1 || anio > 9999);

    /* se arma el texto dd/mm/aaaa a mano */
    fecha[0] = '0' + dia / 10;
    fecha[1] = '0' + dia % 10;
    fecha[2] = '/';
    fecha[3] = '0' + mes / 10;
    fecha[4] = '0' + mes % 10;
    fecha[5] = '/';
    fecha[6] = '0' + (anio / 1000) % 10;
    fecha[7] = '0' + (anio / 100) % 10;
    fecha[8] = '0' + (anio / 10) % 10;
    fecha[9] = '0' + anio % 10;
    fecha[10] = '\0';

    /* buffers de texto dinamicos: se reservan una vez y se liberan al salir */
    salaTematica = (char *) malloc(100 * sizeof(char));
    salaNombre = (char *) malloc(100 * sizeof(char));
    salaHorarioInicio = (char *) malloc(6 * sizeof(char));
    salaHorarioCierre = (char *) malloc(6 * sizeof(char));
    nuevoNombre = (char *) malloc(100 * sizeof(char));
    nombre = (char *) malloc(100 * sizeof(char));
    nombreLibro = (char *) malloc(100 * sizeof(char));
    publicacion = (char *) malloc(100 * sizeof(char));
    genero = (char *) malloc(100 * sizeof(char));
    fechaPublicacion = (char *) malloc(100 * sizeof(char));
    if (salaTematica == NULL || salaNombre == NULL || salaHorarioInicio == NULL ||
        salaHorarioCierre == NULL || nuevoNombre == NULL || nombre == NULL ||
        nombreLibro == NULL || publicacion == NULL || genero == NULL ||
        fechaPublicacion == NULL) {
        printf("No hay memoria\n");
        free(salaTematica);
        free(salaNombre);
        free(salaHorarioInicio);
        free(salaHorarioCierre);
        free(nuevoNombre);
        free(nombre);
        free(nombreLibro);
        free(publicacion);
        free(genero);
        free(fechaPublicacion);
        free(tienda);
        return 1;
    }

    while (estadoTienda){
        /* la hora se pide cada vez que se entra al menu */
        do {
            printf("\nHora actual (0-23): ");
            leidos = scanf("%d", &hAhora);
            if (leidos != 1) {
                if (!limpiarLineaEntrada()) {
                    estadoTienda = 0;
                    break;
                }
                hAhora = -1;
            }
            printf("Minutos (0-59): ");
            leidos = scanf("%d", &mAhora);
            if (leidos != 1) {
                if (!limpiarLineaEntrada()) {
                    estadoTienda = 0;
                    break;
                }
                mAhora = -1;
            }
        } while (hAhora < 0 || hAhora > 23 || mAhora < 0 || mAhora > 59);

        if (!estadoTienda) {
            break;
        }

        /* se arma el texto hh:mm a mano */
        hora[0] = '0' + hAhora / 10;
        hora[1] = '0' + hAhora % 10;
        hora[2] = ':';
        hora[3] = '0' + mAhora / 10;
        hora[4] = '0' + mAhora % 10;
        hora[5] = '\0';
        printf("Fecha: %s  Hora: %s\n", fecha, hora);

        /* aviso si se acerca la hora de termino */
        minutosAhora = hAhora * 60 + mAhora;
        minutosRestantes = minutosCierre - minutosAhora;
        if (minutosRestantes < 0) {
            printf("ADVERTENCIA: la tienda ya cerro (termino a las %s)\n", tienda->hora_termino);
        } else if (minutosRestantes <= avisoMinutos) {
            printf("ADVERTENCIA: faltan %d minutos para el cierre (%s)\n", minutosRestantes, tienda->hora_termino);
        }

        printf("1.Fila de entrada 2.Clientes 3.Salas 4.Reservas 5.Datos tienda 6.Pases del dia 7.Pases de un cliente 8.Libros 9.Consultas 0.Salir\n");
        scanf("%d",&opcion);
        switch (opcion){
            case 1:
                /* lista filaEntrada */
                if (tienda->filaEntrada == NULL) {
                    tienda->filaEntrada = crearFila();
                    if (tienda->filaEntrada == NULL) {
                        printf("No hay memoria\n");
                        break;
                    }
                }
                printf("1.Agregar 2.Atender 3.Buscar 4.Eliminar 5.Listar 6.Modificar\n");
                scanf("%d",&subopcion);
                switch (subopcion)
                {
                    case 1:
                        printf("Rut del cliente: ");
                        scanf("%d", &rut);
                        encontrado = buscarCliente(tienda->headClientes, rut);
                        if (encontrado == NULL) {
                            printf("No se encontro el cliente\n");
                            break;
                        }
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        pase = buscarPase(encontrado->dato->pases, encontrado->dato->plibrepases, codigoPase);
                        if (pase == NULL) {
                            printf("No se encontro el pase\n");
                            break;
                        }
                        if (pase->estado != 1) {
                            printf("El pase no esta activo\n");
                            break;
                        }
                        nodoPase = crearNodoPase(pase);
                        if (encolarFila(tienda->filaEntrada, nodoPase) == 1) {
                            printf("Pase agregado a la fila\n");
                        } else {
                            free(nodoPase);   /* no quedo enlazado */
                            printf("No se pudo agregar (pase repetido o invalido)\n");
                        }
                        break;
                    case 2:
                        /* atender: solo dentro del horario de la tienda, desde 1 hora antes de abrir */
                        minutosDesde = minutosApertura - anticipacion;
                        if (minutosDesde < 0) {
                            minutosDesde = 0;
                        }
                        if (minutosAhora < minutosDesde || minutosAhora > minutosCierre) {
                            printf("Fuera de horario: se atiende de %02d:%02d a %s\n", minutosDesde / 60, minutosDesde % 60, tienda->hora_termino);
                            break;
                        }
                        /* sale el primero de la fila, el pase queda utilizado y se registra en los pases del dia */
                        nodoPase = desencolarFila(tienda->filaEntrada);
                        if (nodoPase == NULL) {
                            printf("La fila esta vacia\n");
                        } else {
                            nodoPase->paseVigente->estado = 3;   /* utilizado */
                            if (tienda->filaPasesDeldia == NULL) {
                                tienda->filaPasesDeldia = crearPasesDelDia();
                            }
                            agregarPaseDelDia(tienda->filaPasesDeldia, nodoPase->paseVigente);
                            printf("Ingresa el pase %d\n", nodoPase->paseVigente->codigo);
                            free(nodoPase);   /* solo el nodo, el pase sigue existiendo */
                        }
                        break;
                    case 3:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        nodoPase = buscarEnFila(tienda->filaEntrada, codigoPase);
                        if (nodoPase != NULL) {
                            printf("Pase %d esta en la fila (tipo %d)\n", nodoPase->paseVigente->codigo, nodoPase->paseVigente->tipoPase);
                        }else {
                            printf("No se encontro el pase en la fila\n");
                        }
                        break;
                    case 4:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        nodoPase = buscarEnFila(tienda->filaEntrada, codigoPase);
                        if (nodoPase != NULL && quitarDeFila(tienda->filaEntrada, codigoPase) == 1) {
                            free(nodoPase);
                            printf("Pase eliminado de la fila\n");
                        } else {
                            printf("No se encontro el pase en la fila\n");
                        }
                        break;
                    case 5:
                        if (tienda->filaEntrada->head == NULL) {
                            printf("La fila esta vacia\n");
                        } else {
                            pos = 0;
                            recPase = tienda->filaEntrada->head;
                            while (recPase != NULL) {
                                pos++;
                                printf("%d) Pase: %d, Tipo: %d\n", pos, recPase->paseVigente->codigo, recPase->paseVigente->tipoPase);
                                recPase = recPase->sig;
                            }
                        }
                        break;
                    case 6:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        nodoPase = buscarEnFila(tienda->filaEntrada, codigoPase);
                        if (nodoPase == NULL) {
                            printf("No se encontro el pase en la fila\n");
                            break;
                        }
                        printf("Nuevo tipo: ");
                        scanf("%d", &tipoPase);
                        printf("Nuevo valor: ");
                        scanf("%d", &valorPase);
                        resultado = modificarPase(nodoPase->paseVigente, tipoPase, valorPase);
                        if (resultado == 1) printf("Pase modificado\n");
                        else printf("Tipo o valor no validos\n");
                        break;
                    default:
                        printf("Opcion no valida\n");
                        break;
                }
                break;
            case 2:
                /*cliente*/
                printf("1.Agregar 2.Buscar 3.Eliminar 4.Modificar 5.Listar\n");
                scanf("%d", &subopcion);
                switch (subopcion) {
                    case 1:
                        printf("Nombre: ");
                        scanf(" %99[^\n]", nombre);
                        printf("Rut: ");
                        scanf("%d", &rut);
                        resultado = agregarCliente(&(tienda->headClientes), nombre, rut);
                        if (resultado == 1) printf("Cliente agregado\n");
                        else printf("El rut ya existe\n");
                        break;
                    case 2:
                        printf("Rut: ");
                        scanf("%d", &rut);
                        encontrado = buscarCliente(tienda->headClientes, rut);
                        if (encontrado != NULL)
                            printf("Cliente: %s rut: %d\n", encontrado->dato->nombre, encontrado->dato->rut);
                        else printf("No se encontro el cliente\n");
                        break;
                    case 3:
                        printf("Rut: ");
                        scanf("%d", &rut);
                        resultado = eliminarCliente(tienda, rut);
                        if (resultado == 1) printf("Cliente eliminado\n");
                        else if (resultado == -1) printf("No se puede eliminar: tiene un pase en la fila\n");
                        else if (resultado == -2) printf("No se puede eliminar: tiene un pase que ya ingreso\n");
                        else if (resultado == -3) printf("No se puede eliminar: tiene una reserva pendiente\n");
                        else printf("No se encontro el cliente\n");
                        break;
                    case 4:
                        printf("Rut actual: ");
                        scanf("%d", &rut);
                        printf("Nuevo rut: ");
                        scanf("%d", &nuevoRut);
                        printf("Nuevo nombre: ");
                        scanf(" %99[^\n]", nuevoNombre);
                        resultado = modificarCliente(tienda->headClientes, rut, nuevoNombre, nuevoRut);
                        if (resultado == 1) printf("Cliente modificado\n");
                        else if (resultado == 0) printf("El cliente no existe\n");
                        else if (resultado == -1) printf("El nuevo rut ya existe\n");
                        else printf("No hay memoria\n");
                        break;
                    case 5:
                        listarClientes(tienda->headClientes);
                        break;
                    default:
                        printf("Opcion no valida\n");
                        break;
                }
                break;

            case 3:
                /*salaTematica*/
                printf("1.Crear 2.Mostrar 3.Modificar 4.Eliminar 5.Buscar\n");
                scanf("%d", &subopcion);
                switch (subopcion) {
                    case 1:
                        printf("Codigo: ");
                        scanf("%d", &salaCodigo);
                        if (buscarSala(tienda->headSalas, salaCodigo) != NULL) {
                            printf("Ya existe una sala con ese codigo\n");
                            break;
                        }
                        printf("Tematica: ");
                        scanf(" %99[^\n]", salaTematica);
                        printf("Nombre de la sala: ");
                        scanf(" %99[^\n]", salaNombre);
                        printf("Aforo maximo: ");
                        scanf("%d", &salaAforoMaximo);
                        /* horario de atencion de la sala: hora y minutos, la apertura debe ser antes del cierre */
                        hIni = -1;
                        mIni = -1;
                        hFin = -1;
                        mFin = -1;
                        do {
                            printf("Apertura (hora minutos, ej: 9 30): ");
                            leidos = scanf("%d %d", &hIni, &mIni);
                            if (leidos != 2) {
                                if (!limpiarLineaEntrada()) {
                                    estadoTienda = 0;
                                    break;
                                }
                                hIni = -1;
                                continue;
                            }
                            printf("Cierre (hora minutos, ej: 18 0): ");
                            leidos = scanf("%d %d", &hFin, &mFin);
                            if (leidos != 2) {
                                if (!limpiarLineaEntrada()) {
                                    estadoTienda = 0;
                                    break;
                                }
                                hIni = -1;
                                continue;
                            }
                        } while (hIni < 0 || hIni > 23 || mIni < 0 || mIni > 59 ||
                                 hFin < 0 || hFin > 23 || mFin < 0 || mFin > 59 ||
                                 hIni * 60 + mIni >= hFin * 60 + mFin);
                        if (!estadoTienda) {
                            break;
                        }
                        /* se arma el texto hh:mm a mano */
                        salaHorarioInicio[0] = (char)('0' + hIni / 10);
                        salaHorarioInicio[1] = (char)('0' + hIni % 10);
                        salaHorarioInicio[2] = ':';
                        salaHorarioInicio[3] = (char)('0' + mIni / 10);
                        salaHorarioInicio[4] = (char)('0' + mIni % 10);
                        salaHorarioInicio[5] = '\0';
                        salaHorarioCierre[0] = (char)('0' + hFin / 10);
                        salaHorarioCierre[1] = (char)('0' + hFin % 10);
                        salaHorarioCierre[2] = ':';
                        salaHorarioCierre[3] = (char)('0' + mFin / 10);
                        salaHorarioCierre[4] = (char)('0' + mFin % 10);
                        salaHorarioCierre[5] = '\0';
                        sala = crearSala(salaTematica, salaNombre, salaHorarioInicio, salaHorarioCierre, salaCodigo, salaAforoMaximo);
                        if (sala == NULL) {
                            printf("No se pudo crear la sala (aforo maximo menor a 1 o sin memoria)\n");
                            break;
                        }
                        insertarSala(&(tienda->headSalas), crearNodoSala(sala));
                        printf("Sala creada\n");
                        break;
                    case 2:/*listar*/
                        mostrarTodasSalas(tienda->headSalas);
                        break;
                    case 3:
                        printf("Codigo actual: ");
                        scanf("%d", &salaCodigo);
                        printf("Nuevo codigo: ");
                        scanf("%d", &nuevoCodigo);
                        printf("Nueva tematica: ");
                        scanf(" %99[^\n]", salaTematica);
                        printf("Nuevo nombre de la sala: ");
                        scanf(" %99[^\n]", salaNombre);
                        printf("Nuevo aforo maximo: ");
                        scanf("%d", &nuevoAforo);
                        /* horario de atencion de la sala: hora y minutos, la apertura debe ser antes del cierre */
                        hIni = -1;
                        mIni = -1;
                        hFin = -1;
                        mFin = -1;
                        do {
                            printf("Apertura (hora minutos, ej: 9 30): ");
                            leidos = scanf("%d %d", &hIni, &mIni);
                            if (leidos != 2) {
                                if (!limpiarLineaEntrada()) {
                                    estadoTienda = 0;
                                    break;
                                }
                                hIni = -1;
                                continue;
                            }
                            printf("Cierre (hora minutos, ej: 18 0): ");
                            leidos = scanf("%d %d", &hFin, &mFin);
                            if (leidos != 2) {
                                if (!limpiarLineaEntrada()) {
                                    estadoTienda = 0;
                                    break;
                                }
                                hIni = -1;
                                continue;
                            }
                        } while (hIni < 0 || hIni > 23 || mIni < 0 || mIni > 59 ||
                                 hFin < 0 || hFin > 23 || mFin < 0 || mFin > 59 ||
                                 hIni * 60 + mIni >= hFin * 60 + mFin);
                        if (!estadoTienda) {
                            break;
                        }
                        /* se arma el texto hh:mm a mano */
                        salaHorarioInicio[0] = (char)('0' + hIni / 10);
                        salaHorarioInicio[1] = (char)('0' + hIni % 10);
                        salaHorarioInicio[2] = ':';
                        salaHorarioInicio[3] = (char)('0' + mIni / 10);
                        salaHorarioInicio[4] = (char)('0' + mIni % 10);
                        salaHorarioInicio[5] = '\0';
                        salaHorarioCierre[0] = (char)('0' + hFin / 10);
                        salaHorarioCierre[1] = (char)('0' + hFin % 10);
                        salaHorarioCierre[2] = ':';
                        salaHorarioCierre[3] = (char)('0' + mFin / 10);
                        salaHorarioCierre[4] = (char)('0' + mFin % 10);
                        salaHorarioCierre[5] = '\0';
                        resultado = modificarSala(&(tienda->headSalas), salaCodigo, nuevoCodigo, nuevoAforo, salaTematica, salaNombre, salaHorarioInicio, salaHorarioCierre);
                        if (resultado == 1) printf("Sala modificada\n");
                        else printf("No se pudo modificar (sala inexistente, codigo en uso o aforo menor al actual)\n");
                        break;
                    case 4:
                        printf("Codigo: ");
                        scanf("%d", &salaCodigo);
                        resultado = EliminarSala(&(tienda->headSalas), salaCodigo);
                        if (resultado == 1) printf("Sala eliminada\n");
                        else printf("No se encontro la sala\n");
                        break;
                    case 5:
                        printf("Codigo: ");
                        scanf("%d", &salaCodigo);
                        nodoSala = buscarSala(tienda->headSalas, salaCodigo);
                        if (nodoSala != NULL)
                            printf("Sala %d: %s, tematica: %s, horario: %s a %s, aforo: %d/%d\n",
                                   nodoSala->dato->codigo, nodoSala->dato->nombre, nodoSala->dato->tematica,
                                   nodoSala->dato->horarioInicio, nodoSala->dato->horarioCierre,
                                   nodoSala->dato->aforoActual, nodoSala->dato->aforoMaximo);
                        else printf("No se encontro la sala\n");
                        break;
                    default:
                        printf("Opcion no valida\n");
                        break;
                }
                break;

            case 4:
                /*reservaProducto*/
                printf("1.Reservar 2.Mostrar cola 3.Buscar 4.Modificar 5.Eliminar (venta efectiva) 6.Eliminar (cancelada)\n");
                scanf("%d",&subopcion);
                if (subopcion < 1 || subopcion > 6) {
                    printf("Opcion no valida\n");
                    break;
                }
                printf("Codigo de la sala: ");
                scanf("%d", &salaCodigo);
                nodoSala = buscarSala(tienda->headSalas, salaCodigo);
                if (nodoSala == NULL) {
                    printf("No se encontro la sala\n");
                    break;
                }
                printf("Codigo del libro: ");
                scanf("%d", &codigoLibro);
                nodoLibro = buscarLibro(nodoSala->dato->raiz, codigoLibro);
                if (nodoLibro == NULL) {
                    printf("No se encontro el libro\n");
                    break;
                }
                libro = nodoLibro->dato;
                switch (subopcion)
                {
                    case 1:
                        printf("Rut del cliente: ");
                        scanf("%d", &rut);
                        encontrado = buscarCliente(tienda->headClientes, rut);
                        if (encontrado == NULL) {
                            printf("No se encontro el cliente\n");
                            break;
                        }
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        pase = buscarPase(encontrado->dato->pases, encontrado->dato->plibrepases, codigoPase);
                        if (pase == NULL) {
                            printf("No se encontro el pase\n");
                            break;
                        }
                        if (reservarLibro(libro, encontrado->dato, pase) == 1) printf("Reserva realizada\n");
                        else printf("No se pudo reservar (libro fuera de preventa, pase no valido o ya tiene reserva)\n");
                        break;
                    case 2:
                        mostrarCola(libro);
                        printf("En espera: %d\n", cantidadEnEspera(libro));
                        break;
                    case 3:
                        printf("Rut del cliente: ");
                        scanf("%d", &rut);
                        if (buscarReserva(libro, rut) != NULL) printf("El cliente tiene una reserva en este libro\n");
                        else printf("El cliente no tiene reserva en este libro\n");
                        break;
                    case 4:
                        printf("Rut del cliente: ");
                        scanf("%d", &rut);
                        encontrado = buscarCliente(tienda->headClientes, rut);
                        if (encontrado == NULL) {
                            printf("No se encontro el cliente\n");
                            break;
                        }
                        printf("Codigo del pase nuevo: ");
                        scanf("%d", &codigoPase);
                        pase = buscarPase(encontrado->dato->pases, encontrado->dato->plibrepases, codigoPase);
                        if (pase == NULL) {
                            printf("No se encontro el pase\n");
                            break;
                        }
                        if (modificarReserva(libro, encontrado->dato, pase) == 1) printf("Reserva modificada\n");
                        else printf("No se pudo modificar la reserva\n");
                        break;
                    case 5:
                        /* la venta respeta stock, orden de reservas y recaudacion */
                        printf("Rut del cliente: ");
                        scanf("%d", &rut);
                        if (venderEjemplares(tienda, nodoSala->dato->raiz,
                                             codigoLibro, rut, 1) == 1) {
                            ventaLibros += libro->precio;
                            printf("Venta efectiva registrada, valor %d, stock: %d\n",
                                   libro->precio, libro->ejemplaresDisponibles);
                        } else {
                            printf("Venta rechazada (stock agotado o no es el primero en la reserva)\n");
                        }
                        break;
                    case 6:
                        /* la cancelacion devuelve el ejemplar y no altera la recaudacion */
                        printf("Rut del cliente: ");
                        scanf("%d", &rut);
                        if (cancelarReserva(libro, rut) == 1) {
                            printf("Reserva cancelada\n");
                        } else {
                            printf("El cliente no tiene reserva en este libro\n");
                        }
                        break;
                }
                break;
            case 5:
                /*datos tienda*/
                printf("Fecha: %s\n", fecha);
                printf("Horario: %s a %s\n", tienda->hora_inicio, tienda->hora_termino);
                printf("Pases vendidos: %d\n", contadorPases);
                printf("Venta de pases: %d\n", ventaPases);
                printf("Venta de libros: %d\n", ventaLibros);
                printf("Recaudacion total: %d\n", tienda->recaudacion);
                break;
            case 6:
                /*pasesDelDia*/
                if (tienda->filaPasesDeldia == NULL) {
                    tienda->filaPasesDeldia = crearPasesDelDia();
                }
                printf("1.Buscar 2.Modificar 3.Eliminar 4.Listar 5.Contar por estado\n");
                scanf("%d", &subopcion);
                switch (subopcion)
                {
                    case 1:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        nodoPase = buscarPaseDelDia(tienda->filaPasesDeldia, codigoPase);
                        if (nodoPase != NULL) printf("Pase %d, Tipo: %d, Valor: %d, Estado: %d\n", nodoPase->paseVigente->codigo, nodoPase->paseVigente->tipoPase, nodoPase->paseVigente->valor, nodoPase->paseVigente->estado);
                        else printf("No se encontro el pase\n");
                        break;
                    case 2:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        printf("Nuevo tipo: ");
                        scanf("%d", &tipoPase);
                        printf("Nuevo valor: ");
                        scanf("%d", &valorPase);
                        resultado = modificarPaseDelDia(tienda->filaPasesDeldia, codigoPase, tipoPase, valorPase);
                        if (resultado == 1) printf("Pase modificado\n");
                        else if (resultado == 0) printf("No se encontro el pase\n");
                        else printf("Tipo o valor no validos\n");
                        break;
                    case 3:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        resultado = eliminarPaseDelDia(tienda->filaPasesDeldia, codigoPase);
                        if (resultado == 1) printf("Pase eliminado de los pases del dia\n");
                        else if (resultado == -1) printf("No hay pases del dia\n");
                        else printf("No se encontro el pase\n");
                        break;
                    case 4:
                        listarPasesDelDia(tienda->filaPasesDeldia);
                        break;
                    case 5:
                        printf("Estado (1.Activo 2.Anulado 3.Utilizado 4.Vencido): ");
                        scanf("%d", &subopcion);
                        printf("Cantidad: %d\n", contarPasesPorEstado(tienda->filaPasesDeldia, subopcion));
                        break;
                    default:
                        printf("Opcion no valida\n");
                        break;
                }
                break;
            case 7:
                /*pases de un cliente*/
                printf("Rut del cliente: ");
                scanf("%d", &rut);
                encontrado = buscarCliente(tienda->headClientes, rut);
                if (encontrado == NULL) {
                    printf("No se encontro el cliente\n");
                    break;
                }
                cliente = encontrado->dato;
                printf("1.Agregar 2.Buscar 3.Modificar 4.Eliminar 5.Listar 6.Ordenar por valor\n");
                scanf("%d", &subopcion);
                switch (subopcion)
                {
                    case 1:
                        printf("Tipo (1.Infantil 2.Familiar 3.General 4.Prioritario): ");
                        scanf("%d", &tipoPase);
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        printf("Valor: ");
                        scanf("%d", &valorPase);
                        if (codigoPaseUsado(tienda, codigoPase) == 1 ||
                            buscarPaseDelDia(tienda->filaPasesDeldia, codigoPase) != NULL) {
                            printf("Ya existe un pase con ese codigo\n");
                            break;
                        }
                        pase = crearPase(tipoPase, codigoPase, valorPase, 1, fecha, hora);
                        if (agregarPase(cliente->pases, &(cliente->plibrepases), MAXPASES, pase) == 1) {
                            ventaPases += pase->valor;
                            contadorPases++;
                            tienda->recaudacion += pase->valor;
                            printf("Pase agregado, valor %d\n", pase->valor);
                        } else {
                            liberarPase(pase);
                            printf("No se pudo agregar el pase (datos invalidos, codigo repetido o cliente lleno)\n");
                        }
                        break;
                    case 2:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        pase = buscarPase(cliente->pases, cliente->plibrepases, codigoPase);
                        if (pase != NULL) printf("Pase %d, Tipo: %d, Valor: %d, Estado: %d, Fecha: %s, Hora: %s\n", pase->codigo, pase->tipoPase, pase->valor, pase->estado, pase->fecha, pase->hora);
                        else printf("No se encontro el pase\n");
                        break;
                    case 3:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        pase = buscarPase(cliente->pases, cliente->plibrepases, codigoPase);
                        if (pase == NULL) {
                            printf("No se encontro el pase\n");
                            break;
                        }
                        printf("Nuevo tipo: ");
                        scanf("%d", &tipoPase);
                        printf("Nuevo valor: ");
                        scanf("%d", &valorPase);
                        resultado = modificarPase(pase, tipoPase, valorPase);
                        if (resultado == 1) printf("Pase modificado\n");
                        else printf("Tipo o valor no validos\n");
                        break;
                    case 4:
                        printf("Codigo del pase: ");
                        scanf("%d", &codigoPase);
                        resultado = eliminarPase(tienda, rut, codigoPase);
                        if (resultado == 1) printf("Pase eliminado\n");
                        else if (resultado == -1) printf("No se puede eliminar: el pase esta en la fila\n");
                        else if (resultado == -2) printf("No se puede eliminar: el pase ya ingreso\n");
                        else printf("No se encontro el pase\n");
                        break;
                    case 5:
                        listarPases(cliente->pases, cliente->plibrepases);
                        break;
                    case 6:
                        ordenarPasesPorValor(cliente->pases, cliente->plibrepases);
                        listarPases(cliente->pases, cliente->plibrepases);
                        break;
                    default:
                        printf("Opcion no valida\n");
                        break;
                }
                break;
            case 8:
                /*libros: arbol dentro de cada sala*/
                printf("1.Agregar 2.Buscar 3.Modificar precio 4.Eliminar 5.Listar 6.Recibir despacho 7.Descatalogar\n");
                scanf("%d", &subopcion);
                if (subopcion < 1 || subopcion > 7) {
                    printf("Opcion no valida\n");
                    break;
                }
                if (subopcion == 1) {
                    /*primero se pregunta si es de coleccion; solo si no lo es se pide el codigo de sala*/
                    printf("De coleccion (1 si, 0 no): ");
                    scanf("%d", &deColeccion);
                    if (deColeccion == 1) {
                        nodoSala = buscarBoveda(tienda->headSalas);
                        if (nodoSala == NULL) {
                            printf("No existe la Boveda de Coleccionistas, crearla primero\n");
                            break;
                        }
                        salaCodigo = nodoSala->dato->codigo;
                        printf("El libro se agregara a la Boveda de Coleccionistas\n");
                    } else {
                        printf("Codigo de la sala: ");
                        scanf("%d", &salaCodigo);
                        nodoSala = buscarSala(tienda->headSalas, salaCodigo);
                        if (nodoSala == NULL) {
                            printf("No se encontro la sala\n");
                            break;
                        }
                        if (esBoveda(nodoSala->dato)) {
                            printf("La boveda solo acepta libros de coleccion\n");
                            break;
                        }
                    }
                } else {
                    printf("Codigo de la sala: ");
                    scanf("%d", &salaCodigo);
                    nodoSala = buscarSala(tienda->headSalas, salaCodigo);
                    if (nodoSala == NULL) {
                        printf("No se encontro la sala\n");
                        break;
                    }
                }
                switch (subopcion)
                {
                    case 1:
                        printf("Codigo del libro: ");
                        scanf("%d", &codigoLibro);
                        if (existeLibro(nodoSala->dato->raiz, codigoLibro) == 1) {
                            printf("Ya existe un libro con ese codigo\n");
                            break;
                        }
                        printf("Nombre: ");
                        scanf(" %99[^\n]", nombreLibro);
                        printf("Precio: ");
                        scanf("%d", &precioLibro);
                        printf("Ejemplares disponibles: ");
                        scanf("%d", &ejemplares);
                        printf("Publicacion (editorial): ");
                        scanf(" %99[^\n]", publicacion);
                        printf("Clasificacion de edad: ");
                        scanf("%d", &clasificacionEdad);
                        printf("Disponibilidad (1.Disponible 2.Agotado 3.Preventa 4.Descatalogado): ");
                        scanf("%d", &disponibilidad);
                        printf("Genero: ");
                        scanf(" %99[^\n]", genero);
                        printf("Fecha de publicacion: ");
                        scanf(" %99[^\n]", fechaPublicacion);
                        libro = crearLibro(codigoLibro, nombreLibro, precioLibro, ejemplares, publicacion, clasificacionEdad, disponibilidad, genero, fechaPublicacion, deColeccion);
                        if (libro == NULL) {
                            printf("Datos invalidos o sin memoria (agotado exige 0 ejemplares; disponible y preventa exigen al menos 1)\n");
                            break;
                        }
                        nodoLibro = crearNodoLibro(libro);
                        if (nodoLibro == NULL) {
                            liberarLibro(libro);
                            free(libro);
                            printf("No se pudo agregar el libro\n");
                            break;
                        }
                        if (agregarLibroASala(tienda->headSalas, salaCodigo, nodoLibro) == 1) {
                            printf("Libro agregado a la sala %s\n", nodoSala->dato->nombre);
                        } else {
                            liberarLibro(libro);
                            free(libro);
                            free(nodoLibro);
                            printf("No se pudo agregar el libro\n");
                        }
                        break;
                    case 2:
                        printf("Codigo del libro: ");
                        scanf("%d", &codigoLibro);
                        nodoLibro = buscarLibro(nodoSala->dato->raiz, codigoLibro);
                        if (nodoLibro != NULL) {
                            printf("Libro %d: %s, precio: %d, ejemplares: %d, disponibilidad: %d, genero: %s, publicacion: %s, fecha: %s\n",
                                   nodoLibro->dato->codigo, nodoLibro->dato->nombre, nodoLibro->dato->precio,
                                   nodoLibro->dato->ejemplaresDisponibles, nodoLibro->dato->disponibilidad,
                                   nodoLibro->dato->genero, nodoLibro->dato->publicacion, nodoLibro->dato->fechaPublicacion);
                        } else printf("No se encontro el libro\n");
                        break;
                    case 3:
                        printf("Codigo del libro: ");
                        scanf("%d", &codigoLibro);
                        printf("Nuevo precio: ");
                        scanf("%d", &precioLibro);
                        resultado = modificarPrecio(nodoSala->dato->raiz, codigoLibro, precioLibro);
                        if (resultado == 1) printf("Precio modificado\n");
                        else printf("No se pudo modificar (libro inexistente o precio invalido)\n");
                        break;
                    case 4:
                        printf("Codigo del libro: ");
                        scanf("%d", &codigoLibro);
                        resultado = eliminarLibro(&(nodoSala->dato->raiz), codigoLibro);
                        if (resultado == 1) printf("Libro eliminado\n");
                        else printf("No se encontro el libro\n");
                        break;
                    case 5:
                        if (nodoSala->dato->raiz == NULL) {
                            printf("La sala no tiene libros\n");
                        } else {
                            mostrarLibrosPreorden(nodoSala->dato->raiz);
                            printf("Total de libros: %d\n", contarLibros(nodoSala->dato->raiz));
                        }
                        break;
                    case 6:
                        printf("Codigo del libro: ");
                        scanf("%d", &codigoLibro);
                        printf("Cantidad que llego: ");
                        scanf("%d", &ejemplares);
                        if (recibirDespacho(nodoSala->dato->raiz, codigoLibro, ejemplares) == 1) printf("Despacho registrado\n");
                        else printf("No se pudo registrar el despacho\n");
                        break;
                    case 7:
                        printf("Codigo del libro: ");
                        scanf("%d", &codigoLibro);
                        if (descatalogarLibro(nodoSala->dato->raiz, codigoLibro, &resultado) == 1)
                            printf("Libro descatalogado, reservas canceladas: %d\n", resultado);
                        else printf("No se encontro el libro\n");
                        break;
                }
                break;
            case 9:
                /*consultas*/
                printf("1.Libro con mas espera 2.Libros agotados 3.Tiempo estimado de espera\n");
                scanf("%d", &subopcion);
                if (subopcion == 1) {
                    libro = libroMasEspera(tienda);
                    if (libro == NULL) printf("Nadie esta en lista de espera\n");
                    else printf("%s: %d en espera\n", libro->nombre, cantidadEnEspera(libro));
                } else if (subopcion == 2) {
                    for (nodoSala = tienda->headSalas; nodoSala != NULL; nodoSala = nodoSala->sig) {
                        listarAgotadosArbol(nodoSala->dato->raiz);
                    }
                } else if (subopcion == 3) {
                    printf("Codigo de la sala: ");
                    scanf("%d", &salaCodigo);
                    nodoSala = buscarSala(tienda->headSalas, salaCodigo);
                    if (nodoSala == NULL) {
                        printf("No se encontro la sala\n");
                        break;
                    }
                    printf("Codigo del libro: ");
                    scanf("%d", &codigoLibro);
                    nodoLibro = buscarLibro(nodoSala->dato->raiz, codigoLibro);
                    if (nodoLibro == NULL) {
                        printf("No se encontro el libro\n");
                        break;
                    }
                    printf("Ejemplares por despacho: ");
                    scanf("%d", &ejemplares);
                    printf("Dias entre despachos: ");
                    scanf("%d", &diasEntre);
                    resultado = tiempoEsperaEstimado(nodoLibro->dato, ejemplares, diasEntre);
                    if (resultado < 0) printf("Datos invalidos\n");
                    else printf("Espera estimada: %d dias\n", resultado);
                } else {
                    printf("Opcion no valida\n");
                }
                break;
            case 0:
                estadoTienda = 0;
                break;

            default:
                printf("no se selecciono ninguna opcion Valida\n");
                break;
            }
    }

    /* al salir: vaciar la fila de entrada y los pases del dia */
    if (tienda->filaEntrada != NULL) {
        nodoPase = desencolarFila(tienda->filaEntrada);
        while (nodoPase != NULL) {
            free(nodoPase);
            nodoPase = desencolarFila(tienda->filaEntrada);
        }
        free(tienda->filaEntrada);
        tienda->filaEntrada = NULL;
    }
    if (tienda->filaPasesDeldia != NULL) {
        while (tienda->filaPasesDeldia->head != NULL) {
            eliminarPaseDelDia(tienda->filaPasesDeldia, tienda->filaPasesDeldia->head->paseVigente->codigo);
        }
        free(tienda->filaPasesDeldia);
        tienda->filaPasesDeldia = NULL;
    }

    printf("\n--- Cierre de la tienda ---\n");
    printf("Venta de pases: %d (%d pases)\n", ventaPases, contadorPases);
    printf("Venta de libros: %d\n", ventaLibros);
    printf("Recaudacion total: %d\n", tienda->recaudacion);

    return 0;
}
