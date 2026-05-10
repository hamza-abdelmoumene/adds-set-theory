# ADDS Set Theory — Full Code Explanation

## Architecture Overview

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

---

## Data Structure Hierarchy

```
FileList               (linked list of loaded files)
 └─ FileNode           (one loaded file)
     └─ ParagraphList  (linked list of paragraphs)
         └─ ParagraphNode
             └─ SentenceList  (linked list of sentences)
                 └─ SentenceNode
                     └─ WordNode *val  (BST of unique words)
                         ├─ left
                         └─ right
```

Every layer uses the **same abstract machine pattern**: `Allocate`, `Free`, `Ass_val`, `Ass_adr`, `Value`, `Next` macros. Trees use `AllocateNode`, `FreeNode`, `Ass_LC`, `Ass_RC`, `LC`, `RC`, `Ass_Node_Val`, `NodeValue`.

---

## File-by-File Breakdown

### 1. `main.c` — Entry Point (8 lines)

| Function | Role |
|----------|------|
| `main()` | Calls `RunAnimation()` then `RunMenu()`, returns 0. |

The entire program flow is: show the intro animation, then enter the interactive menu. All logic lives in `user_interface.c`.

---

### 2. `utils.c` — Cross-Platform Utilities

#### Platform Layer
| Function | Role |
|----------|------|
| `SleepMillis(ms)` | Suspends execution. Windows: `Sleep()`. POSIX: `nanosleep()`. |
| `InitTerminal()` | Windows: enables ANSI escapes + UTF-8 codepage. POSIX: no-op. |
| `ReadChar()` | Blocking single-key read. Sets terminal to raw mode, reads one char, restores. |
| `ReadCharNonBlocking()` | Same but with 100ms timeout (`VTIME=1`). Returns -1 if no key pressed. |
| `GetTerminalSize(w, h)` | Returns terminal dimensions. Windows: `GetConsoleScreenBufferInfo`. POSIX: `ioctl(TIOCGWINSZ)`. |

#### Primitive Helpers
| Function | Role |
|----------|------|
| `ClearScreen()` | Prints `\033[2J\033[3J\033[H` (clear screen + scrollback + home cursor). |
| `FlushOutput()` | `fflush(stdout)` wrapper. |
| `MoveCursor(r, c)` | ANSI cursor positioning `\033[r+1;c+1H`. |
| `PadSpaces(n)` | Prints n spaces. |
| `VisibleLength(s)` | Counts visible characters, skipping ANSI escape sequences and UTF-8 continuation bytes. |
| `TruncateVisible(s, max, buf, sz)` | Copies string into buffer, stopping when visible length hits max. Preserves ANSI sequences. |

#### Text Processing
| Function | Role |
|----------|------|
| `NormalizeWord(word)` | Strips non-alpha characters and lowercases in place. `"Hello!"` → `"hello"`. |
| `IsBlank(word)` | Returns 1 if word is NULL, empty, or all whitespace. |

---

### 3. `word_bst.c` — Binary Search Tree (Words)

Each sentence's unique words are stored in a BST. Words are compared with `strcmp`.

| Function | Role |
|----------|------|
| `Search(word, r, &p, &q)` | Iterative BST search. Returns found node in `p`, its parent in `q`. If not found, `p=NULL`, `q=last visited node`. |
| `Insert(word, &r)` | Calls `Search` to find position. If not found, allocates a new node and links it to `q`. Silently ignores duplicates (set semantics). |
| `Inorder(r)` | Recursive Left→Root→Right traversal. Prints words alphabetically separated by spaces. |
| `FreeTree(&r)` | Post-order traversal: frees children before self. Sets root to NULL. |
| `CopyTree(src, &dest)` | Pre-order traversal of `src`, inserts each word into `dest`. Used by set operations. |
| `CollectWords(root, &array, &size, &cap)` | In-order traversal into a dynamic `char**` array. Array grows by doubling (starts at 8). Words are `malloc`-copied. |
| `SortWords(array, size)` | `qsort` with `strcmp` comparator. |
| `MedianInsert(array, left, right, &root)` | Recursively inserts `array[mid]` first, then recurses on left and right halves. Guarantees O(log n) height without AVL. |

