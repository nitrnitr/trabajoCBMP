/***********************************************************************
 *
 * Módulo: Implementación de las funciones que verifican y procesan
 *         los parámetros recibidos al programa.
 * Autor:  Martín Aguilar
 *
 **********************************************************************/


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "../headers/validar.h"
#include "../headers/bmp.h"

void ayuda()
{
    printf( "Ayuda:\n"
            "• -? o -h: muestra un texto explicativo de cómo invocar a la aplicación\n"
            "• -s: muestra información sobre el header del archivo BMP. Si no especifica\n"
            "ninguna otra opción (salvo -i) entonces no guarda un archivo de salida.\n"
            "• -p: flip vertical\n"
            "• -r: rota la imagen 90º\n"
            "• -n: genera el negativo de la imagen\n"
            "• -d: duplica el tamaño de la imagen\n"
            "• -f: reduce a la mitad el tamaño de la imagen\n"
            "• -b RATIO: produce el efecto “blur” (enfocar/desenfocar) con RATIO pixels\n"
            "• -lh WIDTH SPACE COLOR: dibuja líneas de COLOR horizontales de WIDTH\n"
            "pixels, separadas por SPACE pixels. Color debe ir en hexadecimal: "
            "FF0000 RED, 00FF00 GREEN, 0000FF BLUE. Cada color varía desde 00 hasta FF.\n"
            "• -lv WIDTH SPACE COLOR: dibuja líneas de COLOR verticales de WIDTH\n"
            "pixels, separadas por SPACE pixels. Color debe ir en hexadecimal: "
            "FF0000 RED, 00FF00 GREEN, 0000FF BLUE. Cada color varía desde 00 hasta FF.\n"
            "• -o OUTPUT: especifica el nombre de archivo en el cual se almacenará la\n"
            "imagen resultante. En caso de no ser ingresado, se utilizará out.bmp .\n"
            "• -i INTPUT: el nombre del archivo con la imagen a procesar.\n");
}

// Funcion para transformar string a long, ya que atoi no sirve.
bool string_a_long(const char *str, long *convertir) {
    char *ptr;
    *convertir = strtol( str, &ptr, 16);
    return ( errno != ERANGE && *ptr == '\0' );
}

/*
 * Transforma un LONG en un COLOR, para luego usarlo en el
 * "AgregarLineas".
 */
bmpcolor_t colordesdeint(const long color_int)
{
    bmpcolor_t color;
    color.red = (color_int & 0xFF0000) >> 16;
    color.green = (color_int &0x00FF00) >> 8;
    color.blue = (color_int & 0x0000FF);
    color.alpha = 0;
    return color;
}

/* Procesa los parametros que recibe el programa, si hay alguno mal, devuelve false. Recibe el arreglo de parametros, la cantidad
y una variable con datix para guardar los datos que se lean en los parametros */

