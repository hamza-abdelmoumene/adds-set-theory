# Implementation Plan — ADDS Set Theory

## Phase 1 — Core Logic (no UI, no connections)

All Phase 1 functions are currently left as `/* TODO */` placeholders in the source files. The descriptions below are the intended behavior when you start implementing.

### 1.1 word_bst.c

Implement the BST abstract machine. Every other module depends on this.

| Function | Description |
|---|---|
| `Search(word, r, &p, &q)` | recursive search, returns node and parent |
| `Insert(word, &r)` | uses Search to find position, links new node (used by balanced builder) |
| `Inorder(r)` | recursive inorder, prints words alphabetically |
| `FreeTree(&r)` | postorder free, sets r to NULL |
| `CopyTree(src, &dest)` | inorder traversal of src, inserts each word into dest |
| `CollectWords(root, &array, &size)` | collects words into a dynamic array |
| `SortWords(array, size)` | sorts words alphabetically (qsort + strcmp) |
| `MedianInsert(array, left, right, &root)` | inserts words median-first to build a balanced BST |

Balanced BST approach (no AVL): collect all words into a dynamic array, sort alphabetically (qsort + strcmp), then insert median-first recursively to guarantee height O(log n). This leverages dynamic arrays while keeping the BST interface unchanged.

### 1.2 utils.c

Word cleaning before any insertion into the BST.

| Function | Description |
|---|---|
| `normalize(word)` | lowercase + strip punctuation in place |
| `is_stopword(word)` | returns 1 if word is in the stopword list |
| `is_empty(word)` | returns 1 if word is empty or only spaces |

### 1.3 sentence_ll.c

Linked list of sentences. Each sentence holds a BST root.

| Function | Description |
|---|---|
| `CreateSentenceList()` | returns an empty SentenceList |
| `AddSentence(&list, bst_root)` | appends a new SentenceNode at the end |
| `GetSentence(list, id)` | returns the SentenceNode at position id |
| `PrintSentences(list)` | prints all sentences with their words |
| `FreeSentenceList(&list)` | frees all nodes and their BSTs |

### 1.4 paragraph_ll.c

Linked list of paragraphs. Each paragraph holds a SentenceList.

| Function | Description |
|---|---|
| `CreateParagraphList()` | returns an empty ParagraphList and initializes the index array |
| `AddParagraph(&list, sentence_list)` | appends a new ParagraphNode and stores it in the index array |
| `GetParagraph(list, id)` | returns the ParagraphNode at position id in O(1) using the index array |
| `PrintParagraphs(list)` | prints all paragraphs with their sentences |
| `FreeParagraphList(&list)` | frees all nodes, sentence lists, BSTs, and the index array |
| `BuildIndex(list, &index)` | walks the list and builds the index array |
| `GetParagraphByIndex(index, i)` | returns the ParagraphNode pointer at index i |

Paragraphs use a linked list plus a parallel dynamic array of node pointers (`index`) for O(1) access by id during set operations. After parsing, BuildIndex walks the list once to fill the array; menu selection uses the index array for O(1) access.

### 1.5 file_parser.c

Reads a file and builds the full ParagraphList.

| Function | Description |
|---|---|
| `ParseFile(filename)` | opens file, reads char by char, returns a ParagraphList |

Parsing logic (two-pass per sentence):
- First pass: collect words into a dynamic array for the sentence
- Sort array alphabetically (qsort + strcmp)
- Build a balanced BST using MedianInsert
- `.` → end of sentence → save BST as SentenceNode → reset sentence storage
- `\n` → end of paragraph → save SentenceList as ParagraphNode → reset list
- `EOF` → flush remaining word, sentence, paragraph if not empty

### 1.6 sets_ops.c

All nine set operations across three levels.

**Word level** — operates on two BSTs:

| Function | Description |
|---|---|
| `WordUnion(A, B)` | insert all of A then all of B into new BST |
| `WordIntersection(A, B)` | walk A, insert word only if it exists in B |
| `WordDifference(A, B)` | walk A, insert word only if it does NOT exist in B |

**Sentence level** — operates on two SentenceLists:

