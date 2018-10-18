/***********************************************************************
 *
 * Módulo: Se modulariza la correción de todos los parámetros y luego
 *         el procesamiento
 * Autor:  Martín Aguilar
 *
 **********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "headers/validar.h"


int main( int argc, char *argv[] )
{
    datix datos;
    datos.ayuda = false;
    datos.entrada = NULL;
    datos.salida = NULL;
    // Si los parámetros son incorrectos, error, si no, sigue.
    if ( ( !parametros_correctos( argv, argc, &datos ) ) )
    {
        printf( "Error en los parámetros\n" );
        return( EXIT_FAILURE );
    }
    // Si ayuda es true, no importa lo demás, sólo se informa la ayuda.
    if (datos.ayuda) {
        ayuda();
        return EXIT_SUCCESS;
    }

    // Procesa los parámetros, si devuelve falso, informa el error.
    if ( !procesar( argv, argc, &datos ) )
    {
        fprintf( stderr, "Error al procesar los parámetros");
        return ( EXIT_FAILURE );
    }

    return EXIT_SUCCESS;


}
