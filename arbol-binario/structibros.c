#include "../estructuras.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
-crea un libro en memoria dinamica con los datos recibidos
-los datos son: codigo, nombre, precio, ejemplares, publicacion, clasificacionEdad, disponibilidad, genero, fechaPublicacion y deColeccion (lectura en el main)
-retorna el puntero al libro creado
*/

struct Libro *crearLibro(int codigo, char *nombre, int precio, int ejemplares,
    char *publicacion, int clasificacionEdad,
    int disponibilidad, char *genero, char *fechaPublicacion, int deColeccion) {

    struct Libro *nuevoLibro ;

    nuevoLibro = (struct Libro *)malloc(sizeof(struct Libro)) ;


    /*los campos numericos se copian directamente*/
    nuevoLibro -> codigo = codigo ;
    nuevoLibro -> precio = precio ;
    nuevoLibro -> ejemplaresDisponibles = ejemplares ;
    nuevoLibro -> clasificacionEdad = clasificacionEdad ;
    nuevoLibro -> disponibilidad = disponibilidad ;
    nuevoLibro -> deColeccion = deColeccion ;


    /*los campos de tipo char* se copian con malloc y strcpy para evitar problemas de memoria*/
    nuevoLibro -> nombre = (char *)malloc(strlen(nombre) + 1) ;
    strcpy(nuevoLibro -> nombre, nombre) ;

    nuevoLibro -> publicacion = (char *)malloc(strlen(publicacion) + 1) ;
    strcpy(nuevoLibro -> publicacion, publicacion) ;

    nuevoLibro -> genero = (char *)malloc(strlen(genero) + 1) ;
    strcpy(nuevoLibro -> genero, genero) ;

    nuevoLibro -> fechaPublicacion = (char *)malloc(strlen(fechaPublicacion) + 1) ;
    strcpy(nuevoLibro -> fechaPublicacion, fechaPublicacion) ;

    nuevoLibro -> headReserva = NULL ;

    return nuevoLibro ;
}

/*
-hace que un libro se convierta en un nodo de arbol binario
-recibe un puntero a libro y retorna un puntero a nodo de libro
*/

struct NodoLibro *crearNodoLibro(struct Libro *dato) {

    struct NodoLibro *nuevoNodo = (struct NodoLibro *)malloc(sizeof(struct NodoLibro)) ;

    nuevoNodo -> dato = dato ;
    nuevoNodo -> izq = NULL ;
    nuevoNodo -> der = NULL ;

    return nuevoNodo ;
}


/*
-busca un libro por codigo aprovechando que el arbol esta ordenado
-recibe la raiz del arbol y el codigo a buscar
-retorna el nodo si lo encuentra, o NULL si no existe
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
-recibe la raiz del subarbol, que no debe ser NULL
*/

struct NodoLibro *menorNodo(struct NodoLibro *raiz) {
    while (raiz->izq != NULL) {
        raiz = raiz->izq ;
    }
    return raiz ;
}
