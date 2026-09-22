#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>

#define MAX_FILES 1000
#define NAME_SIZE 260
#define BASELINE_FILE "baseline.db"
#define CONFIG_FILE "config.txt"
#define LOG_FILE "audit_log.txt"

typedef struct
{
    char filename[NAME_SIZE];
    unsigned long long size;
    uint32_t hash;
} FileRecord;

/* ---------- FNV-1a 32-bit Hash ---------- */
uint32_t calculate_hash(const char *filepath)
{
    FILE *file;
    unsigned char buffer[4096];
    size_t bytesRead;

    uint32_t hash = 2166136261u;

    file = fopen(filepath, "rb");

    if (file == NULL)
    {
        return 0;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        size_t i;

        for (i = 0; i < bytesRead; i++)
        {
            hash ^= buffer[i];
            hash *= 16777619u;
        }
    }

    fclose(file);

    return hash;
}

/* ---------- Remove Newline ---------- */
void remove_newline(char *text)
{
    size_t len = strlen(text);

    while (len > 0 &&
           (text[len - 1] == '\n' || text[len - 1] == '\r'))
    {
        text[len - 1] = '\0';
        len--;
    }
}

/* ---------- Check Folder ---------- */
int folder_exists(const char *folder)
{
    DWORD attributes = GetFileAttributesA(folder);

    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return 0;
    }

    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return 1;
    }

    return 0;
}

/* ---------- Save Folder Path ---------- */
void save_config(const char *folder)
{
    FILE *file = fopen(CONFIG_FILE, "w");

    if (file == NULL)
    {
        printf("Error: Cannot save configuration.\n");
        return;
    }

    fprintf(file, "%s\n", folder);

    fclose(file);
}

/* ---------- Load Folder Path ---------- */
int load_config(char *folder)
{
    FILE *file = fopen(CONFIG_FILE, "r");

    if (file == NULL)
    {
        return 0;
    }

    if (fgets(folder, MAX_PATH, file) == NULL)
    {
        fclose(file);
        return 0;
    }

    remove_newline(folder);

    fclose(file);

    return 1;
}

/* ---------- Scan Folder ---------- */
int scan_folder(const char *folder, FileRecord records[])
{
    WIN32_FIND_DATAA data;
    HANDLE handle;
    char searchPath[MAX_PATH];

    int count = 0;

    snprintf(searchPath, sizeof(searchPath), "%s\\*", folder);

    handle = FindFirstFileA(searchPath, &data);

    if (handle == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    do
    {
        if (strcmp(data.cFileName, ".") == 0 ||
            strcmp(data.cFileName, "..") == 0)
        {
            continue;
        }

        /* Ignore folders */
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            continue;
        }

        if (count >= MAX_FILES)
        {
            break;
        }

        strncpy(records[count].filename,
                data.cFileName,
                NAME_SIZE - 1);

        records[count].filename[NAME_SIZE - 1] = '\0';

        records[count].size =
            ((unsigned long long)data.nFileSizeHigh << 32) |
            data.nFileSizeLow;

        char fullPath[MAX_PATH];

        snprintf(fullPath,
                 sizeof(fullPath),
                 "%s\\%s",
                 folder,
                 data.cFileName);

        records[count].hash = calculate_hash(fullPath);

        count++;

    } while (FindNextFileA(handle, &data));

    FindClose(handle);

    return count;
}

/* ---------- Save Baseline ---------- */
int save_baseline(FileRecord records[], int count)
{
    FILE *file = fopen(BASELINE_FILE, "wb");

    if (file == NULL)
    {
        printf("Error: Cannot create baseline.db\n");
        return 0;
    }

    fwrite(&count, sizeof(int), 1, file);

    fwrite(records,
           sizeof(FileRecord),
           count,
           file);

    fclose(file);

    return 1;
}

/* ---------- Load Baseline ---------- */
int load_baseline(FileRecord records[])
{
    FILE *file = fopen(BASELINE_FILE, "rb");

    int count;

    if (file == NULL)
    {
        return -1;
    }

    if (fread(&count, sizeof(int), 1, file) != 1)
    {
        fclose(file);
        return -1;
    }

    if (count < 0 || count > MAX_FILES)
    {
        fclose(file);
        return -1;
    }

    if (fread(records,
              sizeof(FileRecord),
              count,
              file) != (size_t)count)
    {
        fclose(file);
        return -1;
    }

    fclose(file);

    return count;
}

/* ---------- Find File in Records ---------- */
int find_file(FileRecord records[],
              int count,
              const char *filename)
{
    int i;

    for (i = 0; i < count; i++)
    {
        if (strcmp(records[i].filename, filename) == 0)
        {
            return i;
        }
    }

    return -1;
}

/* ---------- Write Log ---------- */
void write_log(const char *message)
{
    FILE *file = fopen(LOG_FILE, "a");

    if (file == NULL)
    {
        return;
    }

    fprintf(file, "%s\n", message);

    fclose(file);
}

