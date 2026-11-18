#include "Tree.c"
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <curses.h> // FIXME readme

enum {MAIN_NORMAL = 0, MAIN_READ_ERROR = 1, MAIN_EMPTY_ERROR = 2,
      MAIN_DATA_ERROR = 3, MAIN_ALLOCATE_ERROR = 4};
typedef enum {W_TREE_NORMAL = 0, W_TREE_OPEN_ERROR = 1, W_TREE_TREE_ERROR = 2} W_tree_err_t;
enum {CMP = -1, EXIT = -2, GO = -3, YES = -4, NO = -5, NONE = -6, PLAY = -7, INFO = -8, BACK = -9};
typedef enum {CORRECT = 0, INCORRECT = 1}                                      Data_correctness_t;

struct Option_t {const char* option; int result;};
struct Node_info_t {Tree_node_t* node; unsigned long long int path; int path_length;};

#define DATA_FILE_NAME "akinator_data.txt"

#define MAX_VALUE_SIZE 200
#define MAX_QUESTION_SIZE 200
#define MAX_OUTPUT_STR_SIZE 500
#define DEFAULT_Y_OFFSET 2


int ShowInfo(const char* filename);

Node_info_t* GetNodeArray(Tree_t* tree, ssize_t* p_leafs_count);
void RecursiveGetNodeArray(Tree_t* tree, ssize_t* p_leafs_count, Tree_node_t* current_node,
                           unsigned long long int path, int path_length, Node_info_t** p_in_node_arr_ptr);

void AskComparison(Tree_t* tree, Node_info_t* node_info_arr, ssize_t leafs_count);

void CompareFeatures(Tree_t* tree, Node_info_t* node_info_arr, ssize_t leafs_count);

Tree_node_t* PrintSimilarities(Tree_t* tree, Node_info_t* node_info_arr, ssize_t first_index, ssize_t second_index,
                               int* p_offset);

int PrintFeatures(Tree_t* tree, Tree_node_t* current_node, int offset, Node_info_t);

int Play(const char* filename);

void RememberNewCharacter(Tree_t* tree, Tree_node_t* current_node);

int GetOption(const char* question, int options_count, Option_t options[]);

void GetSentence(const char* question, char* new_character);

char* GetTextFileContent(const char* filename);

Tree_t* ReadTree(const char* filename);
Data_correctness_t RecursiveReadTree(Tree_t* tree, Tree_node_t* parent_node, Tree_node_t** branch,
                                     char* buffer, char** p_i_ptr);

void SkipSpaces(char** p_i_ptr);

void GetValue(char** p_i_ptr, char* buffer);

W_tree_err_t WriteTree(Tree_t* tree, const char* filename);
void RecursiveWriteTree(FILE* output_file, Tree_node_t* current_node);

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    while (true) {
        Option_t options[] = {{"-info-", INFO}, {"-play-", PLAY}, {"-exit-", EXIT}};
        int ask_show_info_result = GetOption("menu", (sizeof(options) / sizeof(Option_t)), options);

        if (ask_show_info_result == INFO) {
            int show_info_res = MAIN_NORMAL;

            if ((show_info_res = ShowInfo(DATA_FILE_NAME)) != MAIN_NORMAL) {
                endwin();
                return show_info_res;
            }
        }
        else if (ask_show_info_result == PLAY) {
            int play_res = MAIN_NORMAL;

            if ((play_res = Play(DATA_FILE_NAME)) != MAIN_NORMAL) {
                endwin();
                return play_res;
            }
        }
        else { /* exit */
            endwin();
            return MAIN_NORMAL;
        }
    }

    endwin();
    return MAIN_NORMAL;
}


int ShowInfo(const char* filename) {
    assert(filename != NULL);

    Tree_t* tree = ReadTree(DATA_FILE_NAME);
    if (tree == NULL)
        return MAIN_READ_ERROR;

    if (tree->root == NULL) {
        printf("data is empty\n");
        TreeDestroy(tree);
        return MAIN_EMPTY_ERROR;
    }

    ssize_t leafs_count = 0;
    Node_info_t* node_info_arr = GetNodeArray(tree, &leafs_count);
    if (node_info_arr == NULL) {
        printf("can't allocate memory for node_info_arr\n");
        TreeDestroy(tree);
        return MAIN_ALLOCATE_ERROR;
    }

    AskComparison(tree, node_info_arr, leafs_count);

    TreeDestroy(tree);
    free(node_info_arr);
    return MAIN_NORMAL;
}


