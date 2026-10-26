#include "../../include/fila.h"

#include <stdlib.h>

/*
-crea una fila vacia en memoria dinamica
-inicializa head y tail en NULL porque todavia no hay personas esperando
-retorna un puntero a la fila creada, o NULL si falla la reserva de memoria
*/
struct Fila *crearFila(void) {
    struct Fila *fila;

    fila = malloc(sizeof(*fila));
    if (fila == NULL) {
        return NULL; /* No se pudo reservar memoria para la fila. */
    }

    /* La fila comienza vacia, sin primer ni ultimo nodo. */
    fila->head = NULL;
    fila->tail = NULL;
    return fila;
}

/*
-crea un nodo de fila con la persona que solicita entrar y el pase que presenta
-recibe punteros a un cliente y a un pase; no copia ni pasa a ser propietario de ellos
-inicializa sig en NULL para que el nodo pueda agregarse a una fila
-retorna el nodo creado, o NULL si algun dato es NULL o falla la reserva de memoria
*/
struct NodoFila *crearNodoFila(struct Cliente *cliente, struct Pase *pase) {
    struct NodoFila *nuevo;

    if (cliente == NULL || pase == NULL) {
        return NULL; /* Un nodo de espera necesita persona y pase. */
    }

    nuevo = malloc(sizeof(*nuevo));
    if (nuevo == NULL) {
        return NULL; /* No se pudo reservar memoria para el nodo. */
    }

    /* Se guardan referencias: este nodo no copia ni libera cliente o pase. */
    nuevo->cliente = cliente;
    nuevo->pase = pase;
    nuevo->codigoSalaPreferida = FILA_SALA_PRINCIPAL;
    nuevo->sig = NULL;
    return nuevo;
}

/*
-agrega un nodo a la fila y mantiene primero a quienes tienen pase prioritario
-recibe una fila y un nodo separado que no debe estar enlazado
-rechaza datos incompletos, nodos repetidos, clientes que ya esperan y pases ya usados en la fila
-los pases de tipo 4 se insertan despues de los prioritarios existentes y antes de los normales
-los demas pases se agregan al final, conservando el orden de llegada de cada grupo
-tambien comprueba que head y tail describan correctamente los extremos de la fila
-retorna 1 si agrega el nodo y 0 si no puede agregarlo
-si tiene exito, la fila administra el nodo; el cliente y el pase siguen siendo externos
*/
int encolarFila(struct Fila *fila, struct NodoFila *nuevo) {
    struct NodoFila *rec;
    struct NodoFila *anterior;

    /* Rechaza nodos incompletos o que ya pertenecen a una lista. */
    if (fila == NULL || nuevo == NULL || nuevo->cliente == NULL ||
        nuevo->pase == NULL || nuevo->sig != NULL) {
        return 0;
    }

    /* Verifica que ni la persona ni el mismo pase esten ya esperando. */
    anterior = NULL;
    rec = fila->head;
    while (rec != NULL) {
        if (rec == nuevo || rec->cliente == NULL || rec->pase == NULL ||
            rec->cliente->rut == nuevo->cliente->rut ||
            rec->pase == nuevo->pase) {
            return 0;
        }
        anterior = rec;
        rec = rec->sig;
    }

    /* No inserta si los extremos guardados contradicen los enlaces recorridos. */
    if ((fila->head == NULL && fila->tail != NULL) ||
        (fila->head != NULL && anterior != fila->tail)) {
        return 0;
    }

    /* En una fila vacia, el nuevo nodo es simultaneamente head y tail. */
    if (fila->head == NULL) {
        fila->head = nuevo;
        fila->tail = nuevo;
    } else if (nuevo->pase->tipoPase != 4) {
        /* Los pases normales se agregan detras de todos los que ya esperan. */
        fila->tail->sig = nuevo;
        fila->tail = nuevo;
    } else {
        /* Se busca el final del grupo prioritario para conservar su orden de llegada. */
        rec = fila->head;
        anterior = NULL;
        while (rec != NULL && rec->pase->tipoPase == 4) {
            anterior = rec;
            rec = rec->sig;
        }

        /* Si no habia prioritarios, se inserta al principio. */
        if (anterior == NULL) {
            nuevo->sig = fila->head;
            fila->head = nuevo;
        } else {
            /* Si los habia, se inserta despues del ultimo prioritario. */
            nuevo->sig = rec;
            anterior->sig = nuevo;
        }

        /* Si queda al final, tambien pasa a ser el ultimo nodo. */
        if (nuevo->sig == NULL) {
            fila->tail = nuevo;
        }
    }

    return 1;
}

