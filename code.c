#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#define PI 3.14159265358979323

// Structures principales

struct pixel_s{
    int red;
    int green;
    int blue;
};

typedef struct pixel_s pixel;

struct image_s{
    int largeur;
    int hauteur;
    int max;
    pixel** matrice;
};

typedef struct image_s image;

struct grp_image_s{
    int nb_grp;
    int largeur_image;
    int hauteur_image;
    int nb_hauteur;
    int nb_largeur;
    image** groupe;
    int* valide;
};

typedef struct grp_image_s grp_image;

// Structure outil

struct matrice_rayon_s{
    int hauteur;
    int largeur;
    int rayon_min;
    int rayon_max;
    int pas;
    int longueur_tab;
    int*** matrice_3d;
};

typedef struct matrice_rayon_s matrice_rayon;


// Fonction qui permet la création d'une structure image à partir d'une image en format PPM
image* create_img(char* nom_fichier){
    FILE* fichier = fopen(nom_fichier, "r");
    assert(fichier != NULL);

    int l,h,m;
    fscanf(fichier, "P3 %d %d %d", &l, &h, &m);
    image* img = (image*)malloc(sizeof(image));
    assert(img != NULL);
    img->largeur = l;
    img->hauteur = h;
    img->max = m;
    img->matrice = (pixel**)(malloc(sizeof(pixel*) * h));
    assert(img->matrice != NULL); 

    for(int i = 0; i < h; i++){
        img->matrice[i] = (pixel*)(malloc(sizeof(pixel)* l));
        assert(img->matrice[i] != NULL);
        for(int j = 0; j < l; j++){
            int r,g,b;
            fscanf(fichier, "%d %d %d", &r, &g, &b);
            img->matrice[i][j].red = r;
            img->matrice[i][j].green = g;
            img->matrice[i][j].blue = b;
        }
    }
    fclose(fichier);
    return img;
}


image* create_img_vierge(int h, int l){
    image* img = (image*)malloc(sizeof(image));
    assert(img != NULL);
    img->hauteur = h; 
    img->largeur = l;
    img->matrice = (pixel**)(malloc(sizeof(pixel*) * h));
    assert(img->matrice != NULL);
    for(int i = 0; i < h; i++){
        img->matrice[i] = (pixel*)(malloc(sizeof(pixel) * l));
        assert(img->matrice[i] != NULL);
    }
    return img;
}

// Fonction qui permet de libérer une structure image
void free_image(image* img){
    if(img != NULL){
        for(int i = 0; i < img->hauteur; i++){
            if(img->matrice[i] != NULL){
                free(img->matrice[i]);
            }
        }

        free(img->matrice);
    }
    free(img);
}


// Fonction qui enregistre une image_s dans un fichier
void save_image(image* img, char* nom_fichier){
    FILE* fichier = fopen(nom_fichier, "w" );
    assert(fichier != NULL);
    assert(img != NULL);
    fprintf(fichier, "P3\n%d %d\n", img->largeur, img->hauteur);
    fprintf(fichier, "%d\n", img->max);
    for(int i = 0; i < img->hauteur; i++){
        for(int j = 0; j < img->largeur; j++){
            fprintf(fichier, "%d %d %d ", img->matrice[i][j].red, img->matrice[i][j].green, img->matrice[i][j].blue);
        }
        fprintf(fichier,"\n");
    }

    fclose(fichier);
}

// Fonction qui isole une couleur de l'image passée en paramètre (rgb), retourne une nouvelle image
image* isolement_couleur(image* img_originale, int rmin, int rmax, int gmin, int gmax, int bmin, int bmax, bool fond_w){
    assert(img_originale != NULL);

    // Création de la nouvelle image
    image * img_vide = create_img_vierge(img_originale->hauteur, img_originale->largeur);
    assert (img_vide != NULL);

    assert(rmin <= rmax && gmin <= gmax && bmin <= bmax);
    int max = img_originale->max;
    assert(rmin >= 0 && gmin >= 0 && bmin >= 0 && rmax <= max && bmax <= max && gmax <= max );

    //Remplissage de la nouvelle matrice
    img_vide->max = img_originale->max;
    assert(img_originale->matrice != NULL); assert(img_vide->matrice != NULL);

    pixel b_or_w;

    if(fond_w){
        b_or_w.red = 255; b_or_w.blue = 255; b_or_w.green = 255;
    } else {
        b_or_w.red = 0; b_or_w.blue = 0; b_or_w.green = 0;
    }

    for(int i = 0; i < img_vide->hauteur; i++){
        for(int j = 0; j < img_vide->largeur; j++){
            pixel pix = img_originale->matrice[i][j];
            if (pix.red > rmax || pix.red < rmin || pix.blue > bmax || pix.blue < bmin || pix.green > gmax || pix.green < gmin){
                img_vide->matrice[i][j].red =  b_or_w.red;
                img_vide->matrice[i][j].green =  b_or_w.green;
                img_vide->matrice[i][j].blue =  b_or_w.blue;
            } else {
                img_vide->matrice[i][j] = pix;
            }
        }
    }  
    return img_vide;
}


