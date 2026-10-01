/*
========================================================================================
Course: EECS / Data Structures & Algorithms
Assignment: Assignment 3 - Priority Queue & GenAI Code Analysis
File Name: main.cpp
Author: Aadi Patel
Date Created: October 1, 2026
Last Modified: October 1, 2026
Purpose:
    Implements a custom object-oriented Binary Max-Heap priority queue from scratch
    over a dynamic list (std::vector) to manage an executive email inbox for a CEO.
    The system reads commands sequentially from an external text file and processes:
      - EMAIL <sender>, <subject>, <date> : Enqueues an email into the heap.
      - NEXT : Displays the top-priority email without removing it.
      - READ : Removes the top-priority email without displaying it.
      - COUNT : Displays the total count of unread emails.
    
Priority Invariants:
    1. Sender Category Rank: Boss (5) > Subordinate (4) > Peer (3) > ImportantPerson (2) > Other (1)
    2. Date Ordering: More recent calendar date takes precedence (YYYYMMDD integer comparison)
    3. Arrival Ordering: Earliest arrival (lower sequence counter) breaks exact ties (FIFO)

Collaborators & Sources:
    1. Claude (Anthropic): Evaluated baseline implementation (Code 1) for MaxHeap mechanics and parsing.
    2. Gemini (Google): Evaluated comparative implementation (Code 2) for template/date abstractions.
    3. Course Lecture Notes & Standard Algorithm Text: Binary Heap complete tree parent/child indexing formulas.
========================================================================================
*/

#include <iostream>   // Provides std::cout and std::endl for console output operations
#include <fstream>    // Provides std::ifstream to read external command files
#include <sstream>    // Provides std::stringstream for string-to-integer conversion
#include <string>     // Provides std::string data type and operations
#include <vector>     // Provides std::vector used as dynamic array backing the MaxHeap

using namespace std;  // Eliminates repetitive std:: scope resolution prefixes

/* ========================================================================= */
/*                               Email Class                                 */
/* ========================================================================= */

class Email {
private:
    string sender;    // Stores the raw sender category string from input file
    string subject;   // Stores the email subject line
    string date;      // Stores original MM-DD-YYYY date representation for printing
    int rank;         // Numerical priority calculated from the sender category
    int dateKey;      // Normalized integer date (YYYYMMDD) for fast comparison
    long sequence;    // Monotonically increasing sequence number for FIFO tie-breaking

    // Maps sender string to a strictly ordered priority integer (5 down to 1)
    static int rankOf(const string& category) {
        if (category == "Boss") return 5;             // Highest priority tier
        if (category == "Subordinate") return 4;      // Second priority tier
        if (category == "Peer") return 3;             // Third priority tier
        if (category == "ImportantPerson") return 2;  // Fourth priority tier
        return 1;                                     // Lowest priority ("Other Person" / unrecognized)
    }

    // Converts "MM-DD-YYYY" into numeric YYYYMMDD to permit direct integer ordering
    static int keyOf(const string& d) {
        int month = 0;                                // Variable to capture month integer
        int day = 0;                                  // Variable to capture day integer
        int year = 0;                                 // Variable to capture 4-digit year integer
        char dash1 = '\0';                            // Container for first '-' delimiter
        char dash2 = '\0';                            // Container for second '-' delimiter
        stringstream ss(d);                           // Stream to parse tokens from input string
        ss >> month >> dash1 >> day >> dash2 >> year; // Extract fields formatted as MM-DD-YYYY
        return year * 10000 + month * 100 + day;      // Compute comparable integer: YYYYMMDD
    }

public:
    // Default constructor: allows dynamic array pre-allocation and resizing
    Email() : sender(""), subject(""), date(""), rank(0), dateKey(0), sequence(0) {}

