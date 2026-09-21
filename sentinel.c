#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_FILES 100

typedef struct {
    char filename[100];
    unsigned long size;
    uint32_t hash;
} FileRecord;

/* FNV-1a 32-bit hash */
uint32_t fnv1a_hash(const char *data) {
    uint32_t hash = 2166136261u;

    while (*data) {
        hash ^= (unsigned char)*data;
        hash *= 16777619u;
        data++;
    }

    return hash;
}

/* Display a stored file record */
void display_record(FileRecord record) {
    printf("\nFile: %s\n", record.filename);
    printf("Size: %lu bytes\n", record.size);
    printf("Hash: %u\n", record.hash);
}

int main() {
    FileRecord records[MAX_FILES];
    int count = 0;
    int choice;

    printf("=====================================\n");
    printf("        SentinelC - File Monitor\n");
    printf("=====================================\n");

    do {
        printf("\n1. Add File Record\n");
        printf("2. View File Records\n");
        printf("3. Check File Integrity\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            if (count >= MAX_FILES) {
                printf("Record limit reached.\n");
                continue;
            }

            printf("Enter filename: ");
            fgets(records[count].filename,
                  sizeof(records[count].filename), stdin);

            records[count].filename[
                strcspn(records[count].filename, "\n")
            ] = '\0';

            printf("Enter file size (bytes): ");
            scanf("%lu", &records[count].size);

            /*
             * In this prototype, the filename is used as
             * sample data for demonstrating hashing.
             */
            records[count].hash = fnv1a_hash(records[count].filename);

            printf("\nFile record added successfully.\n");
            display_record(records[count]);

            count++;
        }

        else if (choice == 2) {
            if (count == 0) {
                printf("\nNo file records available.\n");
            } else {
                printf("\nStored File Records:\n");

                for (int i = 0; i < count; i++) {
                    display_record(records[i]);
                }
            }
        }

        else if (choice == 3) {
            if (count == 0) {
                printf("\nNo file records available for checking.\n");
            } else {
                printf("\nIntegrity checking uses the stored hash values.\n");
                printf("Stored records: %d\n", count);
                printf("Prototype integrity check completed.\n");
            }
        }

        else if (choice == 4) {
            printf("\nExiting SentinelC...\n");
        }

        else {
            printf("\nInvalid choice.\n");
        }

    } while (choice != 4);

    return 0;
}
