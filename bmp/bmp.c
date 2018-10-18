/***********************************************************************
 *
 *  Módulo: Se modulariza la implementación de todas las funciones
 *          para procesar BMP's.
 *  Autor:  Martín Aguilar
 *
 * ********************************************************************/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../headers/bmp.h"
#include <stdbool.h>
#include <math.h>


/*
 * Tipo para la paleta de colores, conteniendo una lista de colores
 * y la cantidad que son.
 */
typedef struct
{
    uint64_t cant;
    bmpcolor_t *colores;
} paleta_color;


/*
 * Tipo del Bitmap File Header, el MagicNumber está separado, para
 * que la estructura esté alineada y se pueda usar para leer
 * directamente y no campo a campo.
 */
typedef struct
{
    uint32_t filesz;
    uint32_t reserved;
    uint32_t bmp_offset;
} bitmapfileheader;

/*
 * Tipo para el Bitmap Info Header, contiene los datos con la
 * información del encabezado
 */
typedef struct
{
    uint32_t header_sz;
    int32_t width;
    int32_t height;
    uint16_t nplanes;
    uint16_t bitspp;
    uint32_t tipo_compres;
    uint32_t bmp_bytesz;
    int32_t hres;
    int32_t vres;
    uint32_t ncolores;
    uint32_t n_colores_imp;
} bitmapinfoheader;

/*
 * Tipo BMP. De esta forma se representa la imágen completa en la
 * memoria. Incluye un File header, un Info header, una paleta y
 * una matriz de colores. También se incluye el MagicNumber.
 */
struct bmp
{
    uint16_t magic;
    bitmapfileheader fileheader;
    bitmapinfoheader infoheader;
    paleta_color      paleta;
    bmpcolor_t         **pixels;
};

// ENCABEZADOS FUNCIONES

uint8_t coloresde_paleta( const bmp_t *imagen, const bmpcolor_t color );

bool pintar_pixel( const bmp_t *const imagen,
                   const uint32_t x,
                   const uint32_t y,
                   const uint8_t r,
                   const uint8_t g,
                   const uint8_t b );

bool pintar_pixelcolor( const bmp_t *imagen,
                        const uint32_t x,
                        const uint32_t y,
                        const bmpcolor_t color );

bmpcolor_t promediopixels( bmpcolor_t **pixels,
                           const int32_t ancho,
                           const int32_t alto,
                           const int32_t x,
                           const int32_t y,
                           const int32_t radio );

bool grabar_paleta( FILE *fbmp, const bmp_t *imagen );

void liberar_pixels( const bmp_t *imagen );

bool grabar_file_header( FILE *fbmp, bmp_t *imagen );

bool grabar_info_header( FILE *fbmp, bmp_t *imagen );

bool grabar_pixels_1bpp( bmp_t *imagen, FILE *fbmp, uint32_t alineada );

bool grabar_pixels_8bpp( bmp_t *imagen, FILE *fbmp, uint32_t alineada );

bool grabar_pixels_24bpp( bmp_t *imagen, FILE *fbmp, uint32_t alineada );

bool leer_pixels_8bpp(   FILE *fbmp, bmp_t *imagen,
                         const uint32_t fila_alineada );

bool leer_pixels_24bpp(  FILE *fbmp,
                         bmp_t *imagen,
                         const uint32_t fila_alineada );

bmpcolor_t **crear_matriz_pixels(   const int32_t width,
                                    const int32_t height );

bool leer_pixels(FILE *fbmp, bmp_t *imagen );

// FIN ENCABEZADOS

/*
 * Lee los píxeles de una imágen de 1BPP, y los guarda en la matriz
 * en memoria. Recibe el tamaño de cada fila de la imágen alineada
 * (incluyendo el padding)
 */
bool leer_pixels_1bpp(   FILE *fbmp,
                         bmp_t *imagen,
                         uint32_t fila_alineada ) {
    int i;
    long x, y;
    uint8_t *ptmp;
    uint8_t  ctmp;
    long   nshift;
    int32_t contador, alto, ancho;

    uint8_t bufferfila[fila_alineada];

    alto = imagen->infoheader.height;
    ancho  = imagen->infoheader.width;
    contador  = alto;

    i = -1;
    y = ( alto - 1 );

    for ( ; contador--; y += i )
    {

        if ( fread( bufferfila, sizeof( uint8_t ), fila_alineada, fbmp ) != fila_alineada )
        {
            fprintf( stderr, "Error leyendo fila de pixeles.\n" );
            return false;
        }

        ptmp = bufferfila;
        ctmp = *ptmp++;

        for ( x = 0L, nshift = 8L; x < ancho; x++ ) /* COLUMNAS - ANCHO */
        {
            if ( !nshift )
            {
                nshift = 8L;
                ctmp = *ptmp++;
            }

            imagen->pixels[y][x] = imagen->paleta.colores[ ( ctmp >> --nshift ) & 1 ];
        }
    }

    return true;
}

