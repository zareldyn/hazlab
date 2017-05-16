/*
crealab.c
GENERATION DU LABYRINTHE (MATRICE CARREE + VERTICES OPENGL)
LabyIJ[i][j] =0 --> MUR
            !=0 --> PASSAGE
chaque bloc est de dimensions 1 x 1 x HTR_MUR (dimlab.h)
*/



/*=== externe ================================================================*/
#include <time.h>
#include <stdlib.h>
#include <GL/gl.h>
#include "hazlab.h"
#include "dimlab.h"
extern int TailleLab, * LabyPtr, ** LabyIJ; /*main.c*/



/*=== global(partagé) ========================================================*/
GLfloat* SommetsPtrMurs; /*début de la zone mémoire contenant les sommets (X,Y,Z) des murs*/
GLfloat* CoordTxPtrMurs; /*idem pour les coordonnées de texture (U,V)*/
int      NbVerticesMurs; /*nombre de vertices pour les murs*/

GLfloat* SommetsPtrSups; /*idem pour les faces supérieures*/
GLfloat* CoordTxPtrSups;
int      NbVerticesSups;

int Sortie; /*pour repérer la sortie en une seule coordonnée*/



/*=== global(crealab.c) ======================================================*/
#define HASARD( nMax )  (int)(rand()*(double)(nMax)/RAND_MAX) /*donne un entier pseudo-aléatoire compris entre 0 et nMax-1*/
/*FILE* fic;*/
#define QUITTER  fprintf( stderr, hMSG_ERREUR_1 );\
                 exit( EXIT_FAILURE )



/*=== création des vertices correspondants aux murs du labyrinthe ============*/
static void GenererVerticesMurs( void )
{
    size_t   nVtcMax = ((TailleLab>>1)*(TailleLab>>1)<<3)+14;
    GLfloat* somPtr = NULL, *ctxPtr = NULL;
    int      i = 0, j = 0;
    char     direction = 2;
    GLfloat  x, z, ax, az, longueur = 0.0f, dirx = 0.0f, dirz = 0.0f;

    if( (SommetsPtrMurs = somPtr = malloc( nVtcMax*sizeof( GLfloat )*3 )) ==NULL  /*taille temporaire, valeur maximum possible*/
    || (CoordTxPtrMurs = ctxPtr = malloc( nVtcMax*sizeof( GLfloat )<<1 )) ==NULL )
    {
        free( LabyPtr );
        free( LabyIJ );
        if( somPtr!=NULL )
            free( SommetsPtrMurs );
        if( ctxPtr!=NULL )
            free( CoordTxPtrMurs );
        QUITTER;
    }
    *somPtr++ = ax = -0.5f; /*X*/   /*1er couple de vertices (GL_QUAD_STRIP)*/
    *somPtr++ = ALT_MUR;    /*Y*/
    *somPtr++ = az = -0.5f; /*Z*/
    *ctxPtr++ = 0.0f;       /*U*/
    *ctxPtr++ = 1.0f;       /*V*/

    *somPtr++ = -0.5f;
    *somPtr++ = ALT_SOL;
    *somPtr++ = -0.5f;
    *ctxPtr++ = 0.0f;
    *ctxPtr++ = 0.0f;

    NbVerticesMurs = 1;
    do /*on recherche les coordonnées de tous les angles*/
    {
        switch( direction )
        {
        case 0 : /*vers les Z négatifs*/
            direction = 3;
            dirx = 0.5f;
            dirz = -0.5f;
            while( --i >=0 && !LabyIJ[i][j] ) /*on longe le mur jusqu'au prochain angle*/
            {
                if( j<TailleLab-1 && !LabyIJ[i][j+1] )
                {
                    direction = 1;
                    dirz = 0.5f;
                    i--;
                    break;
                }
            }
            i++;
            break;

        case 1 : /*vers les X positifs*/
            direction = 0;
            dirx = 0.5f;
            dirz = 0.5f;
            while( ++j <TailleLab && !LabyIJ[i][j] )
            {
                if( i<TailleLab-1 && !LabyIJ[i+1][j] )
                {
                    direction = 2;
                    dirx = -0.5f;
                    j++;
                    break;
                }
            }
            j--;
            break;

        case 2 : /*vers les Z positifs*/
            direction = 1;
            dirx = -0.5f;
            dirz = 0.5f;
            while( ++i <TailleLab && !LabyIJ[i][j] )
            {
                if( j>0 && !LabyIJ[i][j-1] )
                {
                    direction = 3;
                    dirz = -0.5f;
                    i++;
                    break;
                }
            }
            i--;
            break;

        case 3 : /*vers les X négatifs*/
            direction = 2;
            dirx = -0.5f;
            dirz = -0.5f;
            while( --j >=0 && !LabyIJ[i][j] )
            {
                if( i>0 && !LabyIJ[i-1][j] )
                {
                    direction = 0;
                    dirx = 0.5f;
                    j--;
                    break;
                }
            }
            j++;

        }
        *somPtr++ = x = j+dirx; /*X*/   /*couple de vertices suivant*/
        *somPtr++ = ALT_MUR;    /*Y*/
        *somPtr++ = z = i+dirz; /*Z*/
          longueur += z-az+x-ax;
          az = z;
          ax = x;
        *ctxPtr++ = longueur;   /*U*/
        *ctxPtr++ = 1.0f;       /*V*/

        *somPtr++ = x;          /*X*/
        *somPtr++ = ALT_SOL;    /*Y*/
        *somPtr++ = z;          /*Z*/
        *ctxPtr++ = longueur;   /*U*/
        *ctxPtr++ = 0.0f;       /*V*/
        NbVerticesMurs++;
    }
    while( x>0 || z>0 ); /*est-on revenu au point de départ ?*/
    NbVerticesMurs<<=1;

    if( (CoordTxPtrMurs = realloc( CoordTxPtrMurs, NbVerticesMurs*sizeof( GLfloat )<<1 )) ==NULL
    || (SommetsPtrMurs = realloc( SommetsPtrMurs, NbVerticesMurs*sizeof( GLfloat )*3 )) ==NULL )
    {
        free( LabyPtr );
        free( LabyIJ );
        free( SommetsPtrMurs );
        free( CoordTxPtrMurs );
        QUITTER;
    }
}



