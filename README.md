# University Course Prerequisite System

A console program written in C++17 that stores university courses and their prerequisites as a graph, and keeps the data in CSV files so it is still there the next time the program is opened.

## 1. Project Overview

Many university courses can only be taken after another course has been passed. For example, a student normally studies Programming I before Data Structures. When there are many courses these rules form a network, and it becomes hard to see in what order courses should be completed, or which courses are affected if one course changes.

This system stores each course and its prerequisite relationships, and then answers questions such as:

- What must be completed before a given course, directly and indirectly?
- Which courses depend on a given course?
- In what order can all the courses be completed without breaking a prerequisite rule?

It also refuses prerequisite rules that would create a loop (for example, Algorithms needing Programming I while Programming I needs Algorithms), because no student could ever start those courses.

## 2. Project Objectives

- Represent university courses using a graph.
- Establish prerequisite relationships between courses.
- Identify direct and indirect prerequisites.
- Identify courses that depend on a particular course.
- Detect circular dependencies.
- Suggest a valid course completion order.
- Store course information permanently using CSV files.

## 3. Technologies Used

| Technology | Use in the project |
|---|---|
| C++ | Programming language |
| C++17 | Language standard used to compile the program |
| Code::Blocks IDE | Development environment for building and running the program |
| Graph data structure | Stores courses (vertices) and prerequisites (directed edges) |
| CSV files | Permanent storage of courses and relationships |

The program uses only the C++ standard library (`vector`, `unordered_map`, `unordered_set`, `queue`, `stack`, `string`, `fstream`).

## 4. Initial Demonstration Dataset

The program starts with five courses:

1. Programming I
2. Data Structures
3. Algorithms
4. Database Systems
5. Advanced Database Systems

and three prerequisite relationships:

- Programming I → Data Structures
- Data Structures → Algorithms
- Database Systems → Advanced Database Systems

```
Programming I  ->  Data Structures  ->  Algorithms

Database Systems  ->  Advanced Database Systems
```

An arrow means the course at the start must be completed before the course at the end.

**Note:** this data is a demonstration set for the project. The relationships are examples and should not be treated as verified, official curriculum requirements of any university.

## 5. Data Structure and Algorithms

### Directed Acyclic Graph (DAG)

Each course is a vertex and each prerequisite is a directed edge from the course that must come first to the course that comes after it. A graph is suitable because a course can have several prerequisites and can be a prerequisite for several other courses. A tree cannot show this because every tree node has only one parent.

The graph must be *acyclic*. If there were a loop, none of the courses in it could ever be started. The program checks every new prerequisite before adding it, so the graph stays a DAG.

### Adjacency list representation

For each course, the program stores only the courses it is directly connected to. This suits a prerequisite network because most courses are connected to just a few others, so a full V × V matrix would be mostly empty.

Every course keeps two lists:

```cpp
struct CourseNode {
    string name;                   // name as typed by the user
    vector<string> prerequisites;  // courses needed BEFORE this one (incoming edges)
    vector<string> dependents;     // courses that need THIS one (outgoing edges)
};

unordered_map<string, CourseNode> graph;   // lookup by course name
```

Both lists are needed so that "show prerequisites" and "show dependents" do not have to search the whole graph. The cost is that every change must be made in both lists. In `addPrerequisite()` both lists are updated together, and `removeCourse()` removes the course from both sides.

Course names are matched without caring about capital letters or extra spaces, so "data structures" and "Data Structures" are the same course.

### Depth-First Search (DFS)

DFS is used for **cycle detection**. See the next section for the details.

### Breadth-First Search (BFS)

BFS is used for **Display Course Prerequisites** and **Display Dependent Courses**. Starting from the chosen course, it follows the prerequisite lists (or the dependent lists) and records how many steps away each course is. Because BFS visits the nearest courses first:

- distance 1 means a **direct** prerequisite (or dependent)
- distance 2 or more means an **indirect** one

Example for Algorithms using the demonstration data: Data Structures is a direct prerequisite, and Programming I is an indirect prerequisite (level 2).

### Cycle detection

Before adding "A is a prerequisite of B", the program asks whether B can already reach A by following the existing arrows. This is a DFS that uses a stack and a visited set. If B can reach A, adding A → B would complete a loop, so the prerequisite is rejected and the graph is left unchanged.

Example: with Programming I → Data Structures → Algorithms in the graph, trying to add Algorithms → Programming I gives:

```
Error: circular dependency! "Programming I" already leads to "Algorithms", so this prerequisite would create a cycle.
```

### Kahn's algorithm (topological sorting)