/*
 * Lee los píxeles de una imágen de 8BPP, y los guarda en la matriz
 * en memoria. Recibe el tamaño de cada fila de la imágen alineada
 * (incluyendo el padding)
 */
bool leer_pixels_8bpp(   FILE *fbmp, bmp_t *imagen,
                         const uint32_t fila_alineada ) {
    int32_t i;
    long x, y;
    int32_t height, width, contador;
    uint8_t *ptmp;

    uint8_t bufferfila[fila_alineada];

    height = imagen->infoheader.height;
    width  = imagen->infoheader.width;
    contador  = height;

    i = -1;
    y = ( height - 1 );
    for ( ; contador--; y += i ) /* bucle para las filas */
    {
        /* leer la fila */
        if ( fread( bufferfila, sizeof( uint8_t ), fila_alineada, fbmp ) != fila_alineada )
        {
            fprintf( stderr, "Error leyendo fila de pixeles.\n" );
            return false;
            /*liberar matriz*/
        }
        ptmp = bufferfila;

        for ( x = 0L; x < width; x++ )
        {
            imagen->pixels[y][x] = imagen->paleta.colores[ *ptmp++ ];
        }

    }

    return true;
}


/*
 * Lee los píxeles de una imágen de 24BPP, y los guarda en la matriz
 * en memoria. Recibe el tamaño de cada fila de la imágen alineada
 * (incluyendo el padding)
 */
bool leer_pixels_24bpp(  FILE *fbmp,
                         bmp_t *imagen,
                         const uint32_t fila_alineada ) {
    int32_t i;
    long x, y;
    int32_t contador, height, width;
    uint8_t *ptmp;

    uint8_t bufferfila[fila_alineada];

    height = imagen->infoheader.height;
    width  = imagen->infoheader.width;
    contador  = height;

    i = -1;
    y = ( height - 1 );

    for ( ; contador--; y += i ) /* bucle para las filas */
    {
        /* leer la fila */
        if ( fread( bufferfila, sizeof( uint8_t ), fila_alineada, fbmp ) != fila_alineada )
        {
            fprintf( stderr, "Error leyendo fila de pixeles.\n" );
            return false;
            /* liberar matriz */
        }

        ptmp = bufferfila;
        bmpcolor_t color;
        for ( x = 0L; x < width; x++ )
        {
            color.blue  = *ptmp++;
            color.green = *ptmp++;
            color.red   = *ptmp++;
            color.alpha = 0;

            imagen->pixels[y][x] = color;
        }
    }

    return true;
}

/*
 * Aloca memoria para la matriz de píxeles de la imágen en memoria.
 * Recibe el alto y el ancho que debe tener dicha matriz.
 */
bmpcolor_t **crear_matriz_pixels(   const int32_t width,
                                    const int32_t height ) {
    bmpcolor_t **pixels = NULL;
    int i;

    /* alocar memoria para el arreglo de filas */
    pixels = ( bmpcolor_t ** ) malloc( sizeof( bmpcolor_t * ) * height );
    if ( pixels == NULL )
    {
        fprintf( stderr, "Error alocando memoria para el arreglo de filas\n" );
        return NULL;
    }

    /* alocar memoria para cada fila */
    for ( i = 0; i <  height; i++ )
    {
        pixels[i] = ( bmpcolor_t * ) malloc( sizeof( bmpcolor_t ) *  width );
        if ( pixels[i] == NULL )
            break;
    }

    /* si salimos de alocar filas por error de memoria, debemos liberar */
    if ( i < height )
    {
        while ( i )
        {
            free( pixels[--i] );
        }

        free( pixels );
        fprintf( stderr, "Error alocando memoria para las filas\n" );

        return NULL;
    }

    return pixels;
}

