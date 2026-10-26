#include "../../include/libros.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
-los datos son: codigo, nombre, precio, ejemplares, publicacion, clasificacionEdad, disponibilidad, genero, fechaPublicacion y deColeccion (lectura en el main)
-retorna el puntero al libro creado
*/

struct Libro *crearLibro(int codigo, char *nombre, int precio, int ejemplares,
    char *publicacion, int clasificacionEdad,
    int disponibilidad, char *genero, char *fechaPublicacion, int deColeccion) {

    struct Libro *nuevoLibro = malloc(sizeof(*nuevoLibro));
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
    nuevoLibro->recaudacionDia = 0;

    nuevoLibro->nombre = copiarTexto(nombre);
    nuevoLibro->publicacion = copiarTexto(publicacion);
    nuevoLibro->genero = copiarTexto(genero);
    nuevoLibro->fechaPublicacion = copiarTexto(fechaPublicacion);
    if (nuevoLibro->nombre == NULL || nuevoLibro->publicacion == NULL ||
        nuevoLibro->genero == NULL || nuevoLibro->fechaPublicacion == NULL) {
        liberarLibro(nuevoLibro);
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