    // Parameterized constructor: initializes text and precomputes priority keys
    Email(const string& s, const string& subj, const string& d, long seq)
        : sender(s),                                  // Copy sender string
          subject(subj),                              // Copy subject line
          date(d),                                    // Copy date string
          rank(rankOf(s)),                            // Map category to numerical rank
          dateKey(keyOf(d)),                          // Convert date string to integer key
          sequence(seq) {}                            // Assign arrival order identifier

    // Accessor: returns original sender category
    string getSender() const { return sender; }

    // Accessor: returns email subject line
    string getSubject() const { return subject; }

    // Accessor: returns original formatted date string
    string getDate() const { return date; }

    // Evaluates whether this Email object has higher priority than another
    bool hasHigherPriority(const Email& other) const {
        // Criterion 1: Higher sender category rank takes precedence
        if (rank != other.rank) {
            return rank > other.rank;
        }
        // Criterion 2: If sender ranks match, newer date takes precedence
        if (dateKey != other.dateKey) {
            return dateKey > other.dateKey;
        }
        // Criterion 3: If dates match, earlier arrival order (FIFO) wins
        return sequence < other.sequence;
    }
};

/* ========================================================================= */
/*                              MaxHeap Class                                */
/* ========================================================================= */

class MaxHeap {
private:
    vector<Email> items; // Contiguous array backing the implicit complete binary tree

    // Returns parent node index: (i - 1) / 2 for zero-based arrays
    int parentOf(int i) const { return (i - 1) / 2; }

    // Returns left child index: 2i + 1
    int leftOf(int i) const { return 2 * i + 1; }

    // Returns right child index: 2i + 2
    int rightOf(int i) const { return 2 * i + 2; }

    // Swaps elements at indices a and b
    void swapItems(int a, int b) {
        Email temp = items[a]; // Hold element at index a in temporary variable
        items[a] = items[b];   // Assign element at index b to index a
        items[b] = temp;       // Assign stored temporary element to index b
    }

    // Percolates an element up to restore max-heap ordering invariant: O(log n)
    void siftUp(int i) {
        while (i > 0) {                                      // Continue until root is reached
            int p = parentOf(i);                             // Locate parent index
            if (items[i].hasHigherPriority(items[p])) {      // Check if child outranks parent
                swapItems(i, p);                             // Swap if child is higher priority
                i = p;                                       // Advance current pointer to parent
            } else {
                break;                                       // Invariant satisfied, terminate early
            }
        }
    }

    // Percolates an element down to restore max-heap ordering invariant: O(log n)
    void siftDown(int i) {
        int n = static_cast<int>(items.size());              // Cache heap element count
        while (true) {
            int left = leftOf(i);                            // Calculate left child index
            int right = rightOf(i);                          // Calculate right child index
            int best = i;                                    // Initialize current node as highest

            // If left child exists and outranks current best, designate it as best
            if (left < n && items[left].hasHigherPriority(items[best])) {
                best = left;
            }
            // If right child exists and outranks current best, designate it as best
            if (right < n && items[right].hasHigherPriority(items[best])) {
                best = right;
            }

            // If current node remains highest priority, sift down is complete
            if (best == i) break;

            swapItems(i, best);                              // Swap with higher-ranking child
            i = best;                                        // Advance current pointer to child
        }
    }

public:
    // Returns true if no elements reside in the heap
    bool isEmpty() const { return items.empty(); }

    // Returns the total number of stored elements
    int size() const { return static_cast<int>(items.size()); }

    // Appends new element and bubbles it into place: O(log n)
    void insert(const Email& e) {
        items.push_back(e);                                  // Insert at terminal position
        siftUp(static_cast<int>(items.size()) - 1);          // Restore heap order upward
    }

    // Inspects root element without removal: O(1)
    const Email& peek() const {
        return items[0];                                     // Root always holds the maximum
    }

    // Deletes root element and restructures heap: O(log n)
    void extractMax() {
        items[0] = items.back();                             // Overwrite root with last element
        items.pop_back();                                    // Shrink array by one element
        if (!items.empty()) {                                // Restructure only if elements remain
            siftDown(0);                                     // Sift replacement element down
        }
    }
};