/*Operaciones necesarias antes de leer los pixels, como el cálculo del
 * tamaño de la fila alineada, el llamado a la alocación de memoria para
 * la matriz, y luego el switch de
 * 1, 8 o 24 bits por pixels, dependiendo la imágen.
*/
bool leer_pixels(FILE *fbmp, bmp_t *imagen ) {
    long fila_alineada;
    long bitsxfila;

    //Cantidad de bits por fila que va a tener el bmp
    bitsxfila = imagen->infoheader.width * imagen->infoheader.bitspp;
    //Se redondea a múltiplo de 32
    if ( bitsxfila % 32 )
    {
        bitsxfila += 32 - ( bitsxfila % 32 );
    }
    /* expresar el tamaño en BYTES */
    fila_alineada = bitsxfila / 8UL;



    // Controlar
    if ( imagen->infoheader.bmp_bytesz != ( bitsxfila * imagen->infoheader.height ) / 8UL )
    {
        /* se debe comparar esto con la información guardada en el info-header */
        fprintf( stderr, "El tamaño del arreglo de pixeles no coincide\n" );
        return false;
    }

    /* alocar memoria para el arreglo de filas */
    imagen->pixels = crear_matriz_pixels( imagen->infoheader.width, imagen->infoheader.height );
    if ( imagen->pixels == NULL )
    {
        fprintf( stderr, "Error alocando memoria para el arreglo de filas\n" );
        return false;
    }

    switch(imagen->infoheader.bitspp) {
    case 1: {
        if(!leer_pixels_1bpp(fbmp, imagen, fila_alineada )) {
            fprintf( stderr, "Error leyendo los pixels del archivo \n");
            return false;
        }
        break;
    }
    case 8: {
        if(!leer_pixels_8bpp(fbmp, imagen, fila_alineada )) {
            fprintf( stderr, "Error leyendo los pixels del archivo \n");
            return false;
        }
        break;
    }
    case 24: {
        if(!leer_pixels_24bpp(fbmp, imagen, fila_alineada )) {
            fprintf( stderr, "Error leyendo los pixels del archivo \n");
            return false;
        }
        break;
    }
    } // switch
    return true;
} // end leer pixels general



/* Crea un bmp_t en la memoria a partir de un archivo .bmp, recibido
 * como parámetro bajo el nombre de filename.
*/
bmp_t *crear_imagen_archivo( const char *filename )
{
    FILE *fbmp;
    if( (fbmp = fopen (filename, "r")) == NULL) {
        fprintf( stderr, "Error al abrir el archivo\n");
        return NULL;
    }

    // Lectura MAGIC NUMBER del BMP
    uint16_t magic;

    if ( fread( &magic, sizeof( uint16_t ), 1, fbmp ) != 1  )
    {
        fprintf( stderr, "No se pudo leer el magic number de %s\n", filename );
        fclose(fbmp); // Se cierra el archivo
        return NULL;
    }

    // Si no es un BMP, no se sigue procesando.
    if ( magic != 0x4d42 )
    {
        fprintf( stderr, "El archivo %s NO es un BMP\n", filename );
        fclose(fbmp); // Se cierra el archivo
        return NULL;
    }

    // Lectura bit map file header
    bitmapfileheader bfh;
    if ( fread ( &bfh, sizeof ( bfh ), 1, fbmp) != 1)
    {
        fprintf( stderr, "Error al leer el bitmap file header de %s\n", filename);
        fclose(fbmp); // Se cierra el archivo
        return NULL;
    }

    // Lecutra bit map info header
    bitmapinfoheader bih;
    if ( fread ( &bih, sizeof ( bih ), 1, fbmp) != 1)
    {
        fprintf( stderr, "Error al leer el bitmap info header de %s\n",
                 filename);
        fclose(fbmp);
        return NULL;
    }

    // Verificar que sea de 1, 8 o 24 bpp
    if (bih.bitspp != 1 && bih.bitspp != 8 && bih.bitspp != 24)
    {
        fprintf( stderr, "Error: imagen no soportada, BPP invalido" );
        fclose(fbmp);
        return NULL;
    }

    // Creo la variable bmp, ya que el archivo es válido
    bmp_t *imagen;
    imagen = ( bmp_t* ) malloc ( sizeof ( bmp_t) );
    if ( imagen == NULL ) {
        fprintf( stderr, "Error al alocar memoria para la imagen");
        fclose(fbmp);
        return NULL;
    }
    imagen->magic = magic;
    imagen->infoheader = bih;
    imagen->fileheader = bfh;

    // Si es de 1 o 8 bpp, hay que leer la paleta
    if ( bih.bitspp  == 1 || bih.bitspp == 8 )
    {
        uint32_t ncolores;
        if ( bih.ncolores ) // Si el info header tiene la cantidad de colores, la usamos, si no, se calcula por el else
            ncolores = bih.ncolores;
        else
            ncolores = 1 << bih.bitspp; // Igual a 2^BPP
        imagen->paleta.colores = ( bmpcolor_t * ) malloc( sizeof( bmpcolor_t ) * ncolores );
        if ( imagen->paleta.colores == NULL ) {
            fprintf( stderr, "Error al alocar memoria para la paleta de colores");
            fclose(fbmp); // Se cierra el archivo
            return NULL;
        }

        // Leo la paleta y la guardo donde aloqué la memoria
        if ( fread( imagen->paleta.colores, sizeof( bmpcolor_t ), ncolores, fbmp ) != ncolores  )
        {
            // Si falla al leer, liberamos lo alocado
            fprintf( stderr, "Error al leer la paleta");
            free( imagen->paleta.colores );
            imagen->paleta.colores = NULL;
            fclose(fbmp); // Se cierra el archivo
            return NULL;
        }
        imagen->paleta.cant = ncolores;

    } // Termina leer paleta
    // Lectura pixels --->

    if ( !leer_pixels( fbmp, imagen ) )
    {
        fprintf( stderr, "Error leyendo los pixels del archivo %s\n", filename );
        fclose( fbmp );

        if ( imagen->paleta.cant && imagen->paleta.colores )
            free( imagen->paleta.colores );
        free( imagen );
        return NULL;
    }

// endIF del leer de archivo
    fclose(fbmp); // Se cierra el archivo
    return imagen;
}

