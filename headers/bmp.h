/***********************************************************************
 * 
 * Módulo: Header de las funciones del bmp.c, con los encabezados
 *         de las funciones
 * Autor:  Martín Aguilar
 * 
 **********************************************************************/

#ifndef BMP_H
#define BMP_H
#include <stdint.h>
#include <stdbool.h>

//Estructura para el color
typedef struct
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} bmpcolor_t;


typedef struct bmp bmp_t;



/*
 *  Mostrar header recibe un puntero a un archivo bmp ya cargado
 *  en memoria, e imprime los headers en stdout.
*/
void mostrar_header( bmp_t *imagen );


/*
 * Rota la imágen 90 grados.
 */
void rotar( bmp_t *const imagen );

/*
 * Produce el "negativo" de la imágen.
 */
void negativo( const bmp_t *const imagen );


/*
 * Redimensiona la imágen al doble de tamaño.
 */
void redimensionar2x( bmp_t *const imagen );

/*
 * Redimensiona la imágen a la mitad de tamaño.
 */
void redimensionar1_2x( bmp_t *const imagen );

/*
 * Produce el efecto 'blur' en la imágen, con el valor que rate
 * lo indique.
 */
void blur( const uint32_t rate, bmp_t *const imagen );

/*
 * Agrega líneas verticales a la imágen, el ancho de las líneas, el
 * espacio que hay entre ellas, y el color de las mismas, son recibidos
 * como parámetros.
 */
void addlineasv( uint32_t ancho,
                 uint32_t espacio,
                 bmpcolor_t color,
                 const bmp_t *const imagen );


/*
 * Agrega líneas horizontales a la imágen, el ancho de las líneas, el
 * espacio que hay entre ellas, y el color de las mismas, son recibidos
 * como parámetros.
 */
void addlineash( uint32_t ancho,
                 uint32_t espacio,
                 bmpcolor_t color,
                 const bmp_t *const imagen );



/*
 * Graba el archivo desde la memoria a un .bmp, determinado por el
 * parámetro "salida"
 */
bool grabar_archivo( bmp_t *imagen, const char *salida );

/*
 * Destruye el bmp de la memoria.
 */
bool destruir_bmp( bmp_t *imagen );


/*
 * Recibe un nombre de archivo, lo abre, y lo vuelca a la memoria, retornando un puntero a una estructura representando el archivo bmp.
 */
bmp_t *crear_imagen_archivo( const char *filename );

/*
 * Realiza un "flip vertical" de la imágen. Es decir, la da vuelta.
 */
void flip_vertical( const bmp_t *const imagen );




#endif

