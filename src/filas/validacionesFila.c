#include "../../include/filaInterno.h"

#include <ctype.h>
#include <string.h>

/*
-revisa que todos los caracteres de un texto, entre dos posiciones, sean digitos (0 a 9)
-recibe el texto, la posicion donde empieza a revisar (inicio) y donde termina (fin)
-las dos posiciones estan incluidas en la revision
-retorna 1 si todos son digitos, 0 si alguno no lo es
*/
int soloDigitos(char *texto, int inicio, int fin) {
    int i;

    if (texto == NULL || inicio < 0 || fin < inicio ||
        (size_t)fin >= strlen(texto)) {
        return 0;
    }

    /* Se recorren todas las posiciones indicadas, incluyendo inicio y fin. */
    for (i = inicio; i <= fin; i++) {
        /* El cast evita comportamiento indefinido con caracteres fuera de ASCII. */
        if (!isdigit((unsigned char)texto[i])) {
            return 0;
        }
    }

    return 1;
}

/*
-convierte a un numero entero los digitos de un texto entre dos posiciones incluidas
-asume que el rango existe y contiene solo digitos, comprobado previamente
*/
int numeroDe(char *texto, int inicio, int fin) {
    int i;
    int numero;

    numero = 0;
    for (i = inicio; i <= fin; i++) {
        /* Cada cifra desplaza el valor acumulado y agrega el digito actual. */
        numero = numero * 10 + (texto[i] - '0');
    }

    return numero;
}

/*
-indica si un anio es bisiesto
-retorna 1 si es bisiesto y 0 si no lo es
*/
int esBisiesto(int anio) {
    return anio % 400 == 0 || (anio % 4 == 0 && anio % 100 != 0);
}

/*
-valida una hora HH:MM y la convierte a minutos desde medianoche
-el formato debe ser exacto: 5 caracteres, dos digitos, dos puntos, dos digitos
-retorna 1 si el formato y los valores son correctos, o 0 si no lo son
*/
int validarHoraFila(char *hora, int *minutos) {
    int horaNumero;
    int minutoNumero;

    /* Primero se comprueban punteros, longitud, separador y cifras. */
    if (hora == NULL || minutos == NULL || strlen(hora) != 5 ||
        hora[2] != ':' || !soloDigitos(hora, 0, 1) ||
        !soloDigitos(hora, 3, 4)) {
        return 0;
    }

    /* Solo despues de validar el formato se convierten las partes numericas. */
    horaNumero = numeroDe(hora, 0, 1);
    minutoNumero = numeroDe(hora, 3, 4);
    if (horaNumero > 23 || minutoNumero > 59) {
        return 0;
    }

    /* La conversion permite comparar horas sin depender del texto. */
    *minutos = horaNumero * 60 + minutoNumero;
    return 1;
}

/*
-valida una fecha DD/MM/AAAA, incluyendo dias por mes y anios bisiestos
-el formato debe ser exacto: 10 caracteres, con barras en las posiciones 2 y 5
-retorna 1 si la fecha existe en el calendario y 0 en caso contrario
*/
int validarFechaFila(char *fecha) {
    int dia;
    int mes;
    int anio;
    int diasMes;

    /* Se valida el formato completo antes de convertir dia, mes y anio. */
    if (fecha == NULL || strlen(fecha) != 10 ||
        fecha[2] != '/' || fecha[5] != '/' ||
        !soloDigitos(fecha, 0, 1) || !soloDigitos(fecha, 3, 4) ||
        !soloDigitos(fecha, 6, 9)) {
        return 0;
    }

    /* El formato ya esta validado, asi que se convierten las tres partes. */
    dia = numeroDe(fecha, 0, 1);
    mes = numeroDe(fecha, 3, 4);
    anio = numeroDe(fecha, 6, 9);
    if (anio == 0 || mes < 1 || mes > 12) {
        return 0;
    }

    /* Se determina el ultimo dia valido del mes. */
    diasMes = 31;
    if (mes == 4 || mes == 6 || mes == 9 || mes == 11) {
        diasMes = 30;
    } else if (mes == 2) {
        if (esBisiesto(anio)) {
            diasMes = 29;
        } else {
            diasMes = 28;
        }
    }

    return dia >= 1 && dia <= diasMes;
}