/* ========================================================================= */
/*                          EmailManager Controller                          */
/* ========================================================================= */

class EmailManager {
private:
    MaxHeap inbox;        // Priority queue storing incoming email objects
    long nextSequence;    // Monotonic counter ensuring stable FIFO arrival tracking

    // Strips leading and trailing whitespace characters (spaces, tabs, carriage returns)
    static string trim(const string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");       // Find index of first non-whitespace
        if (start == string::npos) return "";                // Entire string is whitespace
        size_t end = s.find_last_not_of(" \t\r\n");         // Find index of last non-whitespace
        return s.substr(start, end - start + 1);             // Return extracted substring
    }

    // Parses parameters from "EMAIL <sender>, <subject>, <date>" line
    void handleEmail(const string& args) {
        size_t firstComma = args.find(',');                  // Find delimiter between sender & subject
        size_t lastComma = args.rfind(',');                  // Find delimiter between subject & date
        if (firstComma == string::npos || firstComma == lastComma) {
            return;                                          // Skip malformed command lines safely
        }

        string sender = trim(args.substr(0, firstComma));    // Extract and trim sender
        string subject = trim(args.substr(firstComma + 1,    // Extract and trim subject
                                          lastComma - firstComma - 1));
        string date = trim(args.substr(lastComma + 1));      // Extract and trim date

        // Construct Email object and insert into the MaxHeap
        inbox.insert(Email(sender, subject, date, nextSequence++));
    }

    // Displays the current highest-priority email; fails silently on empty queue
    void handleNext() {
        if (inbox.isEmpty()) {
            return;                                          // Silent return on empty inbox
        }
        const Email& top = inbox.peek();                     // Inspect current maximum element
        cout << "Next email:" << endl;                       // Required header line
        cout << "Sender: " << top.getSender() << endl;       // Formatted sender category
        cout << "Subject: " << top.getSubject() << endl;     // Formatted subject text
        cout << "Date: " << top.getDate() << endl;           // Formatted date string
    }

    // Consumes and deletes top email; fails silently on empty queue
    void handleRead() {
        if (inbox.isEmpty()) {
            return;                                          // Silent return on empty inbox
        }
        inbox.extractMax();                                  // Remove highest priority element
    }

    // Prints the exact count of unread emails currently in the inbox
    void handleCount() {
        cout << "There are " << inbox.size() << " emails to read." << endl;
    }

public:
    // Constructor: initializes sequence counter to 0
    EmailManager() : nextSequence(0) {}

    // Processes an external commands file line by line
    void processFile(const string& filename) {
        ifstream in(filename);                               // Open target input file stream
        if (!in) {
            cout << "Error: could not open file " << filename << endl;
            return;                                          // Handle file opening errors gracefully
        }

        string line;                                         // Line buffer container
        while (getline(in, line)) {                          // Read file until EOF
            line = trim(line);                               // Strip surrounding whitespace
            if (line.empty()) continue;                      // Ignore blank lines

            // Command dispatch based on prefix or token match
            if (line.rfind("EMAIL ", 0) == 0) {
                handleEmail(line.substr(6));                 // Delegate payload after "EMAIL "
            } else if (line == "NEXT") {
                handleNext();                                // Delegate NEXT instruction
            } else if (line == "READ") {
                handleRead();                                // Delegate READ instruction
            } else if (line == "COUNT") {
                handleCount();                               // Delegate COUNT instruction
            }
        }
        in.close();                                          // Cleanly close input stream
    }
};

/* ========================================================================= */
/*                               Main Driver                                 */
/* ========================================================================= */

int main(int argc, char* argv[]) {
    // Select input file from CLI parameter if supplied, else fallback to default
    string filename = (argc > 1) ? argv[1] : "commands.txt";

    EmailManager manager;                                    // Initialize manager instance
    manager.processFile(filename);                           // Execute commands workflow

    return 0;                                                // Exit with code 0 (success)
}