| Function | Description |
|---|---|
| `SentenceUnion(A, B)` | merge: all sentences from A and B into new list |
| `SentenceIntersection(A, B)` | sentences whose word sets intersect (non-empty result) |
| `SentenceDifference(A, B)` | sentences in A whose content does not appear in B |

**Paragraph level** — operates on two ParagraphLists:

| Function | Description |
|---|---|
| `ParagraphUnion(A, B)` | all paragraphs from both lists |
| `ParagraphIntersection(A, B)` | paragraphs whose sentence sets intersect |
| `ParagraphDifference(A, B)` | paragraphs in A not present in B |

---

## Phase 2 — Connections (wiring everything together)

### 2.1 File loading

- `main.c` calls `ParseFile(filename)` and stores the result in a global or local `ParagraphList`
- Support loading up to 2 files simultaneously
- If a file is already loaded, ask the user to confirm overwrite

### 2.2 Selection logic

Write helper functions in `main.c` or a separate `ui.c`:

| Function | Description |
|---|---|
| `select_sentence(list)` | shows sentences, prompts user to pick one by id |
| `select_paragraph(index)` | shows paragraphs, prompts user to pick one by index |
| `select_file()` | prompts user to pick file 1 or file 2 |

Selection logic should use the paragraph index array built by BuildIndex instead of walking the linked list.

### 2.3 Operation dispatch

Based on the user's choices (level + scope + operation), call the correct function:

```
level=WORD      → get two sentences → call WordUnion / WordIntersection / WordDifference
level=SENTENCE  → get two paragraphs → call SentenceUnion / ...
level=PARAGRAPH → use both files → call ParagraphUnion / ...
```

### 2.4 Result display

| Function | Description |
|---|---|
| `PrintWordResult(bst)` | inorder print of result BST |
| `PrintSentenceResult(list)` | print each sentence in result list |
| `PrintParagraphResult(list)` | print each paragraph in result list |

---

## Phase 3 — UI, Menu, and Animations

### 3.1 Menu system

All menus follow the same box style from intro.c. Implement:

| Function | Description |
|---|---|
| `draw_box(title, options[], n)` | draws a bordered menu box with n options |
| `menu_main()` | Load File / Display Structure / Set Operations / Exit |
| `menu_level()` | Word / Sentence / Paragraph |
| `menu_scope()` | Same file / Two files |
| `menu_operation()` | Union / Intersection / Difference |

Menu items appear one by one with a 30ms delay between each line.

### 3.2 Animations

| Function | Description |
|---|---|
| `loading_bar(label, duration_ms)` | animated progress bar while parsing a file |
| `typewrite_result(words[], n)` | prints result words one by one with delay |
| `highlight_print(word, color)` | prints a single word in the given ANSI color |
| `animated_separator()` | reuse show_separator() from intro.c |

### 3.3 Live operation visualization

When an operation is selected, show:

```
Sentence A:   the   quick   brown   fox
Sentence B:   the   lazy    dog     fox

Computing Union...

Result:   brown   dog   fox   lazy   quick   the
```

Implement:

| Function | Description |
|---|---|
| `print_bst_inline(bst, color)` | prints all words of a BST on one line with color |
| `visualize_operation(A, B, result, op_name)` | shows A, B, computing message, then result |

### 3.4 Tree visualizer

Print the BST rotated 90 degrees (right subtree at top, left at bottom), indented by depth.

```
        [fox]
    [dog]
        [cat]
[and]
        [brown]
    [as]
```

| Function | Description |
|---|---|
| `print_tree(root, depth)` | recursive reverse inorder with indentation |

---

## Implementation Order

```
1. utils.c
2. word_bst.c
3. sentence_ll.c
4. paragraph_ll.c
5. file_parser.c
6. sets_ops.c         ← word level first, then sentence, then paragraph
7. main.c connections
8. menu system
9. animations
10. live visualization
11. tree visualizer
```

---

## Notes

- Test each module independently before connecting to the next
- Use the files in tests/ after each phase to validate output
- The blind test file will come on presentation day — the parser must handle any clean text