Used by **Display Course Completion Order**. The program counts how many prerequisites each course has. Courses with none go into Stage 1. After a course is placed, each course that depends on it has one prerequisite fewer; when that count reaches zero, the course moves into the next stage. Courses in the same stage do not depend on each other, so they can be taken in any order or together.

For the demonstration data the output is:

```
Stage 1: Database Systems, Programming I
Stage 2: Advanced Database Systems, Data Structures
Stage 3: Algorithms
```

If some courses were never placed there would be a cycle. This cannot normally happen because cycles are blocked when prerequisites are added, so the program only uses this as a safety check.

## 6. System Features

```
1. Add Course
2. Add Prerequisite
3. Remove Course
4. Display Course Prerequisites
5. Display Dependent Courses
6. Display All Courses
7. Display Complete Graph
8. Search Course
9. Display Course Completion Order
10. Display System Statistics
11. Load Sample Data
0. Exit
```

| Option | What it does |
|---|---|
| 1. Add Course | Adds a new course. Empty names, duplicate names, and names containing a comma or double quote are rejected. Saves `courses.csv`. |
| 2. Add Prerequisite | Asks for the prerequisite course and the dependent course. Rejects courses that do not exist, a course being its own prerequisite, duplicate relationships, and relationships that would create a cycle. Saves `prerequisites.csv`. |
| 3. Remove Course | Removes a course and every relationship that involves it, from both adjacency lists. Saves both CSV files. |
| 4. Display Course Prerequisites | Shows the direct and indirect prerequisites of a course, or says it has none. |
| 5. Display Dependent Courses | Shows the courses that depend directly and indirectly on a course, or says none do. |
| 6. Display All Courses | Lists all courses in alphabetical order with a count. |
| 7. Display Complete Graph | Prints each course followed by the courses that need it (the adjacency list). |
| 8. Search Course | Says whether a course exists and how many direct prerequisites and dependents it has. |
| 9. Display Course Completion Order | Shows the stages produced by Kahn's algorithm. |
| 10. Display System Statistics | Shows the number of courses, the number of relationships, and how many courses have no prerequisites or no dependents. |
| 11. Load Sample Data | Adds the five demonstration courses and three relationships if they are missing. Existing records are not duplicated or overwritten. |
| 0. Exit | Closes the program. |

After every operation the program returns to the menu. Invalid menu choices and non-existent course names show an error message. Course names with spaces are read with `getline()`.

The final source code does not include a separate graph consistency checker, so none is described here.

## 7. Project File Structure

