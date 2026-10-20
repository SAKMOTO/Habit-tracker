// habit_tracker.c
// CS50x Final Project - Command-line habit tracker written in C.
//
// NOTE ON AI USE: I used an AI assistant (opencode / big-pickle) as a helper
// to scaffold this project and to review my C code, as permitted for the final
// project by CS50's policy. The design, the feature set, and the logic were
// chosen and understood by me. The assistant's suggestions were checked against
// the course material and modified where needed.
//
// The program stores habits in a plain-text data file (habits.dat) so that
// data persists between runs. Each habit is one line, comma-separated:
//
//     name,streak,total,lastCompleted(YYYY-MM-DD)
//
// Usage:
//     ./habittracker

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_HABITS 100
#define NAME_LEN 64
#define DATA_FILE "habits.dat"

typedef struct
{
    char name[NAME_LEN];
    int streak;
    int total;
    char last_done[11]; // "YYYY-MM-DD" plus null terminator
} habit;

// Global array of habits, plus the number currently in use.
habit habits[MAX_HABITS];
int habit_count = 0;

// --- Date helpers -----------------------------------------------------------

// Fills buffer (size >= 11) with today's date as YYYY-MM-DD.
void today_string(char *buffer)
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    strftime(buffer, 11, "%Y-%m-%d", local);
}

// Returns 1 if two YYYY-MM-DD date strings are consecutive days (b is one day
// after a), else 0.
int is_consecutive(const char *a, const char *b)
{
    struct tm ta = {0}, tb = {0};
    int ya, ma, da, yb, mb, db;
    if (sscanf(a, "%d-%d-%d", &ya, &ma, &da) != 3)
    {
        return 0;
    }
    if (sscanf(b, "%d-%d-%d", &yb, &mb, &db) != 3)
    {
        return 0;
    }
    ta.tm_year = ya - 1900;
    ta.tm_mon = ma - 1;
    ta.tm_mday = da;
    tb.tm_year = yb - 1900;
    tb.tm_mon = mb - 1;
    tb.tm_mday = db;

    time_t time_a = mktime(&ta);
    time_t time_b = mktime(&tb);
    double diff = difftime(time_b, time_a);
    return (diff == 86400.0); // exactly one day apart
}

// --- Persistence ------------------------------------------------------------

// Loads habits from DATA_FILE into the global array. Missing file is not an
// error; it just means we start with no habits.
void load_habits(void)
{
    FILE *file = fopen(DATA_FILE, "r");
    if (file == NULL)
    {
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL && habit_count < MAX_HABITS)
    {
        // Strip trailing newline.
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0)
        {
            continue;
        }

        char name[NAME_LEN];
        int streak, total;
        char last[11] = "";

        // Split on commas. last_done may be empty for a brand-new habit.
        char *first = strchr(line, ',');
        if (first == NULL)
        {
            continue;
        }
        *first = '\0';
        strncpy(name, line, NAME_LEN - 1);
        name[NAME_LEN - 1] = '\0';

        char *second = strchr(first + 1, ',');
        if (second == NULL)
        {
            continue;
        }
        *second = '\0';
        streak = atoi(first + 1);

        char *third = strchr(second + 1, ',');
        if (third == NULL)
        {
            continue;
        }
        *third = '\0';
        total = atoi(second + 1);

        strncpy(last, third + 1, 10);
        last[10] = '\0';

        strncpy(habits[habit_count].name, name, NAME_LEN - 1);
        habits[habit_count].name[NAME_LEN - 1] = '\0';
        habits[habit_count].streak = streak;
        habits[habit_count].total = total;
        strncpy(habits[habit_count].last_done, last, 10);
        habits[habit_count].last_done[10] = '\0';
        habit_count++;
    }

    fclose(file);
}

// Writes all habits back to DATA_FILE, overwriting its contents.
void save_habits(void)
{
    FILE *file = fopen(DATA_FILE, "w");
    if (file == NULL)
    {
        printf("Error: could not save data.\n");
        return;
    }

    for (int i = 0; i < habit_count; i++)
    {
        fprintf(file, "%s,%d,%d,%s\n", habits[i].name, habits[i].streak,
                habits[i].total, habits[i].last_done);
    }

    fclose(file);
}

// --- Core actions -----------------------------------------------------------

// Adds a new habit if it does not already exist.
void add_habit(const char *name)
{
    if (habit_count >= MAX_HABITS)
    {
        printf("You have reached the maximum of %d habits.\n", MAX_HABITS);
        return;
    }

    for (int i = 0; i < habit_count; i++)
    {
        if (strcmp(habits[i].name, name) == 0)
        {
            printf("A habit named \"%s\" already exists.\n", name);
            return;
        }
    }

    strncpy(habits[habit_count].name, name, NAME_LEN - 1);
    habits[habit_count].name[NAME_LEN - 1] = '\0';
    habits[habit_count].streak = 0;
    habits[habit_count].total = 0;
    habits[habit_count].last_done[0] = '\0';
    habit_count++;

    save_habits();
    printf("Added habit \"%s\".\n", name);
}