// Découpe une image et renvoie un groupe
grp_image* create_groupe(int l, int h, image* img){
    
    assert(img != NULL);

    // Création du groupe
    grp_image* grp = (grp_image*)malloc(sizeof(grp_image));
    assert(grp != NULL);
    grp->hauteur_image = img->hauteur;
    grp->largeur_image = img->largeur;

    // Découpage théorique de l'image
    int un_de_plus_l = 0; 
    int un_de_plus_h = 0;

    if (img->largeur % l){
        grp->nb_grp = (img->largeur / l + 1);
        un_de_plus_l++;
        if(img->hauteur % h){
            grp->nb_grp = (grp->nb_grp) * (img->hauteur / h + 1);
            un_de_plus_h++;
        } else {
            grp->nb_grp = (grp->nb_grp) * (img->hauteur / h);
        }
    } else {
        grp->nb_grp = img->largeur / l;
        if(img->hauteur % h){
            grp->nb_grp = (grp->nb_grp) * (img->hauteur / h + 1);
            un_de_plus_h++;
        } else {
            grp->nb_grp = (grp->nb_grp) * (img->hauteur / h);
        }
    }

    grp->nb_hauteur = img->hauteur / h + un_de_plus_h;
    grp->nb_largeur = img->largeur / l + un_de_plus_l;

    // Allocation de l'espace pour le groupe
    grp->groupe = (image**)malloc(sizeof(image*) * grp->nb_grp);
    assert(grp->groupe != NULL);
    
    for(int i = 0; i < grp->nb_grp; i++){
        if(un_de_plus_l && !((i + 1) % grp->nb_largeur)){ // Dépassement en largeur
            if(un_de_plus_h && (i >= grp->nb_grp - 1 - img->largeur / l )){ // Dans le dépassement en hauteur
                grp->groupe[i] = create_img_vierge(img->hauteur % h, img->largeur % l);
                assert(grp->groupe[i] != NULL);
                grp->groupe[i]->max = img->max;
                
            } else { // Pas dans le dépassement en hauteur 
                grp->groupe[i] = create_img_vierge(h, img->largeur % l);
                assert(grp->groupe[i] != NULL);
                grp->groupe[i]->max = img->max;
            }
        } else { // Pas dans le dépassement en largeur
            if(un_de_plus_h && (i > grp->nb_grp - 1 - grp->nb_largeur )){ // Dans le dépassement en hauteur
                grp->groupe[i] = create_img_vierge(img->hauteur % h, l);
                assert(grp->groupe[i] != NULL);
                grp->groupe[i]->max = img->max;
            } else { // Pas dans le dépassement en hauteur 
                grp->groupe[i] = create_img_vierge(h, l);
                assert(grp->groupe[i] != NULL);
                grp->groupe[i]->max = img->max;
            }
        }
    }

    // Remplissage du groupe 
    for(int w = 0; w < grp->nb_grp; w++){
        for(int i = 0; i < grp->groupe[w]->hauteur; i++){
            for(int j = 0; j < grp->groupe[w]->largeur; j++){
                int un = i + (w / (grp->nb_largeur)) * h;
                int deux = j + (w % (grp->nb_largeur)) * l;
                assert(un < img->hauteur);
                assert(deux < img->largeur);
                grp->groupe[w]->matrice[i][j] = img->matrice[un][deux];
            }
        }
    }

    // Tableau valide
    grp->valide = (int*)malloc(sizeof(int) * grp->nb_grp);
    assert(grp->valide != NULL);
    for(int i = 0; i < grp->nb_grp; i++){
        grp->valide[i] = 1;
    }

    return grp;
}

// Fonction qui libère un groupe
void free_groupe(grp_image* grp){
    assert(grp != NULL);
    assert(grp->groupe != NULL);
    for(int w = 0; w < grp->nb_grp; w++){
        free_image(grp->groupe[w]);
    }
    free(grp->groupe);
    free(grp->valide);
    free(grp);
}

