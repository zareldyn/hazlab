/*
textures.c
CHARGEMENT DES TEXTURES
à partir de fichiers-image au format BMP/DIB(Windows V3) 24 bits non compressé uniquement
*/



/*=== externe ================================================================*/
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include "hazlab.h"
#include "textures.h"
extern void   InverserLong( hulong* const, char const );        /*main.c*/
extern void   EchangerGLbyte( GLubyte* const, GLubyte* const );
extern huchar CompterBits1( hulong );
extern char   Codage, TailleChar;



/*=== global(partagé) ========================================================*/
GLuint IdTextures[NBR_TEXTURES]; /*tableau d'identifiants de texture*/



/*=== global(textures.c) =====================================================*/
#define LIRE_LG( var )  fread( &var, 4l/TailleChar, 1, fichier )
#define QUITTER( var )  TermTextures( var );\
                        Erreur = hVRAI;\
                        exit( EXIT_FAILURE )

static char     NomFic[20];                /*nom du fichier en cours de traitement*/
static hbool    Erreur = hFAUX;            /*vrai en cas d'erreur mémoire*/
static GLubyte* AdrTextures[NBR_TEXTURES]; /*tableau de pointeurs vers les textures chargées*/
#ifdef UTIL_MIPMAPS
static GLint    ModeTexturage;             /*la qualité de texturage utilisée*/
#endif

typedef struct
        {
        GLubyte Bleu, Vert, Rouge;
        }
    PIXEL_BMP;



/*=== destruction des données ================================================*/
void TermTextures( char n )
{
    unsigned char i;

    if( Erreur )
        return;

    for( i = 0; i<n; i++ )
        free( AdrTextures[i] );

    glDeleteTextures( NBR_TEXTURES, IdTextures );
}