/*
-retira de la fila a la primera persona que debe ser atendida
-recibe la fila; si esta vacia o es NULL, no modifica nada
-actualiza head y, si la fila queda vacia, tambien actualiza tail
-desenlaza el nodo antes de retornarlo; no libera el nodo, el cliente ni el pase
-retorna el nodo retirado o NULL si no hay ninguno
*/
struct NodoFila *desencolarFila(struct Fila *fila) {
    struct NodoFila *primero;

    if (fila == NULL || fila->head == NULL) {
        return NULL;
    }

    /* Guarda el primer nodo y adelanta head al siguiente en la fila. */
    primero = fila->head;
    fila->head = primero->sig;
    if (fila->head == NULL) {
        /* Al retirar el unico nodo, la fila queda sin extremos. */
        fila->tail = NULL;
    }
    /* Separa el nodo para que quien lo recibe pueda administrarlo. */
    primero->sig = NULL;
    return primero;
}

/*
-busca a una persona dentro de la fila usando su RUT
-recibe la fila y el RUT que se desea encontrar
-recorre los nodos desde head hasta encontrar coincidencia o llegar al final
-retorna el nodo encontrado sin modificar la fila, o NULL si no existe
*/
struct NodoFila *buscarEnFila(struct Fila *fila, int rut) {
    struct NodoFila *rec;

    if (fila == NULL) {
        return NULL;
    }

    /* Recorre la fila y compara el RUT almacenado en cada cliente. */
    for (rec = fila->head; rec != NULL; rec = rec->sig) {
        if (rec->cliente != NULL && rec->cliente->rut == rut) {
            return rec;
        }
    }

    return NULL;
}

/*
-busca y elimina de la fila a la primera persona cuyo RUT coincide
-recibe la fila y el RUT que se desea quitar
-enlaza los nodos vecinos y actualiza head o tail cuando elimina un extremo
-libera solamente el nodo de fila; no libera al cliente ni al pase
-retorna 1 si elimina a la persona y 0 si la fila es NULL o no la encuentra
*/
int quitarDeFila(struct Fila *fila, int rut) {
    struct NodoFila *rec;
    struct NodoFila *anterior;

    if (fila == NULL) {
        return 0;
    }

    /* Avanza hasta el RUT buscado, conservando el nodo anterior para enlazar. */
    anterior = NULL;
    rec = fila->head;
    while (rec != NULL &&
           (rec->cliente == NULL || rec->cliente->rut != rut)) {
        anterior = rec;
        rec = rec->sig;
    }

    if (rec == NULL) {
        return 0;
    }

    /* Conecta el anterior con el siguiente; si era head, mueve el inicio. */
    if (anterior == NULL) {
        fila->head = rec->sig;
    } else {
        anterior->sig = rec->sig;
    }

    /* Si se quito el ultimo nodo, tail retrocede al anterior. */
    if (fila->tail == rec) {
        fila->tail = anterior;
    }

    /* Solo se libera el nodo: cliente y pase pertenecen a otras estructuras. */
    free(rec);
    return 1;
}

/*
-elimina todos los nodos de una fila sin liberar los clientes ni los pases
-recibe una fila; si es NULL, no realiza ninguna accion
-recorre y libera cada nodo, y deja head y tail en NULL
*/
void vaciarFila(struct Fila *fila) {
    struct NodoFila *rec;
    struct NodoFila *siguiente;

    if (fila == NULL) {
        return;
    }

    /* Guarda el siguiente enlace antes de liberar el nodo actual. */
    rec = fila->head;
    while (rec != NULL) {
        siguiente = rec->sig;
        free(rec);
        rec = siguiente;
    }

    /* Reinicia los extremos para dejar la fila reutilizable y vacia. */
    fila->head = NULL;
    fila->tail = NULL;
}

/*
-libera una fila completa
-primero libera todos sus nodos mediante vaciarFila y luego libera la estructura
-los clientes y pases referenciados por los nodos no se liberan
-si recibe NULL, no realiza ninguna accion
*/
void liberarFila(struct Fila *fila) {
    if (fila == NULL) {
        return;
    }

    /* Primero elimina los nodos y despues libera la estructura de la fila. */
    vaciarFila(fila);
    free(fila);
}
