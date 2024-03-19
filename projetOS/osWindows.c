#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h> // Include the necessary header file
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PARTITION_SIZE 1000000 // Taille de la partition en octets

typedef struct
{
    char *fileName;
    char *address;
    int position; // Position actuelle dans le fichier
    int size;     // Taille du fichier
} file;

char *partition; // Partition simulée

char *actualpartition;

void saveActualPartition()
{
    FILE *file = fopen("partitionnamesaver.txt", "w");
    if (file != NULL)
    {
        fprintf(file, "%s", actualpartition);
        fclose(file);
    }
}

void loadActualPartition()
{
    FILE *file = fopen("partitionnamesaver.txt", "r");
    if (file != NULL)
    {
        char buffer[100]; // Taille du buffer à ajuster selon vos besoins
        if (fgets(buffer, sizeof(buffer), file) != NULL)
        {
            actualpartition = strdup(buffer);
        }
        fclose(file);
    }
}

int createfile(char *fileName)
{
    loadActualPartition(); // Load the value of actualpartition from the file

    FILE *partitionFile = fopen(actualpartition, "a"); // Use "a" mode to append to the file
    if (partitionFile == NULL)
    {
        return 0; // Failed to open the partition file
    }

    // Write the file information to the partition file
    fseek(partitionFile, 0, SEEK_SET); // Move the file pointer to the beginning of the file
    fwrite("Filename: \"", sizeof(char), strlen("Filename: \""), partitionFile);
    fwrite(fileName, sizeof(char), strlen(fileName), partitionFile);
    fwrite("\",\n", sizeof(char), strlen("\",\n"), partitionFile);
    fwrite("Content : \"\",\n", sizeof(char), strlen("Content : \"\",\n"), partitionFile);
    fwrite("Size : 0,\n", sizeof(char), strlen("Size : 0,\n"), partitionFile);
    fwrite("Address : 0\n", sizeof(char), strlen("Address : 0\n"), partitionFile);
    fwrite(";", sizeof(char), strlen(";"), partitionFile);
    fwrite("\n\n", sizeof(char), strlen("\n\n"), partitionFile);

    fclose(partitionFile);
    return 1; // File creation successful
}

void affiche()
{
    loadActualPartition(); // Chargse la partition actuelle

    FILE *partitionFile = fopen(actualpartition, "r"); // Ouvre le fichier de partition en mode lecture
    if (partitionFile == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de partition.\n");
        return;
    }

    char buffer[100]; // Taille du buffer à ajuster selon vos besoins
    while (fgets(buffer, sizeof(buffer), partitionFile) != NULL)
    {
        printf("%s", buffer); // Affiche chaque ligne du fichier de partition
    }

    fclose(partitionFile);
}

int myFormat(char *partitionName)
{
    loadActualPartition();
    if (actualpartition != NULL && strcmp(partitionName, actualpartition) == 0)
    {
        // Vide simplement le fichier
        int fd = open(partitionName, O_WRONLY | O_TRUNC);
        if (fd == -1)
        {
            return -1; // Échec de l'ouverture du fichier
        }
        if (ftruncate(fd, PARTITION_SIZE) == -1)
        {
            close(fd);
            return -1; // Échec de l'allocation de la taille de la partition
        }
        close(fd);
    }
    else
    {
        if (actualpartition != NULL)
        {
            // Supprime l'ancienne partition
            remove(actualpartition);
            free(actualpartition);
        }

        // Crée la nouvelle partition
        actualpartition = strdup(partitionName);
        if (actualpartition == NULL)
        {
            return -1; // Échec de l'allocation de mémoire
        }

        int fd = open(partitionName, O_WRONLY | O_CREAT, 0644);
        if (fd == -1)
        {
            return -1; // Échec de création du fichier
        }
        saveActualPartition();
        close(fd);
    }e la partition actuelle

    FILE *partitionFile = fopen(actualpartition, "r"); // Ouverture du fichier de partition en mode lecture
    if (partitionFile == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de partition.\n");
        return NULL;
    }

    return 0; // Formatage réussi
}