// Fonction qui convertit un groupe d'image en une unique image
image* image_from_groupe(grp_image* grp){
    // Création image
    assert(grp != NULL);
    assert(grp->groupe != NULL);
    image* img = create_img_vierge(grp->hauteur_image, grp->largeur_image);
    assert(img != NULL);
    img->max = grp->groupe[0]->max;

    pixel pix;
    pix.red = 0; pix.blue = 0; pix.green = 0;

    pixel pix1;
    pix1.red = 0; pix1.blue = 255; pix1.green = 0;

    // Remplissage
    for(int w = 0; w < grp->nb_grp; w++){
        for(int i = 0; i < grp->groupe[w]->hauteur; i++){
            for(int j = 0; j < grp->groupe[w]->largeur; j++){
                int premier = i + (w / (grp->nb_largeur)) * grp->groupe[0]->hauteur;
                int deux = j + (w % (grp->nb_largeur)) * grp->groupe[0]->largeur;
                assert(premier < img->hauteur); 
                assert(deux < img->largeur);
                if(grp->valide[w] > 0){
                    img->matrice[premier][deux] = grp->groupe[w]->matrice[i][j];
                } else {
                    if(grp->valide[w] == 0){
                        img->matrice[premier][deux] = pix;
                    } else {
                        img->matrice[premier][deux] = pix1;
                    }
                }       
            }
        }
    }

    return img;
}

// Fonction qui passe à false les images du groupe qui ne possède pas assez d'informations 
// Prend en paramètre un booléen pour savoir si le fond de l'image est noir ou blanc
grp_image* suppression(grp_image* grp, bool blanc){
    assert(grp != NULL);
    assert(grp->groupe != NULL);
    if(blanc){
        for(int w = 0; w < grp->nb_grp; w++){
            int compt = 0;
            for(int i = 0; i < grp->groupe[w]->hauteur; i++){
                for(int j = 0; j < grp->groupe[w]->largeur; j++){
                    if(grp->groupe[w]->matrice[i][j].red <= 230 || grp->groupe[w]->matrice[i][j].blue <= 230 || grp->groupe[w]->matrice[i][j].green <= 230){
                        compt++;
                    }
                }
            }

            if(compt < grp->groupe[w]->largeur){
                grp->valide[w] = 0;
            }
        }
    } else {
        for(int w = 0; w < grp->nb_grp; w++){
            int compt = 0;
            for(int i = 0; i < grp->groupe[w]->hauteur; i++){
                for(int j = 0; j < grp->groupe[w]->largeur; j++){
                    if(grp->groupe[w]->matrice[i][j].red >= 25 || grp->groupe[w]->matrice[i][j].blue >= 25 || grp->groupe[w]->matrice[i][j].green >= 25){
                        compt++;
                    }
                }
            }

            if(compt < grp->groupe[w]->largeur){
                grp->valide[w] = -1;
            }
        }
    }

    return grp;
}

// Tri selection
void tri_selection(int* tab, int taille){
    int debut = 0;
    while(debut < taille - 1){
        int j = debut;
        for(int i = debut; i < taille; i++){
            if(tab[j] > tab[i]){
                j = i;
            }
        }
        int c = tab[debut];
        tab[debut] = tab[j];
        tab[j] = c;
        debut++;
    }
}


// Fonction qui calcule la médiane d'un pixel à partir de ses huits voisins, renvoie un pixel
pixel mediane_pix(image* img, int x, int y){
    int nb_pixels = 0;
    for(int i = x - 1; i < x + 2; i++){
        for(int j = y - 1; j < y + 2; j++){
            if((i >= 0) && (i < img->hauteur) && (j >= 0) && (j < img->largeur)){
                nb_pixels++;
            }
        }
    }

    int* red = (int*)malloc(sizeof(int) * nb_pixels);
    int* blue = (int*)malloc(sizeof(int) * nb_pixels);
    int* green = (int*)malloc(sizeof(int) * nb_pixels);
    assert(red != NULL && blue != NULL && green != NULL);

    // Remplissage des tableaux de médiane
    int indice = 0;
    for(int i = x - 1; i < x + 2; i++){ 
        for(int j = y - 1; j < y + 2; j++){
            if((i >= 0) && (i < img->hauteur) && (j >= 0) && (j < img->largeur)){
                red[indice] = img->matrice[i][j].red;
                blue[indice] = img->matrice[i][j].blue;
                green[indice] = img->matrice[i][j].green;
                indice++;
            }
        }
    }

    // Tri
    tri_selection(red, nb_pixels);
    tri_selection(blue, nb_pixels);
    tri_selection(green, nb_pixels);

    // Sauvegarde
    pixel pix;
    if(nb_pixels % 2){
        pix.red = red[nb_pixels / 2];
        pix.blue = blue[nb_pixels / 2];
        pix.green = green[nb_pixels / 2];
    } else {
        pix.red = (red[nb_pixels / 2] + red[nb_pixels / 2 - 1]) / 2;
        pix.blue = (blue[nb_pixels / 2] + blue[nb_pixels / 2 - 1]) / 2;
        pix.green = (green[nb_pixels / 2] + green[nb_pixels / 2 - 1]) / 2;
    }
    
    free(red);
    free(green);
    free(blue);

    return pix;
} 

