# ADDS Lab Exam — Set Theory on Linguistic Structures

Lab work for the Algorithmics and Dynamic Data Structures course (ADDS).  
The goal is to represent text paragraphs as sets using dynamic data structures in C,
and to implement the classic set operations: union, intersection, and difference.

---

## Authors

ABDELMOUMENE Hamza Ayoub & TALBI Baghdad  
First Year Preparatory Cycle - 1CP  
Ecole Superieure d'Informatique (ESI), Algiers  
Academic Year: 2025–2026

---

## What this project does

You give it one or two plain text files. It reads them, splits them into paragraphs,
and stores each paragraph as a Binary Search Tree of unique words (so it naturally
behaves like a mathematical set). Then you can run union, intersection, or difference
on any two paragraphs, either from the same file or from two different files.

The program runs entirely in the terminal with a simple menu.

---

## Data structures used

- **BST (Binary Search Tree)** — one tree per paragraph, each node is a unique word.
  Chosen because insert and search are O(log n), and duplicates are rejected naturally,
  which matches the definition of a set.

- **Linked List** — chains the paragraphs of a file together dynamically.
  Chosen because the number of paragraphs is not known in advance.

---

## Project structure

```
./
├── docs/
├── include/
│   ├── bst_model.h
│   ├── file_parser.h
│   ├── intro.h
│   ├── ll_model.h
│   ├── sets_ops.h
│   └── utils.h
├── src/
│   ├── bst_model.c
│   ├── file_parser.c
│   ├── intro.c
│   ├── ll_model.c
│   ├── main.c
│   ├── sets_ops.c
│   └── utils.c
├── tests/
│   ├── test1.txt
│   └── test2.txt
├── lab_assignment.pdf
├── LICENSE
├── Makefile
└── README.md

5 directories, 19 files
```

---

## How to build and run

### Linux / macOS

Requirements: gcc, make

```bash
make
./adds-set-theory
```

To clean compiled files:

```bash
make clean
```

If make is not available:

```bash
gcc src/*.c -Iinclude -o adds-set-theory
./adds-set-theory
```

### Windows

Install MinGW, then open a terminal in the project folder and run:

```bash
gcc src\*.c -Iinclude -o adds-set-theory.exe
adds-set-theory.exe
```

Or if you have WSL installed, follow the Linux instructions above.

---

## Set operations

| Operation    | What it returns                              |
|--------------|----------------------------------------------|
| Union        | All words present in either paragraph        |
| Intersection | Only the words that appear in both           |
| Difference   | Words in paragraph A that are not in B       |

The user selects at runtime whether to compare paragraphs from the same file
or from two different files.

---

## Notes

- Words are normalized before insertion: lowercased and stripped of punctuation.
- Paragraphs are delimited by periods '.' and newline characters '\n'.
- The result of each operation is printed in alphabetical order (inorder traversal).
- Memory is freed properly at exit.

---

## Submission

Deadline: 10 May 2026
