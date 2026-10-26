#ifndef LIBROS_H
#define LIBROS_H

#include "estructuras.h"

struct Libro *crearLibro(int codigo, char *nombre, int precio, int ejemplares,
                         char *publicacion, int clasificacionEdad,
                         int disponibilidad, char *genero,
                         char *fechaPublicacion, int deColeccion);
struct NodoLibro *crearNodoLibro(struct Libro *dato);
struct NodoLibro *buscarLibro(struct NodoLibro *raiz, int codigo);
int insertarLibro(struct NodoLibro **raiz, struct NodoLibro *nuevo);
int existeLibro(struct NodoLibro *raiz, int codigo);
int contarLibros(struct NodoLibro *raiz);
void mostrarLibrosPreorden(struct NodoLibro *raiz);
int modificarPrecio(struct NodoLibro *raiz, int codigo, int nuevoPrecio);
int venderEjemplar(struct NodoLibro *raiz, int codigo);
int venderEjemplares(struct NodoLibro *raiz, int codigo, int rut, int cantidad);
int recibirDespacho(struct NodoLibro *raiz, int codigo, int cantidad);
int calcularRecaudacionDiaria(struct Tienda *tienda);
void reiniciarRecaudacionDiaria(struct Tienda *tienda);
int descatalogarLibro(struct NodoLibro *raiz, int codigo);
void liberarLibro(struct Libro *libro);
int eliminarLibro(struct NodoLibro **raiz, int codigo);

#endif
