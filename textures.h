/*
textures.h
DECLARATIONS RELATIVES AUX TEXTURES
*/

#ifndef _HAZLAB_TEX_
#define _HAZLAB_TEX_

/*format "printf" des noms des fichiers-texture, doit contenir un %d, maximum 20 caractères*/
#define FORMAT_NOM_FIC  "textures/%d.bmp"

/*tailles extremum en pixels des textures*/
#define LIM_SUP_TEX  1024
#define LIM_INF_TEX     4

/*nombre de textures différentes utilisées*/
#define NBR_TEXTURES    7

/*liste des NBR_TEXTURES identifiants, modifier en conséquence Afficher() de jeu.c*/
#define ID_TEXTURE_SOL  IdTextures[0]
#define ID_TEXTURE_MUR  IdTextures[1]
#define ID_TEXTURE_SUP  IdTextures[2]
#define ID_TEXT_SOL_HD  IdTextures[3]
#define ID_TEXT_SOL_BD  IdTextures[4]
#define ID_TEXT_MUR_BD  IdTextures[5]
#define ID_TEXT_SUP_BD  IdTextures[6]

/*autres, commenter ou décommenter si nécessaire*/
#define UTIL_MIPMAPS /*pour utiliser les mipmaps*/

#endif