bool parametros_correctos( char *argv[], int argc, datix *datos )
{
    int i = 1;
    bool error = false;
    if(argc<2) {
        printf("Se debe enviar al menos un parámetro\n");
        datos->ayuda = true;
        return true;
    }
    while ( i < argc && !error )
    {
        if ( argv[i][0] == '-'  ) //control empiece con guión
        {
            switch ( argv[i][1] )
            {

            case '?': {
                datos->ayuda = true;    // Si se manda ayuda, los demas parametros no importan
                if( (argv[i][2]) != '\0')return false;
                return true;
            }
            case 'h': {
                datos->ayuda = true;    //Control de parámetros de una sola letra
                if( (argv[i][2]) != '\0')return false;
                return true;
            }
            case 's': {
                if( (argv[i][2]) != '\0')return false;
                break;
            }
            // en todos estos no se hace nada, ya que es sólo un control de que esten ok
            case 'p': {
                if( (argv[i][2]) != '\0')return false;
                break;
            }
            case 'r': {
                if( (argv[i][2]) != '\0')return false;    // Control en cada uno parametros 1 letra
                break;
            }
            case 'n': {
                if( (argv[i][2]) != '\0')return false;
                break;
            }
            case 'd': {
                if( (argv[i][2]) != '\0')return false;
                break;
            }
            case 'f': {
                if( (argv[i][2]) != '\0')return false;
                break;
            }
            case 'b':      //guardo el ratio del blur
            {
                if( (argv[i][2]) != '\0')return false;
                if ( argv[i + 1] )
                {
                    long aux_long;
                    if (!(string_a_long(argv[i+1],&aux_long))) {
                        printf("Error al convertir la cadena a un entero");
                        return false;
                    }
                    if(!( aux_long>0 && aux_long <= 0xFFFFFF )) {
                        printf("Valor incorrecto del blur");
                        return false;
                    }
                    datos->blur_rate=aux_long;
                    i++;
                    break;
                }
                else
                {
                    printf( "Error, opcion -b debe tener un RATIO ... use -h para ayuda.\n" );
                    error = true;
                    break;
                }
            }
            case 'l': { // guardo los parametros para LH//LV
                switch ( argv[i][2] )
                {
                case 'h':
                {
                    if ( ( argv[i + 1] ) && ( argv[i + 2] ) && ( argv[i + 3] ) ) // Si LH tiene 3 parámetros...
                    {
                        long aux_long2;
                        if (!(string_a_long(argv[i+1],&aux_long2))) {
                            printf("Error al convertir la cadena a un entero");
                            return false;
                        }
                        datos->lineas_hor_ancho = aux_long2;

                        aux_long2 = 0;
                        if (!(string_a_long(argv[i+2],&aux_long2))) {
                            printf("Error al convertir la cadena a un entero");
                            return false;
                        }
                        datos->lineas_hor_espacio = aux_long2;

                        aux_long2 = 0;
                        if (!(string_a_long(argv[i+3],&aux_long2))) {
                            printf("Error al convertir la cadena a un entero");
                            return false;
                        }
                        if( ! ( aux_long2 > 0 &&
                                aux_long2 <= 0xFFFFFF ) ) {
                            printf( "Color incorrecto para las lineas\n" );
                            return false;
                        }

                        datos->lineas_hor_color = colordesdeint(aux_long2);

                        i += 3;
                        break;
                    }
                    else
                    {
                        printf( "Error, -LH debe tener 3 opciones ... use -h para ayuda.\n" );
                        error = true;
                        break;
                    }
                }
                case 'v':
                {
                    if ( ( argv[i + 1] ) && ( argv[i + 2] ) && ( argv[i + 3] ) ) // Si LV tiene 3 parámetros
                    {
                        long aux_long2;
                        if (!(string_a_long(argv[i+1],&aux_long2))) {
                            printf("Error al convertir la cadena a un entero\n");
                            return false;
                        }
                        datos->lineas_ver_ancho = aux_long2;
                        aux_long2 = 0;
                        if (!(string_a_long(argv[i+2],&aux_long2))) {
                            printf("Error al convertir la cadena a un entero\n");
                            return false;
                        }
                        datos->lineas_ver_espacio = aux_long2;
                        aux_long2 = 0;
                        if (!(string_a_long(argv[i+3],&aux_long2))) {
                            printf("Error al convertir la cadena a un entero\n");
                            return false;
                        }

                        if( ! ( aux_long2 > 0 &&
                                aux_long2 <= 0xFFFFFF ) ) {
                            printf( "Color incorrecto para las lineas\n" );
                            return false;
                        }

                        datos->lineas_ver_color = colordesdeint(aux_long2);
                        i += 3;
                        break;
                    }
                    else
                    {
                        printf( "Error, -LV debe tener 3 opciones ... use -h para ayuda.\n" );
                        error = true;
                        break;
                    }
                }
                default:
                    printf("Parametro incorrecto ... use -h para ayuda.\n");
                    break;
                }//switch LH LV
                break; //del switch de LH LV
            } // case "L"
            case 'o': //guardo salida
                if( argv[i][2] != '\0')return false;
                if ( argv[i + 1] )
                {
                    datos->salida = argv[i + 1];
                    i++;
                    break;
                }
                else
                {
                    printf( "la opcion -o debe tener un parametro ... use -h para ayuda.\n" );
                    error = true;
                    break;
                }
            case 'i': //guardo entrada
                if( argv[i][2] != '\0')return false;
                if ( argv[i + 1] )
                {
                    datos->entrada = argv[i + 1];
                    i++;
                    break;
                }
                else
                {
                    printf( "la opcion -i debe tener un parametro ... use -h para ayuda.\n" );
                    error = true;
                    break;
                }
            default:
            {
                printf( "Parametro incorrecto ... use -h para ayuda.\n" );
                error = true;
                break;
            }
            } //switch
        } //if
        else // Si el par   am no es "-algo"
        {
            printf( "Cada parametro debe comenzar con \"-\" ... use -h para ayuda.\n" );
            error = true;
        }
        i++;
    } //while
    if ( datos->entrada == NULL ) //si no se ingreso archivo para abrir, error
    {
        printf( "Error, no se ingreso archivo de entrada\n" );
        error = true;
    }
    if ( error ) return false;
    return true; // Si no hubo error, se retorna true
} //funcion


