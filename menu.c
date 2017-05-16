/*
menu.c
FENETRE DU MENU D'ENTREE (CHOIX DE LA TAILLE DU LABYRINTHE)
*/



/*=== externe ================================================================*/
#include <GL/glut.h>
#include <setjmp.h>
#include "hazlab.h"



/*=== global(menu.c)==========================================================*/
static char Choix = 3; /*(Choix-1) indexe Niveaux[]*/
#define MAX_CHOIX   5  /*tailles différentes proposées, modifier Niveaux[] en conséquence*/
static char const* const Niveaux[] = {"MINUSCULE", "PETIT", "MOYEN", "GRAND", "GIGANTESQUE"};

#define COULEUR_ARRIERE_PLAN   0.8f, 0.9f, 1.0f, 1.0f /*dimensions et couleurs de la fenêtre*/
#define COULEUR_RECTANGLE      0.5f, 0.6f, 0.7f
#define COULEUR_POLICE         0.9f, 0.9f, 0.9f
#define COULEUR_CADRE_SELECT   0.0f, 0.0f+Clic, 0.0f+Clic
#define COULEUR_POLICE_SELECT  0.0f, 0.0f+Clic, 0.0f+Clic
#define TAILLE_X               214
#define TAILLE_Y               60*MAX_CHOIX

static hbool PosCursOK; /*concernent la gestion de la souris*/
static hbool Clic;

static jmp_buf FinMenu; /*pour pouvoir sortir de glutMainLoop() !!!*/



/*=== en cas de redimensionnement de la fenêtre ==============================*/
static void Redimensionner( int const largeur, int const hauteur )
{
    glutReshapeWindow( TAILLE_X, TAILLE_Y );
}



/*=== affichage du menu ======================================================*/
static void Afficher( void )
{
    int i, j, k;

    glClear( GL_COLOR_BUFFER_BIT );

    for( i = 0; i<MAX_CHOIX; i++ )
    {
        k = 60*MAX_CHOIX-56-60*i;

        glColor3f( COULEUR_RECTANGLE );
        glRecti( 4, k, TAILLE_X-4, k+52 );

        glColor3f( COULEUR_POLICE );
        if( Choix==i+1 )
        {
            glColor3f( COULEUR_CADRE_SELECT );
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            glRecti( 4, k, TAILLE_X-4, k+52 );
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

            glColor3f( COULEUR_POLICE_SELECT );
        }
        glRasterPos2i( 20, k+16 );
        for( j = 0; Niveaux[i][j]; j++ )
            glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, Niveaux[i][j] );
    }

    glutSwapBuffers();
}



/*=== mise à jour du choix par les touches directionnelles haut et bas =======*/
static void MAJChoixClavier( int const touche, int const x, int const y )
{
    if( Clic )
        return;

    switch( touche )
    {
    case GLUT_KEY_DOWN :
        if( ++Choix ==MAX_CHOIX+1 )
            Choix = 1;
        break;

    case GLUT_KEY_UP :
        if( ! --Choix )
            Choix = MAX_CHOIX;
        break;

    default :
        return;
    }

    glutPostRedisplay();
}



/*=== validation du choix au clavier =========================================*/
static void ValiderChoixClavier( huchar const touche, int const x, int const y )
{
    if( Clic )
        return;

    switch( touche )
    {
    case 27 : /*Echap*/
        Choix = 0;
    case 13 : /*Entrée*/
        break;
    default :
        return;
    }

    longjmp( FinMenu, 1 );
}



/*=== mise à jour du choix par la position du curseur de la souris ===========*/
static void MAJChoixSouris( int const x, int const y )
{
    if( (PosCursOK = (x>0 && x<TAILLE_X && y>0 && y<TAILLE_Y)) )
    {
        char t = Choix;

        Choix = y/60+1;
        if( t!=Choix )
            glutPostRedisplay();
    }
}



/*=== validation du choix à la souris ========================================*/
static void ValiderChoixSouris( int const bouton, int const etat, int const x, int const y )
{
    if( bouton!=GLUT_LEFT_BUTTON )
        return;

    if( etat==GLUT_UP )
        if( PosCursOK )
            longjmp( FinMenu, 1 );
        else
            Clic = hFAUX;

    else
        Clic = hVRAI;

    glutPostRedisplay();
}



/*=== création de la fenêtre de menu =========================================*/
char Menu( void )
{
    int   fenetreMenu;
    short resolEcranX = glutGet( GLUT_SCREEN_WIDTH );
    short resolEcranY = glutGet( GLUT_SCREEN_HEIGHT );

    glutInitWindowSize( TAILLE_X, TAILLE_Y );
    glutInitWindowPosition( (resolEcranX-TAILLE_X)/2, (resolEcranY-TAILLE_Y)/2 );
    fenetreMenu = glutCreateWindow( "HAZLAB" );

    glClearColor( COULEUR_ARRIERE_PLAN );
    glMatrixMode( GL_PROJECTION );
    gluOrtho2D( 0.0, TAILLE_X, 0.0, TAILLE_Y);
    Clic = hFAUX;

    glutReshapeFunc( Redimensionner );
    glutDisplayFunc( Afficher );
    glutSpecialFunc( MAJChoixClavier );
    glutKeyboardFunc( ValiderChoixClavier );
    glutMotionFunc( MAJChoixSouris );
    glutPassiveMotionFunc( MAJChoixSouris );
    glutMouseFunc( ValiderChoixSouris );

    if( setjmp( FinMenu )==0 )
        glutMainLoop();

    glutMotionFunc( NULL );
    glutPassiveMotionFunc( NULL );
    glutMouseFunc( NULL );
    glutDestroyWindow( fenetreMenu );
    return Choix;
}
