/*
dimlab.h
DIMENSIONS ET POSITIONNEMENT DU LABYRINTHE ET DE LA BULLE FLOTTANTE
pour la vitesse des mouvements, cf le début de navig.c
pour les paramètres de la caméra, cf GererActionTouchesGlut() et Jeu() dans jeu.c
*/

#ifndef _HAZLAB_DIM_
#define _HAZLAB_DIM_

/*paramètres modifiables*/
#define COEFF_TAILLE  3           /*influe sur la taille de la matrice-labyrinthe*/
#define HTR_MUR       1.000f      /*hauteur des murs*/
#define ALT_SOL      -0.833f      /*altitude du sol (coordonnée Y)*/

#define GEOM_BULLE        24      /*paramètres de la bulle qui flotte au centre de la fenètre*/
#define GEOM_OMBRE_BULLE  12
#define RAYON_BULLE        0.100  /*doit être supérieur à VITESSE_TRANSLAT (navig.c)*/
#define ALT_O_BULLE        0.750f+ALT_SOL /*coordonnée Y d'origine*/
#define AMP_MVT_BULLE      0.125f /*coefficient d'amplitude du mouvement sinusoïdal*/

/*autres*/
#define ALT_MUR   ALT_SOL+HTR_MUR    /*coordonnée Y du haut des murs*/
#define DIST_MAX  (0.5-RAYON_BULLE)  /*pour délimiter les zones interdites à la bulle*/

#endif
