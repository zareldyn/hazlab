/*
hazlab.h
DECLARATIONS DIVERSES
*/

#ifndef _HAZLAB_
#define _HAZLAB_

/*bibliothèque standard*/
#include <stdio.h>
#include <stddef.h>

/*optimisations compilateur*/
/*
#ifdef __GNUC__
    #define hINLINE( fonction )  __attribute__((always_inline)) fonction
#elif defined(_MSC_VER)
    #define hINLINE( fonction )  __inline fonction
#else
    #define hINLINE( fonction )  fonction
#endif
*/
 #define hINLINE( fonction ) fonction

/*par souci de portabilité*/
enum {hLITTLE_ENDIAN, hBIG_ENDIAN};

/*messages d'erreurs*/
#define hMSG_ERREUR_1  "* Erreur: l'allocation memoire a echoue *\n"
#define hMSG_ERREUR_2  "* Erreur: le fichier \"%s\" est manquant *\n"
#define hMSG_ERREUR_3  "* Erreur: \"%s\" n'est pas au format BMP/DIB(Windows V3) 24 bits non compresse *\n"
#define hMSG_ERREUR_4  "* Erreur: \"%s\" -> au moins une dimension est trop grande ou trop petite *\n"
#define hMSG_ERREUR_5  "* Erreur: \"%s\" -> au moins une dimension n'est pas une puissance de 2 *\n"

/*types perso*/
typedef unsigned long hulong;
typedef unsigned char huchar;
typedef unsigned char hbool;
    enum {hFAUX, hVRAI};

/*autres, commenter ou décommenter si nécessaire*/
/*#define CAMERA_LIBRE*/ /*pour passer à travers les murs*/

#endif