**Balanced BST Strategy**: Instead of AVL rotations, we collect all words → sort → insert median-first. This gives the same O(log n) guarantee with simpler code.

---

### 4. `sentence_ll.c` — Sentence Linked List

Each paragraph is a linked list of sentences. Each sentence node holds a BST root.

| Function | Role |
|----------|------|
| `CreateSentenceList()` | Returns `{NULL, NULL, 0, NULL, 0}`. |
| `AddSentence(&list, bst_root)` | Allocates a new `SentenceNode`, assigns `id = count`, appends via tail pointer (O(1)), increments count. |
| `GetSentence(list, id)` | Linear scan fallback — walks the list until `node->id == id`. |
| `BuildSentenceIndex(&list)` | Allocates `count` pointers, walks the list once, fills `index[node->id] = node`. After this, O(1) access by index. |
| `GetSentenceByIndex(&list, i)` | Returns `index[i]` — O(1). |
| `PrintSentences(list)` | Iterates and prints `"Sentence N: "` + `Inorder(node->val)`. |
| `FreeSentenceList(&list)` | Walks list, frees each BST (`FreeTree`), frees each node, frees the index array. Resets all fields to zero/NULL. |

---

### 5. `paragraph_ll.c` — Paragraph Linked List

Same pattern as `sentence_ll.c`, one level higher.

| Function | Role |
|----------|------|
| `CreateParagraphList()` | Returns empty list. |
| `AddParagraph(&list, sentence_list)` | Allocates node, stores `SentenceList` as value, appends via tail. |
| `GetParagraph(list, id)` | Linear scan fallback. |
| `BuildIndex(&list)` | Builds the `index[]` array for O(1) access. |
| `GetParagraphByIndex(&list, i)` | Returns `index[i]`. |
| `PrintParagraphs(list)` | Iterates paragraphs, calls `PrintSentences` for each. |
| `FreeParagraphList(&list)` | Frees all sentences (which frees all BSTs), frees nodes, frees index array. |

---

### 6. `file_parser.c` — Text File Parser

The most critical module. Reads a text file character-by-character and builds the full 3-level structure.

#### Internal State (`ParserState`)
```c
typedef struct {
    char     word[256];         // current word buffer
    int      word_len;          // chars written so far
    char   **words;             // dynamic array of words for current sentence
    int      word_count;        // how many words collected
    int      word_capacity;     // array capacity
    SentenceList current_sentences;  // sentences for current paragraph
} ParserState;
```

#### Helper Functions
| Function | Role |
|----------|------|
| `FlushWord(s)` | Null-terminates `word[]`, normalizes it, skips if blank, appends a `strdup` copy to `words[]`. Grows array by doubling. |
| `FlushSentence(s)` | Sorts `words[]` alphabetically, builds balanced BST via `MedianInsert`, calls `AddSentence`. Frees the temp array. |
| `FlushParagraph(s, &result)` | Calls `BuildSentenceIndex`, then `AddParagraph` to append the paragraph. Resets `current_sentences`. |

#### Main Function
| Function | Role |
|----------|------|
| `ParseFile(filename)` | Opens file, reads char-by-char in a loop: space/tab → `FlushWord`, `.` → `FlushWord + FlushSentence`, `\n` → `FlushWord + FlushSentence + FlushParagraph`, other → append to word buffer. After EOF, flushes remaining data. Calls `BuildIndex` at the end. |

**Two-pass sentence building**: For each sentence, words are first collected into a dynamic array (pass 1), sorted (pass 2), then median-inserted into a BST. This guarantees balanced trees.