// Fonction qui applique un filtre médian à une image
image* filtre_median(image* img){
    assert(img != NULL);
    assert(img->matrice != NULL);

    image* new_image = create_img_vierge(img->hauteur, img->largeur);
    assert(new_image != NULL);

    new_image->max = 255;

    for(int i = 0; i < img->hauteur; i++){
        for(int j = 0; j < img->largeur; j++){
            new_image->matrice[i][j] = mediane_pix(img, i, j);
        }
    }

    return new_image;
}

// Fonction qui effectue la valeur absolue d'un entier
int val_abs(int a){
    if(a < 0){
        return -a;
    }
    return a;
}

double val_abs_d(double a){
    if(a < 0){
        return -a;
    }
    return a;
}

// Fonction qui effectue la valeur absolue d'un pixel
pixel val_abs_pix(pixel pix){
    pix.red = val_abs(pix.red);
    pix.green = val_abs(pix.green);
    pix.blue = val_abs(pix.blue);
    return pix;
}

// Fonction qui additionne deux pixels
pixel addition(pixel pix1, pixel pix2){
    pix1.red += pix2.red;
    pix1.green += pix2.green;
    pix1.blue += pix2.blue;
    return pix1;
}

// Fonction qui effectue un gradien horizontal et vertical et qui renvoie l'image associée
image* gradien(image* img_originale){
    assert(img_originale != NULL);
    int h = img_originale->hauteur;
    int l = img_originale->largeur;
    image* img_grad = create_img_vierge(h, l);
    img_grad->max = 255;
    for(int i = 0; i < h; i++){ 
        for(int j = 0; j < l; j++){
            pixel pix_gl; // Egal : la valeur du pixel de droite - la valeur du pixel de gauche 
            pix_gl.red = 0; pix_gl.blue = 0; pix_gl.green = 0;
            if(j - 1 >= 0){
                pix_gl.red -= (img_originale->matrice[i][j - 1].red);
                pix_gl.blue -= (img_originale->matrice[i][j - 1].blue);
                pix_gl.green -= (img_originale->matrice[i][j - 1].green);
            }

            if(j + 1 < l){
                pix_gl.red += img_originale->matrice[i][j + 1].red;
                pix_gl.blue += img_originale->matrice[i][j + 1].blue;
                pix_gl.green += img_originale->matrice[i][j + 1].green;
            }

            pixel pix_hl; // Egal : la valeur du pixel du dessous - la valeur du pixel du dessus
            pix_hl.red = 0; pix_hl.blue = 0; pix_hl.green = 0;

            if(i - 1 >= 0){
                pix_hl.red -= (img_originale->matrice[i - 1][j].red);
                pix_hl.blue -= (img_originale->matrice[i - 1][j].blue);
                pix_hl.green -= (img_originale->matrice[i - 1][j].green);
            }

            if(i + 1 < h){
                pix_hl.red += img_originale->matrice[i + 1][j].red;
                pix_hl.blue += img_originale->matrice[i + 1][j].blue;
                pix_hl.green += img_originale->matrice[i + 1][j].green;
            }

            // Sauvegarde des données
            img_grad->matrice[i][j] = addition(val_abs_pix(pix_hl), val_abs_pix(pix_gl));
        }
    }

    return img_grad;
}

// Fonction qui applique un filtre de Canny à une image, seuillage de l'image gradien
image* filtre_canny(image* img){
    assert(img != NULL);
    image* grad = gradien(img);
    image* contrastes = create_img_vierge(grad->hauteur, grad->largeur);
    contrastes->max = 255;
    for (int i = 0; i < grad->hauteur; i++){
        for(int j = 0; j < grad->largeur; j++){
            pixel pix;
            if(i == 0 || i == grad->hauteur - 1 || j == 0 || j == grad->largeur - 1){ 
                pix.red = 0;
                pix.blue = 0;
                pix.green = 0;
            } else {
                if(grad->matrice[i][j].red > 100 || grad->matrice[i][j].blue > 100 || grad->matrice[i][j].green > 100 ){
                    pix.red = 255;
                    pix.blue = 255;
                    pix.green = 255;
                } else {
                    pix.red = 0;
                    pix.blue = 0;
                    pix.green = 0;
                }
            }
            contrastes->matrice[i][j] = pix;
        }
    }

    free_image(grad);
    return contrastes;
}