/*
 * Imprime en stdout el header de la imágen en memoria, recibida por
 * parámetro.
 */
void mostrar_header( bmp_t *imagen )
{
    fprintf( stdout, "\nBitmap fileheader\n\n"
             "\tSignature:        %X\n"
             "\tFile size:        %d\n"
             "\tReserved:         %d\n"
             "\tData Offset:      %d\n",

             imagen->magic,
             imagen->fileheader.filesz,
             imagen->fileheader.reserved,
             imagen->fileheader.bmp_offset );

    fprintf( stdout, "\nBitmap info header\n\n"
             "\tHeader size:      %d\n"
             "\tWidth:            %d\n"
             "\tHeight:           %d\n"
             "\tPlanes:           %d\n"
             "\tBPP (BitCount):   %d\n"
             "\tCompression:      %d\n"
             "\tDataSize:         %d\n"
             "\tX_PPM:            %d\n"
             "\tY_PPM:            %d\n"
             "\tColors used:      %d\n"
             "\tColors important: %d\n",

             imagen->infoheader.header_sz,
             imagen->infoheader.width,
             imagen->infoheader.height,
             imagen->infoheader.nplanes,
             imagen->infoheader.bitspp,
             imagen->infoheader.tipo_compres,
             imagen->infoheader.bmp_bytesz,
             imagen->infoheader.hres,
             imagen->infoheader.vres,
             imagen->infoheader.ncolores,
             imagen->infoheader.n_colores_imp );

    fprintf( stdout, "\nPaleta de colores:\n\n" );

    if ( imagen->infoheader.bitspp  == 1 || imagen->infoheader.bitspp == 8 )
    {
        fprintf( stdout, "\t%-10s%-10s%-10s%-10s\n", "Id", "Blue", "Green", "Red" );
        int i;
        bmpcolor_t color;
        for ( i = 0; i < ( int ) imagen->paleta.cant; i++ )
        {
            color = imagen->paleta.colores[i];
            fprintf( stdout, "\t%-10d%-10d%-10d%-10d\n", i, color.blue, color.green, color.red );
        }
    }
    else
    {
        fprintf( stdout, "\tNo tiene\n" );
    }
}

/*
 * Realiza un "flip vertical" de la imágen. Es decir, la da vuelta.
 */
void flip_vertical( const bmp_t *const imagen )
{
    bmpcolor_t **ppio, **final;
    bmpcolor_t *tmp;

    ppio = imagen->pixels;
    final = ppio + ( imagen->infoheader.height - 1 );

    while ( ppio < final )
    {
        tmp = *ppio;
        *ppio = *final;
        *final = tmp;

        final--;
        ppio++;
    }

    return;
}

/*
 * Rota la imágen 90 grados.
 */
void rotar( bmp_t *const imagen )
{
    int32_t i, j, ancho, alto;
    uint32_t res;

    /* intercambiar ancho y alto */
    alto = imagen->infoheader.width;
    ancho  = imagen->infoheader.height;

    /* intercambiar resolucion vert y horiz */
    res = imagen->infoheader.vres;
    imagen->infoheader.vres = imagen->infoheader.hres;
    imagen->infoheader.hres = res;

    /* bmp_bytesz se calcula al guardar */

    bmpcolor_t **pixels = crear_matriz_pixels( ancho, alto );
    if ( pixels == NULL )
    {
        fprintf( stderr, "Error alocando para pixels\n" );
        return;
    }

    /* Roto la matriz*/
    for ( i = 0; i <  imagen->infoheader.height; i++ )
    {
        for ( j = 0; j < imagen->infoheader.width; j++ )
            pixels[alto - ( j + 1 )][i] = imagen->pixels[i][j];
    }

    /* Libero original */
    liberar_pixels( imagen );

    /* Actualizo */
    imagen->pixels = pixels;
    imagen->infoheader.height = alto;
    imagen->infoheader.width = ancho;

    return;
}