Node_info_t* GetNodeArray(Tree_t* tree, ssize_t* p_leafs_count) {
    assert(tree != NULL); assert(p_leafs_count != NULL);

    Node_info_t* node_arr = (Node_info_t*)calloc(tree->size, sizeof(Node_info_t));
    if (node_arr == NULL)
        return NULL;

    *p_leafs_count = 0;
    Node_info_t* in_node_arr_ptr = node_arr;

    RecursiveGetNodeArray(tree, p_leafs_count, tree->root, 0, 0, &in_node_arr_ptr);

    return node_arr;
}

void RecursiveGetNodeArray(Tree_t* tree, ssize_t* p_leafs_count, Tree_node_t* current_node,
                           unsigned long long int path, int path_length, Node_info_t** p_in_node_arr_ptr) {
    assert(tree != NULL);  assert(p_leafs_count != NULL);  assert(p_in_node_arr_ptr != NULL);

    if (current_node != NULL) {
        if ((current_node->left_branch == NULL) && (current_node->right_branch == NULL)) {
            *(*p_in_node_arr_ptr)++ = {current_node, path, path_length};
            (*p_leafs_count)++;
        }

        RecursiveGetNodeArray(tree, p_leafs_count, current_node->left_branch,
                              path | (1 << path_length), path_length + 1, p_in_node_arr_ptr);
        RecursiveGetNodeArray(tree, p_leafs_count, current_node->right_branch,
                              path | (0 << path_length), path_length + 1, p_in_node_arr_ptr);
    }
}


void AskComparison(Tree_t* tree, Node_info_t* node_info_arr, ssize_t leafs_count) {
    assert(tree != NULL); assert(node_info_arr!= NULL);

    Option_t* options = (Option_t*)calloc(leafs_count + 2, sizeof(Option_t));
    if (options == NULL) {
        PrintErrorMessage("Failed to allocate memory", options);
        return;
    }

    int actions_count = 2;
    options[0] = {"-back-", BACK}; options[1] = {"-compare-", CMP};
    for (int option_i = actions_count; option_i < (leafs_count + actions_count); option_i++)
        options[option_i] = {node_info_arr[option_i - actions_count].node->value, option_i - actions_count + 1};

    while (true) {
        int character_number = GetOption("choose option", leafs_count + actions_count, options);
        if ((character_number > 0) && (character_number <= leafs_count)) {
            if (PrintFeatures(tree, tree->root, 0, node_info_arr[character_number - 1]) == BACK) {
                free(options);
                return;
            }
        }
        else if (character_number == CMP)
            CompareFeatures(tree, node_info_arr, leafs_count);
        else {
            free(options);
            return;
        }
    }

    free(options);
    return;
}


void CompareFeatures(Tree_t* tree, Node_info_t* node_info_arr, ssize_t leafs_count) {
    assert(tree != NULL); assert(node_info_arr!= NULL);

    Option_t* options = (Option_t*)calloc(leafs_count + 2, sizeof(Option_t));
    if (options == NULL) {
        PrintErrorMessage("Failed to allocate memory", options);
        return;
    }

    int actions_count = 1;
    options[0] = {"-back-", BACK};
    for (int option_i = actions_count; option_i < (leafs_count + actions_count); option_i++)
        options[option_i] = {node_info_arr[option_i - actions_count].node->value, option_i - actions_count + 1};

    int first_character_number  =  GetOption("first character", leafs_count + actions_count, options);
    if ((first_character_number < 1) || (first_character_number > leafs_count)) {
        free(options);
        return;
    }

    int second_character_number = GetOption("second character", leafs_count + actions_count, options);
    if ((second_character_number < 1) || (second_character_number > leafs_count)) {
        free(options);
        return;
    }

    free(options);

    int first_index = first_character_number - 1;
    int second_index = second_character_number - 1;

    int root_offset = 0;

    Tree_node_t* fork_node = PrintSimilarities(tree, node_info_arr, first_index, second_index, &root_offset); // -1 to get index
    int fork_offset = root_offset;
    if (fork_node == NULL)
        return;

    if (PrintFeatures(tree, fork_node, fork_offset, node_info_arr[first_index]) == BACK)
        return;

    if (PrintFeatures(tree, fork_node, fork_offset, node_info_arr[second_index]) == BACK)
        return;

    return;
}


