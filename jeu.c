/*
jeu.c
FENETRE DE JEU
*/



/*=== externe ================================================================*/
#include <GL/glut.h>
#include <setjmp.h>
#include "hazlab.h"
#include "textures.h"
#include "dimlab.h"
extern GLuint   IdTextures[];                             /*textures.c*/
#ifdef UTIL_MIPMAPS
extern void     ChangerModeTexturage( void );
#endif
extern void     InitTextures( void );
extern hbool    LabyConstruit;                            /*main.c*/
extern int      TailleLab;
extern int**    LabyIJ;
extern void     MAJCoord( void ) ;                        /*navig.c*/
extern void     MAJAltBulle( void );
extern GLfloat  ValeurAltBulle, ValeurRayOmbre;
extern GLfloat  ValeurRotatY, ValeurTransX, ValeurTransZ;
extern int      NbVerticesMurs, NbVerticesSups;           /*crealab.c*/
extern GLfloat* SommetsPtrMurs, * CoordTxPtrMurs, * SommetsPtrSups, * CoordTxPtrSups;
extern int      Sortie;



/*=== global(partagé) ========================================================*/
hbool Avance, Recule, Gauche, Droite; /*traduisent les commandes de déplacement*/



/*=== global(jeu.c) ==========================================================*/
#define DELTA_SUPERPOSITION   0.001f /*espace entre deux surfaces normalement dans le même plan - évite des bugs visuels*/

static GLfloat const CouleurBulle[4] = {0.75f, 0.25f, 0.20f, 0.75f}; /*couleurs du jeu*/
static GLfloat       CouleurAmbiance[3];
#define MIN_LUM_FIN           0.400f  /*paramètres de la couleur("lumière") ambiante*/
#define PAS_LUM_FIN           0.005f
#define COEFF_AMB             1.00f
#define COULEUR_ARRIERE_PLAN  0.70f, 0.85f, 1.00f, 1.00f
#define COULEUR_OMBRE         0.00f, 0.00f, 0.00f, 0.15f /*ne pas toucher à alpha*/

#define TAILLE_X  1024                    /*largeur par défaut de la fenêtre*/
static GLdouble Aspect;                   /*rapport largeur/hauteur de l'écran*/
static short    PosFenetreX, PosFenetreY; /*position par défaut de la fenêtre*/
static short    ResolEcranX, ResolEcranY; /*résolution active de l'écran*/
static hbool    PleinEcran;               /*vrai si on est en plein écran*/

static jmp_buf FinJeu; /*pour pouvoir sortir de glutMainLoop() !!!*/

static GLUquadricObj* Bulle;            /*pour dessiner la bulle et son ombre*/
static GLfloat        MatriceGraph[16], MatriceGraph2[16]; /*la matrice MODELVIEW*/
static GLfloat        Tlab, Tlabm;      /*pour calculs de coordonnées*/

static GLfloat Recul_cam;        /*paramètres de placement caméra*/
static GLfloat Angle_cam;
static GLfloat Alti_cam;
static GLdouble Agl_perspective; /*angle d'ouverture de la perspective*/
static GLdouble Zmin;            /*valeurs de clipping*/
#define ZMAX          100.0



/*=== changement du point de vue =============================================*/
static void ChangerCamera( void )
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( Agl_perspective, Aspect, Zmin, ZMAX );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, Alti_cam, -Recul_cam );
    glRotatef( Angle_cam, 1.0f, 0.0f, 0.0f );
    glGetFloatv( GL_MODELVIEW_MATRIX, MatriceGraph );
}



