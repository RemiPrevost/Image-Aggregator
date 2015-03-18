#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>

#include "decrypt.h"


int main ( int argc, char** argv )
{
    char nom_fich[] = "images.myi";

    DEC_createImg(nom_fich);

    return 0;
}