Tree_node_t* PrintSimilarities(Tree_t* tree, Node_info_t* node_info_arr, ssize_t first_index, ssize_t second_index,
                               int* p_root_offset) {
    assert(tree != NULL); assert(node_info_arr!= NULL);

    Tree_node_t* current_node = tree->root;
    *p_root_offset = 0;
    char output_str[MAX_OUTPUT_STR_SIZE] = {};
    char* in_output_str_ptr = output_str;

    in_output_str_ptr += sprintf(in_output_str_ptr, "similarities: ");

    for (int both_turn; (*p_root_offset < node_info_arr[first_index].path_length) &&
                        (*p_root_offset < node_info_arr[second_index].path_length) &&
                        (((node_info_arr[first_index].path ^ node_info_arr[second_index].path) & (1ULL << *p_root_offset)) == 0) &&
                        (current_node != NULL); (*p_root_offset)++) {
        if (*p_root_offset != 0)
            in_output_str_ptr += sprintf(in_output_str_ptr, ", ");

        if ((node_info_arr[first_index/*or second_index*/].path & (1ULL << *p_root_offset)) != 0) {
            in_output_str_ptr += sprintf(in_output_str_ptr, "%s", current_node->value);

            current_node = current_node->left_branch;
        }
        else {
            in_output_str_ptr += sprintf(in_output_str_ptr, "not %s", current_node->value);

            current_node = current_node->right_branch;
        }
    }

    if (*p_root_offset == 0)
        in_output_str_ptr += sprintf(in_output_str_ptr, "no similarities\n");

    Option_t options[] = {{"-continue-", GO}, {"-back-", BACK}};
    if (GetOption(output_str, (sizeof(options) / sizeof(Option_t)), options) == BACK)
        return NULL;

    return current_node;
}


int PrintFeatures(Tree_t* tree, Tree_node_t* current_node, int root_offset, Node_info_t node_info) {
    assert(tree != NULL);

    int start_root_offset = root_offset;
    char output_str[MAX_OUTPUT_STR_SIZE] = {};
    char* in_output_str_ptr = output_str;

    if (current_node != tree->root)
        in_output_str_ptr += sprintf(in_output_str_ptr, "difference %s: ", node_info.node->value);

    for (; (root_offset < node_info.path_length) && (current_node != NULL); root_offset++) {
        if (root_offset != start_root_offset)
            in_output_str_ptr += sprintf(in_output_str_ptr, ", ");

        if ((node_info.path & (1ULL << root_offset)) != 0) {
            in_output_str_ptr += sprintf(in_output_str_ptr, "%s", current_node->value);

            current_node = current_node->left_branch;
        }
        else {
            in_output_str_ptr += sprintf(in_output_str_ptr, "not %s", current_node->value);

            current_node = current_node->right_branch;
        }
    }

    if (root_offset == start_root_offset)
        in_output_str_ptr += sprintf(in_output_str_ptr, "no difference");

    Option_t options[] = {{"-continue-", GO}, {"-back-", BACK}};
    return GetOption(output_str, (sizeof(options) / sizeof(Option_t)), options);
}