/*=== création des vertices correspondant au surfaces supérieures ============*/
static void GenererVerticesSups( void )
{
    int      i, j;
    GLfloat* somPtr = NULL, *ctxPtr = NULL;

    if( (SommetsPtrSups = somPtr = malloc( NbVerticesSups*sizeof( GLfloat )*3 )) ==NULL  /*taille temporaire, valeur maximum possible*/
    || (CoordTxPtrSups = ctxPtr = malloc( NbVerticesSups*sizeof( GLfloat )<<1 )) ==NULL )
    {
        free( LabyPtr );
        free( LabyIJ );
        free( SommetsPtrMurs );
        free( CoordTxPtrMurs );
        if( somPtr!=NULL )
            free( SommetsPtrSups );
        if( ctxPtr!=NULL )
            free( CoordTxPtrSups );
        QUITTER;
    }

    for( i = 0; i<TailleLab; i++ )
        for( j = 0; j<TailleLab; j++ )
            if( !LabyIJ[i][j] ) /*4 vertices à créer (GL_QUADS)*/
            {
                *somPtr++ = j-0.5f ; /*X*/
                *somPtr++ = ALT_MUR; /*Y*/
                *somPtr++ = i-0.5f;  /*Z*/

                *somPtr++ = j-0.5f;
                *somPtr++ = ALT_MUR;
                *somPtr++ = i+0.5f;

                *somPtr++ = j+0.5f;
                *somPtr++ = ALT_MUR;
                *somPtr++ = i+0.5f;

                *somPtr++ = j+0.5f;
                *somPtr++ = ALT_MUR;
                *somPtr++ = i-0.5f;

                if( !i || !j || i==TailleLab-1 || j==TailleLab-1 ) /*un bord*/
                {
                    *ctxPtr++ = 0.0f; /*U*/   /*quart de texture en haut à gauche*/
                    *ctxPtr++ = 1.0f; /*V*/

                    *ctxPtr++ = 0.0f;
                    *ctxPtr++ = 0.5f;

                    *ctxPtr++ = 0.5f;
                    *ctxPtr++ = 0.5f;

                    *ctxPtr++ = 0.5f;
                    *ctxPtr++ = 1.0f;
                }
                else
                    switch( !LabyIJ[i-1][j] + !LabyIJ[i+1][j] + !LabyIJ[i][j-1] + !LabyIJ[i][j+1] ) /*combien de murs sont adjacents ?*/
                    {
                    case 1 : /*un bout*/
                        *ctxPtr++ = 0.5f; /*U*/   /*quart de texture en haut à droite*/
                        *ctxPtr++ = 1.0f; /*V*/

                        *ctxPtr++ = 0.5f;
                        *ctxPtr++ = 0.5f;

                        *ctxPtr++ = 1.0f;
                        *ctxPtr++ = 0.5f;

                        *ctxPtr++ = 1.0f;
                        *ctxPtr++ = 1.0f;
                        break;
                    case 3 :
                    case 4 : /*un croisement*/
                        *ctxPtr++ = 0.5f; /*U*/   /*quart de texture en bas à droite*/
                        *ctxPtr++ = 0.5f; /*V*/

                        *ctxPtr++ = 0.5f;
                        *ctxPtr++ = 0.0f;

                        *ctxPtr++ = 1.0f;
                        *ctxPtr++ = 0.0f;

                        *ctxPtr++ = 1.0f;
                        *ctxPtr++ = 0.5f;
                        break;
                    default :
                        *ctxPtr++ = 0.0f; /*U*/   /*quart de texture en bas à gauche*/
                        *ctxPtr++ = 0.5f; /*V*/

                        *ctxPtr++ = 0.0f;
                        *ctxPtr++ = 0.0f;

                        *ctxPtr++ = 0.5f;
                        *ctxPtr++ = 0.0f;

                        *ctxPtr++ = 0.5f;
                        *ctxPtr++ = 0.5f;
                    }
            }
}