/*
 * Produce el "negativo" de la imágen.
 */
void negativo( const bmp_t *const imagen )
{
    uint32_t i, j;

    if ( imagen->infoheader.bitspp == 1 )
    {
        int indice = 0;
        for ( i = 0; i < ( uint32_t ) imagen->infoheader.height; i++ )
        {
            for ( j = 0; j < ( uint32_t ) imagen->infoheader.width; j++ )
            {
                indice = coloresde_paleta( imagen, imagen->pixels[i][j] );
                imagen->pixels[i][j] = imagen->paleta.colores[!indice];
            }
        }
    }
    else
    {
        for ( i = 0; i < ( uint32_t ) imagen->infoheader.height; i++ )
        {
            for ( j = 0; j < ( uint32_t ) imagen->infoheader.width; j++ )
            {
                imagen->pixels[i][j].red   = 255 - imagen->pixels[i][j].red;
                imagen->pixels[i][j].green = 255 - imagen->pixels[i][j].green;
                imagen->pixels[i][j].blue  = 255 - imagen->pixels[i][j].blue;
            }
        }
    }

    return;
}

/*
 * Devuelve el indice de un color desde la paleta, o el que más
 * se le parezca.
 */
uint8_t coloresde_paleta( const bmp_t *imagen, const bmpcolor_t color )
{
    int i, color_cercano = 0, temp = 0, distancia = 250000;

    for ( i = 0; i < ( int ) imagen->paleta.cant; ++i )
    {
        bmpcolor_t c = imagen->paleta.colores[i];
        temp = ( int ) pow( c.red   - color.red,   2.0 ) +
               ( int ) pow( c.green - color.green, 2.0 ) +
               ( int ) pow( c.blue  - color.blue,  2.0 );

        if ( temp < 1 )
        {
            color_cercano = i;
            break;
        }
        else if ( temp < distancia )
        {
            color_cercano = i;
            distancia = temp;
        }
    }

    return color_cercano;
}

/*
 * Redimensiona la imágen al doble de tamaño.
 */
void redimensionar2x( bmp_t *const imagen )
{
    int32_t j, k, ancho, alto;

    /* cambiar ancho y alto */
    ancho = imagen->infoheader.width * 2;
    alto = imagen->infoheader.height * 2;

    /* Duplicar resolucion */
    /* el tamaño se va a cambiar al guardar el archivo */
    imagen->infoheader.vres = imagen->infoheader.vres * 2;
    imagen->infoheader.hres = imagen->infoheader.hres * 2;


    bmpcolor_t **pixels = crear_matriz_pixels( ancho, alto );

    if ( pixels == NULL )
    {
        fprintf( stderr, "Error alocando memoria para los pixels\n" );
        return;
    }

    /* duplicar la matriz */
    for ( j = 0; j <  alto; j++ )
    {
        for ( k = 0; k < ancho; k++ )
            pixels[j][k] = promediopixels( imagen->pixels,
                                           imagen->infoheader.width,
                                           imagen->infoheader.height,
                                           k / 2U, j / 2U, 1 );
    }

    liberar_pixels( imagen ); // Se libera la original

    //Se guarda el nuevo
    imagen->pixels = pixels;
    imagen->infoheader.height = alto;
    imagen->infoheader.width = ancho;

    return;
}

/*
 * Redimensiona la imágen a la mitad del tamaño.
 */
void redimensionar1_2x( bmp_t *const imagen )
{
    int32_t j, k, ancho, alto;

    /* cambiar ancho y alto */
    ancho = imagen->infoheader.width / 2;
    alto = imagen->infoheader.height / 2;

    /* Duplicar resolucion */
    /* el tamaño se va a cambiar al guardar el archivo */
    imagen->infoheader.vres = imagen->infoheader.vres / 2;
    imagen->infoheader.hres = imagen->infoheader.hres / 2;


    bmpcolor_t **pixels = crear_matriz_pixels( ancho, alto );

    if ( pixels == NULL )
    {
        fprintf( stderr, "Error alocando memoria para los pixels\n" );
        return;
    }

    /* duplicar la matriz */
    for ( j = 0; j <  alto; j++ )
    {
        for ( k = 0; k < ancho; k++ )
            pixels[j][k] = promediopixels( imagen->pixels,
                                           imagen->infoheader.width,
                                           imagen->infoheader.height,
                                           k * 2U, j * 2U, 1 );
    }

    liberar_pixels( imagen ); // Se libera la original

    //Se guarda el nuevo
    imagen->pixels = pixels;
    imagen->infoheader.height = alto;
    imagen->infoheader.width = ancho;

    return;
}

