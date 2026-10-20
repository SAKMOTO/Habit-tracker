# Habit Tracker

#### Video Demo: <https://youtu.be/YOUR_VIDEO_ID>

#### Description:

Habit Tracker is a command-line application written in C that helps a person
build and keep good habits by tracking daily streaks and total completions over
time. It was built as the final project for CS50x. The idea came from a real
problem I face every semester: I start habits like reading or exercising, lose
track of whether I actually did them, and quietly give up. Most habit apps are
phone apps with accounts, notifications, and a lot of friction. I wanted
something small, keyboard-driven, and completely local, so that starting a habit
takes seconds and checking my progress takes even less.

When the program starts it prints a short welcome banner and a help menu, then
shows a `>` prompt. The program reads one command per line. All of the state
lives in memory while it runs and is written back to disk every time something
changes, so the user never has to remember to "save."

The supported commands are:

- `add <name>` — create a new habit. Duplicate names are rejected, and there is
  a cap of 100 habits.
- `done <name>` — mark a habit as completed for today. This is the heart of the
  program: it updates the streak and the total.
- `remove <name>` — delete a habit permanently.
- `list` — print every habit with its current streak, its all-time total, and a
  `[done today]` marker when applicable.
- `help` — reprint the command reference.
- `quit` — save and exit.

The streak logic is what makes the tool interesting. Each habit stores the date
it was last completed as a `YYYY-MM-DD` string. When the user marks a habit done,
the program compares the stored date to today's date. If they are exactly one
day apart, the streak increases by one. If they are the same day, the program
refuses to double-count and tells the user it was already done today. Otherwise
the streak resets to 1, because at least one day was missed. This means a user
who completes a habit every day sees the number climb, while a missed day
honestly resets the counter. I considered using weekdays only or a "grace period"
so a single missed day would not break a streak, but I decided a strict streak is
simpler to explain and more motivating, and I made that trade-off deliberately.

All data is stored in a plain-text file named `habits.dat` next to the program.
Each line represents one habit in a comma-separated format:
`name,streak,total,lastCompleted`. I chose a text format over a binary format so
that the file stays human-readable and easy to debug by hand, and over a database
so the project has no external dependencies and compiles anywhere GCC is
available. A habit name that contains spaces works fine because only the first
comma separates fields.

The project is split into four logical parts inside a single source file,
`habit_tracker.c`, so it reads top to bottom:

1. **Date helpers** (`today_string`, `is_consecutive`) use `time.h` to get the
   current date and to determine whether two dates are exactly one day apart.
2. **Persistence** (`load_habits`, `save_habits`) read and write `habits.dat`. A
   missing file is treated as "no habits yet" rather than an error, which makes
   the first run pleasant.
3. **Core actions** (`add_habit`, `complete_habit`, `remove_habit`,
   `list_habits`) implement the actual behavior and keep the streak and total
   math in one place.
4. **User interface** (`main`, `print_help`) parses input, normalizes the command
   to lowercase so `ADD`, `Add`, and `add` all behave the same, and dispatches to
   the right function.

Writing this taught me several lessons. I had to think carefully about strings in
C, since there is no built-in string type; every field is a fixed-size `char`
array inside a `habit` struct, and I used `strncpy` with explicit null
terminators to avoid buffer overflows. I also learned how `sscanf` can split a
line into a command plus a free-text argument that may itself contain spaces, and
how `mktime` and `difftime` turn two dates into a day count. Testing edge cases —
completing the same habit twice in a day, removing a habit that does not exist,
and running the program with no data file — made the program much more robust
than my first draft.

The build is trivial: `make` compiles `habit_tracker.c` into an executable called
`habittracker` using `-Wall -Wextra -Werror`, so warnings are treated as errors.
Run the program with `./habittracker`.

**Note on AI assistance:** As permitted for the CS50 final project, I used an AI
assistant to help scaffold the file and review error handling. The feature set,
the streak design decision, the data format, and the overall structure were
chosen and understood by me, and I verified the behavior with tests.
