/*
main.c
PROGRAMME PRINCIPAL ET FONCTIONS UTILES
*/



/*=== externe ================================================================*/
#include <stdlib.h>
#include <limits.h>
#include <GL/glut.h>
#include "hazlab.h"
#include "textures.h"
#include "dimlab.h"
extern char     Menu( void );         /*menu.c*/
extern void     CreationLab( void );  /*crealab.c*/
extern GLfloat* SommetsPtrMurs, * CoordTxPtrMurs, * SommetsPtrSups, * CoordTxPtrSups;
extern void     Jeu( void );          /*jeu.c*/
extern void     InitNav( void );      /*navig.c*/
extern void     TermNav( void );
extern void     TermTextures( char ); /*textures.c*/



/*=== global(partagé) ========================================================*/
int*  LabyPtr;       /*début de la zone mémoire contenant la matrice-labyrinthe*/
int** LabyIJ;        /*pour utiliser cette matrice sous forme LabyIJ[i][j]*/
int   TailleLab;     /*taille (de la matrice) d'un coté du labyrinthe*/
hbool LabyConstruit; /*indique l'existence de la matrice*/

char Codage = hLITTLE_ENDIAN; /*par défaut destiné à x86 ;-)*/
char TailleLong = 1;          /*taille (calcul ultérieur de la bonne valeur) en octets d'un entier de type "long"*/
char TailleChar = CHAR_BIT/8; /*idem pour un type "char"*/



/*=== échange de la valeur de deux GLbytes ===================================*/
void EchangerGLbyte( GLubyte* const a, GLubyte* const b )
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}



/*=== comptage des bits égaux à 1 dans un entier de type "long" ==============*/
huchar CompterBits1( hulong entier )
{
    huchar cpt = 0;

    while( entier )
    {
        cpt++;
        entier &= (entier-1);
    }
    return cpt;
}



/*=== inversion des n octets d'un entier de type "long" ======================*/
void InverserLong( hulong* const entier, char const n )
{
    char   i = -1;
    hulong masque = -1l, resultat = 0l;

    masque = ~(masque>>8); /*bits du premier octet à 1, le reste à 0*/
    while( ++i <n )
    {
        resultat |= ((*entier & masque)>>((TailleLong-1-i)<<3)) << (i<<3); /*on sélectionne le bon octet grâce au masque et on l'ajoute à la bonne place au résultat*/
        masque >>= 8; /*on décale le masque sur l'octet suivant*/
    }
    *entier = resultat;
}



/*=== vérification du codage interne des entiers =============================*/
hINLINE( static void VerifierCodage( void ) )
{
    hulong n = -2l; /*dernier bit à 0*/
    char   p = *(char*)&n;

    if( p==-1 )
        Codage = hBIG_ENDIAN;

    n = -1l; /*tous les bits à 1*/
    while( n >>= 8 ) TailleLong++;
}



/*=== destruction du labyrinthe ==============================================*/
static void LibererMemoire( void )
{
    if( LabyConstruit )
    {
        free( LabyPtr );
        free( LabyIJ );
        free( SommetsPtrMurs );
        free( CoordTxPtrMurs );
        free( SommetsPtrSups );
        free( CoordTxPtrSups );
        TermNav();
        TermTextures( NBR_TEXTURES );
    }
}



/*=== point d'entrée =========================================================*/
int main( int nbreArgs, char* listArgs[] )
{
    VerifierCodage();
    atexit( LibererMemoire );

    glutInit( &nbreArgs, listArgs );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

    while( (TailleLab = Menu()) !=0 )
    {
        TailleLab = TailleLab*4*COEFF_TAILLE-1;
        CreationLab();
        LabyConstruit = hVRAI;

        InitNav();
        Jeu();

        LibererMemoire();
        LabyConstruit = hFAUX;
    }

    return EXIT_SUCCESS;
}