/*
 * Produce el efecto 'blur' en la imágen, con el valor que rate
 * lo indique.
 */
void blur( const uint32_t rate, bmp_t *const imagen )

{
    int32_t i, j, ancho, alto;

    /* cambiar ancho y alto */
    ancho = imagen->infoheader.width;
    alto = imagen->infoheader.height;

    bmpcolor_t **pixels = crear_matriz_pixels( ancho, alto );

    if ( pixels == NULL )
    {
        fprintf( stderr, "Error alocando pixels\n" );
        return;
    }

    for ( i = 0; i <  alto; i++ )
    {
        for ( j = 0; j < ancho; j++ )
            pixels[i][j] = promediopixels( imagen->pixels,
                                           ancho,
                                           alto,
                                           j, i, rate );
    }

    /* Libero la original */
    liberar_pixels( imagen );

    /* Actualizo */
    imagen->pixels = pixels;
    return;
}

/*
 * Agrega líneas verticales a la imágen, el ancho de las líneas, el
 * espacio que hay entre ellas, y el color de las mismas, son recibidos
 * como parámetros.
 */
void addlineasv( uint32_t ancho,
                 uint32_t espacio,
                 bmpcolor_t color,
                 const bmp_t *const imagen )

{
    int a, b;
    unsigned c;
    int w, h;
    w = imagen->infoheader.width;
    h = imagen->infoheader.height;

    for ( b = 0; b < h; b++ )
    {
        for ( a = 0; a < w; a += espacio )
        {
            for ( c = 0; c < ancho && a < w; c++, a++ )
                if(!pintar_pixelcolor( imagen, a, b, color ))
                    fprintf(stderr, "Error pintando lineas\n");
        }
    }
}


/*
 * Pinta un píxel ubicado en [X][Y] del color RGB. Todos los datos son
 * recibidos como parámetros.
 */
bool pintar_pixel( const bmp_t *const imagen,
                   const uint32_t x,
                   const uint32_t y,
                   const uint8_t r,
                   const uint8_t g,
                   const uint8_t b )
{
    if ( x > ( u_int32_t )imagen->infoheader.width ||
            y > ( u_int32_t )imagen->infoheader.height )
    {
        return false;
    }

    bmpcolor_t color = { b, g, r, 0 };
    imagen->pixels[y][x] = color;

    return true;
}

/*
 * Pintar pixel general, llama a pintar pixel color, luego de "desarmar"
 * el color en rojo, azul, y verde.
 */
bool pintar_pixelcolor( const bmp_t *imagen,
                        const uint32_t x,
                        const uint32_t y,
                        const bmpcolor_t color )
{
    return pintar_pixel( imagen, x, y, color.red, color.green, color.blue );
}

/*
 * Agrega líneas horizontales a la imágen, el ancho de las líneas, el
 * espacio que hay entre ellas, y el color de las mismas, son recibidos
 * como parámetros.
 */
void addlineash( uint32_t ancho,
                 uint32_t espacio,
                 bmpcolor_t color,
                 const bmp_t *const imagen )
{
    unsigned x, y;
    unsigned f;
    unsigned an, al;
    an = imagen->infoheader.width;
    al = imagen->infoheader.height;

    for ( y = 0; y < al; y += espacio )
    {
        for ( f = 0; f < ancho && y < al; f++, y++ )
            for ( x = 0; x < an; x++ )
                if(!pintar_pixelcolor( imagen, x, y, color ))
                    fprintf(stderr, "Error pintando lineas\n");
    }
}

/*
 * Devuelve el color promedio de los píxeles ubicados en un radio de
 * RADIO desde el pixel ubicado en [X][Y]
 */
bmpcolor_t promediopixels( bmpcolor_t **pixels,
                           const int32_t ancho,
                           const int32_t alto,
                           const int32_t x,
                           const int32_t y,
                           const int32_t radio )
{
    uint32_t xx, yy, maxy, maxx, cont, r, g, b;
    bmpcolor_t retornar;

    xx = x - radio < 0 ? 0 : x - radio;
    yy = y - radio < 0 ? 0 : y - radio;
    maxx = x + radio > ancho  ? ancho  : x + radio;
    maxy = y + radio > alto ? alto : y + radio;

    r = g = b = cont = 0;

    for ( ; yy < maxy; yy++ )
    {
        for ( ; xx < maxx; xx++ )
        {
            r += pixels[yy][xx].red;
            g += pixels[yy][xx].green;
            b += pixels[yy][xx].blue;

            cont++;
        }
    }

    retornar.red   = r / cont;
    retornar.green = g / cont;
    retornar.blue  = b / cont;
    retornar.alpha = 0;

    return retornar;
}


