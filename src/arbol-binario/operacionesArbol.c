#include "../../include/libros.h"
#include "../../include/reservas.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

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
    libro->nombre = NULL ;
    libro->publicacion = NULL ;
    libro->genero = NULL ;
    libro->fechaPublicacion = NULL ;
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

    /*el nodo sale aislado: lo que apuntaba queda en NULL (no se usa free)*/
    aux->izq = NULL ;
    aux->der = NULL ;
    liberarLibro(aux->dato) ;   /*vacia las reservas y deja en NULL los textos del libro*/
    aux->dato = NULL ;
    return 1 ;
}