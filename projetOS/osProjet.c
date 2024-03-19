#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h> // Include the necessary header file
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TAILLE_BLOC 512
#define MAX_FICHIERS 100
#define TAILLE_PARTITION (MAX_FICHIERS * 10 * TAILLE_BLOC)
#define MAX_BLOCS (MAX_FICHIERS * 10)

typedef struct {
    int debut; // Le numéro du premier bloc du fichier dans la partition
    int taille; // Taille du fichier en octets
    time_t dateCreation; // Date de création du fichier
    time_t dateModification; // Date de la dernière modification
    char nom[256]; // Nom du fichier
    int nbBlocs; // Nombre de blocs utilisés par le fichier
} Fichier;

typedef struct {
    char data[TAILLE_BLOC]; // Données du bloc         
    int suivant; // Indice du bloc suivant dans le fichier, -1 si c'est le dernier
} Bloc;

typedef struct {
    Bloc blocs[MAX_FICHIERS * 10]; // Supposons un max de 10 blocs par fichier
    Fichier fichiers[MAX_FICHIERS]; // Informations sur les fichiers , méta donnés des fichiers 
    int blocLibre; // Indice du premier bloc libre dans la partition
    int nbFichiers; // Nombre de fichiers actuels dans la partition
} Partition;

Partition maPartition;
char *actualpartition = NULL;





void saveActualPartition()
{
    FILE *file = fopen("partitionnamesaver.txt", "w");
    if (file != NULL)
    {
        fprintf(file, "%s", actualpartition);
        fclose(file);
    }
}

void loadActualPartition() {
    FILE *file = fopen("partitionnamesaver.txt", "r");
    if (file != NULL) {
        char buffer[100]; // Taille du buffer à ajuster selon vos besoins
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            if (actualpartition != NULL) {
                free(actualpartition); // Libère l'ancienne mémoire si actualpartition était déjà défini
                actualpartition = NULL;
            }
            buffer[strcspn(buffer, "\n")] = '\0'; // Supprime le retour à la ligne potentiellement lu par fgets
            actualpartition = strdup(buffer);
        }
        fclose(file);
    } else {
        // Gestion de l'erreur si le fichier ne peut pas être ouvert
        perror("Impossible d'ouvrir le fichier partitionnamesaver.txt");
    }
}


void afficherDate(const char* prefixe, time_t rawtime) {
    struct tm *info;
    char buffer[80];

    info = localtime(&rawtime);
    // Format : Jour-Mois-Année Heures:Minutes:Secondes
    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", info);
    printf("%s%s\n", prefixe, buffer);
}

/* myFormat*/
int myFormat(char *partitionName) {
    loadActualPartition(); // Charge la partition actuelle si elle existe

    // Suppression de l'ancienne partition, si elle existe
    if (actualpartition != NULL) {
        remove(actualpartition);
        free(actualpartition);
    }

    // Mise à jour et sauvegarde du nom de la nouvelle partition
    actualpartition = strdup(partitionName);
    if (actualpartition == NULL) {
        return -1; // Échec de l'allocation de mémoire
    }
    saveActualPartition();

    // Création et initialisation du fichier de partition
    FILE *partitionFile = fopen(actualpartition, "w");
    if (partitionFile == NULL) {
        free(actualpartition);
        actualpartition = NULL;
        return -2; // Échec de création du fichier de partition
    }

    // Écriture de l'en-tête de la partition avec des informations de base
    fprintf(partitionFile, "Partition: %s\nNombre de blocs par fichier: %d\nTaille du bloc: %d\n;",
            partitionName, 10, TAILLE_BLOC);

    fclose(partitionFile);

    // Initialisation de la structure de données Partition en mémoire
    for (int i = 0; i < MAX_FICHIERS; i++) {
        maPartition.fichiers[i].debut = -1; // Indique que le fichier n'est pas utilisé
        maPartition.fichiers[i].taille = 0;
        strcpy(maPartition.fichiers[i].nom, "");
    }

    for (int i = 0; i < MAX_FICHIERS * 10; i++) {
        maPartition.blocs[i].suivant = -1; // Initialise tous les blocs comme derniers
    }

    maPartition.blocLibre = 0; // Le premier bloc est libre

    return 0; // Formatage réussi
}






