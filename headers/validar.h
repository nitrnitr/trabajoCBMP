/***********************************************************************
 *
 *  Módulo: Header del validar.c con los encabezados de las funciones
 * Autor:   Martín Aguilar
 *
 **********************************************************************/


#ifndef _val_h
#define _val_h
#include <stdbool.h>
#include <stdint.h>

#include "bmp.h"

//Estructura donde voy a guardar los valores que lea en los parametros, para luego enviarlos al procesar
typedef struct
{
    uint32_t lineas_hor_ancho;
    uint32_t lineas_hor_espacio;
    bmpcolor_t lineas_hor_color;
    uint32_t lineas_ver_ancho;
    uint32_t lineas_ver_espacio;
    bmpcolor_t lineas_ver_color;
    uint32_t blur_rate;
    char *entrada;
    char *salida;
    bool ayuda;
    bool no_parametros;
} datix;


/* Imprime la ayuda con la lista completa de funciones que realiza
 * el programa
 */
void ayuda();

/*
 * Procesa los parametros que recibe el programa, si hay alguno mal,
 * devuelve false. Recibe el arreglo de parametros, la cantidad
 *   y una variable tipo datix donde se guardan los valores de las
 * opciones de cada parámetro para aplicarlas luego al procesar.
 */
bool parametros_correctos( char *argv[], int argc, datix *datos );


/* Recibe los valores de los parámetros recolectados en
 * parametros_correctos y los emplea para llamar a la función
 * correspondiente en cada caso.
 * Recibe también los parámetros al programa, y la cantidad que son
 */
bool procesar( char *argv[], int argc, datix *datos );

/*
 * Transforma un long en un color, y retorna un tipo color.
 */
bmpcolor_t colordesdeint(const long color_int);

bool leer_pixels_1bpp(   FILE *fbmp,
                         bmp_t *imagen,
                         uint32_t fila_alineada );

#endif
