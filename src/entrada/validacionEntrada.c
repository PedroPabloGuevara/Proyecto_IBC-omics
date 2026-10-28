#include "../../include/entradaInterna.h"
#include "../../include/clientes.h"
#include "../../include/pases.h"

#include <ctype.h>
#include <string.h>

/*
-comprueba que los caracteres del rango indicado sean digitos
-los indices inicio y fin se incluyen en la revision
-retorna 1 si todos son digitos y 0 si el texto o el rango son invalidos
*/
int soloDigitos(char *texto, int inicio, int fin) {
    int i;

    if (texto == NULL || inicio < 0 || fin < inicio ||
        (size_t)fin >= strlen(texto)) {
        return 0;
    }

    /* Revisa cada posicion del rango, incluidos sus extremos. */
    for (i = inicio; i <= fin; i++) {
        if (!isdigit((unsigned char)texto[i])) {
            return 0;
        }
    }

    return 1;
}

/*
-convierte a entero un rango de caracteres numericos
-asume que el rango existe y contiene solo digitos
-retorna el numero formado por los caracteres
*/
int numeroDe(char *texto, int inicio, int fin) {
    int i;
    int numero = 0;

    /* Acumula las cifras de izquierda a derecha. */
    for (i = inicio; i <= fin; i++) {
        numero = numero * 10 + (texto[i] - '0');
    }

    return numero;
}

/*
-determina si el anio es bisiesto con las reglas del calendario gregoriano
-retorna 1 si es bisiesto y 0 en caso contrario
*/
int esBisiesto(int anio) {
    return anio % 400 == 0 || (anio % 4 == 0 && anio % 100 != 0);
}

/*
-valida una hora con formato HH:MM y la convierte a minutos desde medianoche
-retorna 1 si el formato y los valores son correctos, o 0 si no lo son
*/
int validarHoraFila(char *hora, int *minutos) {
    int horaNumero;
    int minutoNumero;

    /* Verifica longitud, separador, puntero de salida y digitos. */
    if (hora == NULL || minutos == NULL || strlen(hora) != 5 ||
        hora[2] != ':' || !soloDigitos(hora, 0, 1) ||
        !soloDigitos(hora, 3, 4)) {
        return 0;
    }

    horaNumero = numeroDe(hora, 0, 1);
    minutoNumero = numeroDe(hora, 3, 4);
    if (horaNumero > 23 || minutoNumero > 59) {
        return 0;
    }

    /* La conversion permite comparar horas como valores numericos. */
    *minutos = horaNumero * 60 + minutoNumero;
    return 1;
}

/*
-valida una fecha DD/MM/AAAA, incluidos los dias existentes en cada mes
-retorna 1 si la fecha existe en el calendario y 0 si no es valida
*/
int validarFechaFila(char *fecha) {
    int dia;
    int mes;
    int anio;
    int diasMes = 31;

    /* Verifica formato exacto antes de convertir sus componentes. */
    if (fecha == NULL || strlen(fecha) != 10 ||
        fecha[2] != '/' || fecha[5] != '/' ||
        !soloDigitos(fecha, 0, 1) || !soloDigitos(fecha, 3, 4) ||
        !soloDigitos(fecha, 6, 9)) {
        return 0;
    }

    dia = numeroDe(fecha, 0, 1);
    mes = numeroDe(fecha, 3, 4);
    anio = numeroDe(fecha, 6, 9);
    if (anio == 0 || mes < 1 || mes > 12) {
        return 0;
    }

    /* Ajusta la cantidad de dias para meses cortos y febrero. */
    if (mes == 4 || mes == 6 || mes == 9 || mes == 11) {
        diasMes = 30;
    } else if (mes == 2) {
        diasMes = esBisiesto(anio) ? 29 : 28;
    }

    return dia >= 1 && dia <= diasMes;
}

/*
-comprueba que la hora actual este dentro del horario de atencion de la tienda
-considera valida la apertura y excluye la hora de cierre
-retorna 1 si esta dentro del horario y 0 si las horas son invalidas
*/
int horaDentroHorarioTienda(struct Tienda *tienda, char *horaActual) {
    int actual;
    int apertura;
    int cierre;

    if (tienda == NULL ||
        !validarHoraFila(horaActual, &actual) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura) {
        return 0;
    }

    /* Los ingresos se permiten desde apertura hasta antes del cierre. */
    return actual >= apertura && actual < cierre;
}

/*
-busca al titular de un pase recorriendo la lista circular de clientes
-compara el pase con las referencias del arreglo Cliente.pases
-retorna el cliente titular o NULL si el pase no pertenece a un cliente
*/
struct Cliente *buscarClientePorPaseFila(struct Tienda *tienda,
                                         struct Pase *pase) {
    struct NodoCliente *nodoCliente;
    int i;

    if (tienda == NULL || pase == NULL || tienda->headClientes == NULL) {
        return NULL;
    }

    /* Recorre la lista hasta volver al primer cliente. */
    nodoCliente = tienda->headClientes;
    do {
        if (nodoCliente->dato != NULL && nodoCliente->dato->pases != NULL) {
            for (i = 0; i < MAXPASES; i++) {
                if (nodoCliente->dato->pases[i] == pase) {
                    return nodoCliente->dato;
                }
            }
        }
        nodoCliente = nodoCliente->sig;
    } while (nodoCliente != NULL && nodoCliente != tienda->headClientes);

    return NULL;
}

/*
-valida que el pase pertenezca al cliente y coincida con fecha y horario
-permite el primer ingreso activo o el reingreso de uno ya utilizado
-rechaza pases anulados, vencidos o fuera de su intervalo horario
-retorna 1 si la solicitud es valida y 0 si falla alguna comprobacion
*/
int validarSolicitudFila(struct Tienda *tienda, struct Cliente *cliente,
                         struct Pase *pase, char *horaActual) {
    struct NodoPase *registroIngreso;
    int actual;
    int apertura;
    int cierre;

    /* Valida fecha, titularidad y horario de la tienda; no usa la hora de compra. */
    if (tienda == NULL || cliente == NULL || pase == NULL ||
        pase->fecha == NULL ||
        pase->tipoPase < 1 || pase->tipoPase > 4 ||
        !validarFechaFila(tienda->fecha) ||
        !validarHoraFila(horaActual, &actual) ||
        !validarHoraFila(tienda->hora_inicio, &apertura) ||
        !validarHoraFila(tienda->hora_termino, &cierre) ||
        cierre <= apertura || actual >= cierre ||
        strcmp(pase->fecha, tienda->fecha) != 0 ||
        buscarClientePorPaseFila(tienda, pase) != cliente) {
        return 0;
    }

    registroIngreso = buscarPase(tienda->filaPasesDeldia, pase->codigo);

    /* Un pase activo se admite por primera vez; uno usado debe estar registrado. */
    if (pase->estado == 1) {
        return registroIngreso == NULL;
    }
    if (pase->estado == 3) {
        return registroIngreso != NULL;
    }

    /* Los pases anulados, vencidos o con estado desconocido no permiten entrada. */
    return 0;
}