/* Recibe los valores de los parámetros recolectados en parametros_correctos y los emplea para llamar a la función correspondiente en cada caso.
 * Recibe también los parámetros al programa, y la cantidad que son */
bool procesar( char *argv[], int argc, datix *datos )
{
    int i = 1;
    bool guardar=false;
    bmp_t *bmpfile;
    bmpfile = crear_imagen_archivo( datos->entrada );
    while ( i < argc )
    {
        switch ( argv[i][1] )
        {
        case '?':
        {
            ayuda();
            break;
        }
        case 'h':
        {
            ayuda();
            break;
        }
        case 's':
        {
            mostrar_header( bmpfile );
            if(datos->salida != NULL )guardar=true;
            break;
        }
        case 'p':
        {
            flip_vertical( bmpfile );
            guardar=true;
            break;
        }
        case 'r':
        {
            rotar( bmpfile );
            guardar=true;
            break;
        }
        case 'n':
        {
            negativo( bmpfile );
            guardar=true;
            break;
        }
        case 'd':
        {
            redimensionar2x( bmpfile );
            guardar=true;
            break;
        }
        case 'f':
        {
            redimensionar1_2x( bmpfile );
            guardar=true;
            break;
        }
        case 'b':
        {
            blur( datos->blur_rate, bmpfile );    //saltea porque ya se guardo anteriormente
            i++;
            guardar=true;
            break;
        }
        case 'l': {
            switch ( argv[i][2] )
            {
            case 'h':
            {
                addlineash( datos->lineas_hor_ancho, datos->lineas_hor_espacio, datos->lineas_hor_color, bmpfile );
                i += 3;
                guardar=true;
                break;
            }
            case 'v':
            {
                addlineasv( datos->lineas_ver_ancho, datos->lineas_ver_espacio, datos->lineas_ver_color, bmpfile );    //< se saltea porque ya se guardo
                i += 3;
                guardar=true;
                break;
            }
            }
            break;
        }
        case 'o': {
            i++;    //se saltea ya que ya se guardo anteriormente
            guardar=true;
            break;
        }
        case 'i': {
            i++;    //mismo^
            break;
        }
        } //switch
        i++;
    } //while
    // volcar el bmp de memoria a un archivo
    if(guardar)
        if(!grabar_archivo( bmpfile, datos->salida == NULL? "out.bmp" : datos->salida )) {
            fprintf( stderr, "Error al grabar el archivo en el disco");
            return false;
        }
    //destruir el archivo de la memoria
    if(datos->entrada != NULL) {
        if(!destruir_bmp( bmpfile )) {
            fprintf( stderr, "Error al liberar la memoria de la imagen");
            return false;
        }
    }
    return true;
} //funcion

