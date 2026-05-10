# ADDS Set Theory — Full Code Explanation

## 1. Architecture Overview

```
main.c  →  RunAnimation()  →  RunMenu()
               ↓                   ↓
         user_interface.c    user_interface.c
          (intro anim)        (interactive loop)
                                   ↓
                    ┌──────────────┼──────────────┐
                    ↓              ↓              ↓
              file_list.c    sets_ops.c      Display screens
                    ↓              ↓
              file_parser.c   word_bst.c
                    ↓         sentence_ll.c
              paragraph_ll.c  paragraph_ll.c
              sentence_ll.c
              word_bst.c
              utils.c
```

## 2. Data Structure Hierarchy

The project uses a structured, nested representation of the text:

```
FileList               (linked list of loaded files)
 └─ FileNode           (one loaded file)
     └─ ParagraphList  (linked list of paragraphs)
         └─ ParagraphNode
             ├─ char *original (the original raw text of the paragraph)
             └─ SentenceList  (linked list of sentences)
                 └─ SentenceNode
                     ├─ char *original (the original raw text of the sentence)
                     └─ WordNode *val  (BST of unique words)
                         ├─ left
                         └─ right
```

## 3. Abstract Machine Conventions (Machine Abstraite)

The core data structures strictly follow the "Machine Abstraite" macros defined in the lab. This ensures operations act purely on logical nodes without exposing raw C pointer semantics where unnecessary. 

### Linked List Macros (defined in `sentence_ll.h` / `paragraph_ll.h` / `file_list.h`)
- **`Allocate(p)`** : Allocates memory for a node `p` and handles allocation errors.
- **`Free(p)`** : Frees the memory allocated for node `p`.
- **`Ass_val(p, v)`** : Assigns the value `v` to the `val` field of node `p`.
- **`Ass_adr(p, q)`** : Assigns the address (next pointer) `q` to the `addr` field of node `p`.
- **`Value(p)`** : Returns the value stored in the `val` field of node `p`.
- **`Next(p)`** : Returns the address of the next node (`addr` field).

### Binary Search Tree Macros (defined in `word_bst.h`)
- **`AllocateNode(p)`** : Allocates memory for a tree node `p`.
- **`FreeNode(p)`** : Frees the memory for tree node `p`.
- **`Ass_LC(p, q)`** : Assigns `q` as the Left Child (`left`) of node `p`.
- **`Ass_RC(p, q)`** : Assigns `q` as the Right Child (`right`) of node `p`.
- **`LC(p)`** : Returns the Left Child of node `p`.
- **`RC(p)`** : Returns the Right Child of node `p`.
- **`Ass_Node_Val(p, v)`** : Assigns value `v` to the node `p`.
- **`NodeValue(p)`** : Returns the value of the node `p`.

---

## 4. File-by-File Breakdown

### 4.1. `main.c` — Entry Point
| Function | Role | Logic |
|----------|------|-------|
| `main()` | Program entry point. | Calls `RunAnimation()` to display the futuristic intro sequence, then hands over control to `RunMenu()` for the interactive session. |