/*
 * Graba el archivo que estaba en la memoria en un .bmp, cuyo nombre se
 * recibe como parámetro a la función.
 */
bool grabar_archivo( bmp_t *imagen, const char *salida )
{
    FILE *fbmp;

    uint32_t offset, fila_alineada, tamanioimagen;

    /* verificar puntero no nulo */
    if ( !imagen )
    {
        fprintf( stderr, "No hay BMP en memoria\n" );
        return false;
    }

    /* verificar NOMBRE del archivo*/
    if ( !salida )
    {
        fprintf( stderr, "Error con el nombre para guardar\
                            del archivo\n" );
        return false;
    }

    /* Control datos correctos */
    if ( !imagen->infoheader.width || !imagen->infoheader.height )
    {
        fprintf( stderr, "El BMP debe tener un ancho y alto mayor que cero pixel\n" );
        return false;
    }

    /* SETTEAR algunos campos */

    imagen->fileheader.reserved = 0;
    imagen->magic = 0x4d42;
    imagen->infoheader.ncolores = imagen->paleta.cant;
    imagen->infoheader.n_colores_imp = imagen->infoheader.ncolores;

    /* Offset al arreglo de pixeles --->
     * File header, 14 bytes
     * Size del info header
     * Mas el tamaño de la paleta (4 bytes para cada color, rgba)
    */
    offset = 14 + sizeof( bitmapinfoheader )
             + imagen->paleta.cant * 4UL;

    /* tamño del arreglo de pixeles */

    /* Tamaño de cada fila */
    fila_alineada = imagen->infoheader.width * imagen->infoheader.bitspp;
    if ( fila_alineada % 32 )
    {
        fila_alineada += 32 - ( fila_alineada % 32 );
    }

    /* Fila x altura = tamaño total en bits*/
    tamanioimagen = imagen->infoheader.height * fila_alineada;
    imagen->infoheader.bmp_bytesz = tamanioimagen / 8U;

    imagen->fileheader.bmp_offset = offset;
    imagen->fileheader.filesz = offset + tamanioimagen / 8U;

    /* abrir el archivo para escritura */
    if ( ( fbmp = fopen( salida, "w" ) ) == NULL )
    {
        fprintf( stderr, "Error al abrir %s para escribir\n", salida );
        return false;
    }

    if ( !grabar_file_header( fbmp, imagen ) )
    {
        fprintf( stderr, "Error escribiendo el encabezado del archivo BMP\n" );
        fclose( fbmp );
        return false;
    }

    if ( !grabar_info_header( fbmp, imagen ) )
    {
        fprintf( stderr, "Error escribiendo el info header del BMP\n" );
        fclose( fbmp );
        return false;
    }

    if (! ( imagen->infoheader.bitspp == 24 ) ) {
        if ( !imagen->paleta.cant || !imagen->paleta.colores ||
                !grabar_paleta( fbmp, imagen ) )
        {
            fprintf( stderr, "Error escribiendo la plateta de colores del BMP\n" );
            fclose( fbmp );
            return false;
        }
    }
    switch(imagen->infoheader.bitspp) {
    case 1: {
        if(!grabar_pixels_1bpp(imagen, fbmp, fila_alineada/8UL)) {
            fprintf( stderr, "Error guardando imagen\n" );
            fclose( fbmp );
            return false;
        }
        break;
    }
    case 8: {
        if(!grabar_pixels_8bpp(imagen, fbmp, fila_alineada/8UL)) {
            fprintf( stderr, "Error guardando imagen\n" );
            fclose( fbmp );
            return false;
        }
        break;
    }
    case 24: {
        if(!grabar_pixels_24bpp(imagen, fbmp, fila_alineada/8UL)) {
            fprintf( stderr, "Error guardando imagen\n" );
            fclose( fbmp );
            return false;
        }
        break;
    } // 24
    } // Switch

    fclose( fbmp );
    return true;
} // Grabar pixels


/*
 * Graba la paleta de colores en el archivo.
 */
bool grabar_paleta( FILE *fbmp, const bmp_t *imagen )
{
    uint32_t ncolores = imagen->paleta.cant;

    if ( fwrite( imagen->paleta.colores, sizeof( bmpcolor_t ), ncolores, fbmp ) != ncolores )
        return false;

    return true;
}

/*
 * Destruye el BMP de la memoria.
 */
