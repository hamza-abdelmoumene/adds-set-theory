# ADDS Lab Exam — Set Theory on Linguistic Structures

Lab work for the Algorithmics and Dynamic Data Structures course (ADDS).  
The goal is to represent text files as hierarchical set structures using dynamic data structures in C,
and to implement the classic set operations: union, intersection, and difference at three levels.

---

## Authors

ABDELMOUMENE Hamza Ayoub & TALBI Baghdad  
First Year Preparatory Cycle - 1CP  
Ecole Superieure d'Informatique (ESI), Algiers  
Academic Year: 2025–2026

---

## What this project does

You give it one or two plain text files. It reads them, splits them into paragraphs,
then sentences, and stores each sentence as a Binary Search Tree of unique words
(so it naturally behaves like a mathematical set). Then you can run union, intersection,
or difference on any two structures at three levels: **words**, **sentences**, or **paragraphs**,
either from the same file or from two different files.

The program runs entirely in the terminal with a simple menu.

---

## Data structures used

- **BST (Binary Search Tree)** — one tree per sentence, each node is a unique word.
  Words are collected into a dynamic array, sorted, then inserted median-first to keep
  the tree balanced (height O(log n)) without AVL overhead.

- **Linked List (Sentences)** — chains the sentences of a paragraph together dynamically.
  After parsing, a parallel index array provides O(1) sentence access by id.

- **Linked List (Paragraphs)** — chains the paragraphs of a file together dynamically.
  After parsing, a parallel index array provides O(1) paragraph access by id.

---

## Project structure

```
./
├── docs/
├── include/
│   ├── word_bst.h
│   ├── sentence_ll.h
│   ├── paragraph_ll.h
│   ├── sets_ops.h
│   ├── file_parser.h
│   ├── intro.h
│   └── utils.h
├── src/
│   ├── word_bst.c
│   ├── sentence_ll.c
│   ├── paragraph_ll.c
│   ├── sets_ops.c
│   ├── file_parser.c
│   ├── intro.c
│   ├── main.c
│   └── utils.c
├── tests/
│   ├── test1.txt
│   └── test2.txt
├── lab_assignment.pdf
├── LICENSE
├── Makefile
└── README.md

5 directories, 21 files
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
| Union        | All elements present in either structure     |
| Intersection | Only the elements that appear in both        |
| Difference   | Elements in structure A that are not in B    |

Operations are supported at three levels:

| Level       | Description                                  |
|-------------|----------------------------------------------|
| Word        | Compares BSTs of words from two sentences    |
| Sentence    | Compares sentences from two paragraphs       |
| Paragraph   | Compares paragraphs from two files           |

The user selects at runtime the operation level and whether to compare structures
from the same file or from two different files.

---

## Notes

- Words are normalized before insertion: lowercased and stripped of punctuation.
- Sentences are delimited by periods '.'.
- Paragraphs are delimited by newline characters '\n'.
- The result of each operation is printed in alphabetical order (inorder traversal).
- Sentences are built with a two-pass flow: collect words, sort, then median-insert.
- Memory is freed properly at exit.

---

## Submission

Deadline: 10 May 2026