### 4.2. `utils.c` — Cross-Platform Utilities
| Function | Role | Logic |
|----------|------|-------|
| `PrintError(context, message)` | Standard error printer. | Prints all non-fatal internal errors in one consistent `Context: message` format. |
| `HandleError(context, message, fatal)` | Central error handler. | Calls `PrintError`; when `fatal != 0`, exits the program. Used for unrecoverable cases such as allocation failure. |
| `CheckedMalloc(size, context)` | Safe allocation. | Wraps `malloc`, normalizes zero-byte requests to 1 byte, and terminates through `HandleError` if allocation fails. |
| `CheckedRealloc(ptr, size, context)` | Safe reallocation. | Wraps `realloc` and terminates through `HandleError` on out-of-memory. |
| `CheckedStrDup(s, context)` | Safe string duplication. | Duplicates strings using `CheckedMalloc`; returns `NULL` only when the source string is `NULL`. |
| `IsReadableRegularFile(path)` | File-path validator. | Rejects empty paths, unreadable paths, and directories such as `tests/`; only regular readable files can be loaded. |
| `SleepMillis(ms)` | Cross-platform sleep. | Suspends execution using `Sleep()` on Windows or `nanosleep()` on POSIX. |
| `InitTerminal()` | Terminal initialization. | Enables ANSI escapes and UTF-8 codepage on Windows. No-op on POSIX. |
| `ReadChar()` | Blocking key reader. | Sets terminal to raw mode, reads a single keystroke directly without waiting for Enter. |
| `ReadCharNonBlocking()`| Non-blocking key reader. | Uses `VTIME=1` for a 100ms timeout to detect multi-byte escape sequences like arrow keys. |
| `GetTerminalSize(w, h)`| Gets window bounds. | Uses `GetConsoleScreenBufferInfo` (Win) or `ioctl(TIOCGWINSZ)` (POSIX) for responsive UI rendering. |
| `ClearScreen()` | Screen clearing. | Emits ANSI sequences `\033[2J\033[3J\033[H` to clear screen and scrollback. |
| `FlushOutput()` | Output syncer. | Wrapper for `fflush(stdout)`. |
| `NormalizeWord(word)` | String normalizer. | Lowercases characters and strips punctuation in place (`"Hello!"` → `"hello"`). |
| `IsBlank(word)` | Blank string checker. | Returns 1 if string is empty or contains only whitespace. |

### 4.3. `word_bst.c` — Binary Search Tree
Each sentence stores its unique words in a balanced BST. Words are compared using `strcmp`.

| Function | Role | Logic |
|----------|------|-------|
| `Search(word, r, &p, &q)` | Iterative search. | Traverse BST. Returns node in `p`, parent in `q`. If not found, `p=NULL`. |
| `Insert(word, &r)` | Insertion. | Uses `Search`. If not found, calls `AllocateNode` and links via `Ass_LC` or `Ass_RC`. Ignores duplicates (Set behavior). |
| `Inorder(r)` | Traversal. | Left→Root→Right traversal printing words alphabetically. |
| `FreeTree(&r)` | Memory cleanup. | Post-order traversal to call `FreeNode` on children before the root. |
| `CopyTree(src, &dest)` | Deep copying. | Pre-order traversal inserting each word into the new tree. |
| `CollectWords(...)` | Tree flattening. | In-order traversal to populate a dynamically doubling `char**` array. |
| `SortWords(array, size)` | Array sorting. | Uses standard `qsort` with a `strcmp` comparator. |
| `MedianInsert(...)` | Tree balancing. | Recursively inserts the median of a sorted array first, guaranteeing optimal O(log N) tree height without AVL rotations. |

### 4.4. `sentence_ll.c` — Sentence Linked List
Sentences within a paragraph are stored as a linked list.

| Function | Role | Logic |
|----------|------|-------|
| `CreateSentenceList()` | Initialization. | Returns an empty list structure with zeroed fields. |
| `AddSentence(...)` | Appends a sentence. | `Allocate`s a node, stores the BST root and `strdup` of original text, links via tail pointer (O(1) append). |
| `GetSentence(list, id)` | Fallback lookup. | Linear scan using `Next(current)` until ID matches. |
| `BuildSentenceIndex(...)` | O(1) Index building. | Allocates an array of pointers to nodes, walking the list once. Allows random access by index later. |
| `FreeSentenceList(...)` | Memory cleanup. | Walks the list, calls `FreeTree`, frees `original` strings, and calls `Free` on nodes. |

### 4.5. `paragraph_ll.c` — Paragraph Linked List
Paragraphs are stored as a linked list of SentenceLists.

| Function | Role | Logic |
|----------|------|-------|
| `CreateParagraphList()` | Initialization. | Returns an empty paragraph list. |
| `AddParagraph(...)` | Appends a paragraph. | Uses `Allocate`, stores the sentence list and `original` unparsed text, links via tail pointer. |
| `BuildIndex(...)` | O(1) Index building. | Maps list nodes to an array for fast random access during operations. |
| `FreeParagraphList(...)`| Memory cleanup. | Walks the list, cascading frees down to sentences and words, then frees the index. |

