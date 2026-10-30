#include "../../include/libros.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