int Play(const char* filename) {
    assert(filename != NULL);

    while (true) {
        Tree_t* tree = ReadTree(DATA_FILE_NAME);
        if (tree == NULL)
            return MAIN_READ_ERROR;

        if (tree->root == NULL) {
            printf("data is empty\n");
            TreeDestroy(tree);
            return MAIN_EMPTY_ERROR;
        }

        Option_t options[] = {{"-continue-", GO}, {"-back-", BACK}};
        int ask_continue_result = GetOption("pick a character", (sizeof(options) / sizeof(Option_t)), options);
        if (ask_continue_result == GO); // go farther
        else { /* exit */
            TreeDestroy(tree);
            return MAIN_NORMAL;
        }

        for (Tree_node_t* current_node = tree->root; ; ) {
            if ((current_node->left_branch == NULL) && (current_node->right_branch == NULL)) {
                char question[MAX_QUESTION_SIZE] = {};
                sprintf(question, "is it %s?", current_node->value);

                Option_t optionsYesNo[] = {{"-yes-", YES}, {"-no-", NO}, {"-back-", BACK}};
                int ask_question_result = GetOption(question, (sizeof(optionsYesNo) / sizeof(Option_t)), optionsYesNo);
                if (ask_question_result == YES); // go farther
                else if (ask_question_result == NO)
                    RememberNewCharacter(tree, current_node);
                else { /* exit */
                    TreeDestroy(tree);
                    return MAIN_NORMAL;
                }

                Option_t optionsGo[] = {{"-continue-", GO}, {"-back-", BACK}};
                int ask_continue_result = GetOption("do you want to play again?", (sizeof(optionsGo) / sizeof(Option_t)), optionsGo);
                if (ask_continue_result == GO)
                    break;
                else { /* exit */
                    TreeDestroy(tree);
                    return MAIN_NORMAL;
                }
            }
            else if ((current_node->left_branch != NULL) && (current_node->right_branch != NULL)) {
                char question[MAX_QUESTION_SIZE] = {};
                sprintf(question, "%s?", current_node->value);

                Option_t options[] = {{"-yes-", YES}, {"-no-", NO}, {"-back-", BACK}};
                int ask_question_result = GetOption(question, (sizeof(options) / sizeof(Option_t)), options);
                if (ask_question_result == YES)
                    current_node = current_node->left_branch;
                else if (ask_question_result == NO)
                    current_node = current_node->right_branch;
                else { /* exit */
                    TreeDestroy(tree);
                    return MAIN_NORMAL;
                }
            }
            else {
                printf("incorrect data\n");
                TreeDestroy(tree);
                return MAIN_DATA_ERROR;
            }
        }

        TreeDestroy(tree);
    }

    return MAIN_NORMAL;
}


void RememberNewCharacter(Tree_t* tree, Tree_node_t* current_node) {
    assert(tree != NULL); assert(current_node != NULL);

    char new_character[MAX_VALUE_SIZE] = {};
    GetSentence("who is it?", new_character);

    char question[MAX_QUESTION_SIZE] = {};
    sprintf(question, "how does %s differ from %s? %s...\n", new_character, current_node->value, new_character);

    char difference[MAX_VALUE_SIZE] = {};
    GetSentence(question, difference);

    TreeAddLeftBranch(tree, current_node, new_character);
    TreeAddRightBranch(tree, current_node, current_node->value);

    strncpy((char*)current_node->value, difference, MAX_VALUE_SIZE);

    WriteTree(tree, DATA_FILE_NAME);
}


int GetOption(const char* question, int options_count, Option_t options[]) {
    assert(options != NULL);

    int selected = 0;
    int key = 0;

    while (true) {
        clear();

        int offset = DEFAULT_Y_OFFSET;

        if (question != NULL) {
            mvprintw(offset, 2, "%s", question);
            offset += 2;
        }

        for (int i_answer = 0; i_answer < options_count; i_answer++) {
            if (i_answer == selected) {
                attron(A_REVERSE);
                mvprintw(offset++, 2, "> %s", options[i_answer].option);
                attroff(A_REVERSE);
            }
            else
                mvprintw(offset++, 2, "  %s", options[i_answer].option);
        }

        refresh();

        key = getch();
        switch (key) {
            case KEY_UP:
                if (selected > 0)
                    selected--;
                break;
            case KEY_DOWN:
                if (selected < (options_count - 1))
                    selected++;
                break;
            case KEY_ENTER: case 10:
                return options[selected].result;
                break;
            default:
                break;
        }
    }
}


void GetSentence(const char* question, char* new_character) {
    assert(question != NULL); assert(new_character != NULL);

    int offset = DEFAULT_Y_OFFSET;

    clear();

    mvprintw(offset, 2, "%s", question);
    offset += 2;

    mvprintw(offset, 2, "");

    refresh();

    curs_set(1);
    echo();

    getnstr(new_character, MAX_VALUE_SIZE - 1);

    curs_set(0);
    noecho();
}


char* GetTextFileContent(const char* filename) { // REVIEW binary
    assert(filename != NULL);

    struct stat file_stat = {};
    stat(filename, &file_stat);

    char* content = (char*)calloc(file_stat.st_size, sizeof(char));
    if (content == NULL) {
        PrintErrorMessage("Failed to allocate memory", content);
        return NULL;
    }

    int file_descriptor = open(filename, O_RDONLY | O_BINARY);
    if (file_descriptor < 0) {
        PrintErrorMessage("Failed to open with mode O_RDONLY", filename);
        free(content);
        return NULL;
    }

    ssize_t code_size = read(file_descriptor, content, file_stat.st_size);
    if (code_size < 0) {
        PrintErrorMessage("Failed to read", filename);
        free(content);
        close(file_descriptor);
        return NULL;
    }

    close(file_descriptor);
    return content;
}