```
project-folder/
|-- DSPROJECT.cpp
|-- courses.csv
|-- prerequisites.csv
`-- README.md
```

| File | Purpose |
|---|---|
| `DSPROJECT.cpp` | The complete source code: the graph class, the algorithms, file loading and saving, and the menu. |
| `courses.csv` | Stores the course names. The first line is the header `Course`. |
| `prerequisites.csv` | Stores the relationships in two columns, `Prerequisite` and `Dependent`. |
| `README.md` | This documentation. |

Contents of the starting CSV files:

`courses.csv`
```
Course
Programming I
Data Structures
Algorithms
Database Systems
Advanced Database Systems
```

`prerequisites.csv`
```
Prerequisite,Dependent
Programming I,Data Structures
Data Structures,Algorithms
Database Systems,Advanced Database Systems
```

## 8. Installation and Execution

### Using Code::Blocks

1. Install Code::Blocks. On Windows, download the installer that includes the MinGW compiler (its file name contains `mingw`) so that a compiler is installed together with the IDE.
2. Put `DSPROJECT.cpp`, `courses.csv` and `prerequisites.csv` together in one folder.
3. In Code::Blocks choose **File → New → Project → Console application**, select **C++**, and create the project in that folder. Remove the default `main.cpp` from the project, then add `DSPROJECT.cpp` (**Project → Add files**).
4. Enable C++17: **Settings → Compiler → Compiler settings → Compiler Flags**, then tick the option for the C++17 ISO standard (`-std=c++17`).
5. Check the working directory: **Project → Properties → Build targets**, and make sure **Execution working directory** is `.` (the project folder). The program opens the CSV files by name only, so they must be in this folder.
6. Press **F9** (Build and run).
7. At the start the program should print:

```
Loading saved data...
Loaded 5 course(s) and 3 relationship(s) from the CSV files.
```

If it shows a warning that a CSV file was not found, the files are not in the working directory. If you run the `.exe` directly from `bin\Debug`, copy the two CSV files into that folder as well.

### Compiling from a terminal (optional)

```
g++ -std=c++17 DSPROJECT.cpp -o prerequisite_system
```

Run it from the folder that contains the CSV files:

- **Windows (Command Prompt):** `prerequisite_system.exe`
- **Linux / macOS:** `./prerequisite_system`

## 9. How Data Persistence Works

**Loading at start-up.** The function `loadFromFiles()` runs once when the program starts.

1. It reads `courses.csv` with `getline()` and calls `addCourse()` for each line.
2. It then reads `prerequisites.csv`, splits each line at the comma, and calls `addPrerequisite()`.

Courses are loaded first because a relationship can only be added when both courses already exist. Because the file rows go through the same functions as menu input, they are checked by the same rules: no duplicate courses, no duplicate relationships, both courses must exist, and no circular dependencies. A row that breaks a rule is skipped, and the program shows the line number and the reason. This rebuilds the graph and both adjacency lists.

**Saving after changes.** A variable called `autoSave` is false while loading (so reading never rewrites the files) and becomes true afterwards.

| Action | File rewritten |
|---|---|
| Add course | `courses.csv` |
| Add prerequisite | `prerequisites.csv` |
| Remove course | both files, because the course and its relationships are removed |

Each save rewrites the whole file from the current graph, so the file always matches what is in memory.

**Missing or unreadable files.** If a CSV file cannot be opened, the program shows a warning and starts with an empty graph instead of stopping. The file is created at the next save, and option 11 can load the five demonstration courses. If a file cannot be written, a warning is shown and the change is kept only until the program closes.

**CSV format.** Course names cannot contain commas or double quotes, because these would break the simple CSV format.

## 10. Testing

The tests below were run by feeding scripted menu input to the compiled program in a Linux environment (compiled with `g++ -std=c++17 -Wall -Wextra -pedantic`, no warnings). They have **not** yet been repeated inside Code::Blocks. The last column is left for the author to fill in after running the same tests there.

| # | Test | Result of scripted run (Linux) | Repeated in Code::Blocks |
|---|---|---|---|
| 1 | Load the five demonstration courses | Passed: 5 courses loaded | [ ] |
| 2 | Load the three prerequisite relationships | Passed: 3 relationships loaded | [ ] |
| 3 | Direct and indirect prerequisites of Algorithms | Passed: direct Data Structures, indirect Programming I (level 2) | [ ] |
| 4 | Dependent courses of Programming I | Passed: direct Data Structures, indirect Algorithms (level 2) | [ ] |
| 5 | Prevent a duplicate course (including different capital letters) | Passed: rejected | [ ] |
| 6 | Prevent a circular dependency (Algorithms → Programming I) | Passed: rejected | [ ] |
| 7 | Remove a course that has relationships | Passed: course and its relationships removed, both CSV files updated | [ ] |
| 8 | Close and reopen the program | Passed: five courses and three relationships were loaded again with the CSV files unchanged; a course and a relationship added in an earlier run were still present after reopening | [ ] |

Other checks that were run: duplicate relationship, a course as its own prerequisite, a prerequisite with a course that does not exist, a course name containing a comma, searching for a course that does not exist, displaying the graph with no courses, running option 11 when the data already exists (nothing duplicated), missing CSV files, CSV files containing invalid rows, and a CSV file that could not be written.

The tests used small data sets only. No large-scale or performance testing was done.

## 11. Limitations

- The starting dataset is small and is meant for demonstration.
- The application is console-based and has no graphical interface.
- CSV storage is enough for this project but is not a replacement for a database management system. The whole file is rewritten at every save, and an interruption during writing could damage a file.
- Only course names are stored. There are no course codes, credit units or semesters.
- The completion order only reflects prerequisite relationships. It is not an official academic timetable.
- All prerequisites are treated as required. "Course A or Course B" is not supported.
- There is no separate option for removing a single prerequisite. Relationships are removed only when a course is removed.
- Course names cannot contain commas or double quotes.
- The software is a student project and is not production-ready.

## 12. Possible Future Improvements

- A graphical user interface.
- Database integration instead of CSV files.
- Course registration support.
- Student-specific course progression (checking what a student has completed).
- Exporting prerequisite reports.
- An option to remove a single prerequisite.

## 13. Author and Academic Context

This is a Level 300 Information Technology project for the Data Structures course (Project 8: University Course Prerequisite System).

| Item | Details |
|---|---|
| Student name | [Student name] |
| Index / student number | [Index number] |
| Programme | [Programme of study] |
| Course code | [Course code] |
| Lecturer | [Lecturer's name] |
| Institution | [Institution name] |
| Group members (if any) | [Names, or "Individual project"] |
| Submission date | [Date] |
