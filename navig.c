/*
navig.c
NAVIGATION 3D ET GESTION DES MOUVEMENTS
*/



/*=== externe ================================================================*/
#include <GL/gl.h>
#include <math.h>
#include <stdlib.h>
#include "hazlab.h"
#include "dimlab.h"
extern hbool Avance, Recule, Gauche, Droite; /*jeu.c*/
extern void  SortieTrouvee( void );
extern int   TailleLab;                      /*main.c*/
extern int** LabyIJ;
extern int   Sortie;                         /*crealab.c*/



/*=== global(partagé) ========================================================*/
GLfloat ValeurAltBulle;             /*coordonnée Y de la bulle flottante ...*/
GLfloat ValeurRayOmbre;             /*... rayon de son ombre en fonction de Y ...*/
GLfloat ValeurRotatY;               /*... valeur en degrés de l'angle de rotation /Y ...*/
GLfloat ValeurTransX, ValeurTransZ; /*... et coordonnées de déplacement à transmettre au rendu 3D*/



/*=== global(navig.c) ========================================================*/
#define VITESSE_TRANSLAT  0.075f  /*vitesse de déplacement, doit être inférieur à RAYON_BULLE (dimlab.h)*/

#define CONV_DR           0.017453f /*coeff de conversion DEG->RAD*/
#define EXTENSION_OMBRE  14.0f      /*coeff de variation du rayon de l'ombre de la bulle*/

#define NBRE_VALEURS_ROTAT  90   /*nombre de valeurs possibles (détermine la vitesse) pour l'angle de rotation /Y de la caméra*/
typedef struct agl               /*pour enregistrer les angles en liste chaînée*/
        {
        GLfloat     degres, sinus, cosinus;
        struct agl* prec, * suiv;
        }
    ANGLE;
static ANGLE* AngleY;            /*angle courant de rotation /Y*/
static hbool  Erreur = hFAUX;    /*vrai en cas d'erreur mémoire*/

#define NBRE_VALEURS_ALT  120    /*nombre de valeurs possibles (détermine la vitesse) pour l'altitude de la bulle*/
typedef struct alt               /*pour enregistrer les valeurs d'altitude en liste chaînée*/
        {
        GLfloat     valeur, autre;
        struct alt* suiv;
        }
    ALTI;
static ALTI*   AltitudeBulle;    /*coordonnée Y courante de la bulle*/
static hbool   Erreur2 = hFAUX;  /*vrai en cas d'erreur mémoire*/
static int     I, J;             /*coordonnées courantes dans la matrice LabyIJ*/



/*=== initialisation de la rotation de la caméra =============================*/
static void InitRotationCam( void )
{
    short   i = NBRE_VALEURS_ROTAT-1;
    ANGLE*  tPtrAgl[NBRE_VALEURS_ROTAT];
    GLfloat tmp;

    do /*on enregistre les valeurs mathématiques ...*/
    {
        if( (tPtrAgl[i] = malloc( sizeof( ANGLE ) )) ==NULL )
        {
            fprintf( stderr, hMSG_ERREUR_1 );
            Erreur = hVRAI;
            for( ++i ; i<NBRE_VALEURS_ROTAT; i++ )
                free( tPtrAgl[i] );
            exit( EXIT_FAILURE );
        }

        tPtrAgl[i]->degres  = i*360.0f/NBRE_VALEURS_ROTAT;
        tPtrAgl[i]->sinus   = VITESSE_TRANSLAT*sin( tmp = tPtrAgl[i]->degres*CONV_DR );
        tPtrAgl[i]->cosinus = VITESSE_TRANSLAT*cos( tmp );
    }
    while( --i >=0 );

    tPtrAgl[0]->suiv = tPtrAgl[1]; /*... puis on "accroche" les maillons*/
    tPtrAgl[0]->prec = tPtrAgl[NBRE_VALEURS_ROTAT-1];
    tPtrAgl[NBRE_VALEURS_ROTAT-1]->suiv = tPtrAgl[0];
    tPtrAgl[i = NBRE_VALEURS_ROTAT-1]->prec = tPtrAgl[NBRE_VALEURS_ROTAT-2];
    while( --i )
    {
        tPtrAgl[i]->suiv = tPtrAgl[i+1];
        tPtrAgl[i]->prec = tPtrAgl[i-1];
    }

    AngleY = *tPtrAgl;
}