/*=== chargement en mémoire d'une image bitmap ===============================*/
hINLINE( static void LireBMP( FILE* const fichier, unsigned char const indice ) )
{
    long   largeur = 0l, hauteur = 0l; /*seul le type "long" est assuré par l'ISO de pouvoir contenir 4 octets*/
    hulong debut = 0, enTete = 0, nbPixels = 0, compress = 0, i = 0;

    fseek( fichier, 10l/TailleChar, SEEK_SET );
    if( !LIRE_LG( debut ) || !LIRE_LG( enTete ) || !LIRE_LG( largeur )
    || !LIRE_LG( hauteur ) || !LIRE_LG( nbPixels ) || !LIRE_LG( compress ) )
    {
        fprintf( stderr, hMSG_ERREUR_3, NomFic );
        QUITTER( indice );
    }

    if( Codage==hBIG_ENDIAN ) /*le standard BMP/DIB impose au fichier le codage "little endian"*/
    {
        InverserLong( &debut, 4 );
        InverserLong( &enTete, 4 );
        InverserLong( (hulong*)&largeur, 4 );
        InverserLong( (hulong*)&hauteur, 4 );
        nbPixels <<= 16; /*les deux premiers octets ne nous intéressent pas ...*/
        InverserLong( &nbPixels, 2 );
    }
    else
        nbPixels >>= 16; /*... mais attention au sens du décalage pour les virer ;-) (représentation logique et non physique)*/

    if( compress || enTete!=40ul || nbPixels!=24ul )
    {
        fprintf( stderr, hMSG_ERREUR_3, NomFic );
        QUITTER( indice );
    }
    if( CompterBits1( largeur )!=1 || CompterBits1( hauteur )!=1 )
    {
        fprintf( stderr, hMSG_ERREUR_5, NomFic );
        QUITTER( indice );
    }
    if( largeur>LIM_SUP_TEX || hauteur>LIM_SUP_TEX || largeur<LIM_INF_TEX || hauteur<LIM_INF_TEX )
    {
        fprintf( stderr, hMSG_ERREUR_4, NomFic );
        QUITTER( indice );
    }

    nbPixels = largeur*hauteur;
    if( (AdrTextures[indice] = calloc( nbPixels, 3*sizeof( GLubyte ) )) ==NULL )
    {
        fprintf( stderr, hMSG_ERREUR_1 );
        QUITTER( indice );
    }

    fseek( fichier, debut/TailleChar, SEEK_SET );
    switch( TailleChar )
    {
    case 1 : /*char 8 bits, banal pour nos architectures x86 ...*/
        {
        PIXEL_BMP pix;
        do
        {
            if( !fread( &pix, 3, 1, fichier ) ) /*3 chars (3 octets), 1 par 1*/
                break;
            AdrTextures[indice][3*i]   = pix.Rouge;
            AdrTextures[indice][3*i+1] = pix.Vert;
            AdrTextures[indice][3*i+2] = pix.Bleu;
        }
        while( ++i <nbPixels );
        break;
        }

    case 2 : /*... et char 16 bits, exotique mais vive la portabilité ;-)*/
        {
        char     comps[3]; /*6 composantes(octets), soit 3 chars (2 pixels) à lire*/
        GLubyte  r1, v1, b1, r2, v2, b2;
        do
        {
            if( !fread( comps, 1, 3, fichier ) ) /*1 char (2 octets), 3 par 3*/
                break;
            b1 = comps[0]>>8;
            v1 = (comps[0]<<8)>>8;
            r1 = comps[1]>>8;
            b2 = (comps[1]<<8)>>8;
            v2 = comps[2]>>8;
            r2 = (comps[2]<<8)>>8;
            if( Codage==hLITTLE_ENDIAN )
            {
                EchangerGLbyte( &b1, &v1 );
                EchangerGLbyte( &r1, &b2 );
                EchangerGLbyte( &v2, &r2 );
            }
            AdrTextures[indice][3*i]       = r1;
            AdrTextures[indice][3*i+1]     = v1;
            AdrTextures[indice][3*i+2]     = b1;
            AdrTextures[indice][3*i+3]     = r2;
            AdrTextures[indice][3*i+4]     = v2;
            AdrTextures[indice][3*(i++)+5] = b2;
        }
        while( ++i <nbPixels );
        }
    }

    glBindTexture( GL_TEXTURE_2D, IdTextures[indice] );

#ifdef UTIL_MIPMAPS
    gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, largeur, hauteur, GL_RGB, GL_UNSIGNED_BYTE, AdrTextures[indice] );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ModeTexturage );
#else
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, largeur, hauteur, 0, GL_RGB, GL_UNSIGNED_BYTE, AdrTextures[indice] );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
#endif
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glBindTexture( GL_TEXTURE_2D, 0 ); /*commande d'usage ... mais ici sert-elle à quelque chose ???*/
}



#ifdef UTIL_MIPMAPS
/*=== changement de la qualité de texturage MIPMAP ===========================*/
void ChangerModeTexturage( void )
{
    static int i;

    if( ModeTexturage==GL_LINEAR_MIPMAP_LINEAR )
        ModeTexturage = GL_LINEAR_MIPMAP_NEAREST;
    else
        ModeTexturage = GL_LINEAR_MIPMAP_LINEAR;

    for( i = -1; ++i <NBR_TEXTURES; )
    {
        glBindTexture( GL_TEXTURE_2D, IdTextures[i] );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ModeTexturage );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}
#endif



/*=== construction des données ===============================================*/
void InitTextures( void )
{
    char i;
    FILE* fic;

    glEnable( GL_TEXTURE_2D );
    glGenTextures( NBR_TEXTURES, IdTextures );
#ifdef UTIL_MIPMAPS
    ModeTexturage = GL_LINEAR_MIPMAP_LINEAR;
#endif

    for( i = 0; i<NBR_TEXTURES; i++ )
    {
        sprintf( NomFic, FORMAT_NOM_FIC, i+1 );
        if( (fic = fopen( NomFic, "rb" )) ==NULL )
        {
            fprintf( stderr, hMSG_ERREUR_2, NomFic );
            QUITTER( i );
        }
        LireBMP( fic, i );
        fclose( fic );
    }
}
