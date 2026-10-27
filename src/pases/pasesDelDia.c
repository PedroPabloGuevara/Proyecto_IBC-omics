#include "../../include/pases.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
-crea un pase con los datos recibidos y copia fecha y hora
-retorna NULL si los textos son NULL o falla una reserva de memoria
*/
struct Pase *crearPase(int tipoPase, int codigo, int valor, int estado,
                       char *fecha, char *hora) {
    struct Pase *nuevo;

    if (fecha == NULL || hora == NULL) {
        return NULL;
    }

    /* Reserva el pase y sus textos por separado para que cada objeto los posea. */
    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }

    nuevo->fecha = malloc(strlen(fecha) + 1);
    if (nuevo->fecha == NULL) {
        free(nuevo);
        return NULL;
    }

    nuevo->hora = malloc(strlen(hora) + 1);
    if (nuevo->hora == NULL) {
        free(nuevo->fecha);
        free(nuevo);
        return NULL;
    }

    strcpy(nuevo->fecha, fecha);
    strcpy(nuevo->hora, hora);
    /* Copia los datos simples despues de reservar correctamente ambos textos. */
    nuevo->tipoPase = tipoPase;
    nuevo->codigo = codigo;
    nuevo->valor = valor;
    nuevo->estado = estado;
    return nuevo;
}

/*
-libera los textos dinamicos y la estructura del pase
-no hace nada si recibe NULL
*/
void liberarPase(struct Pase *pase) {
    if (pase == NULL) {
        return;
    }

    /* Libera los textos antes del struct que contiene sus punteros. */
    free(pase->fecha);
    free(pase->hora);
    free(pase);
}

/*
-crea un nodo que guarda un pase
-retorna NULL si el pase es NULL o falla la reserva de memoria
*/
struct NodoPase *crearNodoPase(struct Pase *pase) {
    struct NodoPase *nuevo;

    if (pase == NULL) {
        return NULL;
    }

    /* El nodo solo conserva una referencia al pase, no copia el objeto. */
    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL;
    }

    nuevo->paseVigente = pase;
    nuevo->sig = NULL;
    return nuevo;
}

/*
-crea una lista vacia para guardar los pases del dia
-retorna NULL si falla la reserva de memoria
*/
struct pasesDelDia *crearPasesDelDia(void) {
    struct pasesDelDia *nueva = malloc(sizeof(*nueva));

    if (nueva != NULL) {
        /* Una lista nueva comienza sin pases registrados. */
        nueva->head = NULL;
    }
    return nueva;
}

/*
-busca un pase por codigo
-retorna su nodo o NULL si la lista o el pase no existen
*/
struct NodoPase *buscarPase(struct pasesDelDia *lista, int codigo) {
    struct NodoPase *rec;

    if (lista == NULL) {
        return NULL;
    }

    /* Recorre la lista enlazada y compara el codigo unico de cada pase. */
    rec = lista->head;
    while (rec != NULL) {
        if (rec->paseVigente->codigo == codigo) {
            return rec;
        }
        rec = rec->sig;
    }
    return NULL;
}

/*
-agrega un pase al final si sus datos son validos y no repite codigo
-retorna 1 si se agrego o 0 si hay datos invalidos o falla memoria
-si falla, el pase sigue perteneciendo a quien lo envio
*/
int agregarPase(struct pasesDelDia *lista, struct Pase *pase) {
    struct NodoPase *rec;
    struct NodoPase *nuevo;

    if (lista == NULL || pase == NULL || pase->tipoPase < 1 ||
        pase->tipoPase > 4 || pase->estado < 1 || pase->estado > 4 ||
        pase->valor < 0 || buscarPase(lista, pase->codigo) != NULL) {
        return 0;
    }

    nuevo = crearNodoPase(pase);
    if (nuevo == NULL) {
        return 0;
    }

    /* Enlaza el nodo al final para conservar el orden de registro. */
    if (lista->head == NULL) {
        lista->head = nuevo;
    } else {
        rec = lista->head;
        while (rec->sig != NULL) {
            rec = rec->sig;
        }
        rec->sig = nuevo;
    }
    return 1;
}

