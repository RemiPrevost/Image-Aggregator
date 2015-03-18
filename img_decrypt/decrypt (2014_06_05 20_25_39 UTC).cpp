#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <string.h>

#include "decrypt.h"

#define ERR fprintf(stderr,"ERREUR FROM DECRYPT : ");

//--------------------------------//
/////////////PRIVATE////////////////
//--------------------------------//

//Retourne le pointeur vers le fichier crypé en mode lecture seule
FILE* ouvre_fichier(char nom_crypt[])
{
    FILE* crypt = NULL;

    crypt = fopen(nom_crypt,"r");
    if (crypt == NULL)
    {
        ERR;
        fprintf(stderr,"fichier %s inexistant\n",nom_crypt);
    }

    return crypt;
}

//Créer le fichier qui sera modifié au cours des créations d'images
FILE* creer_temp()
{
    FILE* temp = NULL;

    temp = fopen("images_temp.myi","w+");
    if (temp == NULL)
    {
        ERR;
        fprintf(stderr,"Impossible de créer le fichier temporaire images");
    }

    return temp;

}

//Copie le contenu d'unfichier vers un autre
void copier_fichier(FILE* out, FILE* in)
{
    char caract;

    rewind(out);
    rewind(in);

    caract = fgetc(in);

    while (caract != -1)
    {
        fputc(caract,out);
        caract = fgetc(in);
    }
}

//retourne le nombre de caractères dans la première ligne du fichier
//pour pouvoir ultérieurement stoker la ligne dans un tableau
int nbCaract_ligne(FILE* fichier)
{
    int compteur = 0;
    char caract = 0;

    if (fichier == NULL)
    {
        ERR;
        fprintf(stderr,"paramètre NULL");
        return -1;
    }

    while (caract != '\n' && caract != -1)
    {
        caract=fgetc(fichier);
        compteur++;
    }

    rewind(fichier);

    return compteur + 1;
}

//Retrourne le nombre de caractères contenus dans le nom de l'image à créer
int nbCaract_nom(FILE* fichier)
{
    int compteur = 0;

    rewind(fichier);

    while (getc(fichier) != ' ')
        compteur ++;

    rewind(fichier);

    if (compteur == 0)
        return 0;
    else
        return compteur + 5;
}

//retourne le 1 si le fichier est vide, 0 sinon
int est_vide(FILE* fichier)
{
    if (fgetc(fichier) == -1)
    {
        rewind(fichier);
        return 1;
    }
    rewind(fichier);
    return 0;
}

//Supprime la première ligne du fichier
FILE* supp_ligne(FILE* fIN)
{
    FILE* fOUT;
    char caract;

    if (fIN == NULL)
    {
        ERR;
        fprintf(stderr,"fichier inexistant\n");
        return NULL;
    }

    //Création du fichier temp
    fOUT = fopen("images.tmp","w+");
    if (fOUT== NULL)
    {
        ERR;
        fprintf(stderr,"Impossible de créer le fichier image.tmp\n");
        return NULL;
    }

    if (est_vide(fIN))
    {
        ERR;
        fprintf(stderr,"Fichier vide\n");
        return NULL;
    }

    //On lit les caractères de la première ligne sans les traiter
    do
        caract = fgetc(fIN);
    while (caract != '\n' && caract != -1);

    caract = fgetc(fIN);

    //On écrit toutes les autres lignes dans le nouveau fichier
    while (caract != -1)
    {
        fputc(caract,fOUT);
        caract = fgetc(fIN);
    }

    //on ferme les deux fichiers pour pouvoir effectuer les renommages, suppressions
    fclose(fIN);
    fclose(fOUT);

    //On supprime l'ancien fichier pour pouvoir renommer le nouveau comme l'ancien
    if (remove("images_temp.myi") != 0)
    {
        ERR;
        fprintf(stderr,"Impossible de supprimer le fichier images_temp.myi\n");
        return NULL;
    }

    //On renomme le nouveau fichier comme l'ancien
    if (rename("images.tmp","images_temp.myi") != 0)
    {
        ERR;
        fprintf(stderr,"Impossible de renommer l'image temp\n");
        return NULL;
    }

    //Réouverture de images.myi en vue de nouvelles modifications
    fIN = fopen("images_temp.myi","r+");
    if (fIN == NULL)
    {
        ERR;
        fprintf(stderr,"Impossible de réouvrir l'image modifiée\n");
        return NULL;
    }

    return fIN;
}

//passe le nom de l'image qui n'est pas utile pour le codage
char* recup_nom(FILE* fichier)
{
    char* nom;
    char extension[] = ".bmp";
    char caract[2];
    int nb_caract = nbCaract_nom(fichier);

    if (nb_caract == 0)
    {
        nom = (char*)malloc(12*sizeof(char));
        strcpy(nom,"default.bmp");
        return nom;
    }

    nom = (char*)malloc(nb_caract*sizeof(char));

    nom[0] = 0;
    caract[1] = 0;

    caract[0] = fgetc(fichier);
    while(caract[0] != ' ')
    {
        strcat(nom,caract);
        caract[0] = fgetc(fichier);
    }

    strcat(nom,extension);

    return nom;
}