/*=== affichage du rendu 3D, version "normale" ===============================*/
static void Afficher1( void )
{
    static GLfloat tmp, tmp2;

    glLoadMatrixf( MatriceGraph );
    glRotatef( ValeurRotatY, 0.0f, 1.0f, 0.0f );
    glTranslatef( ValeurTransX, 0.0f, ValeurTransZ );
    glColor3fv( CouleurAmbiance );

    glBindTexture( GL_TEXTURE_2D, ID_TEXTURE_SUP );
    glVertexPointer( 3, GL_FLOAT, 0, SommetsPtrSups );
    glTexCoordPointer( 2, GL_FLOAT, 0, CoordTxPtrSups );
    glDrawArrays( GL_QUADS, 0, NbVerticesSups );

    glBindTexture( GL_TEXTURE_2D, ID_TEXTURE_MUR );
    glVertexPointer( 3, GL_FLOAT, 0, SommetsPtrMurs );
    glTexCoordPointer( 2, GL_FLOAT, 0, CoordTxPtrMurs );
    glDrawArrays( GL_QUAD_STRIP, 0, NbVerticesMurs );

    glBindTexture( GL_TEXTURE_2D, ID_TEXTURE_SOL );
    glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, Tlab );
      glVertex3f( -0.5f, ALT_SOL, -0.5f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( -0.5f, ALT_SOL, Tlabm );
      glTexCoord2f( Tlab, 0.0f );
      glVertex3f( Tlabm, ALT_SOL, Tlabm );
      glTexCoord2f( Tlab, Tlab );
      glVertex3f( Tlabm ,ALT_SOL, -0.5f );
    glEnd();

    glEnable( GL_BLEND );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glEnable( GL_LIGHTING );
    glLoadMatrixf( MatriceGraph );
    glTranslatef( 0.0f, ValeurAltBulle, 0.0f );
    gluSphere( Bulle, RAYON_BULLE, GEOM_BULLE, GEOM_BULLE );
    glDisable( GL_LIGHTING );

    glLoadMatrixf( MatriceGraph2 );
    glColor4f( COULEUR_OMBRE );
    gluDisk( Bulle, 0.0, ValeurRayOmbre, GEOM_BULLE, 1 ); /*ombre douce, pour un minimum de réalisme tout de même ^^*/
    glColor4f( COULEUR_OMBRE-0.03f );
    gluDisk( Bulle, ValeurRayOmbre, tmp = ValeurRayOmbre*1.1f, GEOM_BULLE, 1 );
    glColor4f( COULEUR_OMBRE-0.06f );
    gluDisk( Bulle, tmp, tmp2 = ValeurRayOmbre*1.2f, GEOM_OMBRE_BULLE, 1 );
    glColor4f( COULEUR_OMBRE-0.09f );
    gluDisk( Bulle, tmp2, tmp = ValeurRayOmbre*1.3f, GEOM_OMBRE_BULLE, 1 );
    glColor4f( COULEUR_OMBRE-0.12f );
    gluDisk( Bulle, tmp, ValeurRayOmbre*1.4f, GEOM_OMBRE_BULLE, 1 );

    glDisable( GL_BLEND );

    glutSwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}



/*=== affichage du rendu 3D, version "aérien" ================================*/
static void Afficher3( void )
{
    glLoadMatrixf( MatriceGraph );
    glRotatef( ValeurRotatY, 0.0f, 1.0f, 0.0f );
    glTranslatef( ValeurTransX, 0.0f, ValeurTransZ );
    glColor3fv( CouleurAmbiance );

    glBindTexture( GL_TEXTURE_2D, ID_TEXT_SUP_BD );
    glVertexPointer( 3, GL_FLOAT, 0, SommetsPtrSups );
    glTexCoordPointer( 2, GL_FLOAT, 0, CoordTxPtrSups );
    glDrawArrays( GL_QUADS, 0, NbVerticesSups );

    glBindTexture( GL_TEXTURE_2D, ID_TEXT_MUR_BD );
    glVertexPointer( 3, GL_FLOAT, 0, SommetsPtrMurs );
    glTexCoordPointer( 2, GL_FLOAT, 0, CoordTxPtrMurs );
    glDrawArrays( GL_QUAD_STRIP, 0, NbVerticesMurs );

    glBindTexture( GL_TEXTURE_2D, ID_TEXT_SOL_BD );
    glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, Tlab );
      glVertex3f( -0.5f, ALT_SOL, -0.5f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( -0.5f, ALT_SOL, Tlabm );
      glTexCoord2f( Tlab, 0.0f );
      glVertex3f( Tlabm, ALT_SOL, Tlabm );
      glTexCoord2f( Tlab, Tlab );
      glVertex3f( Tlabm ,ALT_SOL, -0.5f );
    glEnd();

    glEnable( GL_BLEND );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glEnable( GL_LIGHTING );
    glLoadMatrixf( MatriceGraph );
    glTranslatef( 0.0f, ValeurAltBulle, 0.0f );
    gluSphere( Bulle, RAYON_BULLE, GEOM_BULLE, GEOM_BULLE );
    glDisable( GL_LIGHTING );
    glDisable( GL_BLEND );

    glutSwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}