// Fonction qui applique une transformée de Hough à une image, renvoie le tableau d'accumulation
int* Hough_transform(image* contour, int deg){
    // Création du tableau
    assert(contour != NULL);
    int* tab_acc= (int*)malloc(sizeof(int) * 100);
    assert(tab_acc !=  NULL);
    for(int i = 0; i < 100; i++){
        tab_acc[i] = 0;
    }
    
    // Remplissage du tableau
    for(int i = 0; i < contour->hauteur; i++){
        for(int j = 0; j < contour->largeur; j++){
            pixel pix = contour->matrice[i][j];
            if(pix.red == 255 && pix.blue == 255 && pix.green == 255){
                double p = j * cos(deg * (PI / 180)) + i * sin(deg * (PI / 180));
                p = val_abs_d(p);
                int a = (int)(round(p));
                tab_acc[a/3] = tab_acc[a/3] + 1; // Pas de 3 pour la largeur p
            }
        }
    }

    return tab_acc;
}


// Fonction qui à partir d'une matrice issue d'une transformée de Hough détecte la présence de ligne dans l'image
bool possede_lignes(image* contour, int deg, int taille){
    assert(contour != NULL);
    int* tab = Hough_transform(contour, deg);

    for(int j = 0; j < 75; j++){
        if(tab[j] > taille){
            free(tab);
            return true;
        }
    }

    free(tab);
    return false;
}

int mediane(int* tab, int n){
    tri_selection(tab, n);
    if(n % 2 == 0){
        return ((tab[n/2] + tab[(n/2) + 1]) / 2);
    }

    return (tab[(n/2) + 1]);
}

// Renvoie une nouvelle image
image* filtre_canny_propre(image* img, int n){
    assert(img != NULL);
    if (n < 1){
        printf("n < 1, Heho ! Canny \n");
        return img;
    } else {
        image* n_img = filtre_median(img);
        for (int i = 0; i < n ; i++){
            image* p = filtre_median(n_img);
            free_image(n_img);
            n_img = p;
        }
        image* n2_img = filtre_canny(n_img);
        free_image(n_img);
        return n2_img;
    }
}

// Renvoie une nouvelle image
image* filtre_median_propre(image* img, int n){
    assert(img != NULL);
    if (n < 2){
        printf("n < 2, Heho ! median\n");
        return img;
    } else {
        image* n_img = filtre_median(img);
        for (int i = 0; i < n ; i++){
            image* p = filtre_median(n_img);
            free_image(n_img);
            n_img = p;
        }
        return n_img;
    }
}


void trouve_lignes_groupe(grp_image* grp, int deg, int taille){
    assert(grp != NULL);

    // Création du groupe vierge sur lequel on applique les transformations
    grp_image* grp_tr = (grp_image*)malloc(sizeof(grp_image));
    grp_tr->nb_grp = grp->nb_grp;
    grp_tr->hauteur_image = grp->hauteur_image;
    grp_tr->largeur_image = grp->largeur_image;
    grp_tr->nb_hauteur = grp->nb_hauteur;
    grp_tr->nb_largeur = grp->nb_largeur;

    grp_tr->valide = (int*)malloc(sizeof(int) * grp_tr->nb_grp);
    assert(grp_tr->valide != NULL);
    for(int i = 0; i < grp_tr->nb_grp; i++){
        grp_tr->valide[i] = grp->valide[i];
    }
   
    grp_tr->groupe = (image**)malloc(sizeof(image*) * grp_tr->nb_grp);
    assert(grp_tr->groupe != NULL);

    // Application d'un filtre de canny propre sur les éléments viables du groupe
    for(int i = 0; i < grp->nb_grp; i++){
        
        if(grp->valide[i] > 0){
            grp_tr->groupe[i] = filtre_canny_propre(grp->groupe[i], 3);
            if(!possede_lignes(grp_tr->groupe[i], deg, taille)){
                grp->valide[i] = -1;
            }
        } else {
            grp_tr->groupe[i] = NULL;
        }
        free_image(grp_tr->groupe[i]);
    }

    // Libération du groupe de travail
    free(grp_tr->valide);
    free(grp_tr->groupe);
    free(grp_tr);
}

