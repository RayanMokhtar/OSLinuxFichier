#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define TAILLE_BLOC 512
#define MAX_FICHIERS 100
#define NOM_FICHIER_MAX 256
#define TAILLE_PARTITION (MAX_FICHIERS * 10 * TAILLE_BLOC)

typedef struct {
    int debut; // Le numéro du premier bloc du fichier dans la partition
    int taille; // Taille du fichier en octets
    time_t dateCreation; // Date de création du fichier
    time_t dateModification; // Date de la dernière modification
    char nom[256]; // Nom du fichier
} Fichier;

typedef struct {
    char data[TAILLE_BLOC]; // Données du bloc
    int suivant; // Indice du bloc suivant dans le fichier, -1 si c'est le dernier
} Bloc;

typedef struct {
    Bloc blocs[MAX_FICHIERS * 10]; // Supposons un max de 10 blocs par fichier
    Fichier fichiers[MAX_FICHIERS]; // Informations sur les fichiers
    int blocLibre; // Indice du premier bloc libre dans la partition
    int nbFichiers; // Nombre de fichiers dans la partition 
} Partition;

Partition maPartition;

// Fonction pour formater la partition
int myFormat(char* partitionName) {
    int fd = open(partitionName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Erreur lors de la création du fichier de partition");
        return -1;
    }

    // Préparation du bloc vide
    char blocVide[TAILLE_BLOC];
    memset(blocVide, 0, TAILLE_BLOC);

    // Initialisation de la partition avec des blocs vides
    for (int i = 0; i < (TAILLE_PARTITION / TAILLE_BLOC); i++) {
        if (write(fd, blocVide, TAILLE_BLOC) != TAILLE_BLOC) {
            perror("Erreur lors de l'écriture du bloc vide dans la partition");
            close(fd);
            return -1;
        }
    }

    close(fd);
    printf("Partition \"%s\" formatée avec succès.\n", partitionName);

    // Initialiser la structure de Partition en mémoire
    for (int i = 0; i < MAX_FICHIERS; i++) {
        maPartition.fichiers[i].debut = -1;
        strcpy(maPartition.fichiers[i].nom, "");
    }
    
    for (int i = 0; i < MAX_FICHIERS * 10; i++) {
        maPartition.blocs[i].suivant = -1;
        memset(maPartition.blocs[i].data, 0, TAILLE_BLOC);
    }
    
    maPartition.blocLibre = 0;

    return 0;
}

int chercherFichier(char* nomFichier);

// Fonction pour "ouvrir" un fichier dans la partition simulée
// Modifiez la fonction myOpen pour allouer un bloc lors de la création d'un fichier
Fichier* myOpen(char* nomFichier) {
    int indexFichier = chercherFichier(nomFichier);

    if (indexFichier != -1) {
        // Le fichier existe déjà
        printf("Fichier \"%s\" ouvert avec succès.\n", nomFichier);
        return &maPartition.fichiers[indexFichier];
    } else {
        // Création d'un nouveau fichier
        for (int i = 0; i < MAX_FICHIERS; i++) {
            if (maPartition.fichiers[i].debut == -1) { // Cherche une entrée vide
                int blocIndex = maPartition.blocLibre; // Supposons que cela représente le prochain bloc libre
                if (blocIndex >= MAX_FICHIERS * 10) {
                    printf("Erreur : Aucun bloc libre disponible.\n");
                    return NULL; // Plus de blocs libres
                }
                
                // Mettez à jour les informations du fichier
                maPartition.fichiers[i].debut = blocIndex; // Définit le début du fichier au bloc libre
                maPartition.fichiers[i].taille = 0; // Taille initiale du fichier
                maPartition.fichiers[i].dateCreation = time(NULL);
                maPartition.fichiers[i].dateModification = time(NULL);
                strncpy(maPartition.fichiers[i].nom, nomFichier, NOM_FICHIER_MAX);

                // Marquez le bloc comme utilisé (simplifié)
                maPartition.blocLibre++; // Incrémente l'index du bloc libre pour le prochain fichier
                
                printf("Fichier \"%s\" créé et ouvert avec succès.\n", nomFichier);
                return &maPartition.fichiers[i];
            }
        }
    }

    printf("Erreur : Impossible de créer le fichier \"%s\". Partition pleine ?\n", nomFichier);
    return NULL; // Partition pleine ou autre erreur
}

// Fonction pour chercher un fichier par son nom
int chercherFichier(char* nomFichier) {
    for (int i = 0; i < MAX_FICHIERS; i++) {
        if (strcmp(maPartition.fichiers[i].nom, nomFichier) == 0) {
            return i; // Fichier trouvé
        }
    }
    return -1; // Fichier non trouvé
}




// Fonction pour écrire dans un fichier
int myWrite(Fichier* fichier, const char* data, int taille) {
    // Exemple : on suppose que 'debut' est l'offset dans le fichier de partition physique
    int fd = open("MaPartition.img", O_WRONLY);
    if (fd == -1) return -1; // Erreur à l'ouverture
    
    if (lseek(fd, fichier->debut, SEEK_SET) == -1) {
        close(fd);
        return -1; // Erreur de positionnement
    }
    
    if (write(fd, data, taille) != taille) {
        close(fd);
        return -1; // Erreur d'écriture
    }
    
    close(fd);
    return 0; // Succès
}

int main() {
    // Formatons d'abord la partition simulée.
    // Cela crée un fichier physique "MaPartition.img" sur le disque.
    if (myFormat("MaPartition.img") == 0) {
        printf("Partition formatée avec succès.\n");
    } else {
        printf("Erreur lors du formatage de la partition.\n");
        return -1;
    }

    // Essayons d'ouvrir un nouveau fichier dans notre partition.
    char* nomFichier = "monPremierFichier";
    Fichier* fichier = myOpen(nomFichier);
    if (fichier != NULL) {
        printf("Fichier '%s' ouvert avec succès.\n", nomFichier);
        // Ici, vous pourriez hypothétiquement écrire dans le fichier avec myWrite,
        // mais comme cette fonction nécessite une implémentation complète de la gestion des blocs,
        // nous ne procéderons pas à l'écriture dans cet exemple.
    } else {
        printf("Erreur lors de l'ouverture du fichier '%s'.\n", nomFichier);
        return -1;
    }

    // Supposons que vous ayez une fonction myClose pour fermer proprement le fichier.
    // myClose(fichier);

    return 0;
}