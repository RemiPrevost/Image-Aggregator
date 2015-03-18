/*
Format de stockage de information :
nom h w flags pitch refcount clip_rect.x clip_rect.y clip_rect.w clip_rect.h Rloss Gloss Bloss Aloss Rshift Gshift Bshift Ashift Rmask Gmask Bmask Amask colorkey alpha {*pixels}
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>

void pause();
FILE* open_file(char nom_img[]);
SDL_Surface* open_img(char nom_img[]);
int write_crypt(FILE* fich, SDL_Surface* img, char nom_imgClair[]);
Uint32 getpixel(SDL_Surface *surface, int x, int y);

int main ( int argc, char** argv )
{
    FILE* fich = NULL;
    char nom_imgCrypt[] = "images.myi"; //nom du fichier sous on enregistre les images
    char* nom_imgClair; //nom de l'image

    SDL_Surface* img = NULL;

    // Vérification des paramètres passés en argument.
    if (argc != 2)
    {
        fprintf(stderr,"Erreur de paramètres");
        return -1;
    }

    //
    nom_imgClair = (char*)malloc((strlen(argv[1])+1)*sizeof(char));
    strcpy(nom_imgClair,argv[1]);

    fich = open_file(nom_imgCrypt);
    if (fich == NULL)
        return -1;

    img = open_img(nom_imgClair);
    if (img == NULL)
        return -1;

    write_crypt(fich,img,nom_imgClair);

    return 0;
}

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;

    /* Here p is the address to the pixel we want to retrieve */
    Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;
    }
}

FILE* open_file(char nom_img[])
{
    FILE* img = NULL;

    img = fopen(nom_img,"a+");
    if (img == NULL)
        fprintf(stderr,"Impossible de creer le fichier %s\n",nom_img);

    return img;
}

SDL_Surface* open_img(char nom_img[])
{
    SDL_Surface* img = NULL;

    img = SDL_LoadBMP(nom_img);
    if (img == NULL)
        fprintf(stderr,"Impossible d'ouvrir l'image %s\n",nom_img);

    return img;
}

//retourne la place nécessaire dans un tableau pour stocker un nombre sous forme de texte
int puissance(int nb)
{
    int compteur = 0;
    int puiss = 1;
    int resultat;
    do
    {
        resultat = nb/puiss;
        puiss = puiss * 10;
        compteur++;
    } while (resultat != 0);

    return (compteur - 1);
}

//convertit un nombre en chaîne de caractères correctement dimenssionnée
char* intstr(int nb)
{
    char* str;

    str = (char*)malloc(32*sizeof(char));
    sprintf(str,"%d",nb);
    return str;
}

char* extract_name(char init[])
{
    char* name;
    int long_init = strlen(init);

    name = (char*)malloc((long_init + 1)*sizeof(char));
    strcpy(name,init);
    name[long_init-4]='\0';
    return name;
}

//écrit dans le fichier .myi les informations de l'image .bmp
int write_crypt(FILE* fich, SDL_Surface* img, char nom_imgClair[])
{
    SDL_PixelFormat* pixelF;
    int hauteur = img->h;
    int largeur = img->w;
    int x,y;
    char* h;
    char* w;
    char* nom_img;

    //Vérifie que l'image ne soit pas vide
    if (hauteur == 0 || largeur == 0)
    {
        fprintf(stderr,"Image vide\n");
        return -1;
    }

    //Vérifie que l'image soit en couleurs 8bits
    pixelF = img->format;
    if (pixelF->BitsPerPixel != 32)
    {
        fprintf(stderr,"Format couleur autre que 32 bits\n");
        return -1;
    }

    //écrit le nom de l'image
    nom_img = extract_name(nom_imgClair);
    fputs(nom_img,fich);
    fputc(' ',fich);

    //écrit la hauteur et la largeur de l'image
    h = intstr(hauteur);
    w = intstr(largeur);
    fputs(h,fich);
    fputc(' ',fich);
    fputs(w,fich);
    fputc(' ',fich);

    //écrit les autres paramètres
    fputs(intstr(img->flags),fich);
    fputc(' ',fich);
    fputs(intstr(img->pitch),fich);
    fputc(' ',fich);
    fputs(intstr(img->refcount),fich);
    fputc(' ',fich);
    fputs(intstr(img->clip_rect.x),fich);
    fputc(' ',fich);
    fputs(intstr(img->clip_rect.y),fich);
    fputc(' ',fich);
    fputs(intstr(img->clip_rect.w),fich);
    fputc(' ',fich);
    fputs(intstr(img->clip_rect.h),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->BitsPerPixel),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->BytesPerPixel),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Rloss),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Gloss),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Bloss),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Aloss),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Rshift),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Gshift),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Bshift),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Ashift),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Rmask),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Gmask),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Bmask),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->Amask),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->colorkey),fich);
    fputc(' ',fich);
    fputs(intstr(img->format->alpha),fich);
    fputc(' ',fich);

    //écrit les couleurs de l'image
    if (SDL_LockSurface(img) == 0)
    {
        for (y=0;y<hauteur;y++)
        {
            for (x=0;x<largeur;x++)
            {
                fputs(intstr(getpixel(img,x,y)),fich);
                fputc(' ',fich);
            }
        }
        SDL_UnlockSurface(img);

        //Insère une nouvelle ligne pour une nouvelle image
        fputs("\n",fich);
    }
    else
    {
        fprintf(stderr,"Echec du blocage de l'image\n");
        return -1;
    }

    return 0;
}