/* ---------- Create Baseline ---------- */
void create_baseline()
{
    char folder[MAX_PATH];
    FileRecord records[MAX_FILES];

    int count;

    printf("\n=====================================\n");
    printf("         CREATE BASELINE\n");
    printf("=====================================\n");

    printf("Enter folder path to monitor:\n");
    printf("> ");

    fgets(folder, sizeof(folder), stdin);
    remove_newline(folder);

    if (!folder_exists(folder))
    {
        printf("\nError: Folder not found.\n");
        return;
    }

    count = scan_folder(folder, records);

    if (count < 0)
    {
        printf("\nError: Could not scan the folder.\n");
        return;
    }

    if (count == 0)
    {
        printf("\nNo files found in this folder.\n");
        return;
    }

    save_config(folder);

    if (save_baseline(records, count))
    {
        printf("\nBaseline created successfully.\n");
        printf("Files recorded: %d\n", count);
        printf("Folder: %s\n", folder);
    }
}

/* ---------- Audit Folder ---------- */
void audit_folder()
{
    char folder[MAX_PATH];

    FileRecord baseline[MAX_FILES];
    FileRecord current[MAX_FILES];

    int baselineCount;
    int currentCount;

    int i;
    int found;

    char logMessage[600];

    printf("\n=====================================\n");
    printf("              AUDIT\n");
    printf("=====================================\n");

    if (!load_config(folder))
    {
        printf("Error: No monitored folder configured.\n");
        printf("Create a baseline first.\n");
        return;
    }

    baselineCount = load_baseline(baseline);

    if (baselineCount < 0)
    {
        printf("Error: baseline.db not found.\n");
        printf("Create a baseline first.\n");
        return;
    }

    if (!folder_exists(folder))
    {
        printf("Error: Monitored folder no longer exists.\n");
        return;
    }

    currentCount = scan_folder(folder, current);

    if (currentCount < 0)
    {
        printf("Error: Could not scan the folder.\n");
        return;
    }

    printf("\nMonitoring folder:\n%s\n\n", folder);

    /* Check current files */
    for (i = 0; i < currentCount; i++)
    {
        found = find_file(baseline,
                          baselineCount,
                          current[i].filename);

        if (found == -1)
        {
            printf("[NEW FILE]    %s\n",
                   current[i].filename);

            snprintf(logMessage,
                     sizeof(logMessage),
                     "[NEW FILE] %s",
                     current[i].filename);

            write_log(logMessage);
        }
        else
        {
            if (baseline[found].size == current[i].size &&
                baseline[found].hash == current[i].hash)
            {
                printf("[MATCH]       %s\n",
                       current[i].filename);
            }
            else
            {
                printf("[MODIFIED]    %s\n",
                       current[i].filename);

                snprintf(logMessage,
                         sizeof(logMessage),
                         "[MODIFIED] %s",
                         current[i].filename);

                write_log(logMessage);
            }
        }
    }

    /* Check deleted files */
    for (i = 0; i < baselineCount; i++)
    {
        found = find_file(current,
                          currentCount,
                          baseline[i].filename);

        if (found == -1)
        {
            printf("[DELETED]     %s\n",
                   baseline[i].filename);

            snprintf(logMessage,
                     sizeof(logMessage),
                     "[DELETED] %s",
                     baseline[i].filename);

            write_log(logMessage);
        }
    }

    printf("\nAudit completed.\n");
}

/* ---------- View Logs ---------- */
void view_logs()
{
    FILE *file;
    char line[600];

    printf("\n=====================================\n");
    printf("             AUDIT LOG\n");
    printf("=====================================\n");

    file = fopen(LOG_FILE, "r");

    if (file == NULL)
    {
        printf("No audit log found.\n");
        return;
    }

    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
    }

    fclose(file);
}

/* ---------- Reset Baseline ---------- */
void reset_baseline()
{
    remove(BASELINE_FILE);
    remove(CONFIG_FILE);

    printf("\nBaseline and configuration reset successfully.\n");
}

/* ---------- Main Menu ---------- */
int main()
{
    int choice;
    char input[20];

    printf("============================================\n");
    printf(" SentinelC - File Integrity Monitoring Tool\n");
    printf("============================================\n");

    while (1)
    {
        printf("\n");
        printf("1. Create Baseline\n");
        printf("2. Run Audit\n");
        printf("3. View Audit Log\n");
        printf("4. Reset Baseline\n");
        printf("5. Exit\n");

        printf("\nEnter your choice: ");

        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        switch (choice)
        {
            case 1:
                create_baseline();
                break;

            case 2:
                audit_folder();
                break;

            case 3:
                view_logs();
                break;

            case 4:
                reset_baseline();
                break;

            case 5:
                printf("\nSentinelC closed.\n");
                return 0;

            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    }

    return 0;
}