/*=== initialisation du mouvement de la bulle ================================*/
static void InitMvtBulle( void )
{
    short   i = NBRE_VALEURS_ALT-1;
    GLfloat tmp;
    ALTI*   tPtrAlt[NBRE_VALEURS_ALT+1]; /*+1 pour éviter à la boucle ci-dessous d'aller lire en mémoire interdite ^^*/

    do
    {
        if( (tPtrAlt[i] = malloc( sizeof( ALTI ) )) ==NULL )
        {
            fprintf( stderr, hMSG_ERREUR_1 );
            Erreur2 = hVRAI;
            for( ++i ; i<NBRE_VALEURS_ALT; i++ )
                free( tPtrAlt[i] );
            exit( EXIT_FAILURE );
        }

        tmp = tPtrAlt[i]->valeur = ALT_O_BULLE + AMP_MVT_BULLE*sin( i*CONV_DR*360.0f/NBRE_VALEURS_ALT );
        tPtrAlt[i]->autre = RAYON_BULLE*1.3f + tmp/EXTENSION_OMBRE;
        tPtrAlt[i]->suiv = tPtrAlt[i+1];
    }
    while( --i >=0 );

    tPtrAlt[NBRE_VALEURS_ALT-1]->suiv = tPtrAlt[0]; /*on boucle sur le début*/

    AltitudeBulle = *tPtrAlt;
}



/*=== mise à jour de l'altitude de la bulle ==================================*/
void MAJAltBulle( void )
{
    AltitudeBulle = AltitudeBulle->suiv;
    ValeurAltBulle = AltitudeBulle->valeur;
    ValeurRayOmbre = AltitudeBulle->autre;
}



/*=== mise à jour des coordonnées ============================================*/
void MAJCoord( void )
{
    static GLfloat deplacement;
#ifndef CAMERA_LIBRE
    static int     ai, aj, ci, cj;
    static GLfloat di, dj, avtz, avtx;
    static hbool   ok;
#endif

    if( Gauche )
        AngleY = AngleY->prec;
    else
        if( Droite )
            AngleY = AngleY->suiv;
    ValeurRotatY = AngleY->degres;

    if( !(deplacement = Avance-Recule) )
        return;

#ifndef CAMERA_LIBRE
    avtz = ValeurTransZ;
    ValeurTransZ += deplacement*AngleY->cosinus;
    ai = I;
    I = floor( 0.5-ValeurTransZ );
    di = I+ValeurTransZ;
    ci = (((di<0)<<1)-1)*(fabs( di )>DIST_MAX);
    avtx = ValeurTransX;
    ValeurTransX -= deplacement*AngleY->sinus;
    aj = J;
    J = floor( 0.5-ValeurTransX );
    dj = J+ValeurTransX;
    cj = (((dj<0)<<1)-1)*(fabs( dj )>DIST_MAX);

    if( ai==Sortie || aj==Sortie )
        SortieTrouvee();

    if( !ci ^ !cj )
    {
        if( ci && !LabyIJ[I+ci][J] )
            ValeurTransZ = -I-DIST_MAX*ci;
        else
            if( !LabyIJ[I][J+cj] )
                ValeurTransX = -J-DIST_MAX*cj;
    }
    else
    {
        if( !cj )
            return;
        ok = hFAUX;
        if( !LabyIJ[I][J+cj] || I!=ai )
        {
            ValeurTransX = -J-DIST_MAX*cj;
            ok = hVRAI;
        }
        if( !LabyIJ[I+ci][J] || J!=aj )
        {
            ValeurTransZ = -I-DIST_MAX*ci;
            ok = hVRAI;
        }
        if( ok )
            return;
        if( ci*cj>0 )
        {
            if( 0.5f-avtx-aj > 0.5f-avtz-ai )
                if( ci>0 )
                    ValeurTransZ = -I-DIST_MAX*ci;
                else
                    ValeurTransX = -J-DIST_MAX*cj;
            else
                if( ci>0 )
                    ValeurTransX = -J-DIST_MAX*cj;
                else
                    ValeurTransZ = -I-DIST_MAX*ci;
        }
        else
        {
            if( 0.5f-avtz-ai > aj+0.5f-avtx )
                if( ci>0 )
                    ValeurTransX = -J-DIST_MAX*cj;
                else
                    ValeurTransZ = -I-DIST_MAX*ci;
            else
                if( ci>0 )
                    ValeurTransZ = -I-DIST_MAX*ci;
                else
                    ValeurTransX = -J-DIST_MAX*cj;
        }
    }
#else
    ValeurTransZ += deplacement*AngleY->cosinus;
    ValeurTransX -= deplacement*AngleY->sinus;
#endif
}



/*=== construction des données ===============================================*/
void InitNav( void )
{
    InitRotationCam();
    ValeurRotatY = 0.0f;
    ValeurTransX = ValeurTransZ = I = J = -TailleLab/2;

    InitMvtBulle();
    ValeurAltBulle = ALT_O_BULLE;
}



/*=== destruction des données ================================================*/
void TermNav( void )
{
    short  i;

    if( Erreur )
        return;
    else
    {
        ANGLE* a = AngleY, * b;
        for( i = 0; i<NBRE_VALEURS_ROTAT; i++ )
        {
            b = a->suiv;
            free( a );
            a = b;
        }
    }

    if( Erreur2 )
        return;
    else
    {
        ALTI* a = AltitudeBulle, * b;
        for( i = 0; i<NBRE_VALEURS_ALT; i++ )
        {
            b = a->suiv;
            free( a );
            a = b;
        }
    }
}