file *myOpen(char *filename)
{
    loadActualPartition(); // Chargement d

    file *fichier = (file *)malloc(sizeof(file)); // Allocation de mémoire pour un pointeur vers la structure file;

    if (fichier == NULL)
    {
        fclose(partitionFile);
        return NULL; // Échec de l'allocation de mémoire
    }

    char buffer[100];  // Taille du buffer à ajuster selon vos besoins
    int fileFound = 0; // Variable pour indiquer si le fichier a été trouvé

    while (fgets(buffer, sizeof(buffer), partitionFile) != NULL && !fileFound)
    {
        if (strncmp(buffer, "filename: ", 10) == 0)
        {
            char *file = buffer + 10;         // Récupération du nom de fichier
            file[strcspn(file, "\n")] = '\0'; // Suppression du caractère de nouvelle ligne

            if (strcmp(file, filename) == 0)
            {
                fileFound = 1; // Le fichier a été trouvé

                strcpy(fichier->fileName, file); // Copie du nom de fichier dans la structure file

                // Lecture des autres informations du fichier dans la partition
                while (fgets(buffer, sizeof(buffer), partitionFile) != NULL)
                {
                    if (strncmp(buffer, "content : ", 10) == 0)
                    {
                        strcpy(fichier->content, buffer + 10);                    // Copie du contenu du fichier
                        fichier->content[strcspn(fichier->content, "\n")] = '\0'; // Suppression du caractère de nouvelle ligne
                    }
                    else if (strncmp(buffer, "size : ", 7) == 0)
                    {
                        fichier->size = atoi(buffer + 7); // Conversion de la taille du fichier en entier
                    }
                    else if (strncmp(buffer, "address : ", 10) == 0)
                    {
                        fichier->address = "1"; // Conversion de l'adresse du fichier en entier
                    }
                    else if (strncmp(buffer, ";", 1) == 0)
                    {
                        break; // Fin de la lecture des informations du fichier
                    }
                }

                break; // Sortie de la boucle, le fichier a été trouvé
            }
        }
    }

    fclose(partitionFile);

    if (!fileFound)
    {
        createfile(filename);
        fichier->fileName = filename;
        fichier->content = "";
        fichier->address = "1";
        fichier->position = 0;
        fichier->size = 0;
    }

    return fichier; // Retourne la structure file initialisée
}

int myWrite(file *f, void *buffer, int nBytes)
{
    if (f == NULL || buffer == NULL || nBytes <= 0)
    {
        return -1; // Échec de l'écriture
    }
    if (f->position + nBytes > PARTITION_SIZE)
    {
        return -1; // Dépassement de la taille de la partition
    }
    memcpy(partition + f->position, buffer, nBytes);
    f->position += nBytes;
    if (f->position > f->size)
    {
        f->size = f->position;
    }

    FILE *file = fopen(f->fileName, "wb");
    if (file == NULL)
    {
        return -1; // Échec d'ouverture du fichier
    }
    fwrite(partition, sizeof(char), f->size, file);
    fclose(file);

    return nBytes; // Écriture réussie
}

int myRead(file *f, void *buffer, int nBytes)
{
    if (f == NULL || buffer == NULL || nBytes <= 0)
    {
        return -1; // Échec de la lecture
    }
    if (f->position + nBytes > f->size)
    {
        return -1; // Dépassement de la taille du fichier
    }
    memcpy(buffer, partition + f->position, nBytes);
    f->position += nBytes;
    return nBytes; // Lecture réussie
}

int mySeek(file *f, int offset, int base)
{
    if (f == NULL)
    {
        return -1; // Fichier invalide
    }
    if (base == SEEK_SET)
    {
        f->position = offset;
    }
    else if (base == SEEK_CUR)
    {
        f->position += offset;
    }
    else if (base == SEEK_END)
    {
        f->position = f->size + offset;
    }
    if (f->position < 0)
    {
        f->position = 0;
    }
    if (f->position > f->size)
    {
        f->position = f->size;
    }
    return 0; // Tout s'est bien passé
}

int main()
{
    int choice;
    char fileName[100];
    file *f = NULL;

    do
    {
        printf("Menu:\n");
        printf("1. Format the partition\n");
        printf("2. Open a file\n");
        printf("3. Write to a file\n");
        printf("4. Read from a file\n");
        printf("5. Seek in a file\n");
        printf("6. creer un fichier vide\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
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
            if (f != NULL)
            {
                printf("File opened successfully\n");
                printf("File name: %s %s %s %s %s\n", f->fileName, f->content, f->address, f->position, f->size);
            }
            e
            {
                printf("Failed to open the file\n");
            }
            break;
        case 3:
            if (f == NULL)
            {
                printf("No file is currently open\n");
            }
            else
            {
                char buffer[100];
                printf("Enter the data to write: ");
                scanf("%s", buffer);
                int result = myWrite(f, buffer, strlen(buffer));
                if (result > 0)
                {
                    printf("Data written successfully\n");
                }
                else
                {
                    printf("Failed to write data\n");
                }
            }
            break;
        case 4:
            if (f == NULL)
            {
                printf("No file is currently open\n");
            }
            else
            {
                char buffer[100];
                int nBytes;
                printf("Enter the number of bytes to read: ");
                scanf("%d", &nBytes);
                int result = myRead(f, buffer, nBytes);
                if (result >= 0)
                {
                    buffer[result] = '\0';
                    printf("Data read: %s\n", buffer);
                }
                else
                {
                    printf("Failed to read data\n");
                }
            }
            break;
        case 5:
            if (f == NULL)
            {
                printf("No file is currently open\n");
            }
            else
            {
                int offset, whence;
                printf("Enter the offset: ");
                scanf("%d", &offset);
                printf("Enter the whence value (0 for SEEK_SET, 1 for SEEK_CUR, 2 for SEEK_END): ");
                scanf("%d", &whence);
                int result = mySeek(f, offset, whence);
                if (result == 0)
                {
                    printf("Seek operation successful\n");
                }
                else
                {
                    printf("Failed to perform seek operation\n");
                }
            }
            break;
        case 6:
            printf("Enter the file name: ");
            scanf("%s", fileName);
            if (createfile(fileName))
            {
                printf("File created successfully\n");
            }
            else
            {
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