/*=== affichage du rendu 3D, version "au sol" ================================*/
static void Afficher2( void )
{
    glLoadMatrixf( MatriceGraph );
    glRotatef( ValeurRotatY, 0.0f, 1.0f, 0.0f );
    glTranslatef( ValeurTransX, 0.0f, ValeurTransZ );
    glColor3fv( CouleurAmbiance );

    glBindTexture( GL_TEXTURE_2D, ID_TEXTURE_MUR );
    glVertexPointer( 3, GL_FLOAT, 0, SommetsPtrMurs );
    glTexCoordPointer( 2, GL_FLOAT, 0, CoordTxPtrMurs );
    glDrawArrays( GL_QUAD_STRIP, 0, NbVerticesMurs );

    glBindTexture( GL_TEXTURE_2D, ID_TEXT_SOL_HD );
    glBegin( GL_QUADS );
      glTexCoord2f( 0.0f, Tlab );
      glVertex3f( -0.5f, ALT_SOL, -0.5f );
      glTexCoord2f( 0.0f, 0.0f );
      glVertex3f( -0.5f, ALT_SOL, Tlabm );
      glTexCoord2f( Tlab, 0.0f );
      glVertex3f( Tlabm, ALT_SOL, Tlabm );
      glTexCoord2f( Tlab, Tlab );
      glVertex3f( Tlabm ,ALT_SOL, -0.5f );
    glEnd();

    glutSwapBuffers();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}



/*=== redimensionnement de la fenêtre ========================================*/
static void Redimensionner( int largeur, int hauteur )
{
    static int h = 0;

    if( !PleinEcran ) {
        if( hauteur!=h )
        {
            largeur = hauteur*Aspect;
            h = hauteur;
        }
        else
            hauteur = largeur/Aspect;
    }

    glutReshapeWindow( largeur, hauteur );
    glViewport( 0, 0, largeur, hauteur );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}



/*=== fonction au nom explicite ;-) ==========================================*/
static void SortirDuPleinEcran( void )
{
    glutSetCursor( GLUT_CURSOR_INHERIT );
    glutPositionWindow( PosFenetreX, PosFenetreY );
    Redimensionner( TAILLE_X, TAILLE_X/Aspect );
    glutPostRedisplay();
}



/*=== lorsque la sortie est atteinte =========================================*/
void SortieTrouvee( void )
{
    glutSpecialFunc( NULL );
    glutSpecialUpFunc( NULL );
    glutKeyboardFunc( NULL );
    Avance = Recule = Gauche = Droite = hFAUX;
    if( PleinEcran )
    {
        PleinEcran = hFAUX;
        SortirDuPleinEcran();
    }
    Sortie = -1;
}



/*=== réponse à l'action des touches GLUT du clavier =========================*/
static void GererActionTouchesGlut( int const touche, int const x, int const y )
{
    switch( touche )
    {
    case GLUT_KEY_UP :
        Avance = hVRAI; break;

    case GLUT_KEY_DOWN :
        Recule = hVRAI; break;

    case GLUT_KEY_LEFT :
        Gauche = hVRAI; break;

    case GLUT_KEY_RIGHT :
        Droite = hVRAI; break;

#ifdef UTIL_MIPMAPS
    case GLUT_KEY_F1 :
        ChangerModeTexturage();
        glutPostRedisplay();
        break;
#endif

    case GLUT_KEY_F5 :
        Recul_cam = 0.0f;
        Angle_cam = 10.0f;
        Alti_cam = 0.5f;
        Agl_perspective = 80.0;
        Zmin = 0.01;
        ChangerCamera();
        glutDisplayFunc( Afficher2 );
        break;

    case GLUT_KEY_F6 :
        Recul_cam = 2.0f; /*valeurs à recopier dans jeu()*/
        Angle_cam = 34.0f;
        Alti_cam = 0.0f;
        Agl_perspective = 54.0;
        Zmin = 0.5;
        ChangerCamera();
        glutDisplayFunc( Afficher1 );
        break;

    case GLUT_KEY_F7 :
        Recul_cam = 10.0f;
        Angle_cam = 40.0f;
        Alti_cam = 0.0f;
        Agl_perspective = 40.0;
        Zmin = 0.5;
        ChangerCamera();
        glutDisplayFunc( Afficher3 );
        break;

    case GLUT_KEY_F12 :
        if( (PleinEcran = !PleinEcran) )
        {
            glutSetCursor( GLUT_CURSOR_NONE );
            glutPositionWindow( 0, 0 );
            Redimensionner( ResolEcranX, ResolEcranY );
            glutPostRedisplay();
        }
        else
            SortirDuPleinEcran();
    }
}