/*
-quita un pase de la lista y lo retorna sin liberar el pase
-libera solamente el nodo para permitir que el pase siga en uso por clientes
-retorna NULL si la lista o el codigo no existen
*/
struct Pase *quitarPase(struct pasesDelDia *lista, int codigo) {
    struct NodoPase *actual;
    struct NodoPase *anterior = NULL;
    struct Pase *pase;

    if (lista == NULL) {
        return NULL;
    }

    /* Busca el nodo y conserva su anterior para poder desenlazarlo. */
    actual = lista->head;
    while (actual != NULL && actual->paseVigente->codigo != codigo) {
        anterior = actual;
        actual = actual->sig;
    }

    if (actual == NULL) {
        return NULL;
    }

    /* Actualiza el inicio o el enlace previo, segun donde estaba el pase. */
    if (anterior == NULL) {
        lista->head = actual->sig;
    } else {
        anterior->sig = actual->sig;
    }

    pase = actual->paseVigente;
    free(actual);
    return pase;
}

/*
-elimina un pase de la lista y libera su nodo, pero no el pase
-retorna 1 si se elimino, 0 si el codigo no existe, -1 si la lista esta vacia
*/
int eliminarPase(struct pasesDelDia *lista, int codigo) {
    if (lista == NULL || lista->head == NULL) {
        return -1;
    }
    /* quitarPase libera el nodo y devuelve el pase sin liberarlo. */
    return quitarPase(lista, codigo) != NULL;
}

/*
-modifica el tipo y el valor del pase, sin cambiar codigo ni estado
-retorna 1 si se modifico, 0 si no existe y -1 si los datos no son validos
*/
int modificarPase(struct pasesDelDia *lista, int codigo, int nuevoTipo,
                  int nuevoValor) {
    struct NodoPase *nodo = buscarPase(lista, codigo);

    if (nodo == NULL) {
        return 0;
    }
    if (nuevoTipo < 1 || nuevoTipo > 4 || nuevoValor < 0) {
        return -1;
    }

    /* Solo actualiza los campos permitidos; codigo y estado se conservan. */
    nodo->paseVigente->tipoPase = nuevoTipo;
    nodo->paseVigente->valor = nuevoValor;
    return 1;
}

/*
-muestra los pases en el orden en que estan en la lista
-indica codigo, tipo, valor, estado, fecha y hora
*/
void listarPases(struct pasesDelDia *lista) {
    struct NodoPase *rec;

    if (lista == NULL || lista->head == NULL) {
        printf("No hay pases registrados hoy.\n");
        return;
    }

    /* Muestra cada pase sin alterar los enlaces de la lista. */
    rec = lista->head;
    while (rec != NULL) {
        printf("Codigo: %d, Tipo: %d, Valor: %d, Estado: %d, Fecha: %s, Hora: %s\n",
               rec->paseVigente->codigo, rec->paseVigente->tipoPase,
               rec->paseVigente->valor, rec->paseVigente->estado,
               rec->paseVigente->fecha, rec->paseVigente->hora);
        rec = rec->sig;
    }
}

/*
-cuenta los pases que tienen el estado indicado
-retorna cero si la lista es NULL o no hay pases con ese estado
*/
int contarPasesPorEstado(struct pasesDelDia *lista, int estado) {
    struct NodoPase *rec;
    int contador = 0;

    if (lista == NULL) {
        return 0;
    }

    rec = lista->head;
    /* Suma una unidad por cada pase cuyo estado coincide con el solicitado. */
    while (rec != NULL) {
        if (rec->paseVigente->estado == estado) {
            contador++;
        }
        rec = rec->sig;
    }
    return contador;
}

/*
-comprueba que el pase tenga la misma fecha que la fecha recibida
-retorna 1 si coincide y 0 si falta algun dato o las fechas son distintas
*/
int paseValidoHoy(struct Pase *pase, char *fecha) {
    /* Comprueba la vigencia comparando las fechas almacenadas como texto. */
    return pase != NULL && pase->fecha != NULL && fecha != NULL &&
           strcmp(pase->fecha, fecha) == 0;
}
