#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAILLE_BLOC 512
#define MAX_FICHIERS 100

typedef struct {
    int debut;
    int taille;
    time_t dateCreation;
    time_t dateModification;
    char nom[256];
} Fichier;

typedef struct {
    char data[TAILLE_BLOC];
    int suivant;
} Bloc;

typedef struct {
    Bloc blocs[MAX_FICHIERS * 10];
    Fichier fichiers[MAX_FICHIERS];
    int blocLibre;
} Partition;

Partition maPartition;
char *actualpartition = NULL;

void freeActualPartition() {
    if (actualpartition != NULL) {
        free(actualpartition);
        actualpartition = NULL;
    }
}

void saveActualPartition() {
    FILE *file = fopen("partitionnamesaver.txt", "w");
    if (file != NULL && actualpartition != NULL) {
        fprintf(file, "%s", actualpartition);
        fclose(file);
    }
}

void loadActualPartition() {
    FILE *file = fopen("partitionnamesaver.txt", "r");
    freeActualPartition(); // Libère la mémoire si nécessaire
    if (file != NULL) {
        char buffer[100];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0; // Supprime le caractère de nouvelle ligne
            actualpartition = strdup(buffer);
        }
        fclose(file);
    }
}

int allouerBloc() {
    if (maPartition.blocLibre < 0 || maPartition.blocLibre >= MAX_FICHIERS * 10) return -1;
    
    int blocAlloue = maPartition.blocLibre;
    maPartition.blocLibre = (blocAlloue + 1) % (MAX_FICHIERS * 10); // Simple gestion de l'index du premier bloc libre
    maPartition.blocs[blocAlloue].suivant = -1;
    return blocAlloue;
}

void initPartition() {
    for (int i = 0; i < MAX_FICHIERS; i++) {
        maPartition.fichiers[i].debut = -1;
        maPartition.fichiers[i].taille = 0;
        strcpy(maPartition.fichiers[i].nom, "");
    }

    for (int i = 0; i < MAX_FICHIERS * 10; i++) {
        maPartition.blocs[i].suivant = i + 1; // Prépare la liste des blocs libres
    }
    maPartition.blocs[MAX_FICHIERS * 10 - 1].suivant = -1; // Dernier bloc n'a pas de suivant

    maPartition.blocLibre = 0;
}

int myFormat(const char *partitionName) {
    freeActualPartition(); // Nettoie l'ancien nom si nécessaire

    actualpartition = strdup(partitionName);
    if (actualpartition == NULL) return -1;

    saveActualPartition(); // Sauvegarde le nouveau nom de partition

    initPartition(); // Réinitialise la partition en mémoire

    return 0;
}

Fichier *myOpen(const char *filename) {
    time_t now = time(NULL);

    for (int i = 0; i < MAX_FICHIERS; i++) {
        if (strcmp(maPartition.fichiers[i].nom, filename) == 0 || maPartition.fichiers[i].taille == 0) {
            if (maPartition.fichiers[i].taille == 0) { // Nouveau fichier
                strncpy(maPartition.fichiers[i].nom, filename, 255);
                maPartition.fichiers[i].nom[255] = '\0';
                maPartition.fichiers[i].debut = allouerBloc();
                if (maPartition.fichiers[i].debut == -1) return NULL; // Échec d'allocation d'un nouveau bloc
                maPartition.fichiers[i].taille = 0;
                maPartition.fichiers[i].dateCreation = now;
            }
            maPartition.fichiers[i].dateModification = now;
            return &maPartition.fichiers[i];
        }
    }

    return NULL; // Partition pleine ou autre erreur
}

int main() {
    char partitionName[100], fileName[256];
    Fichier *f;
    int choix;

    loadActualPartition(); // Charge le nom de la partition actuelle

    do {
        printf("1. Formater la partition\n");
        printf("2. Ouvrir un fichier\n");
        printf("3. Quitter\n");
        printf("Votre choix: ");
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                printf("Nom de la partition: ");
                scanf("%s", partitionName);
                if (myFormat(partitionName) == 0) {
                    printf("Partition formatée avec succès.\n");
                } else {
                    printf("Erreur lors du formatage.\n");
                }
                break;
            case 2:
                printf("Nom du fichier: ");
                scanf("%s", fileName);
                f = myOpen(fileName);
                if (f != NULL) {
                    printf("Fichier ouvert avec succès: %s\n", f->nom);
                } else {
                    printf("Erreur lors de l'ouverture du fichier.\n");
                }
                break;
            case 3:
                printf("Au revoir.\n");
                break;
            default:
                printf("Choix invalide.\n");
        }
    } while (choix != 3);

    freeActualPartition(); // Libère la mémoire allouée

    return 0;
}
