#include "../../include/pases.h"
#include "../../include/clientes.h"

#include <limits.h>
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
    nuevo->tipoPase = tipoPase;
    nuevo->codigo = codigo;
    nuevo->valor = valor;
    nuevo->estado = estado;
    return nuevo;
}

void liberarPase(struct Pase *pase) {
    if (pase == NULL) {
        return;
    }

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
        nueva->head = NULL;
        nueva->ultimoCodigoEmitido = 0;
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
        pase->valor < 0 || buscarPase(lista, pase->codigo) != NULL ||
        (lista->ultimoCodigoEmitido != 0 &&
         pase->codigo <= lista->ultimoCodigoEmitido)) {
        return 0;
    }

    nuevo = crearNodoPase(pase);
    if (nuevo == NULL) {
        return 0;
    }

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

    actual = lista->head;
    while (actual != NULL && actual->paseVigente->codigo != codigo) {
        anterior = actual;
        actual = actual->sig;
    }

    if (actual == NULL) {
        return NULL;
    }

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
    while (rec != NULL) {
        if (rec->paseVigente->estado == estado) {
            contador++;
        }
        rec = rec->sig;
    }
    return contador;
}

int paseValidoHoy(struct Pase *pase, char *fecha) {
    return pase != NULL && pase->fecha != NULL && fecha != NULL &&
           strcmp(pase->fecha, fecha) == 0;
}

int limpiarPasesVencidos(struct Cliente *cliente, char *fecha) {
    int i;
    int cantidad = 0;

    if (cliente == NULL || cliente->pases == NULL || fecha == NULL) {
        return -1;
    }

    for (i = 0; i < MAXPASES; i++) {
        struct Pase *pase = cliente->pases[i];

        if (pase != NULL && paseValidoHoy(pase, fecha)) {
            cliente->pases[cantidad] = pase;
            cantidad++;
        } else {
            liberarPase(pase);
        }
    }

    for (i = cantidad; i < MAXPASES; i++) {
        cliente->pases[i] = NULL;
    }

    cliente->plibrepases = cantidad;
    return cantidad;
}

int mayorCodigoEmitido(struct Tienda *tienda) {
    struct NodoPase *nodoPase;
    struct NodoCliente *nodoCliente;
    int mayor;
    int i;

    mayor = tienda->filaPasesDeldia->ultimoCodigoEmitido;
    for (nodoPase = tienda->filaPasesDeldia->head;
         nodoPase != NULL; nodoPase = nodoPase->sig) {
        if (nodoPase->paseVigente->codigo > mayor) {
            mayor = nodoPase->paseVigente->codigo;
        }
    }

    nodoCliente = tienda->headClientes;
    if (nodoCliente != NULL) {
        do {
            if (nodoCliente->dato != NULL &&
                nodoCliente->dato->pases != NULL) {
                for (i = 0; i < MAXPASES; i++) {
                    struct Pase *pase = nodoCliente->dato->pases[i];

                    if (pase != NULL && pase->codigo > mayor) {
                        mayor = pase->codigo;
                    }
                }
            }
            nodoCliente = nodoCliente->sig;
        } while (nodoCliente != tienda->headClientes);
    }

    return mayor;
}

