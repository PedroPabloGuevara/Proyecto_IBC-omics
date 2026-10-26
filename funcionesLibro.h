#ifndef FUNCIONES_LIBRO_H
#define FUNCIONES_LIBRO_H

#include "estructuras.h"

struct Libro *crearLibro(int codigo, char *nombre, int precio, int ejemplares,
    char *publicacion, int clasificacionEdad, int disponibilidad,
    char *genero, char *fechaPublicacion, int deColeccion);
struct NodoLibro *crearNodoLibro(struct Libro *dato);
int insertarLibro(struct NodoLibro **raiz, struct NodoLibro *nuevo);
struct NodoLibro *buscarLibro(struct NodoLibro *raiz, int codigo);
struct NodoLibro *menorNodo(struct NodoLibro *raiz);
int existeLibro(struct NodoLibro *raiz, int codigo);
int contarLibros(struct NodoLibro *raiz);
void mostrarLibrosPreorden(struct NodoLibro *raiz);
int modificarPrecio(struct NodoLibro *raiz, int codigo, int nuevoPrecio);
int venderEjemplar(struct NodoLibro *raiz, int codigo);
int recibirDespacho(struct NodoLibro *raiz, int codigo, int cantidad);
int descatalogarLibro(struct NodoLibro *raiz, int codigo);
void liberarLibro(struct Libro *libro);
int eliminarLibro(struct NodoLibro **raiz, int codigo);

#endif