// Marks a habit complete today, updating its streak and total. If it was
// already done today, nothing changes.
void complete_habit(const char *name)
{
    char today[11];
    today_string(today);

    for (int i = 0; i < habit_count; i++)
    {
        if (strcmp(habits[i].name, name) == 0)
        {
            if (strcmp(habits[i].last_done, today) == 0)
            {
                printf("\"%s\" is already marked done today.\n", name);
                return;
            }

            if (is_consecutive(habits[i].last_done, today))
            {
                habits[i].streak++;
            }
            else
            {
                habits[i].streak = 1; // streak broken (or first ever)
            }
            habits[i].total++;
            strncpy(habits[i].last_done, today, 10);
            habits[i].last_done[10] = '\0';

            save_habits();
            printf("Nice! \"%s\" done. Streak: %d day(s), total: %d.\n",
                   name, habits[i].streak, habits[i].total);
            return;
        }
    }

    printf("No habit named \"%s\" found.\n", name);
}

// Removes a habit by name.
void remove_habit(const char *name)
{
    for (int i = 0; i < habit_count; i++)
    {
        if (strcmp(habits[i].name, name) == 0)
        {
            for (int j = i; j < habit_count - 1; j++)
            {
                habits[j] = habits[j + 1];
            }
            habit_count--;
            save_habits();
            printf("Removed habit \"%s\".\n", name);
            return;
        }
    }

    printf("No habit named \"%s\" found.\n", name);
}

// Prints all habits with their current stats.
void list_habits(void)
{
    char today[11];
    today_string(today);

    printf("\nYour habits (today is %s):\n", today);
    printf("--------------------------------------------------\n");

    if (habit_count == 0)
    {
        printf("  (none yet - add one!)\n");
    }

    for (int i = 0; i < habit_count; i++)
    {
        printf("  %-20s streak: %-3d total: %-3d ", habits[i].name,
               habits[i].streak, habits[i].total);
        if (strcmp(habits[i].last_done, today) == 0)
        {
            printf("[done today]");
        }
        printf("\n");
    }
    printf("--------------------------------------------------\n");
}

// --- User interface ---------------------------------------------------------

void print_help(void)
{
    printf("\nHabit Tracker - commands:\n");
    printf("  add <name>       add a new habit\n");
    printf("  done <name>      mark a habit done for today\n");
    printf("  remove <name>    delete a habit\n");
    printf("  list             show all habits and streaks\n");
    printf("  help             show this help\n");
    printf("  quit             save and exit\n\n");
}

int main(void)
{
    load_habits();

    printf("========================================\n");
    printf("        Welcome to Habit Tracker\n");
    printf("========================================\n");
    print_help();

    char line[256];
    while (1)
    {
        printf("> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            break; // EOF (Ctrl-D)
        }

        // Strip newline.
        line[strcspn(line, "\n")] = '\0';

        // Parse command and the rest as the argument.
        char command[16] = "";
        char argument[NAME_LEN] = "";
        int scanned = sscanf(line, "%15s %63[^\n]", command, argument);

        if (scanned == 0 || strlen(command) == 0)
        {
            continue; // blank line
        }

        // Lowercase the command so ADD/Add/add all work.
        for (int i = 0; command[i]; i++)
        {
            command[i] = tolower((unsigned char) command[i]);
        }

        if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0)
        {
            save_habits();
            printf("Saved. Goodbye!\n");
            break;
        }
        else if (strcmp(command, "help") == 0)
        {
            print_help();
        }
        else if (strcmp(command, "list") == 0)
        {
            list_habits();
        }
        else if (strcmp(command, "add") == 0)
        {
            if (strlen(argument) == 0)
            {
                printf("Usage: add <name>\n");
            }
            else
            {
                add_habit(argument);
            }
        }
        else if (strcmp(command, "done") == 0)
        {
            if (strlen(argument) == 0)
            {
                printf("Usage: done <name>\n");
            }
            else
            {
                complete_habit(argument);
            }
        }
        else if (strcmp(command, "remove") == 0)
        {
            if (strlen(argument) == 0)
            {
                printf("Usage: remove <name>\n");
            }
            else
            {
                remove_habit(argument);
            }
        }
        else
        {
            printf("Unknown command \"%s\". Type \"help\" for options.\n",
                   command);
        }
    }

    return 0;
}