### 4.6. `file_parser.c` — The Text Parser
Responsible for character-by-character reading and building the 3-level data structure.

| Function | Role | Logic |
|----------|------|-------|
| `FlushWord(s)` | Word finalizer. | Null-terminates the buffer, normalizes it, and if valid, appends to the `ParserState` dynamic word array. |
| `FlushSentence(s)` | Sentence finalizer. | Sorts collected words, calls `MedianInsert` to build a perfectly balanced BST, then calls `AddSentence`. |
| `FlushParagraph(s, &r)` | Paragraph finalizer. | Builds the O(1) sentence index, copies the accumulated paragraph original text, and calls `AddParagraph`. |
| `ParseFile(filename)` | Main logic loop. | Validates the filename, opens the file, then reads `fgetc` until `EOF`. Space/Tab triggers word boundary. `.` triggers sentence boundary. `\n` triggers paragraph boundary. Allocation failures go through the checked utility helpers. |

### 4.7. `file_list.c` — File Management Layer
Top-level linked list holding parsed files.

| Function | Role | Logic |
|----------|------|-------|
| `CreateFileList()` | Initialization. | Returns empty FileList. |
| `AddFile(&list, fname)` | Validates, parses, and appends. | Returns `0` for invalid input, unreadable paths, and directory paths, preventing fake loaded files with zero paragraphs. On success, allocates a `FileNode`, calls `ParseFile()`, links via tail, and returns `1`. |
| `GetFileByIndex(...)` | Random access. | Returns `index[i]`. |
| `FreeFileList(&list)` | Complete cleanup. | Frees everything recursively when exiting the application. |

### 4.8. `sets_ops.c` — Mathematical Set Operations
Implements standard Set Theory operations (Union, Intersection, Difference) across 3 levels.

| Function | Logic |
|----------|-------|
| `WordUnion(A, B)` | Copies A's words. Then attempts to insert B's words. Duplicates are naturally ignored by the BST. `A ∪ B  =  { x | (x ∈ A) ∨ (x ∈ B) }` |
| `WordIntersection(A, B)` | Collects words from A into an array. Filters them via `Search(x, B)`. Rebuilds a balanced BST from the remaining words. `A ∩ B  =  { x | (x ∈ A) ∧ (x ∈ B) }` |
| `WordDifference(A, B)` | Collects words from A into an array. Filters out any found in B via `Search`. Rebuilds balanced BST. `A \ B  =  { x | (x ∈ A) ∧ (x ∉ B) }` |
| `SentenceUnion(A, B)` | Deep-copies A. Scans B; if a sentence in B does not have an equivalent in A (based on identical word BSTs), appends it. |
| `SentenceIntersection` | Walks A, checks if equivalent sentence exists in B. If so, deep-copies and appends to result list. |
| `SentenceDifference` | Walks A, checks if equivalent sentence exists in B. If NOT, deep-copies and appends to result list. |
| `ParagraphUnion(A, B)` | Deep-copies A. Scans B; if paragraph not found in A (based on identical sentence lists), appends it. |
| `ParagraphIntersection`| Scans A, appends deep-copies of paragraphs that have equivalents in B. |
| `ParagraphDifference` | Scans A, appends deep-copies of paragraphs that DO NOT have equivalents in B. |
| `WordCardinality`, `SentenceCardinality`, `ParagraphCardinality` | Return the size of a set result at the matching level. |
| `WordIsSubset`, `SentenceIsSubset`, `ParagraphIsSubset` | Check whether one operand set is included in the other and feed the result metadata panel. |

### 4.9. `user_interface.c` — UI, Layout, & Animations
Handles all visual rendering, ensuring the application stays within the 100-character centered box and never overflows.