bool equal_pix(pixel pix1, pixel pix2){
    return ((pix1.red == pix2.red) && (pix1.blue == pix2.blue) && (pix1.green == pix2.green));
}


void save_groupe(grp_image* grp, char* nom_fichier){
    assert(grp != NULL);
    assert(grp->groupe != NULL);

    FILE* fichier = fopen(nom_fichier, "w");
    assert(fichier != NULL);

    // Remplissage
    fprintf(fichier, "P3\n%d %d\n%d\n", grp->largeur_image, grp->hauteur_image, grp->groupe[0]->max);

    for(int w = 0; w < grp->nb_hauteur; w++){ // On parcourt par hauteur de blocs
        for(int i = 0; i < grp->groupe[w * grp->nb_largeur]->hauteur; i++){ // On parcourt par ligne 
            for(int k = w * grp->nb_largeur; k < w * grp->nb_largeur + grp->nb_largeur; k++){ // On parcourt par blocs dans la largeur

                if(grp->valide[k] > 0){ // Si le groupe est valide 
                    for(int j = 0; j < grp->groupe[k]->largeur - 1; j++){ // On parcourt les colonnes 
                        fprintf(fichier, "%d %d %d ", grp->groupe[k]->matrice[i][j].red, grp->groupe[k]->matrice[i][j].green, grp->groupe[k]->matrice[i][j].blue);
                    }

                    // Spécificité de la dernière colonne du dernier bloc
                    int j = grp->groupe[k]->largeur - 1;

                    if(k ==  w * grp->nb_largeur + grp->nb_largeur - 1){
                        fprintf(fichier, "%d %d %d\n", grp->groupe[k]->matrice[i][j].red, grp->groupe[k]->matrice[i][j].green, grp->groupe[k]->matrice[i][j].blue);
                    } else {
                        fprintf(fichier, "%d %d %d ", grp->groupe[k]->matrice[i][j].red, grp->groupe[k]->matrice[i][j].green, grp->groupe[k]->matrice[i][j].blue);
                    }
                } else {
                    if(grp->valide[k] == 0){
                        for(int j = 0; j < grp->groupe[k]->largeur - 1; j++){ // On parcourt les colonnes 
                            fprintf(fichier, "%d %d %d ", 0, 0, 0);
                        }
                        // Spécificité de la dernière colonne du dernier bloc
                        if(k == w * grp->nb_largeur + grp->nb_largeur - 1){
                            fprintf(fichier, "%d %d %d\n", 0, 0, 0);
                        } else {
                            fprintf(fichier, "%d %d %d ", 0, 0, 0);
                        }
                    } else {
                        for(int j = 0; j < grp->groupe[k]->largeur - 1; j++){ // On parcourt les colonnes 
                            fprintf(fichier, "%d %d %d ", 0, 255, 0);
                        }
                        // Spécificité de la dernière colonne du dernier bloc
                        if(k ==  w * grp->nb_largeur + grp->nb_largeur - 1){
                            fprintf(fichier, "%d %d %d\n", 0, 255, 0);
                        } else {
                            fprintf(fichier, "%d %d %d ", 0, 255, 0);
                        }
                    }
                }
            }
        }
    }
    fclose(fichier);
}


// Renvoie une matrice de rayon 
matrice_rayon* create_matrice_r(int h, int l, int l_t){
    matrice_rayon* mat = (matrice_rayon*)malloc(sizeof(matrice_rayon));
    mat->hauteur= h;
    mat->largeur = l;
    mat->longueur_tab = l_t;
    mat->matrice_3d = (int***)malloc(sizeof(int**) * h);
    assert(mat->matrice_3d != NULL);

    for(int i = 0; i < h; i++){
        mat->matrice_3d[i] = (int**)malloc(sizeof(int*) * l);
        assert(mat->matrice_3d[i] != NULL);
        for(int j = 0; j < l; j++){
            mat->matrice_3d[i][j] = (int*)malloc(sizeof(int) * l_t);
            assert(mat->matrice_3d[i][j] != NULL);
            for(int k = 0; k < l_t; k++){
                mat->matrice_3d[i][j][k] = 0;
            }
        }
    }
    return mat;
}

void free_matrice_r(matrice_rayon* mat){
    for(int i = 0; i < mat->hauteur; i++){
        for(int j = 0; j < mat->largeur; j++){
            free(mat->matrice_3d[i][j]);
        }
        free(mat->matrice_3d[i]);
    }
    free(mat->matrice_3d);
    free(mat);
}

