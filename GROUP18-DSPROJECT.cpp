// ============================================================================
//  Project 8: University Course Prerequisite System
//  Data Structures project (C++17)
//
//  How the data is stored
//  ----------------------
//  The courses and their prerequisites are stored as a directed graph.
//    - each course is a vertex
//    - "A -> B" is an edge meaning A must be passed before B can be taken
//  The graph is stored with adjacency lists. Every course keeps TWO lists:
//    prerequisites : the courses that point into it (what must come before)
//    dependents    : the courses it points to       (what comes after)
//  I keep both so that "show prerequisites" does not have to search the whole
//  graph. The price is that every add/remove must update both lists, so the
//  code always changes them together.
//
//  Course names are matched without caring about capital letters or extra
//  spaces ("data  structures" is the same course as "Data Structures"), so
//  each course is stored under a lower-case "key" and keeps its original
//  spelling for display.
//
//  Saving to files
//  ---------------
//  courses.csv       -> one course per line (first line is a header)
//  prerequisites.csv -> "Prerequisite,Dependent" on each line
//  The files are read when the program starts and rewritten after every
//  successful change, so the data is still there next time the program runs.
//  The files are opened by name only, so they must be in the working
//  directory of the program (the project folder in Code::Blocks).
// ============================================================================

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

const string COURSES_FILE = "courses.csv";
const string PREREQUISITES_FILE = "prerequisites.csv";

// ----------------------------------------------------------------------------
//  Helper functions for names
// ----------------------------------------------------------------------------

// Removes spaces at the start and end and squeezes repeated spaces into one,
// so "  Data    Structures " becomes "Data Structures".
string cleanName(const string& raw) {
    string result = "";
    bool needSpace = false;
    for (unsigned char ch : raw) {
        if (isspace(ch)) {
            if (!result.empty()) needSpace = true;
        } else {
            if (needSpace) result += ' ';
            needSpace = false;
            result += static_cast<char>(ch);
        }
    }
    return result;
}

// Lower-case version of the cleaned name. This is what the graph uses to
// find a course.
string makeKey(const string& raw) {
    string key = cleanName(raw);
    for (char& c : key) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return key;
}