/*myOpen*/

/*int allouerBloc() {
    if (maPartition.blocLibre == -1) return -1; // Plus de blocs libres
    int blocAlloue = maPartition.blocLibre;
    maPartition.blocLibre = maPartition.blocs[blocAlloue].suivant; // Met à jour l'indice du premier bloc libre
    maPartition.blocs[blocAlloue].suivant = -1; // Ce bloc n'a plus de suivant
    return blocAlloue;
}*/

int allouerBloc(int fichierIndex) {
    // Vérifie si le fichier existe et si nbBlocs < 10 qui est ma condition ici dans cet exemple 
    if (fichierIndex < 0 || fichierIndex >= MAX_FICHIERS || maPartition.fichiers[fichierIndex].nbBlocs >= 10) {
        return -1; // Le fichier n'existe pas ou a déjà 10 blocs
    }

    // Vérifie s'il reste des blocs libres dans la partition
    if (maPartition.blocLibre == -1) {
        return -1; // Plus de blocs libres
    }

    // Alloue le bloc
    // je fais le chainage ici 
    int blocAlloue = maPartition.blocLibre;
    maPartition.blocLibre = maPartition.blocs[blocAlloue].suivant; // Met à jour l'indice du premier bloc libre
    maPartition.blocs[blocAlloue].suivant = -1; // Ce bloc n'a plus de suivant

    // Mettre à jour le fichier pour ajouter le bloc alloué
    if (maPartition.fichiers[fichierIndex].debut == -1) {
        // Si c'est le premier bloc du fichier
        maPartition.fichiers[fichierIndex].debut = blocAlloue;
    } else {
        // Trouve le dernier bloc du fichier et met à jour son "suivant"
        int dernierBloc = maPartition.fichiers[fichierIndex].debut;
        while (maPartition.blocs[dernierBloc].suivant != -1) {
            dernierBloc = maPartition.blocs[dernierBloc].suivant;
        }
        maPartition.blocs[dernierBloc].suivant = blocAlloue;
    }

    // Incrémente nbBlocs pour ce fichier
    maPartition.fichiers[fichierIndex].nbBlocs++;

    return blocAlloue; // Retourne l'indice du bloc alloué
}





Fichier *myOpen(const char *filename) {
    time_t now = time(NULL);
    int indexNouveauFichier = -1;
    loadActualPartition(); // Charge la partition actuelle si elle existe

    // Recherche si le fichier existe déjà
    for (int i = 0; i < MAX_FICHIERS; i++) {
        if (maPartition.fichiers[i].taille != 0 && strcmp(maPartition.fichiers[i].nom, filename) == 0) {
            // Fichier trouvé, met à jour la date de modification
            maPartition.fichiers[i].dateModification = now;
            indexNouveauFichier = i;
            break;
        }
    }

    if (indexNouveauFichier == -1) { // Si le fichier n'existe pas, crée un nouveau fichier
        for (int i = 0; i < MAX_FICHIERS; i++) {
            if (maPartition.fichiers[i].taille == 0) { // Trouve un slot libre pour le fichier
                strncpy(maPartition.fichiers[i].nom, filename, 255);
                maPartition.fichiers[i].nom[255] = '\0';
                maPartition.fichiers[i].debut =-1; // Initialisation sans bloc alloué
                maPartition.fichiers[i].nbBlocs = 0; // Initialisation du nombre de blocs
                maPartition.fichiers[i].taille = 0; // Taille initiale à 0
                maPartition.fichiers[i].dateCreation = now;
                maPartition.fichiers[i].dateModification = now;
                indexNouveauFichier = i;
                break;
            }
        }
    }

    if (indexNouveauFichier == -1) {
        // Si c'est un nouveau fichier, alloue un premier bloc
        if (maPartition.fichiers[indexNouveauFichier].nbBlocs == 0) {
            int blocIndex = allouerBloc(indexNouveauFichier);
            if (blocIndex == -1) {
                printf("Erreur : Plus de blocs libres ou limite de blocs atteinte.\n");
                return NULL;
            }
            maPartition.fichiers[indexNouveauFichier].debut = blocIndex;
        }
        ecrireNouveauFichierDansPartition(&maPartition.fichiers[indexNouveauFichier]);
        return &maPartition.fichiers[indexNouveauFichier];
    } else {
        printf("Erreur : La partition est pleine, impossible de créer un nouveau fichier.\n");
        return NULL;
    }
}