| Function | Role | Logic |
|----------|------|-------|
| `RunAnimation()` | Launch sequence. | Orchestrates the futuristic ESI Logo, TP headers, Accomplishment banners, and separator animations with precisely timed `SleepMillis`. |
| `DrawBox(rslots)` | View layout. | Draws the dual-pane ASCII box. Hard-clamps strings strictly to 48 columns using `TruncateVisible()` to guarantee no visual bleed or line-wrapping bugs. |
| `DrawFullBox(rslots)` | Full-width result layout. | Draws a single 100-column rectangle without the ESI/ADDS/TP left panel, used by result display screens. |
| `GenericMenu(...)` | Interactive engine. | Maps input keys (Arrows/Numbers/ESC) to selections and re-renders the `DrawBox` menu. |
| `ShowShortcutsPopup()` | Help overlay. | Pressing `?` opens a compact shortcut panel from menus, file loading, file preview, and result screens. |
| `MenuMain()` | Main selector. | Presents Load File, List Loaded Files, Set Operations, and Exit. The old Display Structure option was removed. |
| `MenuOperation()` | Operation selector. | Shows each operation with spacing: the operation name plus plain-language definition on one line, and the mathematical definition on the next line. |
| `ScreenLoadFile()` / `RunMenu()` | File loading flow. | Keeps the cursor in the path input until Enter is pressed with text or ESC cancels. It rejects unreadable files and directories with `Invalid path — file could not be opened.` |
| `ScreenExploreLoadedFiles()` | File explorer. | Lets the user choose a loaded file and inspect paragraph previews instead of only seeing file names. |
| `ScreenShowFilePreview()` | Large-content preview. | Displays one compact row per paragraph, for example `Paragraph 1: The study of algorithms forms the foundation ...`, with paging for large files so text never overlaps. |
| `ShowLoadingBar(...)` | Progress visualizer. | Renders a centered bordered progress panel with spinner, percentage, gradient fill, and expanding separator line. |
| `ScreenShowResult(...)` | Result screen. | Uses `DrawFullBox`, wraps long result text, numbers displayed rows, shows `∅` for empty results, and displays `|Result|`, `A ⊆ B`, and `B ⊆ A`. |
| `ParagraphListToString`| Result formatting. | Safely stringifies Paragraph structures back into readable format by referring strictly to the `char *original` stored during parsing. |

### 4.10. `tests/` — Test Fixtures
The bundled test files are now longer and paragraph-heavy so the UI preview and paging logic can be exercised.

| File | Purpose | Notes |
|------|---------|-------|
| `tests/test1.txt` | Long, repetitive paragraphs. | Stresses paragraph preview truncation, page navigation, and set operations on larger inputs. |
| `tests/test2.txt` | Medium-length paragraphs. | Provides a second dataset for cross-file operations and subset checks. |

---

## 5. Architectural Integrity and Memory Safety
- **Single Source of Truth**: The original text from the parser is preserved perfectly in `char *original` fields at the Paragraph and Sentence level. Reconstructed outputs use this text, completely solving formatting/punctuation drift.
- **Set Equivalency over Memory Equality**: When comparing two sentences for intersection, they are considered equal if their unique Word BSTs are equal, not just if their original strings match. This allows robust set operations independently of minor punctuation differences.
- **Centralized Error Handling**: Allocation failures and internal invalid inputs now route through `PrintError`, `HandleError`, `CheckedMalloc`, `CheckedRealloc`, and `CheckedStrDup` in `utils.c`.
- **Invalid File Protection**: The UI and `AddFile` both reject unreadable paths before appending a `FileNode`, so invalid paths no longer appear as loaded files with zero paragraphs.
- **Directory Rejection**: Paths that point to directories are rejected before parsing, even if the OS allows opening the directory handle.
- **Explorable Loaded Files**: Loaded files can be selected and previewed paragraph-by-paragraph with compact summaries and pagination.
- **Operation Metadata**: Operation results show cardinality and subset relationships for the two operands.
- **Shortcut Help**: Pressing `?` opens a small help panel with navigation and input shortcuts.
- **Comment-Free Source**: The implementation files keep the behavior in code and the explanation in this document, so the submitted source reads cleaner.
- **Menu Back Behavior**: `ESC` from Set Operations submenus unwinds cleanly to the main menu instead of re-entering a redraw loop.
- **Recursive Teardown**: Closing the application automatically unwinds `FileNode` → `ParagraphNode` → `SentenceNode` → `WordNode` memory paths, ensuring 0 leaks.