---

### 7. `file_list.c` — File Management Layer

Adds a top-level linked list of loaded files. Same abstract machine pattern.

| Function | Role |
|----------|------|
| `CreateFileList()` | Returns empty list. |
| `AddFile(&list, filename)` | Allocates a `FileNode`, copies filename, calls `ParseFile(filename)` to populate `val`, appends to list, grows index array. |
| `GetFileByIndex(&list, i)` | O(1) access via index array. |
| `PrintFileList(list)` | Prints `[id] filename (N paragraphs)` for each file. |
| `FreeFileList(&list)` | Frees all paragraph lists (cascading to sentences → BSTs), frees nodes, frees index array. |

---

### 8. `sets_ops.c` — Set Operations (9 Functions)

#### Internal Helpers
| Function | Role |
|----------|------|
| `BSTsEqual(A, B)` | Collects words from both trees, compares arrays element-by-element. Two sentences are "equal" if they contain the same set of unique words. |
| `SentenceExistsIn(bst, list)` | Linear scan: checks if any sentence in the list has an equivalent BST. |
| `CopySentenceList(src)` | Deep-copies every sentence (via `CopyTree` for each BST). |
| `SentenceListsEqual(A, B)` | Two paragraphs are equal if every sentence in A exists in B and counts match. |
| `ParagraphExistsIn(sentences, list)` | Linear scan: checks if any paragraph in the list has equivalent sentences. |

#### Word Level (BST → BST)
| Function | Logic |
|----------|-------|
| `WordUnion(A, B)` | Copy all words from A into result, then copy all from B. Duplicates are ignored by `Insert`. |
| `WordIntersection(A, B)` | Collect words from A, keep only those found in B via `Search`, build balanced BST from filtered array. |
| `WordDifference(A, B)` | Collect words from A, keep only those NOT found in B, build balanced BST. |

#### Sentence Level (SentenceList → SentenceList)
| Function | Logic |
|----------|-------|
| `SentenceUnion(A, B)` | Deep-copy all of A, then append from B only sentences not already present (checked via `BSTsEqual`). |
| `SentenceIntersection(A, B)` | Walk A, copy sentences whose BST has an equivalent in B. |
| `SentenceDifference(A, B)` | Walk A, copy sentences whose BST does NOT have an equivalent in B. |

#### Paragraph Level (ParagraphList → ParagraphList)
| Function | Logic |
|----------|-------|
| `ParagraphUnion(A, B)` | Copy all paragraphs from A, then append from B only those not already present. |
| `ParagraphIntersection(A, B)` | Walk A, copy paragraphs whose sentence set has an equivalent in B. |
| `ParagraphDifference(A, B)` | Walk A, copy paragraphs whose sentence set does NOT exist in B. |

---

### 9. `user_interface.c` — UI & Animation (1350+ lines)

This file contains **two public functions** declared in `user_interface.h`:

#### `RunAnimation()` — Intro Sequence
Plays a full-screen animated intro:
1. `ShowFullscreenNotice()` — asks user to maximize terminal
2. `ShowEsiLogo()` — draws the ESI logo using half-block Unicode characters on an 80×42 pixel canvas
3. `ShowTp1()` — types "TP" in block letters with gradient colors
4. `ShowAccomplished()` — types "ACCOMPLISHED BY" banner
5. `ShowNames()` — shows author names with a color sweep animation
6. `ShowSeparatorAnimation()` — expanding gradient line

#### `RunMenu()` — Interactive Menu System
Main loop with a `FileList` that persists across the session:

**Menu Flow:**
```
Main Menu
 ├─ [1] Load File        → ScreenLoadFile() → AddFile()
 ├─ [2] List Files       → ScreenListFiles()
 ├─ [3] Display Structure → pick file → ScreenDisplayStructure()
 ├─ [4] Set Operations
 │   ├─ Level:     Word / Sentence / Paragraph
 │   ├─ Operation: Union / Intersection / Difference
 │   ├─ Pick A:    file → paragraph → sentence (depth depends on level)
 │   ├─ Pick B:    file → paragraph → sentence
 │   └─ Execute & display result
 └─ [5] Exit → FreeFileList() → goodbye screen
```