//recupérère le nombre contenu entre deux ' '
int decomp_info(FILE* fichier)
{
    char caract[2];
    char nombre[12];

    nombre[0]=0;
    caract[1]=0;

    caract[0] = fgetc(fichier);
    while(caract[0] != ' ')
    {
        strcat(nombre,caract);
        caract[0] = fgetc(fichier);
    }
    return strtol(nombre,NULL,10);
}

//met la valeur sauvegardé dans le pixel en position (x,y)
void PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

//Créer l'image bmp de la première ligne du fichier crypté. Retourne -1 si une erreur s'est produite, 0 sinon
int createImg(FILE* fichier)
{
    //Déclaration de la surface récupération du fichier crypté
    SDL_Surface* img = NULL;

    //Informations nécessaires à la création de l'image
    char* nom_img;
    Uint16 pitch, CR_w, CR_h;
    Sint16 CR_x, CR_y;
    int w, h, refcount,bitspp;
    Uint32 flags, Rmask, Gmask, Bmask, Amask, colorkey;
    Uint8 Rloss, Gloss, Bloss, Aloss;
    Uint8 Rshift, Gshift, Bshift, Ashift , alpha;

    int x,y;

    nom_img = recup_nom(fichier); //récupération du nom du fichier

    // recupération de toutes les métadonnées
    h=decomp_info(fichier);
    w=decomp_info(fichier);
    flags=(Uint32)decomp_info(fichier);
    pitch=(Uint16)decomp_info(fichier);
    refcount=decomp_info(fichier);
    CR_x=decomp_info(fichier);
    CR_y=decomp_info(fichier);
    CR_w=decomp_info(fichier);
    CR_h=decomp_info(fichier);
    bitspp=decomp_info(fichier);
    decomp_info(fichier); //passe sur le champ bytespp car inutile mais doit être sauté
    Rloss=(Uint8)decomp_info(fichier);
    Gloss=(Uint8)decomp_info(fichier);
    Bloss=(Uint8)decomp_info(fichier);
    Aloss=(Uint8)decomp_info(fichier);
    Rshift=(Uint8)decomp_info(fichier);
    Gshift=(Uint8)decomp_info(fichier);
    Bshift=(Uint8)decomp_info(fichier);
    Ashift=(Uint8)decomp_info(fichier);
    Rmask=(Uint32)decomp_info(fichier);
    Gmask=(Uint32)decomp_info(fichier);
    Bmask=(Uint32)decomp_info(fichier);
    Amask=(Uint32)decomp_info(fichier);
    colorkey=(Uint32)decomp_info(fichier);
    alpha=(Uint8)decomp_info(fichier);

    //Création d'une surface vide
    img = SDL_CreateRGBSurface(flags,w,h,bitspp,Rmask,Gmask,Bmask,Amask);
    if (img == NULL)
    {
        ERR;
        fprintf(stderr,"Impossible de créer la surface vide");
        return -1;
    }

    //insertion des métadonnées non insérées par la création de la surface
    img->format->palette = NULL;
    img->format->colorkey = colorkey;
    img->format->alpha = alpha;
    img->format->Rloss = Rloss;
    img->format->Gloss = Gloss;
    img->format->Bloss = Bloss;
    img->format->Aloss = Aloss;
    img->format->Rshift = Rshift;
    img->format->Gshift = Gshift;
    img->format->Bshift = Bshift;
    img->format->Ashift = Ashift;

    img->pitch = pitch;
    img->refcount = refcount;
    img->clip_rect.x = CR_x;
    img->clip_rect.y = CR_y;
    img->clip_rect.h = CR_h;
    img->clip_rect.w = CR_w;

    //remplissage des couleurs
    SDL_LockSurface(img);
    for (y=0;y<h;y++)
    {
        for (x=0;x<w;x++)
        {
            PutPixel(img,x,y,(Uint32)decomp_info(fichier));
        }
    }
    SDL_UnlockSurface(img);

    return SDL_SaveBMP(img,nom_img);
}
//--------------------------------//
/////////////PUBLIC/////////////////
//--------------------------------//

int DEC_createImg(char nom_fichier[])
{
    FILE* img_crypt;
    FILE* temp;

    char caract;

    // Ouverture images.myi
    img_crypt = ouvre_fichier(nom_fichier); //récupération du fichier crypté
    if (img_crypt == NULL) //vérifie que l'ouverture du fichier crypté a bien fonctionné
        return -1;

    //création du fichier temp
    temp = creer_temp();
    if (temp == NULL) //vérifie que la création du temp a bien fonctionné
        return -1;

    //copie du fichier images.myi vers images_temps.myi
    copier_fichier(temp, img_crypt);

    //fermeture du fichier images.myi
    fclose(img_crypt);


    //on boucle tant qu'il y a des images à générer
    rewind(temp);
    caract = fgetc(temp);
    while (caract != -1 || caract == ' ')
    {
        if (createImg(temp) == -1)
            return -1;

        rewind(temp);
        temp = supp_ligne(temp);
        if (temp == NULL)
            return -1;

        rewind(temp);
        caract = fgetc(temp);
    }

    fclose(temp); //fermeture pour suppression

    remove("images_temp.myi");

    return 0;
}