// Reads a text file into a vector of lines. Returns false if the file cannot
// be opened.
bool readLinesFromFile(const string& fileName, vector<string>& lines) {
    ifstream file(fileName);
    if (!file.is_open()) return false;

    string line;
    while (getline(file, line)) {
        // A file saved by Notepad as "UTF-8 with BOM" starts with 3 hidden
        // bytes. They would end up inside the first course name, so remove them.
        if (lines.empty() && line.size() >= 3 && line.compare(0, 3, "\xEF\xBB\xBF") == 0)
            line.erase(0, 3);
        // Remove the carriage return if the file has Windows line endings.
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    return true;
}

// ----------------------------------------------------------------------------
//  The graph class
// ----------------------------------------------------------------------------
class CoursePrerequisiteSystem {
private:
    struct CourseNode {
        string name;                   // name as the user typed it
        vector<string> prerequisites;  // keys of courses needed BEFORE this one
        vector<string> dependents;     // keys of courses that need THIS one
    };

    unordered_map<string, CourseNode> graph;  // key -> course
    int edgeCount = 0;                        // number of prerequisite relationships

    bool autoSave = false;   // off while loading, so loading never rewrites the files
    bool quietMode = false;  // when true, messages are stored but not printed
    string lastMessage;      // the last message (used to explain skipped CSV rows)

    // ---- small helpers -----------------------------------------------------

    // Prints a message normally. While loading from the CSV files we stay
    // quiet and only keep the message, so the loader can report bad rows in
    // its own way.
    void report(const string& text) {
        lastMessage = text;
        if (!quietMode) cout << text << "\n";
    }

    bool exists(const string& key) const { return graph.find(key) != graph.end(); }

    const string& displayName(const string& key) const { return graph.at(key).name; }

    static bool contains(const vector<string>& list, const string& key) {
        return find(list.begin(), list.end(), key) != list.end();
    }

    static void eraseFrom(vector<string>& list, const string& key) {
        list.erase(remove(list.begin(), list.end(), key), list.end());
    }

    // The unordered_map has no fixed order, so sort the keys to get the same
    // output every time.
    vector<string> sortedKeys() const {
        vector<string> keys;
        for (const auto& entry : graph) keys.push_back(entry.first);
        sort(keys.begin(), keys.end());
        return keys;
    }

    // ---- DFS: used for cycle detection ---------------------------------------
    // Follows the "dependents" arrows from start and checks whether target can
    // be reached. If B can already reach A, then adding A -> B would make a
    // loop, so this tells us whether a new edge is safe.
    bool pathExists(const string& start, const string& target) const {
        unordered_set<string> visited;
        stack<string> toVisit;
        toVisit.push(start);

        while (!toVisit.empty()) {
            string current = toVisit.top();
            toVisit.pop();

            if (current == target) return true;
            if (visited.count(current)) continue;  // already checked this one
            visited.insert(current);

            for (const string& next : graph.at(current).dependents)
                toVisit.push(next);
        }
        return false;
    }

    // ---- BFS: finds all related courses with their distance --------------------
    // If followPrerequisites is true we walk backwards (what must come before),
    // otherwise forwards (what comes after). Distance 1 means direct, 2 or more
    // means indirect. BFS is used because it reaches the nearest courses first,
    // which is exactly what we need to separate direct from indirect.
    vector<pair<string, int>> bfsRelated(const string& start, bool followPrerequisites) const {
        vector<pair<string, int>> found;
        unordered_set<string> visited;
        queue<pair<string, int>> toVisit;

        visited.insert(start);
        toVisit.push(make_pair(start, 0));

        while (!toVisit.empty()) {
            pair<string, int> front = toVisit.front();
            toVisit.pop();

            const CourseNode& node = graph.at(front.first);
            const vector<string>& neighbours =
                followPrerequisites ? node.prerequisites : node.dependents;

            for (const string& next : neighbours) {
                if (visited.count(next)) continue;  // reached through another path already
                visited.insert(next);
                found.push_back(make_pair(next, front.second + 1));
                toVisit.push(make_pair(next, front.second + 1));
            }
        }
        return found;
    }

    // Prints the direct and indirect prerequisites (or dependents) of a course.
    void showRelated(const string& key, bool followPrerequisites) const {
        string label = followPrerequisites ? "prerequisites" : "dependent courses";
        vector<pair<string, int>> related = bfsRelated(key, followPrerequisites);

        vector<string> direct, indirect;
        for (const auto& item : related) {
            if (item.second == 1)
                direct.push_back(displayName(item.first));
            else
                indirect.push_back(displayName(item.first) + " (level " + to_string(item.second) + ")");
        }
        sort(direct.begin(), direct.end());
        sort(indirect.begin(), indirect.end());

        cout << "\n" << (followPrerequisites ? "Prerequisites" : "Dependent courses")
             << " of \"" << displayName(key) << "\":\n";

        if (direct.empty()) {
            if (followPrerequisites)
                cout << "  This course has no prerequisites.\n";
            else
                cout << "  No course depends on this course.\n";
            return;
        }

        cout << "  Direct " << label << ":\n";
        for (const string& name : direct) cout << "    - " << name << "\n";

        if (indirect.empty()) {
            cout << "  Indirect " << label << ": none\n";
        } else {
            cout << "  Indirect " << label << ":\n";
            for (const string& name : indirect) cout << "    - " << name << "\n";
        }
    }

    // ---- saving to the CSV files -----------------------------------------------

    bool saveCourses() const {
        ofstream file(COURSES_FILE);
        if (!file.is_open()) {
            cout << "Warning: could not open " << COURSES_FILE
                 << " for saving. The change is only kept until the program closes.\n";
            return false;
        }
        file << "Course\n";
        for (const string& key : sortedKeys()) file << displayName(key) << "\n";
        if (!file) {
            cout << "Warning: a problem happened while writing " << COURSES_FILE << ".\n";
            return false;
        }
        cout << "  [saved to " << COURSES_FILE << "]\n";
        return true;
    }

    bool savePrerequisites() const {
        ofstream file(PREREQUISITES_FILE);
        if (!file.is_open()) {
            cout << "Warning: could not open " << PREREQUISITES_FILE
                 << " for saving. The change is only kept until the program closes.\n";
            return false;
        }
        file << "Prerequisite,Dependent\n";
        // One line per edge: go through every course and write its outgoing edges.
        for (const string& key : sortedKeys()) {
            vector<string> names;
            for (const string& d : graph.at(key).dependents) names.push_back(displayName(d));
            sort(names.begin(), names.end());
            for (const string& name : names) file << displayName(key) << "," << name << "\n";
        }
        if (!file) {
            cout << "Warning: a problem happened while writing " << PREREQUISITES_FILE << ".\n";
            return false;
        }
        cout << "  [saved to " << PREREQUISITES_FILE << "]\n";
        return true;
    }

    void saveAll() const {
        saveCourses();
        savePrerequisites();
    }

public:
    // ---- A. Add Course ---------------------------------------------------------
    bool addCourse(const string& rawName) {
        string name = cleanName(rawName);
        if (name.empty()) {
            report("Error: course name cannot be empty.");
            return false;
        }
        // A comma would split the name into two columns in the CSV file.
        if (name.find(',') != string::npos || name.find('"') != string::npos) {
            report("Error: course names cannot contain commas or double quotes.");
            return false;
        }
        string key = makeKey(name);
        if (exists(key)) {
            report("Error: course \"" + displayName(key) + "\" already exists.");
            return false;
        }

        graph[key] = CourseNode{name, {}, {}};
        report("Success: course \"" + name + "\" added.");
        if (autoSave) saveCourses();
        return true;
    }

    // ---- B. Add Prerequisite (edge: prerequisite -> dependent) --------------------
    bool addPrerequisite(const string& rawPrereq, const string& rawDependent) {
        string pre = makeKey(rawPrereq);
        string dep = makeKey(rawDependent);

        if (!exists(pre)) {
            report("Error: prerequisite course \"" + cleanName(rawPrereq) + "\" does not exist.");
            return false;
        }
        if (!exists(dep)) {
            report("Error: dependent course \"" + cleanName(rawDependent) + "\" does not exist.");
            return false;
        }
        if (pre == dep) {
            report("Error: a course cannot be its own prerequisite.");
            return false;
        }
        if (contains(graph.at(dep).prerequisites, pre)) {
            report("Error: \"" + displayName(pre) + "\" is already a prerequisite of \"" +
                   displayName(dep) + "\".");
            return false;
        }
        // Check for a cycle BEFORE changing anything, so the graph is never
        // left in a bad state.
        if (pathExists(dep, pre)) {
            report("Error: circular dependency! \"" + displayName(dep) + "\" already leads to \"" +
                   displayName(pre) + "\", so this prerequisite would create a cycle.");
            return false;
        }

        // Update both adjacency lists so they always agree with each other.
        graph.at(dep).prerequisites.push_back(pre);
        graph.at(pre).dependents.push_back(dep);
        edgeCount++;

        report("Success: " + displayName(pre) + " -> " + displayName(dep) + " added.");
        if (autoSave) savePrerequisites();
        return true;
    }

    // ---- C. Remove Course ------------------------------------------------------
    bool removeCourse(const string& rawName) {
        string key = makeKey(rawName);
        if (!exists(key)) {
            report("Error: course \"" + cleanName(rawName) + "\" does not exist.");
            return false;
        }

        CourseNode& node = graph.at(key);
        int removedEdges = static_cast<int>(node.prerequisites.size() + node.dependents.size());

        // If we only erased the course from the map, the other courses would
        // still have its key in their lists. So take it out of those lists first.
        for (const string& p : node.prerequisites) eraseFrom(graph.at(p).dependents, key);
        for (const string& d : node.dependents) eraseFrom(graph.at(d).prerequisites, key);

        string name = node.name;
        graph.erase(key);
        edgeCount -= removedEdges;

        report("Success: course \"" + name + "\" removed along with " +
               to_string(removedEdges) + " relationship(s).");
        if (autoSave) saveAll();  // both the course and its relationships changed
        return true;
    }

    // ---- D. Display Prerequisites ----------------------------------------------
    void displayPrerequisites(const string& rawName) const {
        string key = makeKey(rawName);
        if (!exists(key)) {
            cout << "Error: course \"" << cleanName(rawName) << "\" does not exist.\n";
            return;
        }
        showRelated(key, true);
    }

    // ---- E. Display Dependent Courses ------------------------------------------
    void displayDependents(const string& rawName) const {
        string key = makeKey(rawName);
        if (!exists(key)) {
            cout << "Error: course \"" << cleanName(rawName) << "\" does not exist.\n";
            return;
        }
        showRelated(key, false);
    }

    // ---- Extra features ----------------------------------------------------------
    void displayAllCourses() const {
        if (graph.empty()) {
            cout << "No courses have been registered.\n";
            return;
        }
        cout << "\nRegistered courses (" << graph.size() << "):\n";
        int number = 1;
        for (const string& key : sortedKeys()) {
            cout << "  " << number << ". " << displayName(key) << "\n";
            number++;
        }
    }

    // Prints the adjacency list: each course followed by the courses that need it.
    void displayGraph() const {
        if (graph.empty()) {
            cout << "The graph is empty. Add some courses first.\n";
            return;
        }
        cout << "\nComplete prerequisite graph (course -> courses that need it):\n";
        for (const string& key : sortedKeys()) {
            vector<string> names;
            for (const string& d : graph.at(key).dependents) names.push_back(displayName(d));
            sort(names.begin(), names.end());

            cout << "  " << displayName(key) << " -> ";
            if (names.empty()) cout << "(none)";
            for (size_t i = 0; i < names.size(); i++) {
                if (i > 0) cout << ", ";
                cout << names[i];
            }
            cout << "\n";
        }
    }

    void searchCourse(const string& rawName) const {
        string key = makeKey(rawName);
        if (!exists(key)) {
            cout << "Course \"" << cleanName(rawName) << "\" was NOT found.\n";
            return;
        }
        const CourseNode& node = graph.at(key);
        cout << "Course found: " << node.name << "\n"
             << "  Direct prerequisites: " << node.prerequisites.size() << "\n"
             << "  Direct dependents   : " << node.dependents.size() << "\n";
    }

    // ---- Topological sort (Kahn's algorithm) ------------------------------------
    // Count how many prerequisites each course has. Courses with 0 can be taken
    // first (stage 1). When a course is "taken", every course that depended on
    // it has one prerequisite fewer; when that reaches 0 it moves to the next
    // stage. I print it stage by stage because courses in the same stage do not
    // depend on each other.
    void displayCompletionOrder() const {
        if (graph.empty()) {
            cout << "No courses to order. Add some courses first.\n";
            return;
        }

        unordered_map<string, int> prereqLeft;
        vector<string> currentStage;
        for (const auto& entry : graph) {
            prereqLeft[entry.first] = static_cast<int>(entry.second.prerequisites.size());
            if (entry.second.prerequisites.empty()) currentStage.push_back(entry.first);
        }

        cout << "\nSuggested course completion order:\n";
        size_t processed = 0;
        int stageNumber = 1;

        while (!currentStage.empty()) {
            sort(currentStage.begin(), currentStage.end());
            vector<string> nextStage;

            cout << "  Stage " << stageNumber << ": ";
            for (size_t i = 0; i < currentStage.size(); i++) {
                if (i > 0) cout << ", ";
                cout << displayName(currentStage[i]);
                processed++;

                for (const string& d : graph.at(currentStage[i]).dependents) {
                    prereqLeft[d]--;
                    if (prereqLeft[d] == 0) nextStage.push_back(d);
                }
            }
            cout << "\n";

            currentStage = nextStage;
            stageNumber++;
        }

        // This should never happen because cycles are blocked when edges are
        // added. It is only a safety check.
        if (processed != graph.size())
            cout << "Warning: a cycle was detected in the graph!\n";
        cout << "  (Courses in the same stage can be taken in any order or together.)\n";
    }

    void displayStatistics() const {
        int noPrereq = 0, noDependents = 0;
        for (const auto& entry : graph) {
            if (entry.second.prerequisites.empty()) noPrereq++;
            if (entry.second.dependents.empty()) noDependents++;
        }
        cout << "\nSystem statistics:\n"
             << "  Total courses                   : " << graph.size() << "\n"
             << "  Total prerequisite relationships: " << edgeCount << "\n"
             << "  Courses with no prerequisites   : " << noPrereq << "\n"
             << "  Courses with no dependents      : " << noDependents << "\n";
    }

    // ---- Sample data (exactly 5 courses and 3 relationships) -----------------------
    // Anything that already exists is skipped, so running this again never
    // creates duplicates or overwrites the user's data.
    void loadSampleData() {
        vector<string> courses = {"Programming I", "Data Structures", "Algorithms",
                                  "Database Systems", "Advanced Database Systems"};
        vector<pair<string, string>> links = {
            {"Programming I", "Data Structures"},
            {"Data Structures", "Algorithms"},
            {"Database Systems", "Advanced Database Systems"}};

        bool oldAutoSave = autoSave;
        autoSave = false;   // save once at the end instead of after every record
        quietMode = true;   // no message for each record

        int coursesAdded = 0, coursesSkipped = 0, linksAdded = 0, linksSkipped = 0;
        for (const string& c : courses) {
            if (addCourse(c)) coursesAdded++;
            else coursesSkipped++;
        }
        for (const auto& link : links) {
            if (addPrerequisite(link.first, link.second)) linksAdded++;
            else linksSkipped++;
        }

        quietMode = false;
        autoSave = oldAutoSave;

        cout << "Sample data: " << coursesAdded << " course(s) added, " << coursesSkipped
             << " already present; " << linksAdded << " relationship(s) added, "
             << linksSkipped << " already present or not allowed.\n";
        if (autoSave && (coursesAdded > 0 || linksAdded > 0)) saveAll();
    }

    // ---- Loading from the CSV files (called once when the program starts) ---------
    // The rows are added with the normal addCourse() and addPrerequisite()
    // functions. That way a file that was edited by hand still has to obey the
    // same rules: no duplicates, no missing courses, no cycles. Courses are read
    // first because an edge can only be added when both courses already exist.
    void loadFromFiles() {
        autoSave = false;   // do not rewrite the files while we are reading them
        quietMode = true;

        int coursesLoaded = 0, linksLoaded = 0, skipped = 0;
        vector<string> lines;

        cout << "Loading saved data...\n";

        // --- courses.csv ---
        if (!readLinesFromFile(COURSES_FILE, lines)) {
            cout << "Warning: " << COURSES_FILE << " was not found or cannot be read "
                 << "(it must be in the program's working directory).\n"
                 << "         Starting with no courses. Add a course, or use option 11 "
                 << "to load the five sample courses.\n";
        } else {
            for (size_t i = 0; i < lines.size(); i++) {
                string name = cleanName(lines[i]);
                if (name.empty()) continue;                          // blank line
                if (i == 0 && makeKey(name) == "course") continue;   // header line

                if (addCourse(name)) {
                    coursesLoaded++;
                } else {
                    skipped++;
                    cout << "  Skipped " << COURSES_FILE << " line " << (i + 1) << ": "
                         << lastMessage << "\n";
                }
            }
        }

        // --- prerequisites.csv ---
        lines.clear();
        if (!readLinesFromFile(PREREQUISITES_FILE, lines)) {
            cout << "Warning: " << PREREQUISITES_FILE
                 << " was not found or cannot be read. Starting with no relationships.\n";
        } else {
            for (size_t i = 0; i < lines.size(); i++) {
                if (cleanName(lines[i]).empty()) continue;           // blank line

                // Each line must have exactly one comma (two columns).
                size_t comma = lines[i].find(',');
                if (comma == string::npos || lines[i].find(',', comma + 1) != string::npos) {
                    skipped++;
                    cout << "  Skipped " << PREREQUISITES_FILE << " line " << (i + 1)
                         << ": expected two columns (Prerequisite,Dependent).\n";
                    continue;
                }
                string pre = lines[i].substr(0, comma);
                string dep = lines[i].substr(comma + 1);

                if (i == 0 && makeKey(pre) == "prerequisite" && makeKey(dep) == "dependent")
                    continue;                                        // header line

                if (addPrerequisite(pre, dep)) {
                    linksLoaded++;
                } else {
                    skipped++;
                    cout << "  Skipped " << PREREQUISITES_FILE << " line " << (i + 1) << ": "
                         << lastMessage << "\n";
                }
            }
        }

        cout << "Loaded " << coursesLoaded << " course(s) and " << linksLoaded
             << " relationship(s) from the CSV files";
        if (skipped > 0) cout << " (" << skipped << " row(s) skipped)";
        cout << ".\n";

        quietMode = false;
        autoSave = true;    // from now on every change is saved
    }
};

// ----------------------------------------------------------------------------
//  Menu and main
// ----------------------------------------------------------------------------

// Shows a prompt and reads a whole line, so names with spaces work.
// Returns false if the input has ended.
bool readLine(const string& prompt, string& text) {
    cout << prompt;
    return static_cast<bool>(getline(cin, text));
}

void printMenu() {
    cout << "\n========================================\n"
         << " UNIVERSITY COURSE PREREQUISITE SYSTEM\n"
         << "========================================\n\n"
         << " 1. Add Course\n"
         << " 2. Add Prerequisite\n"
         << " 3. Remove Course\n"
         << " 4. Display Course Prerequisites\n"
         << " 5. Display Dependent Courses\n"
         << " 6. Display All Courses\n"
         << " 7. Display Complete Graph\n"
         << " 8. Search Course\n"
         << " 9. Display Course Completion Order\n"
         << "10. Display System Statistics\n"
         << "11. Load Sample Data\n"
         << " 0. Exit\n\n";
}

int main() {
    CoursePrerequisiteSystem courseSystem;
    string line;

    courseSystem.loadFromFiles();   // rebuild the graph from the CSV files

    while (true) {
        printMenu();
        if (!readLine("Enter your choice: ", line)) break;

        // The choice must be a number of one or two digits.
        string choiceText = cleanName(line);
        bool validNumber = !choiceText.empty() && choiceText.size() <= 2;
        for (char c : choiceText)
            if (!isdigit(static_cast<unsigned char>(c))) validNumber = false;
        if (!validNumber) {
            cout << "Invalid choice. Please enter a number from the menu.\n";
            continue;
        }
        int choice = stoi(choiceText);

        string first, second;
        switch (choice) {
            case 1:
                if (!readLine("Enter new course name: ", first)) return 0;
                courseSystem.addCourse(first);
                break;
            case 2:
                courseSystem.displayAllCourses();
                if (!readLine("Enter PREREQUISITE course (must be taken first): ", first)) return 0;
                if (!readLine("Enter DEPENDENT course (taken after it): ", second)) return 0;
                courseSystem.addPrerequisite(first, second);
                break;
            case 3:
                if (!readLine("Enter course name to remove: ", first)) return 0;
                courseSystem.removeCourse(first);
                break;
            case 4:
                if (!readLine("Enter course name: ", first)) return 0;
                courseSystem.displayPrerequisites(first);
                break;
            case 5:
                if (!readLine("Enter course name: ", first)) return 0;
                courseSystem.displayDependents(first);
                break;
            case 6:
                courseSystem.displayAllCourses();
                break;
            case 7:
                courseSystem.displayGraph();
                break;
            case 8:
                if (!readLine("Enter course name to search: ", first)) return 0;
                courseSystem.searchCourse(first);
                break;
            case 9:
                courseSystem.displayCompletionOrder();
                break;
            case 10:
                courseSystem.displayStatistics();
                break;
            case 11:
                courseSystem.loadSampleData();
                break;
            case 0:
                cout << "Thank you for using the system. Goodbye!\n";
                return 0;
            default:
                cout << "Invalid choice. Please enter a number from the menu.\n";
        }
    }
    return 0;
}