Tree_t* ReadTree(const char* filename) {
    assert(filename != NULL);

    Tree_t* tree = TreeInit();
    if (tree == NULL)
        return NULL;

    char* text_file_content = GetTextFileContent(filename);
    if (text_file_content == NULL) {
        TreeDestroy(tree);
        return NULL;
    }

    char* i_ptr = text_file_content;
    char buffer[MAX_VALUE_SIZE] = {};

    SkipSpaces(&i_ptr);
    GetValue(&i_ptr, buffer);

    if (buffer[0] == '\0')
        return tree;
    else
        TreeAddRoot(tree, buffer);

    if (RecursiveReadTree(tree, tree->root, &tree->root->left_branch, buffer, &i_ptr) != CORRECT) {
        printf("Incorrect data:\n%s", text_file_content);

        TreeDestroy(tree);
        free(text_file_content);
        return NULL;
    }

    if (RecursiveReadTree(tree, tree->root, &tree->root->right_branch, buffer, &i_ptr) != CORRECT) {
        printf("Incorrect data:\n%s", text_file_content);

        TreeDestroy(tree);
        free(text_file_content);
        return NULL;
    }

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL) {
        TreeDestroy(tree);
        free(text_file_content);
        return NULL;
    }

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL) {
        TreeDestroy(tree);
        free(text_file_content);
        return NULL;
    }

    free(text_file_content);
    return tree;
}

Data_correctness_t RecursiveReadTree(Tree_t* tree, Tree_node_t* parent_node, Tree_node_t** branch,
                                     char* buffer, char** p_i_ptr) {
    assert(tree != NULL); assert(parent_node != NULL);

    SkipSpaces(p_i_ptr);

    if (*(*p_i_ptr)++ != '(')
        return INCORRECT;

    SkipSpaces(p_i_ptr);
    GetValue(p_i_ptr, buffer);

    if (buffer[0] == '\0')
        *branch = NULL;
    else {
        TreeAddBranch(tree, parent_node, buffer, branch);

        if (RecursiveReadTree(tree, *branch, &(*branch)->left_branch, buffer, p_i_ptr) != CORRECT)
            return INCORRECT;
        if (RecursiveReadTree(tree, *branch, &(*branch)->right_branch, buffer, p_i_ptr) != CORRECT)
            return INCORRECT;
    }

    SkipSpaces(p_i_ptr);

    if (*(*p_i_ptr)++ != ')')
        return INCORRECT;

    return CORRECT;
}


void SkipSpaces(char** p_i_ptr) {
    assert(p_i_ptr != NULL);

    for(; isspace(**p_i_ptr); (*p_i_ptr)++);
}


void GetValue(char** p_i_ptr, char* buffer) {
    assert(p_i_ptr != NULL); assert(buffer != NULL);

    char* in_buffer_ptr = buffer;
    for(; (isalnum(**p_i_ptr) || ispunct(**p_i_ptr) || isspace(**p_i_ptr)) &&
          (**p_i_ptr != '(') && (**p_i_ptr != ')');
        (*p_i_ptr)++) {
        if (**p_i_ptr == '_')
            *in_buffer_ptr++ = ' ';
        else
            *in_buffer_ptr++ = **p_i_ptr;
    }

    *in_buffer_ptr = '\0';
}


W_tree_err_t WriteTree(Tree_t* tree, const char* filename) { // FIXME nil
    assert(tree != NULL); assert(filename != NULL);

    FILE* output_file = fopen(filename, "w");
    if (output_file == NULL) {
        PrintErrorMessage("Failed to open with \"w\" permission", output_file);
        return W_TREE_OPEN_ERROR;
    }

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL)
        return W_TREE_TREE_ERROR;

    RecursiveWriteTree(output_file, tree->root);

    fclose(output_file);
    return W_TREE_NORMAL;
}

void RecursiveWriteTree(FILE* output_file, Tree_node_t* current_node) {
    assert(output_file != NULL);

    if (current_node != NULL) {
        fprintf(output_file, "%s", current_node->value);

        fputc('(', output_file);
        RecursiveWriteTree(output_file, current_node->left_branch);
        fputc(')', output_file);

        fputc('(', output_file);
        RecursiveWriteTree(output_file, current_node->right_branch);
        fputc(')', output_file);
    }
}