bool destruir_bmp( bmp_t *imagen )
{
    if ( imagen->paleta.cant && imagen->paleta.colores )
        free( imagen->paleta.colores );

    if ( imagen->pixels )
        liberar_pixels( imagen );

    free( imagen );
    imagen = NULL;

    return true;
}

/*
 * Libera el arreglo de colores de la memoria.
 */
void liberar_pixels( const bmp_t *imagen )
{
    int32_t i;
    for ( i = 0; i < imagen->infoheader.height; i++ )
    {
        free( imagen->pixels[i] );
    }

    free( imagen->pixels );
}




/*
 * Graba el file header en un archivo.
 */
bool grabar_file_header( FILE *fbmp, bmp_t *imagen )
{
    if ( fwrite( &imagen->magic, sizeof( imagen->magic ), 1, fbmp ) != 1 )
        return false;

    if ( fwrite( &imagen->fileheader.filesz, sizeof( imagen->fileheader.filesz ), 1, fbmp ) != 1 )
        return false;

    if ( fwrite( &imagen->fileheader.reserved, sizeof( imagen->fileheader.reserved ), 1, fbmp ) != 1 )
        return false;

    if ( fwrite( &imagen->fileheader.bmp_offset, sizeof( imagen->fileheader.bmp_offset ), 1, fbmp ) != 1 )
        return false;


    return true;
}

/*
 * Graba el info header en un archivo.
 */
bool grabar_info_header( FILE *fbmp, bmp_t *imagen )
{
    if ( fwrite( &imagen->infoheader, sizeof( imagen->infoheader ), 1, fbmp ) != 1 )
        return false;

    return true;
}


/*
 * Graba la matriz de píxeles que estaba en memoria, en un archivo.
 * En este caso se trata de imágenes de 1BPP
 */
bool grabar_pixels_1bpp( bmp_t *imagen, FILE *fbmp, uint32_t alineada )
{
    long x, y;
    uint8_t *punt;
    long   nshift;
    uint8_t  ctmp;

    int32_t alto = imagen->infoheader.height;
    int32_t ancho  = imagen->infoheader.width;

    uint8_t bufferfila[alineada];;
    bzero( bufferfila, alineada );

    for ( y = alto - 1; y >= 0L; y-- ) /* bucle para las filas */
    {
        punt = bufferfila;
        ctmp = 0;

        for ( x = 0L, nshift = 8L; x < ancho; x++ )
        {
            if ( !nshift )
            {
                nshift = 8L;
                *punt++ = ctmp;
                ctmp = 0;
            }

            ctmp |= ( ( uint8_t ) coloresde_paleta( imagen, imagen->pixels[y][x] ) << --nshift );
        }

        *punt = ctmp;
        fwrite( bufferfila, sizeof( uint8_t ), alineada, fbmp );

    }

    return true;
}


/*
 * Graba la matriz de píxeles que estaba en memoria, en un archivo.
 * En este caso se trata de imágenes de 8BPP
 */
bool grabar_pixels_8bpp( bmp_t *imagen, FILE *fbmp, uint32_t alineada )
{
    long x, y;
    uint8_t *punt;

    int32_t alto = imagen->infoheader.height;
    int32_t ancho  = imagen->infoheader.width;

    uint8_t bufferfila[alineada];
    bzero( bufferfila, alineada );

    for ( y = alto - 1 ; y >= 0L; y-- ) /* Loop de las filas! */
    {
        punt = bufferfila;
        for ( x = 0L; x < ancho; x++ )
        {
            *punt++ = coloresde_paleta( imagen, imagen->pixels[y][x] );
        }

        fwrite( bufferfila, sizeof( uint8_t ), alineada, fbmp );

    }

    return true;
}


/*
 * Graba la matriz de píxeles que estaba en memoria, en un archivo.
 * En este caso se trata de imágenes de 24BPP
 */
bool grabar_pixels_24bpp( bmp_t *imagen, FILE *fbmp, uint32_t alineada )
{
    long x, y;
    uint8_t *punt;

    int32_t alto = imagen->infoheader.height;
    int32_t ancho  = imagen->infoheader.width;

    uint8_t bufferfila[alineada];
    bzero( bufferfila, alineada );

    for ( y = alto - 1 ; y >= 0L; y-- ) /* Loop de las filas! */
    {
        punt = bufferfila;
        bmpcolor_t color;
        for ( x = 0L; x < ancho; x++ )
        {
            color = imagen->pixels[y][x];

            *punt++ = color.blue;
            *punt++ = color.green;
            *punt++ = color.red;
        }

        fwrite( bufferfila, sizeof( uint8_t ), alineada, fbmp );

    }

    return true;
}
