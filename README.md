# Hazlab
A 3D random maze game (old project)

![Hazlab screenshot](/screenshot.jpg?raw=true)

This is an old, somewhat bad-coded game I made in 2009 that reached a version "1.1".  
I share it just for fun.  
The original purpose of this was to be a **C89<sup>1</sup> / OpenGL** training. Nevertheless, I'm still quite satisfied with how it renders on screen.

## The code

It uses GLUT, a deprecated library for OpenGL window handling.  
I recently recompiled it easily on Linux, although it was originally developed under Windows; this means that the code is not so bad, finally ;-)

Sorry for non-French speakers, but the functions, variables and comments are in French.  
Also, as I made efforts (but with errors!) for it can be compiled strictly under any C89-compliant system, some functions could appear to be useless.

Just use the Makefile to compile it under Linux. You will need first to have GL / GLU / GLUT libraries installed on your system.

<sup>1</sup>: As European, it is ISO C90 for me ;-)

## The game itself

First, let's explain the name: "Haz" is for hasard (the French for random), with a Z because my pseudo is **z**areldyn. And "lab" is for Labyrinthe (maze).

The program launches a graphical menu which lets you choose the maze size (again, it is French text).  
It generates a random perfect maze according to the size you entered, and then you enter the game.

At the beginning you are at the exact center of the maze. Of course, the exit is on one of the edges.

KEYS:
- arrows to move through the maze
- F1 disables/re-enables the use of MIPMAPS
- F5/F6/F7 change the point of view: F5 is for first-person view, F6 is the defaut view, F7 can help on very large mazes
- F12 maximizes/restores the window
- ESC returns to menu or exits the program

Note: On Linux it seems a "segmentation fault" is thrown on exiting game window to return to the menu window.