void ecrireNouveauFichierDansPartition(Fichier* nouveauFichier) {
    loadActualPartition(); // Charge la partition actuelle si elle existe
    if (actualpartition == NULL) {
        printf("Aucune partition sélectionnée.\n");
        return;
    }

    FILE *f = fopen(actualpartition, "a");
    if (!f) {
        perror("Erreur lors de l'ouverture du fichier de partition en mode append");
        return;
    }

    // Récupérer les données du premier bloc du fichier
    const char* data = "";
    if (nouveauFichier->debut != -1) { // Assurez-vous qu'il y a au moins un bloc
        data = maPartition.blocs[nouveauFichier->debut].data;
    }

    // Écriture des informations du fichier avec des sauts de ligne entre chaque champ
    fprintf(f, "\n\n\n\n\nIDFichier: %d  { \n",maPartition.nbFichiers);
    fprintf(f, "NomFichier: %s,\n", nouveauFichier->nom);
    fprintf(f, "Taille: %d,\n", nouveauFichier->taille);
    fprintf(f, "Debut: %d,\n", nouveauFichier->debut);
    fprintf(f, "DateCreation: %ld,\n", (long)nouveauFichier->dateCreation);
    fprintf(f, "DateModification: %ld,\n", (long)nouveauFichier->dateModification);
    fprintf(f, "ContenuFichier: \"%s\",\n\r};\n", data);

    fclose(f);
    maPartition.nbFichiers++;
}


int main() {
    int choice;
    char fileName[256]; // Adapté pour correspondre à la taille dans la structure Fichier
    Fichier* f = NULL; // Doit être de type Fichier* selon vos structures

    loadActualPartition(); // Charger le nom de la partition actuelle au démarrage

    do {
        printf("Menu:\n");
        printf("1. Format the partition\n");
        printf("2. Open a file\n");
        printf("3. Write to a file (not implemented)\n");
        printf("4. Read from a file (not implemented)\n");
        printf("5. Seek in a file (not implemented)\n");
        printf("6. Create an empty file\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter the partition name: ");
                scanf("%s", fileName);
                myFormat(fileName);
                printf("Partition formatted\n");
                break;
            case 2:
                printf("Enter the file name: ");
                scanf("%s", fileName);
                f = myOpen(fileName);
                if (f != NULL) {
                    printf("File opened successfully\n");
                    printf("File name: %s\n", f->nom);
                    // Avant de retourner le fichier dans myOpen
                

                } else {
                    printf("Failed to open the file\n");
                }
                break;
            case 3:
                printf("Write to a file function is not implemented.\n");
                break;
            case 4:
                printf("Read from a file function is not implemented.\n");
                break;
            case 5:
                printf("Seek in a file function is not implemented.\n");
                break;
            case 6:
                // La fonction createfile(fileName) n'existe pas dans votre code. Utilisez plutôt myOpen pour créer.
                printf("Enter the file name: ");
                scanf("%s", fileName);
                f = myOpen(fileName); // myOpen crée le fichier s'il n'existe pas
                if (f != NULL) {
                    printf("File created successfully\n");
                } else {
                    printf("Failed to create the file\n");
                }
                break;
            case 7:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice\n");
                break;
        }

        printf("\n");
    } while (choice != 7);

    return 0;
}