/*=== définition d'un chemin dans le labyrinthe ==============================*/
hINLINE( static void DefinirChemin( int uRef, int vRef, int const idChemin ) )
{
    int   i = uRef, j = vRef, p = uRef, q = vRef;
    unsigned char  direction;
    hbool stop = hFAUX, dirDispo[4];

    while( 1 ) /*tant qu'on est pas bloqué, on trace un chemin au hasard*/
    {
        dirDispo[0] = dirDispo[1] = dirDispo[2] = dirDispo[3] = hVRAI;

        do /*on recherche une direction disponible*/
        {
            direction = HASARD( 4 );
            while( !dirDispo[direction] )
                if( ++direction ==4 )
                    direction = 0;
            switch( direction )
            {
            case 0 :
                i-=2; break;
            case 1 :
                j+=2; break;
            case 2 :
                i+=2; break;
            case 3 :
                j-=2;
            }
            if( i==-1 || j==-1 || i==TailleLab || j==TailleLab || LabyIJ[i][j]==idChemin ) /* si on a atteint un bord, ou si le chemin est revenu sur lui-même*/
            {
                i = uRef;
                j = vRef;
                dirDispo[direction] = hFAUX; /*on évitera de rechoisir cette direction*/
                if( !(dirDispo[0]+dirDispo[1]+dirDispo[2]+dirDispo[3]) ) /*s'il n'y a plus de direction disponible*/
                {
                    if( idChemin==1 ) /*s'il s'agit du premier chemin ...*/
                        return; /*... on abandonne et on s'attaque à un nouveau chemin*/
                    else /*autrement ...*/
                        stop = hVRAI;
                }
            }
        }
        while( !(dirDispo[direction] || stop) );

        if( stop )
        {
            uRef = p;
            vRef = q;
            stop = hFAUX;

            continue; /*... on reprend au début*/
        }

        switch( direction ) /*succés pour le choix de la direction, mise à jour de la matrice*/
        {
        case 0 :
            LabyIJ[i+1][j] = 1; break;
        case 1 :
            LabyIJ[i][j-1] = 1; break;
        case 2 :
            LabyIJ[i-1][j] = 1; break;
        case 3 :
            LabyIJ[i][j+1] = 1;
        }
        NbVerticesSups--;

        if( LabyIJ[i][j] )
            break; /*c'est fini pour ce chemin car il a atteint un chemin précédent*/

        LabyIJ[uRef = i][vRef = j] = idChemin;
    }
}



/*=== création de la matrice-labyrinthe ======================================*/
void CreationLab( void )
{
    int* ligneLab;
    int  i ,j, k = 0;

    srand( time( NULL ) );
    NbVerticesSups = TailleLab*TailleLab;

    if( (LabyPtr = ligneLab = malloc( NbVerticesSups*sizeof( int ) )) ==NULL )
    {
        QUITTER;
    }
    if( (LabyIJ = malloc( TailleLab*sizeof( int* ) )) ==NULL )
    {
        free( LabyPtr );
        QUITTER;
    }

    for( i = TailleLab; --i >=0; ) /*on met la matrice à zéro*/
    {
        LabyIJ[i] = ligneLab;
        for( j = TailleLab; --j >=0; )
            *ligneLab++ = 0;
    }

    for( i = TailleLab; (i-=2) >0; ) /*on défini un chemin dans chaque partie du labyrinthe*/
        for( j = TailleLab; (j-=2) >0; )
        {
        NbVerticesSups--;
            if( !LabyIJ[i][j] )
            {
                LabyIJ[i][j] = ++k;
                DefinirChemin( i, j, k );
            }
        }

    Sortie = 0;
    k = 1+(HASARD( TailleLab>>1 )<<1); /*on défini la sortie du labyrinthe*/
    switch( HASARD( 4 ) )
    {
    case 0 :
        Sortie = TailleLab-1;
        *(LabyPtr+k) = 1;
        break;
    case 1 :
        Sortie = TailleLab-1;
        LabyIJ[k][TailleLab-1] = 1;
        break;
    case 2 :
        *(ligneLab-1-k) = 1;
        break;
    case 3 :
        LabyIJ[k][0] = 1;
    }

    GenererVerticesMurs(); /*ya plus qu'à transformer tout ça graphiquement*/
    NbVerticesSups--;
    NbVerticesSups<<=2;
    GenererVerticesSups();

    /*fic = fopen( "laby.txt", "w" );
    for( i = 0; i<TailleLab; i++ )
    {
        for( j = 0; j<TailleLab; j++ )
            if( LabyIJ[i][j] )
                fprintf( fic, "  " );
            else
                fprintf( fic, "[]" );

        fprintf( fic, "\n" );
    }
    fclose( fic );*/
}
