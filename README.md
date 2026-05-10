# ADDS Lab Exam — Set Theory on Linguistic Structures

> **Algorithmics and Dynamic Data Structures** — Lab Exam Project  
> Represent text files as hierarchical set structures using dynamic data structures in C,  
> and implement the classic set operations — union, intersection, and difference — at three structural levels.

---

## Authors

| Name | School | Year |
|---|---|---|
| ABDELMOUMENE Hamza Ayoub | École Supérieure d'Informatique (ESI), Algiers | 1CP — 2025/2026 |
| TALBI Baghdad | École Supérieure d'Informatique (ESI), Algiers | 1CP — 2025/2026 |

**Supervisor:** Dr. Kermi

---

## Table of Contents

1. [What This Project Does](#what-this-project-does)
2. [Data Structures Used](#data-structures-used)
3. [Project Structure](#project-structure)
4. [How to Build and Run](#how-to-build-and-run)
5. [Set Operations Reference](#set-operations-reference)
6. [Architecture Overview](#architecture-overview)
7. [Module Breakdown](#module-breakdown)
8. [Memory Safety](#memory-safety)
9. [Design Decisions](#design-decisions)
10. [Known Edge Cases](#known-edge-cases)

---

## What This Project Does

You provide one or two plain text files. The program reads them, splits them into paragraphs and then sentences, and stores each sentence as a **Binary Search Tree of unique words** — giving it natural mathematical set behaviour. You can then run **union**, **intersection**, or **difference** on any two structures at three levels:

| Level | What is compared |
|---|---|
| **Word** | The BST of words from two selected sentences/paragraphs |
| **Sentence** | The sentences from two selected paragraphs |
| **Paragraph** | The paragraphs from two loaded files |

The program runs entirely in the terminal through a polished interactive menu with arrow-key navigation, paginated previews, a real-time language toggle (EN/FR), and animated loading screens.

---

## Data Structures Used

### Binary Search Tree (word level)
One BST per sentence. Each node holds a single normalized, unique word. Set uniqueness is enforced by the BST's natural duplicate rejection on insertion.

Words are built using a **two-pass median-insert strategy**:
1. Collect all words into a dynamic array.
2. Sort alphabetically with `qsort`.
3. Insert median-first recursively — guaranteeing height ⌊log₂(n)⌋ without AVL rotations.

### Linked List + Index Array (sentence and paragraph levels)
Both sentence lists and paragraph lists are singly-linked lists with O(1) tail append. After parsing is complete, a **parallel pointer array** is built once (O(n)) to allow O(1) random access by index — required by the interactive menu system.

### Three-Level Nested Structure
```
FileList
 └─ FileNode                      one loaded file
     └─ ParagraphList             linked list of paragraphs
         └─ ParagraphNode
             ├─ int id
             ├─ char *original    raw text of the paragraph
             └─ SentenceList      linked list of sentences
                 └─ SentenceNode
                     ├─ int id
                     ├─ char *original   raw text of the sentence
                     └─ WordNode *val    root of a BST of unique words
                           ├─ left
                           └─ right
```

---

## Project Structure

```
adds-set-theory/
├── docs/                          supplementary documentation
├── include/
│   ├── utils.h                    ANSI colors, memory helpers, terminal I/O
│   ├── word_bst.h                 BST node type, BST macros, BST function prototypes
│   ├── sentence_ll.h              SentenceNode, SentenceList, linked-list macros
│   ├── paragraph_ll.h             ParagraphNode, ParagraphList
│   ├── file_list.h                FileNode, FileList
│   ├── file_parser.h              ParserState, ParseFile prototype
│   ├── sets_ops.h                 All nine set operations + cardinality + subset
│   └── user_interface.h           RunAnimation, RunMenu
├── src/
│   ├── main.c                     Entry point — calls RunAnimation then RunMenu
│   ├── utils.c                    Cross-platform memory, terminal, string utilities
│   ├── word_bst.c                 Search, Insert, Inorder, FreeTree, CopyTree,
│   │                              CollectWords, SortWords, MedianInsert
│   ├── sentence_ll.c              CreateSentenceList, AddSentence, GetSentence,
│   │                              BuildSentenceIndex, FreeSentenceList
│   ├── paragraph_ll.c             CreateParagraphList, AddParagraph, GetParagraph,
│   │                              BuildIndex, FreeParagraphList
│   ├── file_list.c                CreateFileList, AddFile, GetFileByIndex,
│   │                              PrintFileList, FreeFileList
│   ├── file_parser.c              ParseFile — character-by-character parser
│   ├── sets_ops.c                 WordUnion/Intersection/Difference,
│   │                              SentenceUnion/Intersection/Difference,
│   │                              ParagraphUnion/Intersection/Difference,
│   │                              Cardinality, IsSubset
│   └── user_interface.c           Full UI engine — menus, animations, previews,
│                                  result screens, loading bar, language system
├── tests/
│   ├── test1.txt                  Sample file 1 (algorithms and data structures)
│   └── test2.txt                  Sample file 2 (computer science and memory)
├── lab_assignment.pdf             Original lab subject
├── LICENSE                        MIT
├── Makefile                       gcc build rules
└── README.md                      This file
```

> **5 directories, 21 files**

---

## How to Build and Run

### Prerequisites

| Platform | Requirement | Download |
|---|---|---|
| Linux / macOS | `gcc` + `make` | `gcc` is usually pre-installed; install `make` via your package manager (`sudo apt install make` / `brew install make`) |
| Windows | MinGW-w64 (provides `gcc`) | [winlibs.com](https://winlibs.com/) — download the latest GCC + MinGW-w64 bundle and add it to your PATH |
| Windows (alternative) | WSL 2 | [Microsoft WSL install guide](https://learn.microsoft.com/en-us/windows/wsl/install) — then follow the Linux instructions |

> **What is `make`?** `make` is a standard build automation tool that reads the `Makefile` and runs the right compiler commands automatically. On Ubuntu/Debian: `sudo apt install build-essential`. On Fedora: `sudo dnf install make gcc`. On macOS: `xcode-select --install`.

---

### Linux / macOS

```bash
# Clone or extract the project, then:
make
./adds-set-theory
```

To remove the compiled binary:

```bash
make clean
```

If `make` is not available, compile directly:

```bash
gcc src/*.c -Iinclude -o adds-set-theory -lm
./adds-set-theory
```

---

### Windows (MinGW)

Open a terminal (Command Prompt or PowerShell) in the project folder:

```bat
gcc src\*.c -Iinclude -o adds-set-theory.exe -lm
adds-set-theory.exe
```

Or, if you installed MinGW and have `make`:

```bat
make
adds-set-theory.exe
```

---

### Windows (WSL)

Follow the Linux instructions above inside your WSL terminal.

---

### Quick Test

Two sample files are provided in `tests/`. Once the program is running:

1. Select **Load File** and enter `tests/test1.txt`
2. Load `tests/test2.txt` as well
3. Select **Set Operations** → **Paragraph operations** → **Union**
4. Pick File 0 as A and File 1 as B

---

## Set Operations Reference

### Operation Types

| Operation | Definition | Result |
|---|---|---|
| **Union** | A ∪ B = { x \| (x ∈ A) ∨ (x ∈ B) } | All elements present in A or B |
| **Intersection** | A ∩ B = { x \| (x ∈ A) ∧ (x ∈ B) } | Only elements present in both |
| **Difference** | A ∖ B = { x \| (x ∈ A) ∧ (x ∉ B) } | Elements in A that are not in B |

### Operation Levels

| Level | Operands | Equality criterion |
|---|---|---|
| **Word** | Two BSTs (one per paragraph) | String equality after normalization |
| **Sentence** | Two sentence lists (one per paragraph) | BST structural equality — same word set regardless of order or punctuation |
| **Paragraph** | Two paragraph lists (one per file) | Sentence-list equality |

Result display shows the computed set, its cardinality `|R|`, and whether A ⊆ B and B ⊆ A.

---

## Architecture Overview

```
main.c
  ├── RunAnimation()         ← user_interface.c   (ESI logo, TP banner, author names)
  └── RunMenu()              ← user_interface.c   (interactive session loop)
        ├── ScreenLoadFile()
        │     └── AddFile() → ParseFile() → ParagraphList
        ├── ScreenExploreLoadedFiles()
        │     └── ScreenShowFilePreview()
        └── Set Operations dispatch
              ├── MenuLevel() / MenuOperation() / MenuPickFile() / MenuPickParagraph()
              ├── WordUnion / WordIntersection / WordDifference        (word_bst.c)
              ├── SentenceUnion / SentenceIntersection / ...           (sentence_ll.c)
              └── ParagraphUnion / ParagraphIntersection / ...         (paragraph_ll.c)
```

---

## Module Breakdown

### `utils.c` — Cross-Platform Foundation

Every other module depends on `utils.c`.

**Memory helpers**

| Function | Behaviour |
|---|---|
| `CheckedMalloc(size, ctx)` | Wraps `malloc`; exits the process on failure instead of returning NULL |
| `CheckedRealloc(ptr, size, ctx)` | Wraps `realloc`; same fatal behaviour |
| `CheckedStrDup(s, ctx)` | Safe `strdup` — allocates via `CheckedMalloc` then `strcpy` |
| `HandleError(ctx, msg, fatal)` | Prints to `stderr`; exits if `fatal != 0` |

**Terminal I/O**

| Function | Behaviour |
|---|---|
| `InitTerminal()` | Enables ANSI processing + UTF-8 on Windows; no-op on POSIX |
| `ReadChar()` | Raw-mode blocking read — single keypress, no Enter needed |
| `ReadCharNonBlocking()` | 100ms timeout read — used to distinguish bare ESC from arrow-key sequences |
| `GetTerminalSize(w, h)` | `ioctl(TIOCGWINSZ)` on POSIX, `GetConsoleScreenBufferInfo` on Windows; fallback 120×40 |

**String helpers**

| Function | Behaviour |
|---|---|
| `VisibleLength(s)` | Counts printable characters, skipping ANSI escape codes and UTF-8 continuation bytes |
| `TruncateVisible(s, max, buf, sz)` | Copies at most `max` visible characters, preserving ANSI codes intact |
| `NormalizeWord(word)` | In-place: lowercase + strip all non-alphabetic characters (`"Hello!"` → `"hello"`) |
| `IsBlank(word)` | Returns 1 for NULL, empty, or whitespace-only strings |

---

### `word_bst.c` — Binary Search Tree

**Abstract machine macros (BST)**

| Macro | Expands to | Purpose |
|---|---|---|
| `AllocateNode()` | `AllocateNodeImpl()` | Allocate a zeroed tree node |
| `FreeNode(p)` | `free(p)` | Free a tree node |
| `Ass_LC(p, q)` | `(p)->left = (q)` | Set left child |
| `Ass_RC(p, q)` | `(p)->right = (q)` | Set right child |
| `LC(p)` | `(p)->left` | Get left child |
| `RC(p)` | `(p)->right` | Get right child |
| `Ass_Node_Val(p, v)` | `strcpy((p)->val, (v))` | Copy word string into node |
| `NodeValue(p)` | `(p)->val` | Read the word string |

**Key functions**

`Search(word, r, &p, &q)` — Iterative traversal. On exit: `*p` = found node (or NULL), `*q` = parent. Allows `Insert` to link without a second pass.

`Insert(word, &r)` — Calls `Search`. Returns `false` if already present (set semantics). Otherwise allocates with `AllocateNode`, assigns word with `Ass_Node_Val`, links via `Ass_LC`/`Ass_RC`.

`MedianInsert(array, left, right, &root)` — Inserts `array[mid]` first, then recurses on both halves. Produces a perfectly balanced BST of height ⌊log₂(n)⌋ with no rotations.

`CopyTree(src, &dest)` — Pre-order recursive copy using `Insert`. Used in all set operations to produce independent result trees.

`FreeTree(&r)` — Post-order: frees children before root, zeroes pointer.

---

### `sentence_ll.c` & `paragraph_ll.c` — Linked Lists

**Abstract machine macros (list)**

| Macro | Expands to |
|---|---|
| `Allocate(p)` | `(p) = CheckedMalloc(sizeof(*(p)), "Allocate")` |
| `Free(p)` | `free(p)` |
| `Ass_val(p, v)` | `(p)->val = (v)` |
| `Ass_adr(p, q)` | `(p)->addr = (q)` |
| `Value(p)` | `(p)->val` |
| `Next(p)` | `(p)->addr` |

Both modules follow the same pattern: O(1) tail append, post-parse index build for O(1) random access, and a cascade-freeing `Free*List` function that zeroes all pointers to prevent double-frees.

---

### `file_parser.c` — Text Parser

Reads character by character using a `ParserState` accumulator. Three flush operations:

| Trigger | Action |
|---|---|
| Space / Tab | Flush current word into the word array |
| Period `.` | Flush word → sort words → `MedianInsert` into BST → `AddSentence` |
| Newline `\n` | Flush word → flush sentence → `BuildSentenceIndex` → `AddParagraph` |
| EOF | Same as newline — handles files with no trailing newline |

Words are normalized by `NormalizeWord` and filtered by `IsBlank` before insertion. The raw text of each sentence and paragraph is preserved in `char *original` for display and equality checking.

---

### `sets_ops.c` — Set Operations

Nine operations across three levels, all following standard mathematical definitions.

**Word level** — operates directly on BSTs using `CollectWords` → filter → `MedianInsert`.

**Sentence level** — uses `BSTsEqual` (compare sorted word arrays) for membership testing; `CopySentenceList` for deep copying.

**Paragraph level** — uses `SentenceListsEqual` for membership testing; same deep-copy pattern.

Sentence equality is intentionally **word-set-based, not string-based**: two sentences using the same words in different orders are considered equal, which is semantically correct for set theory on linguistic structures.

---

### `user_interface.c` — UI Engine

- **Layout**: centered 100-character box. Dual-pane (`DrawBox`) for menus with ESI/ADDS/TP ASCII art on the left; full-width (`DrawFullBox`) for file previews and results.
- **Navigation**: `ReadKey()` maps raw bytes and ANSI escape sequences to `U` (up), `D` (down), `'\r'` (select), `27` (back). `GenericMenu()` is the universal menu engine used by every selection screen.
- **Language toggle**: pressing `L` at any menu switches between English and French in real time. Every user-facing string is stored as an `{.en, .fr}` pair and resolved at render time via `S(id)`.
- **File preview**: `ScreenShowFilePreview` paginates at 12 paragraphs per page, showing a 10-word preview of each.
- **Result display**: `ScreenShowResult` wraps output to 84-column lines, shows cardinality, and displays `A ⊆ B` / `B ⊆ A` status. Empty results render as `∅`.
- **Loading bar**: `ShowLoadingBar` renders a bordered spinner panel with gradient block fill over a configurable duration.

---

## Memory Safety

| Concern | Solution |
|---|---|
| Allocation failure | `CheckedMalloc`/`CheckedRealloc` exit the process — no NULL dereferences |
| String duplication | `CheckedStrDup` wraps all `strdup` calls |
| BST cleanup | `FreeTree` post-order — always frees children before root |
| Sentence cleanup | `FreeSentenceList` frees the BST, then the original string, then the node |
| Paragraph cleanup | `FreeParagraphList` cascades into `FreeSentenceList` |
| File cleanup | `FreeFileList` cascades into `FreeParagraphList`, then frees the index array |
| Set operation results | Caller is responsible: `FreeTree` / `FreeSentenceList` / `FreeParagraphList` |
| Double-free prevention | Every `Free*List` zeroes `head`, `tail`, and `index` after freeing |

---

## Design Decisions

**BST over hash set** — The assignment requires dynamic data structures from the course syllabus. BST also provides free alphabetical ordering via inorder traversal, which is required for result display without a separate sort step.

**Median-insert over AVL** — AVL rotations add significant code complexity and are harder to explain at an oral defence. Median-insert achieves the same O(log n) height guarantee on a known, static word set with zero rotation logic.

**Linked list + index array** — The linked list satisfies the "unknown count at parse time" dynamic requirement. The parallel index array provides O(1) access by position, needed by menu screens that must jump directly to "Paragraph 3".

**`char *original` in nodes** — Preserves exact source text for display and for sentence-level equality comparisons. Without it, reconstructing readable output from a BST alone would lose word order and punctuation.

**Sentence equality via BST comparison** — Sentences with the same word set but different order or punctuation are semantically equal from a set-theory perspective. String equality would be too strict and linguistically incorrect.

---

## Known Edge Cases

| Case | Behaviour |
|---|---|
| Empty file | Returns an empty `ParagraphList` (`count=0`); UI shows "No paragraphs found." |
| Single-word paragraph | Parsed correctly; `MedianInsert` handles a 1-element array cleanly |
| Paragraph with no period | All words flushed into one sentence at the newline or EOF boundary |
| File ending without `\n` | Final `FlushWord → FlushSentence → FlushParagraph` at EOF handles it |
| All-punctuation word | `NormalizeWord` strips it to empty; `IsBlank` rejects it before BST insertion |
| Duplicate file load | Not blocked — creates two independent memory instances; set operations yield expected results (union = full, difference = empty) |

---

## License

MIT — see [LICENSE](LICENSE).

---

*Submission deadline: 10 May 2026*