 University Course Prerequisite System

A C++17 console application that uses a directed graph to manage university courses and their prerequisite relationships. It supports prerequisite searches, cycle detection, course ordering and CSV-based data persistence.

 1. Project Overview

University courses often require students to complete certain courses before taking others. This system represents these relationships using a graph and helps identify direct and indirect prerequisites, dependent courses and valid course completion orders.

It also prevents circular dependencies that would make course completion impossible.

 2. Project Objectives

- Represent courses and prerequisites using a graph.
- Identify direct and indirect prerequisites and dependents.
- Detect and prevent circular dependencies.
- Generate a valid course completion order.
- Store and retrieve data using CSV files.

3. Technologies Used

| Technology | Purpose |
|---|---|
| C++17 | Programming language |
| Code::Blocks | Development environment |
| Graph (adjacency list) | Represents courses and relationships |
| CSV files | Stores course data permanently |
| C++ standard library | Provides containers, queues and file handling |

 4. Demonstration Dataset

The system starts with five courses and three prerequisite relationships:

- Programming I → Data Structures
- Data Structures → Algorithms
- Database Systems → Advanced Database Systems

These are demonstration relationships, not verified official university requirements.

5. Data Structures and Algorithms

Directed Acyclic Graph (DAG): Represents courses as vertices and prerequisite relationships as directed edges.

Adjacency List: Stores the prerequisites and dependents of each course.

Depth-First Search (DFS): Checks whether adding a prerequisite would create a cycle.

Breadth-First Search (BFS): Identifies direct and indirect prerequisites and dependent courses.

Kahn's Algorithm: Generates a valid course completion order by grouping courses into stages according to their prerequisites.

The main graph operations have a time complexity of \(O(V+E)\), where V represents courses and E represents prerequisite relationships.

6. System Features

.1. Add Course
.2. Add Prerequisite
.3. Remove Course
.4. Display Course Prerequisites
.5. Display Dependent Courses
.6. Display All Courses
.7. Display Complete Graph
.8. Search Course
.9. Display Course Completion Order
.10. Display System Statistics
.11. Load Sample Data
.0. Exit

The system validates inputs, prevents duplicate relationships and automatically saves changes.

 7. Project File Structure

```text
University-Course-Prerequisite-System/
│
├── GROUP18-DSPROJECT.cpp
├── courses.csv
├── prerequisites.csv
└── README.md
```

- GROUP18-DSPROJECT.cpp:Main application source code.
- courses.csv: Stores course names.
- prerequisites.csv: Stores prerequisite relationships.
- README.md: Project documentation.

 8. Installation and Execution

Using Code::Blocks

1. Install Code::Blocks with the MinGW compiler.
2. Open the project and add the C++ source file.
3. Enable the C++17 compiler standard.
4. Ensure both CSV files are in the program's working directory.
5. Build and run using **F9**.

Using a Terminal

Compile:

```bash
g++ -std=c++17 GROUP18-DSPROJECT.cpp -o prerequisite_system
```

Run on Windows:

```bash
prerequisite_system.exe
```

Run on Linux/macOS:

```bash
./prerequisite_system
```

9. Data Persistence

The system loads course information and prerequisite relationships from the CSV files at startup.

Changes made through the application are automatically saved. When the program is reopened, the saved information is loaded to rebuild the graph.

 10. Testing

The application was tested using scripted menu inputs in a Linux environment with the C++17 standard.

Tests covered:

- Loading the demonstration dataset.
- Direct and indirect prerequisite searches.
- Dependent course searches.
- Cycle and duplicate relationship prevention.
- Adding and removing courses.
- Searching and input validation.
- Saving and reloading CSV data.
- Handling missing files and invalid records.

The tests have not yet been repeated in Code::Blocks.

11. Limitations

- Console-based interface.
- Small demonstration dataset.
- No course codes, credit units or semester management.
- CSV storage instead of a database.
- Does not support alternative prerequisites.
- No option to remove an individual prerequisite.
- Not intended for production use.

12. Author and Academic Context

**Programme:** B.Ed Information Technology  
**Course:** Data Structures  
**Project:** GROUP 18 - Project 8 – University Course Prerequisite System

| Group Member | Index Number |
|---|---|
| Lucky Kobby Setsoafia | ET/ICT/24/0056 |
| Manasseh Gyan Mantey | ET/ICT/24/0193 |
| Michael Amissah | ET/ICT/24/0098 |
| Milla Caleb Kwame | ET/ICT/24/0155 |
| Muhammed Mutawakilu | ET/ICT/24/0074 |