int distance(int x1, int y1, int x2, int y2){
    return val_abs(x1 - x2) + val_abs(y1 - y2);
}


matrice_rayon* round_Hough_transform(image* img, int rmin, int rmax, int pas){
    image* contour = filtre_canny(img);
    matrice_rayon* mat3 = create_matrice_r(img->hauteur, img->largeur, 1 + (rmax - rmin + 1) / pas);
    pixel pix;
    pix.red = 255;
    pix.blue = 255;
    pix.green= 255;

    for(int i = 0; i < contour->hauteur; i++){
        for(int j = 0; j < contour->largeur; j++){
            if(equal_pix(pix, contour->matrice[i][j])){
                for(int r = rmin; r <= rmax; r = r + pas){
                    for(int theta = 0; theta < 360; theta = theta + 5){
                        int x_c = (int)round(j + r * cos(theta * (PI / 180)));
                        int y_c = (int)round(i + r * sin(theta * (PI / 180)));
                        if (x_c >= 0 && x_c < mat3->largeur && y_c >= 0 && y_c < mat3->hauteur){
                            mat3->matrice_3d[y_c][x_c][(r - rmin) / pas] += 1;
                        }
                    }
                }
            }

        }
    }

    free_image(contour);
    return mat3;
}


int* trouve_cercles_image(int* nb_trouve, image* img, int rmin, int rmax, int pas){
    matrice_rayon* mat3 = round_Hough_transform(img, rmin, rmax, pas);

    int nb_cercle = 0;
    
    for(int r = rmin; r <= rmax; r += pas){
        int seuil = (int)round(2 * PI * r);
        for(int i = 0; i < img->hauteur; i++){
            for(int j = 0; j < img->largeur; j++){
                int rayon = (r - rmin) / pas;
                int acc = mat3->matrice_3d[i][j][rayon];
                if(acc >= seuil){
                    nb_cercle++;
                }
            }
        }
    }

    if(nb_cercle == 0){
        *nb_trouve = 0;
        free_matrice_r(mat3);
        return NULL;
    }

    *nb_trouve = nb_cercle;
    int* tab = (int*)malloc(sizeof(int) * nb_cercle * 3);
    int w = 0;

    for(int r = rmin; r <= rmax; r += pas){

        int seuil = (int)round(2 * PI * r);
        int rayon = (r - rmin) / pas;

        for(int i = 0; i < img->hauteur; i++){
            for(int j = 0; j < img->largeur; j++){
                int acc = mat3->matrice_3d[i][j][rayon];
                if(acc >= seuil){
                    assert(w + 2 < nb_cercle * 3);
                    tab[w] = r;
                    tab[w + 1] = i;
                    tab[w + 2] = j;
                    w += 3;
                }
            }
        }
    }

    free_matrice_r(mat3);
    return tab;
}



bool possede_cercle(image* img, int rmin, int rmax, int pas, int* nb_trouve){
    int nb = 0;
    int* tab = trouve_cercles_image(&nb, img, rmin, rmax, pas);
    int nb_cercle = nb;
    int num = 0;

    if(nb == 0){
        *nb_trouve = 0;
        free(tab);
        return false;
    } else {
        for(int i = 0; i < (nb - 1) * 3; i += 3){
            if(distance(tab[i + 5], tab[i + 4], tab[i + 2], tab[i + 1]) <= 4){
                nb_cercle--;
            }
        }

        int* res = (int*)malloc(sizeof(int) * nb_cercle * 3);

        for(int i = 0; i < (nb - 1) * 3; i += 3){
            if(!(distance(tab[i + 5], tab[i + 4], tab[i + 2], tab[i + 1]) <= 4)){
                if(num >= nb_cercle * 3){
                    printf("possede_cercle num : pb\n");
                } else {
                    res[num] = tab[i];
                    res[num + 1] = tab[i + 1];
                    res[num + 2] = tab[i + 2];
                    num += 3;
                }
            }
        }
        if(num + 2 < nb_cercle * 3){
            res[num] = tab[(nb_cercle - 1) * 3];
            res[num + 1] = tab[(nb_cercle - 1) * 3 + 1];
            res[num + 2] = tab[(nb_cercle - 1) * 3 + 2];
        } else {
            printf("pb possede_cercle\n");
        }
        num = 0;

        int n = nb_cercle;
        for(int i = 0; i < (nb_cercle - 1) * 3; i += 3){
            if(!(distance(res[i + 5], res[i + 4], res[i + 2], res[i + 1]) <= 4)){
                num++;
            } else {
                n--;
            }
        }
        free(res);
        free(tab);
        *nb_trouve = n;
        return true;
    }
}