/*
-prepara una venta de pases buscando al cliente y el pase que se usara como modelo
-valida los datos, elimina pases vencidos y determina cuantos pases se pueden vender
-entrega los datos calculados mediante sus parametros de salida
-retorna 1 si la venta puede continuar o 0 si no cumple las condiciones necesarias
*/
int prepararVentaPases(struct Tienda *tienda, int rut, int codigoTipo,
                       int cantidad, struct Cliente **cliente,
                       struct Pase **modelo, int *cantidadActual,
                       int *cantidadVenta, int *mayorCodigo) {
    struct NodoCliente *nodoCliente;
    struct NodoPase *nodoModelo;

    /* Comprueba que la tienda y la cantidad solicitada permitan iniciar la venta. */
    if (tienda == NULL || tienda->filaPasesDeldia == NULL ||
        tienda->headClientes == NULL || tienda->fecha == NULL ||
        cantidad <= 0 || tienda->recaudacionPasesDia < 0) {
        return 0;
    }

    /* Busca al cliente comprador y el pase que servira de modelo. */
    nodoCliente = buscarCliente(tienda->headClientes, rut);
    nodoModelo = buscarPase(tienda->filaPasesDeldia, codigoTipo);
    if (nodoCliente == NULL || nodoModelo == NULL) {
        return 0;
    }

    /* Valida que el cliente pueda guardar pases y que el modelo este disponible. */
    *cliente = nodoCliente->dato;
    *modelo = nodoModelo->paseVigente;
    if ((*cliente)->pases == NULL || (*modelo)->estado != 1 ||
        (*modelo)->valor < 0 || (*modelo)->hora == NULL) {
        return 0;
    }

    /* Elimina los pases vencidos y obtiene el espacio actualmente ocupado. */
    *cantidadActual = limpiarPasesVencidos(*cliente, tienda->fecha);
    if (*cantidadActual < 0) {
        return 0;
    }

    /* Limita la compra al espacio libre del cliente. */
    *cantidadVenta = cantidad;
    if (*cantidadVenta > MAXPASES - *cantidadActual) {
        *cantidadVenta = MAXPASES - *cantidadActual;
    }
    /* Evita que la recaudacion supere el maximo representable por un int. */
    if (*cantidadVenta == 0 ||
        ((*modelo)->valor > 0 &&
         *cantidadVenta >
             (INT_MAX - tienda->recaudacionPasesDia) / (*modelo)->valor)) {
        return 0;
    }

    /* Determina el ultimo codigo usado y comprueba que haya codigos disponibles. */
    *mayorCodigo = mayorCodigoEmitido(tienda);
    if (*mayorCodigo > INT_MAX - *cantidadVenta) {
        return 0;
    }

    return 1;
}

/*
-completa la venta creando los nuevos pases y guardandolos en el cliente
-actualiza el ultimo codigo emitido y la recaudacion de la tienda
-recibe los datos de venta validados previamente por prepararVentaPases
-retorna la cantidad de pases registrados o 0 si falla la creacion de alguno
*/
int registrarVentaPases(
    struct Tienda *tienda, struct Cliente *cliente, struct Pase *modelo,
    int cantidadActual, int cantidadVenta, int mayorCodigo) {
    struct Pase *nuevos[MAXPASES];
    int ingreso;
    int i;

    /* Crea todos los pases antes de registrar la venta o enlazarlos al cliente. */
    for (i = 0; i < cantidadVenta; i++) {
        nuevos[i] = crearPase(modelo->tipoPase, mayorCodigo + i + 1,
                              modelo->valor, 1, tienda->fecha, modelo->hora);
        if (nuevos[i] == NULL) {
            /* Si falla una creacion, libera los pases creados y cancela la venta. */
            while (i > 0) {
                i--;
                liberarPase(nuevos[i]);
            }
            return 0;
        }
    }

    /* Registra la venta actualizando codigos emitidos y recaudacion. */
    ingreso = cantidadVenta * modelo->valor;
    tienda->filaPasesDeldia->ultimoCodigoEmitido =
        mayorCodigo + cantidadVenta;
    tienda->recaudacionPasesDia += ingreso;

    /* Enlaza los pases vendidos al cliente y actualiza la cantidad que posee. */
    for (i = 0; i < cantidadVenta; i++) {
        cliente->pases[cantidadActual + i] = nuevos[i];
    }
    cliente->plibrepases = cantidadActual + cantidadVenta;

    return cantidadVenta;
}

/*
-realiza el proceso completo de venta de pases del dia para un cliente y tipo indicados
-prepara la venta, limita la cantidad al espacio disponible y registra los pases
-actualiza los codigos emitidos y la recaudacion al completar la venta
-retorna la cantidad vendida o 0 si la venta no se puede completar
*/
int venderPasesDelDia(struct Tienda *tienda, int rut, int codigoTipo,
                      int cantidad) {
    struct Cliente *cliente;
    struct Pase *modelo;
    int cantidadActual;
    int cantidadVenta;
    int mayorCodigo;

    /* Valida y prepara la venta, calculando cuantos pases se pueden entregar. */
    if (!prepararVentaPases(tienda, rut, codigoTipo, cantidad, &cliente,
                            &modelo, &cantidadActual, &cantidadVenta,
                            &mayorCodigo)) {
        return 0;
    }

    /* Crea los pases, registra la venta y luego los enlaza al cliente. */
    return registrarVentaPases(tienda, cliente, modelo, cantidadActual,
                               cantidadVenta, mayorCodigo);
}
