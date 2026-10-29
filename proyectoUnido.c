
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define MAXPASES 50


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
    struct Pase **pases;
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
    int codigo;
    int aforoAprox;
    struct NodoLibro *raiz;
    /*falta nombre,falta aforoMax y horario de atencion */
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
    struct pasesDelDia *filaPasesDeldia;            /* fila/lista de pases del día */
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


/*funciones salas */
/*=======================================================================================================*/
struct SalaTematica *crearSala(char *tematica,int codigo,int aforoAprox){

    struct SalaTematica *nuevo;

    nuevo = (struct SalaTematica*)malloc(sizeof(struct SalaTematica));
    nuevo->tematica = (char*)malloc(strlen(tematica)+1*sizeof(char));
    strcpy(nuevo->tematica,tematica);
    nuevo->codigo = codigo;
    nuevo->aforoAprox = aforoAprox;
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
        printf("nombre : %s aforo aproximado %d codigo de sala %d\n",headSalas->dato->tematica,headSalas->dato->aforoAprox,headSalas->dato->codigo);
        headSalas = headSalas->sig;
    }
}

int modificarSala(struct NodoSala **headSalas, int codigo, int nuevoCodigo,int nuevoAforo, char *nuevoNombre) {

    struct NodoSala *rec;
    char *copia;

    if (*headSalas == NULL) {
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
    copia = (char *)malloc(strlen(nuevoNombre) + 1);
    if (copia == NULL) {
        return 0; /* no se toco ningun campo */
    }
    strcpy(copia, nuevoNombre);
    rec->dato->tematica = copia;
    rec->dato->codigo = nuevoCodigo;
    rec->dato->aforoAprox = nuevoAforo;
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

static int tieneReservaArbol(struct NodoLibro *raiz, int rut) {
    if (raiz == NULL) {
        return 0;
    }
    if (buscarReserva(raiz->dato, rut) != NULL) {
        return 1;
    }
    return tieneReservaArbol(raiz->izq, rut) ||
           tieneReservaArbol(raiz->der, rut);
}

/* al eliminar un cliente (nodo), el nodo anterior debe saltarse el eliminado. todos los nodos aqui solo tienen "sig" */
/* "ant" sirve para recordar cual es el nodo anterior mientras recorre */
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

    for (sala = tienda->headSalas; sala != NULL; sala = sala->sig) {
        if (tieneReservaArbol(sala->dato->raiz, rut)) {
            return -3;
        }
    }

    for (i = 0; i < MAXPASES; i++) {
        if (cliente->pases[i] != NULL && cliente->pases[i]->estado == 1) {
            return -4;
        }
    }

    if (act->sig == act) {
        tienda->headClientes = NULL;
    } else {
        ant = tienda->headClientes;
        while (ant->sig != act) {
            ant = ant->sig;
        }
        ant->sig = act->sig;
        if (act == tienda->headClientes) {
            tienda->headClientes = act->sig;
        }
    }

    for (i = 0; i < MAXPASES; i++) {
        liberarPase(cliente->pases[i]);
    }
    free(cliente->nombre);
    free(cliente->pases);
    free(cliente);
    free(act);
    return 1;
}

int modificarCliente(struct NodoCliente *head, int rut, char *nuevoNombre, int nuevoRut) { /* 1 : se modifico, 0: no existe, -1: rut ya existe */
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

    copia = malloc(strlen(nuevoNombre) + 1);
    if (copia == NULL) {
        return -2;
    }
    strcpy(copia, nuevoNombre);
    free(nodo->dato->nombre);
    nodo->dato->nombre = copia;
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
-sirve para que quien haga eliminarPase sepa desde donde compactar el arreglo
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
/*=======================================================================================================*/
/*funciones de arbol*/


/*
 * Crea una copia dinamica de un texto para que el objeto destino sea propietario
 * de su memoria; retorna NULL si el texto no existe o malloc falla.
 */
char *copiarTexto(char *texto) {
    char *copia;

    /* No se puede copiar una cadena inexistente. */
    if (texto == NULL) {
        return NULL;
    }

    /* Reserva espacio para el terminador y copia solo si la reserva tuvo exito. */
    copia = malloc(strlen(texto) + 1);
    if (copia != NULL) {
        strcpy(copia, texto);
    }
    return copia;
}

/*
-crea un libro en memoria dinamica con los datos recibidos
-los datos son: codigo, nombre, precio, ejemplares, publicacion, clasificacionEdad, disponibilidad, genero, fechaPublicacion y deColeccion (lectura en el main)
-retorna el puntero al libro creado
*/

struct Libro *crearLibro(int codigo, char *nombre, int precio, int ejemplares,
    char *publicacion, int clasificacionEdad,
    int disponibilidad, char *genero, char *fechaPublicacion, int deColeccion) {

    struct Libro *nuevoLibro;

    if (codigo <= 0 || nombre == NULL || precio < 0 || ejemplares < 0 ||
        publicacion == NULL || genero == NULL || fechaPublicacion == NULL ||
        disponibilidad < 1 || disponibilidad > 4 ||
        (disponibilidad == 1 && ejemplares == 0) ||
        ((disponibilidad == 2 || disponibilidad == 3) && ejemplares != 0)) {
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
    /* Las reservas pertenecen al libro y comienzan como una lista vacia. */
    nuevoLibro->headReserva = NULL;

    /* Copia cada texto para que el libro sea propietario de su memoria. */
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
-recibe un puntero a libro y retorna un puntero a nodo de libro
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
    /* El nodo enlaza el libro recibido; no duplica ni libera ese objeto. */
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
    /* Una rama vacia indica que el codigo no esta en el arbol. */
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
-recibe la raiz del subarbol, que no debe ser NULL
*/
struct NodoLibro *menorNodo(struct NodoLibro *raiz) {
    /* El sucesor minimo se encuentra siguiendo enlaces izquierdos. */
    while (raiz->izq != NULL) {
        raiz = raiz->izq;
    }
    return raiz;
}

/*
-inserta un nodo de libro en el arbol binario de busqueda, ordenado por codigo
-los codigos menores van a la izquierda y los mayores a la derecha
-no permite codigos repetidos
-recibe la raiz por doble puntero porque puede cambiar cuando el arbol esta vacio
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
        /*el codigo ya existe, no se inserta*/
        return 0 ;
    }
}

/*
-indica si ya existe un libro con el codigo dado
-se usa en el menu antes de crear un libro, para no pedir datos de mas
-retorna 1 si el codigo existe, 0 si no existe
*/

int existeLibro(struct NodoLibro *raiz, int codigo){
    /* La busqueda devuelve NULL cuando el codigo no aparece en el arbol. */
    return buscarLibro(raiz, codigo) != NULL ;
}

/*
-cuenta cuantos libros hay en el arbol
-retorna la cantidad de libros, 0 si el arbol esta vacio
*/

int contarLibros(struct NodoLibro *raiz) {
    if (raiz == NULL) {
        return 0 ; /*no hay libros en este subarbol*/
    }

    /*cuenta este libro y los de los subarboles izquierdo y derecho*/
    return 1 + contarLibros(raiz->izq) + contarLibros(raiz->der) ;
}

/*
-muestra todos los libros del arbol recorriendolo en preorden
-en cada nodo muestra el codigo y el nombre del libro
-primero el nodo actual, luego el subarbol izquierdo y luego el derecho
-solo recibe la raiz, no modifica el arbol por el puntero, por eso no es doble puntero
*/

void mostrarLibrosPreorden(struct NodoLibro *raiz) {

    /*caso base: arbol vacio, no hay nada que mostrar*/
    if (raiz == NULL) {
        return ;
    }

    printf("Codigo: %d, Nombre: %s\n", raiz->dato->codigo, raiz->dato->nombre) ;
    mostrarLibrosPreorden(raiz->izq) ;
    mostrarLibrosPreorden(raiz->der) ;
}

/*
-modifica el precio de un libro existente
-el codigo no se modifica porque es la llave del arbol: cambiarlo dejaria al libro en un lugar incorrecto
-los ejemplares no se modifican aqui, cambian con venderEjemplar y recibirDespacho
-no acepta precios negativos
-recibe la raiz del arbol, el codigo del libro y el precio nuevo
-retorna 1 si se modifico, 0 si no existe el libro o el precio es invalido
*/

int modificarPrecio(struct NodoLibro *raiz, int codigo, int nuevoPrecio) {

    struct NodoLibro *nodo ;
    /* Busca primero para no modificar nada si el codigo no esta registrado. */
    nodo = buscarLibro(raiz, codigo) ;

    if (nodo == NULL) {
        return 0 ; /*no se encontro el libro*/
    }

    if (nuevoPrecio < 0) {
        return 0 ; /*precio invalido*/
    }

    nodo->dato->precio = nuevoPrecio ;

    return 1 ; /*se modifico el precio*/
}

/*
-registra la llegada de un despacho: sube ejemplaresDisponibles en la cantidad recibida
-no acepta cantidades menores o iguales a 0 ni libros descatalogados (4)
-si el libro estaba agotado (2), vuelve a quedar disponible (1)
-recibe la raiz del arbol, el codigo del libro y la cantidad de ejemplares que llegaron
-retorna 1 si se registro el despacho, 0 si no se pudo
*/

int recibirDespacho(struct NodoLibro *raiz, int codigo, int cantidad) {

    struct NodoLibro *nodo ;
    /* Resuelve el codigo antes de validar y aplicar el aumento de stock. */
    nodo = buscarLibro(raiz, codigo) ;

    if (nodo == NULL) {
        return 0 ; /*no se encontro el libro*/
    }

    if (cantidad <= 0 || nodo->dato->ejemplaresDisponibles < 0 ||
        nodo->dato->ejemplaresDisponibles > INT_MAX - cantidad) {
        return 0 ; /*cantidad invalida*/
    }

    if (nodo->dato->disponibilidad == 4) {
        return 0 ; /*descatalogado (4): no recibe despachos*/
    }

    nodo->dato->ejemplaresDisponibles += cantidad ;

    if (nodo->dato->disponibilidad == 2 ||
        nodo->dato->disponibilidad == 3) {
        /* Al llegar stock, reactiva agotados y libros que estaban en preventa. */
        nodo->dato->disponibilidad = 1 ;
    }

    return 1 ; /*despacho registrado*/
}

static int registrarVenta(struct Tienda *tienda, struct Libro *libro,
                          int cantidad, int rut, int tieneComprador) {
    int unidadesVendidas;
    int ingreso;
    int reservasPendientes;

    if (tienda == NULL || libro == NULL || cantidad <= 0 ||
        libro->disponibilidad != 1 || libro->ejemplaresDisponibles <= 0 ||
        libro->precio < 0 || tienda->recaudacion < 0) {
        return 0;
    }

    reservasPendientes = libro->headReserva != NULL;
    if (reservasPendientes &&
        (!tieneComprador || libro->headReserva->dato == NULL ||
         libro->headReserva->dato->reservador == NULL ||
         libro->headReserva->dato->reservador->rut != rut)) {
        return 0;
    }

    unidadesVendidas = cantidad;
    if (unidadesVendidas > libro->ejemplaresDisponibles) {
        unidadesVendidas = libro->ejemplaresDisponibles;
    }
    if (reservasPendientes && unidadesVendidas > 1) {
        unidadesVendidas = 1;
    }
    if (libro->precio > 0 &&
        unidadesVendidas > (INT_MAX - tienda->recaudacion) / libro->precio) {
        return 0;
    }

    ingreso = unidadesVendidas * libro->precio;
    libro->ejemplaresDisponibles -= unidadesVendidas;
    tienda->recaudacion += ingreso;
    if (reservasPendientes) {
        eliminarReserva(libro, rut);
    }
    if (libro->ejemplaresDisponibles == 0) {
        libro->disponibilidad = 2;
    }

    return unidadesVendidas;
}

int venderEjemplares(struct Tienda *tienda, struct NodoLibro *raiz,
                     int codigo, int rut, int cantidad) {
    struct NodoLibro *nodo;

    if (tienda == NULL || cantidad <= 0) {
        return 0;
    }
    nodo = buscarLibro(raiz, codigo);
    if (nodo == NULL) {
        return 0;
    }
    return registrarVenta(tienda, nodo->dato, cantidad, rut, 1);
}

/*
-marca un libro como descatalogado (4): la editorial no lo reimprime
-el nodo NO se borra del arbol, porque el libro sigue existiendo en la tienda y su informacion se conserva
-despues de esto, venderEjemplar y recibirDespacho ya no operan sobre ese libro
-informa cuantas reservas se cancelaron; el puntero de salida puede ser NULL
-recibe la raiz del arbol y el codigo del libro
-retorna 1 si se marco como descatalogado, 0 si no existe un libro con ese codigo
*/

int descatalogarLibro(struct NodoLibro *raiz, int codigo,
                      int *reservasCanceladas) {

    struct NodoLibro *nodo ;

    /* Si se solicito la salida, inicializarla evita dejar un valor antiguo. */
    if (reservasCanceladas != NULL) {
        *reservasCanceladas = 0;
    }

    nodo = buscarLibro(raiz, codigo) ;

    if (nodo == NULL) {
        return 0 ; /*no se encontro el libro*/
    }

    if (reservasCanceladas != NULL) {
        *reservasCanceladas = cantidadEnEspera(nodo->dato);
    }
    vaciarCola(nodo->dato);
    nodo->dato->disponibilidad = 4 ; /*descatalogado (4)*/

    return 1 ; /*libro descatalogado*/
}

/*
-deja un libro desvinculado de sus datos, sin usar free
-vacia su cola de reservas y deja en NULL los 4 textos; el struct no se libera
-NO toca el nodo del arbol: eso lo hace eliminarLibro
*/

void liberarLibro(struct Libro *libro) {

    /*si no hay libro, no hay nada que desvincular*/
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
-elimina un libro del arbol segun su codigo y lo desvincula, sin usar free
-recibe la raiz por doble puntero porque puede cambiar (por ejemplo, al eliminar la raiz)
-sin hijos o con un hijo: el padre pasa a apuntar al hijo (o a NULL) y el nodo queda aislado con sus punteros en NULL
-con dos hijos: se intercambian los libros entre el nodo y su sucesor (menor del subarbol derecho) y se elimina el sucesor, que tiene a lo mas un hijo
-retorna 1 si se elimino, 0 si no existe un libro con ese codigo
*/

int eliminarLibro(struct NodoLibro **raiz, int codigo) {

    struct NodoLibro *aux ;
    struct Libro *libro ;

    /*caso base: lugar vacio, el codigo no existe*/
    if (*raiz == NULL) {
        return 0 ;
    }

    /*se busca el nodo igual que en insertarLibro*/
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

    /*el nodo sale aislado y se liberan el libro y el nodo eliminados*/
    aux->izq = NULL ;
    aux->der = NULL ;
    liberarLibro(aux->dato);
    free(aux->dato);
    free(aux);
    return 1 ;
}
/*=======================================================================================================*/
/*funciones de reserva*/
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
    if (libro == NULL || cliente == NULL || cliente->pases == NULL ||
        pase == NULL || libro->disponibilidad != 3 ||
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
-desenlaza el nodo y deja sus punteros en NULL, sin liberar memoria ni tocar al cliente
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

    free(rec->dato);
    free(rec);
    return 1;
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
    if (libro == NULL || cliente == NULL || cliente->pases == NULL ||
        nuevoPase == NULL || libro->disponibilidad != 3 ||
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

/*=======================================================================================================*/
/*funciones filaEntrada*/

/*
-crea una fila vacia reservando memoria dinamica
-inicializa head y tail en NULL
-retorna la fila creada o NULL si no hay memoria disponible
*/
struct Fila *crearFila(void) {
    struct Fila *fila = malloc(sizeof(*fila));

    if (fila == NULL) {
        return NULL;
    }

    /* Una fila recien creada no tiene nodos enlazados. */
    fila->head = NULL;
    fila->tail = NULL;
    return fila;
}

/*
-agrega un nodo de pase a la fila
-rechaza pases invalidos, repetidos o nodos que ya esten enlazados
-inserta los pases de tipo 4 antes de los demas, conservando el orden entre prioritarios;
los otros tipos se agregan al final
-retorna 1 si el nodo se agrego y 0 si no se pudo agregar
-si tiene exito, la fila enlaza el nodo; el pase sigue siendo externo
*/
int encolarFila(struct Fila *fila, struct NodoPase *nuevo) {
    struct NodoPase *rec;
    struct NodoPase *anterior;

    if (fila == NULL || nuevo == NULL || nuevo->paseVigente == NULL ||
        nuevo->sig != NULL || nuevo->paseVigente->tipoPase < 1 ||
        nuevo->paseVigente->tipoPase > 4) {
        return 0;
    }

    /* Rechaza referencias o codigos de pase que ya esten esperando. */
    anterior = NULL;
    for (rec = fila->head; rec != NULL; rec = rec->sig) {
        if (rec == nuevo || rec->paseVigente == NULL ||
            rec->paseVigente == nuevo->paseVigente ||
            rec->paseVigente->codigo == nuevo->paseVigente->codigo) {
            return 0;
        }
        anterior = rec;
    }

    /* Comprueba que head y tail coincidan con los enlaces existentes. */
    if ((fila->head == NULL && fila->tail != NULL) ||
        (fila->head != NULL && anterior != fila->tail)) {
        return 0;
    }

    /* En una fila vacia, el nodo nuevo ocupa ambos extremos. */
    if (fila->head == NULL) {
        fila->head = nuevo;
        fila->tail = nuevo;
    } else if (nuevo->paseVigente->tipoPase != 4) {
        /* Los pases no prioritarios se agregan al final en orden de llegada. */
        fila->tail->sig = nuevo;
        fila->tail = nuevo;
    } else {
        /* Busca el final del grupo prioritario para mantener su orden FIFO. */
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
-actualiza head y tail; ambos quedan en NULL si se retira el ultimo nodo
-retorna el nodo retirado, o NULL si la fila es NULL o esta vacia
-quien llama decide que hace con el nodo retirado; el pase guardado no se toca
*/
struct NodoPase *desencolarFila(struct Fila *fila) {
    struct NodoPase *primero;

    if (fila == NULL || fila->head == NULL) {
        return NULL;
    }

    /* Avanza head y conserva el nodo retirado para devolverlo al llamador. */
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
-recorre desde head hasta encontrar el codigo o llegar al final
-retorna el nodo encontrado o NULL si la fila es NULL o no existe ese codigo
-no modifica la fila ni libera memoria
*/
struct NodoPase *buscarEnFila(struct Fila *fila, int codigoPase) {
    struct NodoPase *rec;

    if (fila == NULL) {
        return NULL;
    }

    /* Examina cada pase enlazado y compara su codigo. */
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
-actualiza head o tail cuando elimina un extremo
-desenlaza el nodo y deja su sig en NULL; no libera memoria ni toca el pase
-retorna 1 si lo quito y 0 si la fila es NULL o no encontro el codigo
*/
int quitarDeFila(struct Fila *fila, int codigoPase) {
    struct NodoPase *rec;
    struct NodoPase *anterior = NULL;

    if (fila == NULL) {
        return 0;
    }

    /* Conserva anterior para reconectar la lista al encontrar el pase. */
    rec = fila->head;
    while (rec != NULL && (rec->paseVigente == NULL || rec->paseVigente->codigo != codigoPase)) {
        anterior = rec;
        rec = rec->sig;
    }
    if (rec == NULL) {
        return 0;
    }

    /* Actualiza el extremo inicial o el enlace del nodo anterior. */
    if (anterior == NULL) {
        fila->head = rec->sig;
    } else {
        anterior->sig = rec->sig;
    }
    if (fila->tail == rec) {
        fila->tail = anterior;
    }
    /* El nodo sale aislado de la fila; el pase que apunta no se toca. */
    rec->sig = NULL;
    return 1;
}

/*
-desenlaza todos los nodos de la fila, dejando el sig de cada uno en NULL
-deja head y tail en NULL para que la fila pueda reutilizarse
-no libera memoria ni toca los pases apuntados por los nodos
-si fila es NULL, no realiza cambios
*/
void vaciarFila(struct Fila *fila) {
    struct NodoPase *rec;
    struct NodoPase *siguiente;

    if (fila == NULL) {
        return;
    }

    /* Guarda el siguiente enlace antes de dejar en NULL el del nodo actual. */
    for (rec = fila->head; rec != NULL; rec = siguiente) {
        siguiente = rec->sig;
        rec->sig = NULL;
    }
    fila->head = NULL;
    fila->tail = NULL;
}

/*
-deja la fila sin nodos para poder soltarla (no usa free)
-desenlaza todos sus nodos; la estructura Fila no se libera
-no toca los pases apuntados por los nodos
-si fila es NULL, no realiza ninguna accion
*/
void liberarFila(struct Fila *fila) {
    if (fila != NULL) {
        /* Solo desenlaza los nodos; quien tenia la fila decide si la suelta. */
        vaciarFila(fila);
    }
}
/*=======================================================================================================*/

static int limpiarLineaEntrada(void) {
    int caracter;

    while ((caracter = getchar()) != '\n' && caracter != EOF) {
    }
    return caracter != EOF;
}

int main(void) {
    int opcion, subopcion;
    int salaCodigo, salaAforoAprox, nuevoCodigo, nuevoAforo;
    int resultado, estadoTienda;
    int rut, nuevoRut;
    int codigoPase, tipoPase, valorPase, codigoLibro;
    int precioLibro, ejemplares, clasificacionEdad, disponibilidad, deColeccion;
    int pos, i;
    int ventaPases, ventaLibros, contadorPases;
    int horaCierre, minCierre, minutosCierre, minutosAhora, minutosRestantes;
    int avisoMinutos;
    int dia, mes, anio, hAhora, mAhora, leidos;
    char salaTematica[100];
    char nuevoNombre[100];
    char nombre[100];
    char nombreLibro[100];
    char publicacion[100];
    char genero[100];
    char fechaPublicacion[100];
    char fecha[11];                /* dd/mm/aaaa */
    char hora[6];                  /* hh:mm */
    char horaInicio[6];
    char horaTermino[6];
    struct Tienda *tienda;
    struct NodoSala *nodoSala;
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

        printf("1.Fila de entrada 2.Clientes 3.Salas 4.Reservas 5.Datos tienda 6.Pases del dia 7.Pases de un cliente 8.Libros 0.Salir\n");
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
                    case 2:/*revisar por que sacamos el pase de la fila pero no esta metido en pasesDelDia*/
                        /* atender: sale el primero de la fila y el pase queda utilizado */
                        nodoPase = desencolarFila(tienda->filaEntrada);
                        if (nodoPase == NULL) {
                            printf("La fila esta vacia\n");
                        } else {
                            nodoPase->paseVigente->estado = 3;   /* utilizado */
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
                    case 5:/*listar hay que modularizar*/
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
                        else if (resultado == -4) printf("No se puede eliminar: tiene pases activos\n");
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
                        else printf("El nuevo rut ya existe\n");
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
                        printf("Tematica: ");
                        scanf("%99s", salaTematica);
                        printf("Codigo: ");
                        scanf("%d", &salaCodigo);
                        printf("Aforo aproximado: ");
                        scanf("%d", &salaAforoAprox);
                        if (buscarSala(tienda->headSalas, salaCodigo) != NULL) {
                            printf("Ya existe una sala con ese codigo\n");
                            break;
                        }
                        insertarSala(&(tienda->headSalas), crearNodoSala(crearSala(salaTematica, salaCodigo, salaAforoAprox)));
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
                        printf("Nuevo aforo: ");
                        scanf("%d", &nuevoAforo);
                        printf("Nuevo nombre: ");
                        scanf("%99s", nuevoNombre);
                        resultado = modificarSala(&(tienda->headSalas), salaCodigo, nuevoCodigo, nuevoAforo, nuevoNombre);
                        if (resultado == 1) printf("Sala modificada\n");
                        else printf("No se pudo modificar\n");
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
                            printf("Sala: %s, aforo: %d, codigo: %d\n", nodoSala->dato->tematica, nodoSala->dato->aforoAprox, nodoSala->dato->codigo);
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
                        /* La venta respeta stock, orden de reservas y recaudacion. */
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
                        /* La cancelacion no altera el stock ni la recaudacion. */
                        printf("Rut del cliente: ");
                        scanf("%d", &rut);
                        if (eliminarReserva(libro, rut) == 1) {
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
                printf("1.Agregar 2.Buscar 3.Modificar 4.Eliminar 5.Listar\n");
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
                        if (buscarPaseDelDia(tienda->filaPasesDeldia, codigoPase) != NULL) {
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
                        pos = buscarPosicionPase(cliente->pases, cliente->plibrepases, codigoPase);
                        if (pos == -1) {
                            printf("No se encontro el pase\n");
                            break;
                        }
                        pase = cliente->pases[pos];
                        /* se saca de la fila y de los pases del dia antes de liberarlo */
                        nodoPase = buscarEnFila(tienda->filaEntrada, codigoPase);
                        if (nodoPase != NULL && nodoPase->paseVigente == pase) {
                            quitarDeFila(tienda->filaEntrada, codigoPase);
                            free(nodoPase);
                        }
                        eliminarPaseDelDia(tienda->filaPasesDeldia, codigoPase);
                        /* se compacta el arreglo del cliente desde pos */
                        for (i = pos; i < cliente->plibrepases - 1; i++) {
                            cliente->pases[i] = cliente->pases[i + 1];
                        }
                        cliente->plibrepases--;
                        cliente->pases[cliente->plibrepases] = NULL;
                        liberarPase(pase);
                        printf("Pase eliminado\n");
                        break;
                    case 5:
                        listarPases(cliente->pases, cliente->plibrepases);
                        break;
                    default:
                        printf("Opcion no valida\n");
                        break;
                }
                break;
            case 8:
                /*libros: arbol dentro de cada sala*/
                printf("1.Agregar 2.Buscar 3.Modificar precio 4.Eliminar 5.Listar\n");
                scanf("%d", &subopcion);
                if (subopcion < 1 || subopcion > 5) {
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
                        printf("De coleccion (1 si, 0 no): ");
                        scanf("%d", &deColeccion);
                        libro = crearLibro(codigoLibro, nombreLibro, precioLibro, ejemplares, publicacion, clasificacionEdad, disponibilidad, genero, fechaPublicacion, deColeccion);
                        if (libro == NULL) {
                            printf("Datos invalidos o sin memoria (preventa y agotado exigen 0 ejemplares, disponible exige al menos 1)\n");
                            break;
                        }
                        nodoLibro = crearNodoLibro(libro);
                        if (nodoLibro != NULL && insertarLibro(&(nodoSala->dato->raiz), nodoLibro) == 1) printf("Libro agregado\n");
                        else printf("No se pudo agregar el libro\n");
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