grp_image* isole_visage(grp_image* grp, image* originale){
    int h_t = grp->groupe[0]->hauteur;
    int l_t = grp->groupe[0]->largeur;
    assert(h_t >= 100);

    grp_image* original = create_groupe(l_t, h_t, originale);

    int nb_valide = 0;
    for(int i = 0; i < grp->nb_grp; i++){
        if(grp->valide[i] > 0){
            nb_valide++;
        }
    }

    grp_image* res = (grp_image*)malloc(sizeof(grp_image));
    res->hauteur_image = 250;
    res->largeur_image = l_t * nb_valide;
    res->nb_grp = nb_valide;
    res->valide = (int*)malloc(sizeof(int) * nb_valide);
    for(int i = 0; i < nb_valide; i++){
        res->valide[i] = 1;
    }
    res->nb_hauteur = 1;
    res->nb_largeur = nb_valide;
    res->groupe = (image**)malloc(sizeof(image*) * nb_valide);

    int n = 0;
    for(int k = 0; k < grp->nb_grp; k++){
        if(grp->valide[k] > 0){
            if(n >= nb_valide){
                printf("isole_visage pb\n");
            } else {
                res->groupe[n] = create_img_vierge(250, l_t);
                res->groupe[n]->max = 255;
                for(int i = h_t - 100; i < h_t; i++){
                    for(int j = 0; j < grp->groupe[k]->largeur; j++){
                        res->groupe[n]->matrice[i - h_t + 100][j] = grp->groupe[k]->matrice[i][j];
                    }
                    for(int j = grp->groupe[k]->largeur; j < l_t; j++){
                        res->groupe[n]->matrice[i - h_t + 100][j].red = 0;
                        res->groupe[n]->matrice[i - h_t + 100][j].green = 0;
                        res->groupe[n]->matrice[i - h_t + 100][j].blue = 0;
                    }
                }

                int voisin = k + original->nb_largeur;
                if(voisin < original->nb_grp){
                    if(original->groupe[voisin]->hauteur >= 150){
                        for(int i = 0; i < 150; i++){
                            for(int j = 0; j < original->groupe[voisin]->largeur; j++){
                                res->groupe[n]->matrice[i + 100][j] = original->groupe[voisin]->matrice[i][j];
                            }
                            for(int j = original->groupe[voisin]->largeur; j < l_t; j++){
                                res->groupe[n]->matrice[i + 100][j].red = 0;
                                res->groupe[n]->matrice[i + 100][j].green = 0;
                                res->groupe[n]->matrice[i + 100][j].blue = 0;
                            }
                        }
                    } else {
                        for(int i = 0; i < original->groupe[voisin]->hauteur; i++){
                            for(int j = 0; j < original->groupe[voisin]->largeur; j++){
                                res->groupe[n]->matrice[i + 100][j] = original->groupe[voisin]->matrice[i][j];
                            }
                            for(int j = original->groupe[voisin]->largeur; j < l_t; j++){
                                res->groupe[n]->matrice[i + 100][j].red = 0;
                                res->groupe[n]->matrice[i + 100][j].green = 0;
                                res->groupe[n]->matrice[i + 100][j].blue = 0;
                            }
                        } 
                        for(int i = original->groupe[voisin]->hauteur; i < 150; i++){
                            for(int j = 0; j < l_t; j++){
                                res->groupe[n]->matrice[i + 100][j].red = 0;
                                res->groupe[n]->matrice[i + 100][j].green = 0;     
                                res->groupe[n]->matrice[i + 100][j].blue = 0;
                            }
                        }
                    }
                }
                n++;
            }
        }
    }
    free_groupe(original);
    return res;
}


int main()
{
    // Exemple d'isolement des lignes rouges horizontales d'une image  
    image* charlie = create_img("Image_Charlie.ppm");

    image* rouge = isolement_couleur(charlie, 150, 255, 0, 90, 0, 90, true);
    image* rouge_p = filtre_median_propre(rouge, 2);
    free_image(rouge);

    grp_image* rg = create_groupe(150, 250, rouge_p);

    rg = suppression(rg, true);
    free_image(rouge_p);

    trouve_lignes_groupe(rg, 90, 80);

    save_groupe(rg, "Lignes_rouges_Charlie.ppm");

    free_groupe(rg);
    free_image(charlie);
    return 0;
}