**Key UI Components:**
| Component | Role |
|-----------|------|
| `DrawBox(rslots[24])` | Renders a 100-column two-panel box. Left panel shows ESI/ADDS/TP art. Right panel shows dynamic content via 24 slot strings. |
| `GenericMenu(title, opts, n, crumb)` | Drives arrow-key/number navigation over `n` options. Returns 0-based index or -1 on ESC. |
| `ReadKey()` | Translates raw key codes into logical codes: `'U'`=up, `'D'`=down, `'\r'`=enter, `27`=ESC, `'1'-'9'`=direct. |
| `FormatOption(buf, idx, label, selected)` | Formats menu items with `▶` cursor and color badges. |
| `DrawBreadcrumb(crumb)` | Shows navigation path like `"Main Menu › Set Operations › Level"`. |
| `DrawStatusBar()` | Animated separator + author/year footer. |

**Result Display Helpers:**
| Function | Role |
|----------|------|
| `InorderToBuffer(r, buf, sz, &pos)` | Recursively writes BST words into a string buffer. |
| `BstToString(r, buf, sz)` | Wrapper: BST → display string. |
| `SentenceListToString(list, buf, sz)` | Formats as `S0{word1 word2} S1{word3}`. |
| `ParagraphListToString(list, buf, sz)` | Formats as `P0[{word1}{word2}] P1[{word3}]`. |
| `BuildParagraphLabels(&pl, labels, ptrs, max)` | Creates display strings like `"Paragraph 0 (2 sentences)"` for picker menus. |
| `BuildSentenceLabels(&sl, labels, ptrs, max)` | Creates display strings like `"S0: brown dog fox"` for picker menus. |
| `ScreenShowFullResult(op, labA, labB, level, ...)` | Renders the result box with operation name, A/B labels, and formatted result string. |

---

## Connection Flow: Set Operation Example

When the user selects **Word Intersection** between two sentences:

```
1.  RunMenu()
2.  → MenuMain()         returns 3 (Set Operations)
3.  → MenuLevel()        returns 0 (Word)
4.  → MenuOperation()    returns 1 (Intersection)
5.  → MenuPickFile()     user picks file 0
6.  → GetFileByIndex()   retrieves FileNode for file 0
7.  → BuildParagraphLabels()  builds "Paragraph 0 (2 sentences)" etc.
8.  → MenuPickParagraph() user picks paragraph 0
9.  → GetParagraphByIndex() retrieves ParagraphNode
10. → BuildSentenceLabels()  builds "S0: brown dog fox..." etc.
11. → MenuPickSentence()  user picks sentence 0
12. → GetSentenceByIndex() retrieves SentenceNode (sentA)
13. [repeat steps 5-12 for B]
14. → WordIntersection(sentA->val, sentB->val)
15.     → CollectWords(A)  collects words from BST A
16.     → Search(word, B)  for each word, check if in B
17.     → MedianInsert()   build balanced BST from filtered words
18. → BstToString(result)  format result as string
19. → ScreenShowResult()   display in the UI box
20. → FreeTree(&result)    clean up
```

---

## Memory Management

Every allocation has a matching free:
- `WordNode` → `FreeTree()` (post-order)
- `SentenceNode` → `FreeSentenceList()` (frees BSTs + nodes + index)
- `ParagraphNode` → `FreeParagraphList()` (frees sentence lists + nodes + index)
- `FileNode` → `FreeFileList()` (frees paragraph lists + nodes + index)
- Set operation results → freed immediately after display

On exit, `RunMenu()` calls `FreeFileList(&file_list)` to release everything.