/*=== réponse au relachement des touches GLUT du clavier =====================*/
static void GererRelachementTouchesGlut( int const touche, int const x, int const y )
{
    switch( touche )
    {
    case GLUT_KEY_UP :
        Avance = hFAUX; break;

    case GLUT_KEY_DOWN :
        Recule = hFAUX; break;

    case GLUT_KEY_LEFT :
        Gauche = hFAUX; break;

    case GLUT_KEY_RIGHT :
        Droite = hFAUX;
    }
}



/*=== réponse à l'action des touches non-GLUT du clavier =====================*/
static void GererActionTouches( huchar const touche, int const x, int const y )
{
    switch( touche )
    {
    case 27 : /*Echap*/
        if( PleinEcran )
        {
            PleinEcran = hFAUX;
            SortirDuPleinEcran();
        }
        else
            longjmp( FinJeu, 1 );
    }
}



/*=== mise à jour de l'affichage =============================================*/
static void RecalculerTout( int const p )
{
    if( Gauche!=Droite || Avance!=Recule )
        MAJCoord();
    MAJAltBulle();

    if( Sortie<0 ) /*sortie trouvée; "animation" de fin*/
    {
        *CouleurAmbiance = CouleurAmbiance[1] = CouleurAmbiance[2] -= PAS_LUM_FIN;
        if( *CouleurAmbiance<MIN_LUM_FIN )
        longjmp( FinJeu, 1 );
    }

    glutPostRedisplay();
    if( LabyConstruit )
        glutTimerFunc( 10, RecalculerTout, 1 );
}



/*=== création de la fenêtre de jeu ==========================================*/
void Jeu( void )
{
    int fenetreJeu;
    GLfloat const positionLum0[4] = {0.0f, 100.0f, 0.0f, 0.0f}; /*lumière directionnelle, uniquement pour éclairer la bulle*/

    Recul_cam = 2.0f; /*valeurs à recopier dans GererActionTouchesGlut()*/
    Angle_cam = 34.0f;
    Alti_cam = 0.0f;
    Agl_perspective = 54.0;
    Zmin =  0.5;

    Avance = Recule = Gauche = Droite = hFAUX;
    PleinEcran = hFAUX;

    CouleurAmbiance[0] = CouleurAmbiance[1] = CouleurAmbiance[2] = COEFF_AMB;

    ResolEcranX = /*glutGet( GLUT_SCREEN_WIDTH )*/1920;
    ResolEcranY = /*glutGet( GLUT_SCREEN_HEIGHT )*/1080;
    Aspect = (GLdouble)ResolEcranX/ResolEcranY;
    PosFenetreX = (ResolEcranX-TAILLE_X)/2;
    PosFenetreY = (ResolEcranY-TAILLE_X/Aspect)/2;

    glutInitWindowSize( TAILLE_X, TAILLE_X/Aspect );
    glutInitWindowPosition( PosFenetreX, PosFenetreY );
    fenetreJeu = glutCreateWindow( "HAZLAB 1.1 par Zareldyn" );

    ChangerCamera();

    glEnable( GL_LIGHT0 );
    glLightfv( GL_LIGHT0, GL_POSITION, positionLum0 );
    glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, CouleurBulle );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glTranslatef( 0.0f, ALT_SOL+DELTA_SUPERPOSITION, 0.0f );
    glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
    glGetFloatv( GL_MODELVIEW_MATRIX, MatriceGraph2 );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glClearColor( COULEUR_ARRIERE_PLAN );
    Tlab = TailleLab;
    Tlabm = Tlab-0.5f;
    InitTextures();
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    Bulle = gluNewQuadric();

    glutDisplayFunc( Afficher1 );
    glutReshapeFunc( Redimensionner );
    glutSpecialFunc( GererActionTouchesGlut );
    glutSpecialUpFunc( GererRelachementTouchesGlut );
    glutKeyboardFunc( GererActionTouches );
    glutTimerFunc( 50, RecalculerTout, 1 );

    if( setjmp( FinJeu )==0 )
        glutMainLoop();

    gluDeleteQuadric( Bulle );
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glutSpecialUpFunc( NULL );
    glutDestroyWindow( fenetreJeu );
}
