#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#define DEBUG

#ifdef DEBUG
    #define ONDEB(...) __VA_ARGS__
#else
    #define ONDEB(...)
#endif

static FILE* Tree_log_file = stdout;
#define TREE_LOG_FILE_NAME "tree_log.txt"
static FILE* Tree_graphviz_file = NULL;
#define TREE_GRAPHVIZ_FILE_NAME "tree_graphviz.dot"
static FILE* Tree_html_log_file = NULL;
#define TREE_HTML_LOG_FILE_NAME "tree_log.html"

#define DEFAULT_THICKNESS 2.0
#define ERROR_THICKNESS   4.0

#define BACK_COLOR "#34252c"

#define FORWARD_ERROR_COLOR "#e13d3d"
#define BACK_ERROR_COLOR    "#780b0b"

#define LINE_NEGATIVE_COLOR "#e13d3d"
#define LINE_POSITIVE_COLOR "#06a07c"

#define ZERO_LINE_COLOR "#260a18"
#define ZERO_FILL_COLOR "#77606b"
#define ZERO_FONT_COLOR "#260a18"

#define DATA_LINE_COLOR "#430523"
#define DATA_FILL_COLOR "#94285c" //"#9534a3"
#define DATA_FONT_COLOR "#260a18" //"#ffffc0"

#define HEAD_LINE_COLOR "#260a18"
#define HEAD_FILL_COLOR "#591837"
#define HEAD_FONT_COLOR "#ca80a2" //"#c0ffff"

#define TAIL_LINE_COLOR "#430523"
#define TAIL_FILL_COLOR "#ca80a2"
#define TAIL_FONT_COLOR "#3f1229"

#define NEXT_LINE_COLOR "#063a2e"
#define NEXT_FILL_COLOR "#06a07c"
#define NEXT_FONT_COLOR "#0a2920"

#define FREE_LINE_COLOR "#03261e"
#define FREE_FILL_COLOR "#037f62"
#define FREE_FONT_COLOR "#05120e"

#undef OVERFLOW

typedef enum {TREE_NORMAL     = 0b00000000, TREE_ROOT_ERROR = 0b00000001,
              TREE_SIZE_ERROR = 0b00000010, TREE_LINK_ERROR = 0b10000000}                     Tree_err_t;
typedef enum {SUCCESS = 0, FATAL_ERROR = 1, OVERFLOW = -1, VACUUM = -2, INCORRECT_INPUT = -3} Tree_report_t;
typedef enum {START_LOG_NORMAL = 0, START_LOG_OPEN_ERROR = 1}                                 Start_log_err_t;
typedef enum {IN = 0, OUT = 1}                                                                Node_status_t;
typedef enum {ROOT = 0, LEFT = 1, RIGHT = 2, LEAF = 3}                                        Walk_div_t;

#define INT     1
#define FLOAT   2
#define DOUBLE  3
#define POINTER 4
#define STRING  5

#define TREE_TYPE STRING

#if TREE_TYPE == INT
    #define Tree_type_t int
    #define TREE_TYPE_SPECIFIER "%d"
    #define POISON 67//1666667
#elif TREE_TYPE == FLOAT
    #define Tree_type_t float
    #define TREE_TYPE_SPECIFIER "%f"
    #define POISON NAN
#elif TREE_TYPE == DOUBLE
    #define Tree_type_t double
    #define TREE_TYPE_SPECIFIER "%lf"
    #define POISON NAN
#elif TREE_TYPE == POINTER
    #define Tree_type_t void*
    #define TREE_TYPE_SPECIFIER "%p"
    #define POISON NULL
#elif TREE_TYPE == STRING
    #define Tree_type_t const char*
    #define TREE_TYPE_SPECIFIER "%s"
    #define POISON NULL
#else
    #define Tree_type_t long long int
    #define TREE_TYPE_SPECIFIER "%lld"
    #define POISON 166666666667
#endif

#define MAX_STRING_NODE_VALUE_SIZE 200

typedef enum {QUIET = 0, LOUD = 1} Volume_t;

#define TREE_DUMP_FOLDER_PREFIX "dump"
#define BACKGROUND_IMAGE_NAME "ArimaKana.png"

#define INCORRECT_INDEX -1

#define MAX_FILE_NAME_SIZE 100
#define MAX_TIME_SUFFIX_SIZE 100
#define MAX_FOLDER_NAME_SIZE 100
#define MAX_MESSAGE_SIZE 200
#define MAX_COMMAND_SIZE 200

struct Node_colors_t {
    const char* line_color;
    const char* fill_color;
    const char* font_color;
};

struct Tree_node_t {
    #if TREE_TYPE == STRING
        const char value[MAX_STRING_NODE_VALUE_SIZE];
    #else
        Tree_type_t  value;
    #endif

    Tree_node_t* parent;
    Tree_node_t* left_branch;
    Tree_node_t* right_branch;
    ssize_t      root_distance;
};

struct Tree_t {
    Tree_node_t* root;

    ssize_t size;

    int graphic_dump_number;

    char dump_folder_name[MAX_FOLDER_NAME_SIZE] = {};

    Tree_err_t error;
    Tree_report_t last_function_success;
};

#define PrintErrorMessage(message, object)                                                         \
    fprintf(stderr, "%s: %s(): %s - %s, line %d\n", __FILE__, __func__, message, #object, __LINE__);

#define TREE_VERIFY(tree, volume) TreeVerify(tree, volume, __func__, __LINE__)

Tree_type_t TreeGetValue(Tree_node_t* tree_node);

Tree_t* TreeInit(void);

void TreeDestroy(Tree_t* tree);

Tree_report_t TreeDelNode(Tree_t* tree, Tree_node_t* node);
void TreeRecursiveFree(Tree_t* tree, Tree_node_t* current_node);

Tree_report_t TreeAddRoot(Tree_t* tree, Tree_type_t value);

Tree_report_t TreeAddLeftBranch(Tree_t* tree, Tree_node_t* parent_node, Tree_type_t value);

Tree_report_t TreeAddRightBranch(Tree_t* tree, Tree_node_t* parent_node, Tree_type_t value);

Tree_report_t TreeAddBranch(Tree_t* tree, Tree_node_t* parent_node, Tree_type_t value,
                            Tree_node_t** branch);

Node_status_t TreeCheckNode(Tree_t* tree, Tree_node_t* node);
Node_status_t TreeRecursiveCheckNode(Tree_t* tree, Tree_node_t* current_node, Tree_node_t* sought_node,
                                     bool* p_stop_recurse, ssize_t* p_checked_nodes_number);

Tree_node_t* TreeAllocNode(Tree_type_t value, Tree_node_t* parent, Tree_node_t* left_branch,
                           Tree_node_t* right_branch, ssize_t root_distance);

Tree_report_t TreeOrderedInsert(Tree_t* tree, Tree_type_t value,
                                int Compare(const void*, const void*));
void TreeRecursiveOrderedInsert(Tree_t* tree, Tree_node_t* current_node, Tree_type_t value,
                                int Compare(const void*, const void*));

void Dump(Tree_node_t* node);

Tree_err_t TreeVerify(Tree_t* tree, Volume_t volume,
                      const char* calling_function_name, int from_line);

Tree_err_t TreeDiagnoseError(Tree_t* tree, Volume_t volume,
                             const char* calling_function_name, int from_line);

Tree_err_t TreeCheckLinkError(Tree_t* tree);
Tree_err_t TreeRecursiveCheckLinkError(Tree_t* tree, Tree_node_t* current_node,
                                       bool* p_stop_recurse, ssize_t* p_checked_nodes_number);

void TreeGraphicDump(Tree_t* tree, const char* message);

Start_log_err_t TreeStartLogging(Tree_t* tree);

void TreeCreateInformationTable(Tree_t* tree);

void TreeHtmlPrintErrors(Tree_t* tree);

void TreeCreateGraphic(Tree_t* tree);

void TreeCreatePng(Tree_t* tree);

void TreeCreateNodes(Tree_t* tree);

void SetNodeColors(Node_colors_t* node_colors,
                   const char* line_color, const char* fill_color, const char* font_color);

void TreeCreateNode(Tree_t* tree, Tree_node_t* node, Node_colors_t* node_colors);
void RecursiveTreeCreateNodes(Tree_t* tree, Tree_node_t* current_node);

void TreeCreateLinks(Tree_t* tree);
void RecursiveCreatLinks(Tree_t* tree, Tree_node_t* current_node, Walk_div_t div);

void TreeCreateLink(Tree_t* tree, Tree_node_t* node, Walk_div_t div);


Tree_type_t TreeGetValue(Tree_node_t* tree_node) {
    assert(tree_node != NULL);

    return tree_node->value;
}


Tree_t* TreeInit(void) {
    Tree_t* tree = (Tree_t*)malloc(sizeof(Tree_t));
    if (tree == NULL) {
        PrintErrorMessage("Failed to allocate memory", tree);
        return NULL;
    }

    ONDEB(
        Tree_log_file = fopen(TREE_LOG_FILE_NAME, "w");
        if (Tree_log_file == NULL) {
            printf("Cant open %s with \"w\" permission\n", TREE_LOG_FILE_NAME);
            free(tree);
            return NULL;
        }
    )

    tree->root = NULL;

    tree->size = 0;

    tree->graphic_dump_number = 1;

    tree->error = TREE_NORMAL;
    tree->last_function_success = SUCCESS;

    return tree;
}


void TreeDestroy(Tree_t* tree) {
    assert(tree != NULL);

    TreeDelNode(tree, tree->root);

    if (tree->graphic_dump_number > 1)
        fclose(Tree_html_log_file);

    free(tree);
}


Tree_report_t TreeDelNode(Tree_t* tree, Tree_node_t* node) {
    assert(tree != NULL);

    if (node != tree->root) {
        if (node->parent->left_branch == node)
            node->parent->left_branch = NULL;
        else if (node->parent->right_branch == node)
            node->parent->right_branch = NULL;
        else {
            printf("from %s(), line %d: Fatal error - parent node don't have %p among child nodes\n",
               __func__, __LINE__, node);
            return tree->last_function_success = FATAL_ERROR;
        }
    }
    else
        tree->root = NULL;

    TreeRecursiveFree(tree, node);

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL)
        return tree->last_function_success = FATAL_ERROR;

    return tree->last_function_success = SUCCESS;
}

void TreeRecursiveFree(Tree_t* tree, Tree_node_t* current_node) { // STUB fill poison
    assert(tree != NULL);

    if (current_node != NULL) {
        TreeRecursiveFree(tree, current_node->left_branch); // REVIEW set to zero left or right for parent
        TreeRecursiveFree(tree, current_node->right_branch);

        free(current_node);
        tree->size--;
    }
}


Tree_report_t TreeAddRoot(Tree_t* tree, Tree_type_t value) {
    assert(tree != NULL);

    if (tree->root != NULL) {
        printf("note from %s(), line %d: root already exists with value " TREE_TYPE_SPECIFIER ", Add was cancelled\n",
               __func__, __LINE__, tree->root->value);
        return SUCCESS;
    }

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL)
        return tree->last_function_success = FATAL_ERROR;

    tree->root = TreeAllocNode(value, NULL, NULL, NULL, 0);
    if (tree->root == NULL)
        return FATAL_ERROR;

    tree->size++;

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL)
        return tree->last_function_success = FATAL_ERROR;

    return tree->last_function_success = SUCCESS;
}


Tree_report_t TreeAddLeftBranch(Tree_t* tree, Tree_node_t* parent_node, Tree_type_t value) {
    assert(tree != NULL); assert(parent_node != NULL);

    return TreeAddBranch(tree, parent_node, value, &parent_node->left_branch);
}


Tree_report_t TreeAddRightBranch(Tree_t* tree, Tree_node_t* parent_node, Tree_type_t value) {
    assert(tree != NULL); assert(parent_node != NULL);

    return TreeAddBranch(tree, parent_node, value, &parent_node->right_branch);
}


Tree_report_t TreeAddBranch(Tree_t* tree, Tree_node_t* parent_node, Tree_type_t value,
                            Tree_node_t** branch) {
    assert(tree != NULL); assert(parent_node != NULL); assert(branch != NULL);

    if (*branch != NULL) {
        printf("note from %s(), line %d: branch already exists with value " TREE_TYPE_SPECIFIER ", Add was cancelled\n",
               __func__, __LINE__, (*branch)->value);
        return SUCCESS; // TODO rewriting
    }

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL)
        return tree->last_function_success = FATAL_ERROR;

    ONDEB(
        if (TreeCheckNode(tree, parent_node) == OUT) {
            printf("note from %s(), line %d: node %p with value " TREE_TYPE_SPECIFIER " is out of tree, %s was cancelled\n",
                   __func__, __LINE__, parent_node, parent_node->value, __func__);
            return INCORRECT_INPUT;
        }
    )

    Tree_node_t* new_node = TreeAllocNode(value, parent_node, NULL, NULL,
                                          parent_node->root_distance + 1);
    if (new_node == NULL)
        return FATAL_ERROR;

    *branch = new_node;

    tree->size++;

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL)
        return tree->last_function_success = FATAL_ERROR;

    return tree->last_function_success = SUCCESS;
}


Node_status_t TreeCheckNode(Tree_t* tree, Tree_node_t* node) { // tree must be verified by check cycle error
    assert(tree != NULL);

    bool stop_recurse = false;
    ssize_t checked_nodes_number = 0;

    return TreeRecursiveCheckNode(tree, tree->root, node, &stop_recurse, &checked_nodes_number);
}

Node_status_t TreeRecursiveCheckNode(Tree_t* tree, Tree_node_t* current_node, Tree_node_t* sought_node,
                                     bool* p_stop_recurse, ssize_t* p_checked_nodes_number) {
    assert(tree != NULL);
    if ((current_node != NULL) && (*p_stop_recurse == false) && (*p_checked_nodes_number < tree->size)) {
        (*p_checked_nodes_number)++;

        if (current_node == sought_node) {
            *p_stop_recurse = true;
            return IN;
        }
        else {
            if ((TreeRecursiveCheckNode(tree, current_node->left_branch, sought_node, p_stop_recurse, p_checked_nodes_number) == IN) ||
                (TreeRecursiveCheckNode(tree, current_node->right_branch, sought_node, p_stop_recurse, p_checked_nodes_number) == IN))
                return IN;
            else
                return OUT;
        }
    }
    else {
        if (*p_checked_nodes_number >= tree->size)
            *p_stop_recurse = true;

        return OUT;
    }
}


Tree_node_t* TreeAllocNode(Tree_type_t value, Tree_node_t* parent, Tree_node_t* left_branch,
                           Tree_node_t* right_branch, ssize_t root_distance) {
    Tree_node_t* new_node = (Tree_node_t*)malloc(sizeof(Tree_node_t));
    if (new_node == NULL) {
        PrintErrorMessage("Failed to allocate memory", new_node);
        return NULL;
    }

    #if TREE_TYPE == STRING
        strncpy((char*)new_node->value, value, MAX_STRING_NODE_VALUE_SIZE);
    #else
        new_node->value = value;
    #endif

    new_node->parent = parent;
    new_node->left_branch = left_branch;
    new_node->right_branch = right_branch;
    new_node->root_distance = root_distance;

    return new_node;
}


Tree_report_t TreeOrderedInsert(Tree_t* tree, Tree_type_t value,
                                int Compare(const void*, const void*)) {
    assert(tree != NULL);

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL)
        return tree->last_function_success = FATAL_ERROR;

    if (tree->root == NULL)
        TreeAddRoot(tree, value);
    else
        TreeRecursiveOrderedInsert(tree, tree->root, value, Compare);

    if (TREE_VERIFY(tree, LOUD) != TREE_NORMAL)
        return tree->last_function_success = FATAL_ERROR;

    return tree->last_function_success = SUCCESS;
}

void TreeRecursiveOrderedInsert(Tree_t* tree, Tree_node_t* current_node, Tree_type_t value,
                                int Compare(const void*, const void*)) {
    assert(tree != NULL); assert(current_node != NULL);

    if (Compare(&value, &current_node->value) >= 0) {
        if (current_node->right_branch == NULL)
            TreeAddRightBranch(tree, current_node, value);
        else
            TreeRecursiveOrderedInsert(tree, current_node->right_branch, value, Compare);
    }
    else {
        if (current_node->left_branch == NULL)
            TreeAddLeftBranch(tree, current_node, value);
        else
            TreeRecursiveOrderedInsert(tree, current_node->left_branch, value, Compare);
    }
}


void Dump(Tree_node_t* node) {
    if (node != NULL) {
        Dump(node->left_branch);
        printf(TREE_TYPE_SPECIFIER " %d\t", node->value, node->root_distance);
        Dump(node->right_branch);
    }
}


Tree_err_t TreeVerify(Tree_t* tree, Volume_t volume,
                      const char* calling_function_name, int from_line) {
    assert(tree != NULL); assert(calling_function_name != NULL);
    ONDEB(assert(Tree_log_file != NULL);)

    (void)TreeDiagnoseError(tree, volume, calling_function_name, from_line);

    ONDEB(
        if (volume == LOUD) {
            char message[MAX_MESSAGE_SIZE];
            sprintf(message, "Diagnostic from %s(), line %d", calling_function_name, from_line);

            TreeGraphicDump(tree, message);
        }
    )

    return tree->error;
}


Tree_err_t TreeDiagnoseError(Tree_t* tree, Volume_t volume,
                             const char* calling_function_name, int from_line) {
    assert(tree != NULL); assert(calling_function_name != NULL);

    ONDEB(
        if (Tree_log_file == NULL)
            Tree_log_file = stdout;
    )

    tree->error = TREE_NORMAL;

    if ((tree->size > 0) && (tree->root == NULL)) {
        ONDEB(fprintf(Tree_log_file, "Diagnostic from %s(), line %d: Fatal error - there is no root\n", calling_function_name, from_line);)
        tree->error = (Tree_err_t)(tree->error | TREE_ROOT_ERROR);
    }
    if ((tree->root != NULL) && (tree->root->parent != NULL)) {
        ONDEB(fprintf(Tree_log_file, "Diagnostic from %s(), line %d: Fatal error - invalid parent pointer for root\n", calling_function_name, from_line);)
        tree->error = (Tree_err_t)(tree->error | TREE_ROOT_ERROR);
    }
    if (tree->size < 0) {
        ONDEB(fprintf(Tree_log_file, "Diagnostic from %s(), line %d: Fatal error - incorrect size %ld, it must be positive\n", calling_function_name, from_line, tree->size);)
        tree->error = (Tree_err_t)(tree->error | TREE_ROOT_ERROR);
    }

    if (volume == LOUD) {
        if (TreeCheckLinkError(tree) == TREE_LINK_ERROR) {
            ONDEB(fprintf(Tree_log_file, "Diagnostic from %s(), line %d: Warning - tree contains cycle\n", calling_function_name, from_line);)
            tree->error = (Tree_err_t)(tree->error | TREE_LINK_ERROR);
        }
    }

    if (tree->error == TREE_NORMAL) {
        ONDEB(fprintf(Tree_log_file, "Diagnostic from %s(), line %d: No fatal damage\n", calling_function_name, from_line);)
    }

    return tree->error;
}

Tree_err_t TreeCheckLinkError(Tree_t* tree) {  // REVIEW link checker
    assert(tree != NULL);

    bool stop_recurse = false;
    ssize_t checked_nodes_number = 0;

    return TreeRecursiveCheckLinkError(tree, tree->root, &stop_recurse, &checked_nodes_number);
}

Tree_err_t TreeRecursiveCheckLinkError(Tree_t* tree, Tree_node_t* current_node,
                                       bool* p_stop_recurse, ssize_t* p_checked_nodes_number) {
    assert(tree != NULL);

    if (current_node == NULL)
        return TREE_NORMAL;

    if ((current_node != NULL) && (*p_stop_recurse == false) && (*p_checked_nodes_number < tree->size)) {
        (*p_checked_nodes_number)++;

        Tree_err_t left_branch_result = TREE_NORMAL;
        Tree_err_t right_branch_result = TREE_NORMAL;

        if (current_node->left_branch != NULL) {
            if ((current_node->left_branch->parent != current_node) ||
                ((current_node->root_distance + 1) != current_node->left_branch->root_distance))
                return TREE_LINK_ERROR;

            left_branch_result = TreeRecursiveCheckLinkError(tree, current_node->left_branch, p_stop_recurse, p_checked_nodes_number);
        }

        if (current_node->right_branch != NULL) {
            if ((current_node->right_branch->parent != current_node) ||
                ((current_node->root_distance + 1) != current_node->right_branch->root_distance))
                return TREE_LINK_ERROR;

            right_branch_result = TreeRecursiveCheckLinkError(tree, current_node->right_branch, p_stop_recurse, p_checked_nodes_number);
        }

        if ((left_branch_result == TREE_LINK_ERROR) || (right_branch_result == TREE_LINK_ERROR))
            return TREE_LINK_ERROR;
        else
            return TREE_NORMAL;
    }

    if (*p_checked_nodes_number >= tree->size)
        *p_stop_recurse = true;

    return TREE_LINK_ERROR;
}


void TreeGraphicDump(Tree_t* tree, const char* message) {
    assert(tree != NULL);

    if (tree->graphic_dump_number == 1) {
        if (TreeStartLogging(tree) != START_LOG_NORMAL)
            return;
    }
    else {
        if (Tree_html_log_file == NULL) {
            printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
                   __func__, __LINE__, TREE_HTML_LOG_FILE_NAME, __func__);
            return;
        }
    }

    if (message == NULL)
        fprintf(Tree_html_log_file, "<h2>dump%d</h2>\n\n", tree->graphic_dump_number);
    else
        fprintf(Tree_html_log_file, "<h2>%s</h2>\n\n", message);

    TreeCreateInformationTable(tree);

    TreeCreateGraphic(tree);

    fprintf(Tree_html_log_file, "<hr>\n\n");

    tree->graphic_dump_number++;
}


Start_log_err_t TreeStartLogging(Tree_t* tree) {
    assert(tree != NULL);

    time_t current_time = time(NULL);
    struct tm* local_time = localtime(&current_time);

    char time_suffix[MAX_TIME_SUFFIX_SIZE] = {};
    strftime(time_suffix, MAX_TIME_SUFFIX_SIZE, "_%H_%M_%S-%d_%m_%Y", local_time);

    char folder_name[MAX_FOLDER_NAME_SIZE] = {};
    sprintf(folder_name, "%s%s", TREE_DUMP_FOLDER_PREFIX, time_suffix);

    strncpy(tree->dump_folder_name, folder_name, MAX_FOLDER_NAME_SIZE);

    char command[MAX_COMMAND_SIZE] = {};
    sprintf(command, "mkdir %s 2>nul", folder_name);

    system(command);

    char html_log_file_path[MAX_FILE_NAME_SIZE] = {};
    sprintf(html_log_file_path, "%s/%s", folder_name, TREE_HTML_LOG_FILE_NAME);

    Tree_html_log_file = fopen(html_log_file_path, "w");
    if (Tree_html_log_file == NULL) {
        printf("note from %s(), line %d: can't open %s, %s was cancelled\n",
               __func__, __LINE__, html_log_file_path, __func__);
        return START_LOG_OPEN_ERROR;
    }

    fprintf(Tree_html_log_file,
        "<meta charset=\"UTF-8\">\n\n"
        "<style>\n"
        "\tbody {background-color: %s; font-family: sans-serif; margin: 20px;}\n"
        "\tbody::before {content: \"\"; position: fixed; top: 0; left: 0; width: 100%%; height: 100%%; z-index: -1;\n"
        "\t\t\t\t  background-image: url('../%s'); background-repeat: no-repeat;\n"
		"\t\t\t\t  background-size: cover; background-position: right;}\n"
        "\th1 {text-align: center; color: %s;}\n"
        "\th2 {text-align: center; color: %s;font-size: 20px; margin-top: 20px; margin-bottom: 20px;}\n"
        "\th3 {color: %s; font-size: 18px; margin-top: 40px; margin-bottom: 10px;}\n"
        "\thr {border: none; height: 3px; background-color: %s; margin: 20px 0;}\n"
        "\ttable {border-collapse: collapse; max-width: fit-content;}\n"
        "\ttd {border: 2px solid %s; padding: 10px; text-align: center; font-weight: bold; color: %s}\n"
        "\t.data_table tr:first-child td {background-color: %s; font-size: 14px; font-weight: bold; color: %s;}\n"
        "\t.tree_information_table td:first-child {background-color: %s; font-size: 14px; font-weight: bold; color: %s;}\n"
        "</style>\n\n"
        "<h1>Log</h1>\n\n"
        "<hr>\n\n",
        BACK_COLOR, BACKGROUND_IMAGE_NAME,
        HEAD_FONT_COLOR, HEAD_FONT_COLOR, HEAD_FONT_COLOR, HEAD_FONT_COLOR, HEAD_LINE_COLOR,
        NEXT_FILL_COLOR, HEAD_FILL_COLOR, HEAD_FONT_COLOR, HEAD_FILL_COLOR, HEAD_FONT_COLOR);

    return START_LOG_NORMAL;
}


void TreeCreateInformationTable(Tree_t* tree) {
    assert(tree != NULL);

    if (Tree_html_log_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_HTML_LOG_FILE_NAME, __func__);
        return;
    }

    fprintf(Tree_html_log_file,
        "<h3>Information</h3>\n"
        "<table class = \"tree_information_table\">\n"
        "\t<tr><td>root</td><td>%p</td></tr>\n"
        "\t<tr><td>size</td><td>%ld</td></tr>\n",
        tree->root, tree->size);

    fprintf(Tree_html_log_file, "\t<tr><td>errors</td><td>");
    TreeHtmlPrintErrors(tree);
    fprintf(Tree_html_log_file, "</td></tr>\n</table>\n\n");
}


#define TREE_HTML_CHECK_ERROR(errors_code, checking_error)  \
    if (((errors_code) & (checking_error)) != 0) {          \
        fprintf(Tree_html_log_file, #checking_error "<br>");\
    }

void TreeHtmlPrintErrors(Tree_t* tree) {
    assert(tree != NULL);

    if (Tree_html_log_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_HTML_LOG_FILE_NAME, __func__);
        return;
    }

    if (tree->error == TREE_NORMAL) {
        fprintf(Tree_html_log_file, "no error");
        return;
    }

    TREE_HTML_CHECK_ERROR(tree->error, TREE_ROOT_ERROR);
    TREE_HTML_CHECK_ERROR(tree->error, TREE_SIZE_ERROR);
    TREE_HTML_CHECK_ERROR(tree->error, TREE_LINK_ERROR);
}


void TreeCreateGraphic(Tree_t* tree) {
    assert(tree != NULL);

    Tree_graphviz_file = fopen(TREE_GRAPHVIZ_FILE_NAME, "w");
    if (Tree_graphviz_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_GRAPHVIZ_FILE_NAME, __func__);
        return;
    }

    TreeCreatePng(tree);

    fclose(Tree_graphviz_file);
    Tree_graphviz_file = NULL;

    char dump_file_name[MAX_FILE_NAME_SIZE] = {};
    sprintf(dump_file_name, "dump%d.png", tree->graphic_dump_number);

    char command[MAX_COMMAND_SIZE] = {};
    sprintf(command, "dot -Tpng %s -o %s/%s",
            TREE_GRAPHVIZ_FILE_NAME, tree->dump_folder_name, dump_file_name);

    system(command);

    fprintf(Tree_html_log_file, "<h3>Graphic</h3>\n<img src=\"%s\" alt=\"%s\">\n\n",
            dump_file_name, dump_file_name);
}


void TreeCreatePng(Tree_t* tree) {
    assert(tree != NULL);

    if (Tree_graphviz_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_GRAPHVIZ_FILE_NAME, __func__);
        return;
    }

    fprintf(Tree_graphviz_file, "digraph G {\n\n");
    fprintf(Tree_graphviz_file, "bgcolor = \"transparent\"\n");//, BACK_COLOR);
    fprintf(Tree_graphviz_file, "node [shape = none];\n\n");

    TreeCreateNodes(tree);

    TreeCreateLinks(tree);

    fprintf(Tree_graphviz_file, "}\n");
}


void TreeCreateNodes(Tree_t* tree) {
    assert(tree != NULL);

    if (Tree_graphviz_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_GRAPHVIZ_FILE_NAME, __func__);
        return;
    }

    RecursiveTreeCreateNodes(tree, tree->root);
}

void RecursiveTreeCreateNodes(Tree_t* tree, Tree_node_t* current_node) {
    assert(tree != NULL);

    if (current_node != NULL) {
        RecursiveTreeCreateNodes(tree, current_node->left_branch);

        Node_colors_t node_colors = {};

        SetNodeColors(&node_colors, TAIL_LINE_COLOR, TAIL_FILL_COLOR, TAIL_FONT_COLOR); // TODO light element
        TreeCreateNode(tree, current_node, &node_colors);

        RecursiveTreeCreateNodes(tree, current_node->right_branch);
    }
}


void SetNodeColors(Node_colors_t* node_colors,
                   const char* line_color, const char* fill_color, const char* font_color) {
    assert(node_colors != NULL);

    if (Tree_graphviz_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_GRAPHVIZ_FILE_NAME, __func__);
        return;
    }

    node_colors->line_color = line_color;
    node_colors->fill_color = fill_color;
    node_colors->font_color = font_color;
}


void TreeCreateNode(Tree_t* tree, Tree_node_t* node, Node_colors_t* node_colors) {
    assert(tree != NULL); assert(node != NULL); assert(node_colors != NULL);

    if (Tree_graphviz_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_GRAPHVIZ_FILE_NAME, __func__);
        return;
    }

    fprintf(Tree_graphviz_file,
        "node%p [label=<\n"
            "\t<TABLE BORDER = \"1\" CELLBORDER = \"0\" CELLSPACING = \"0\" CELLPADDING=\"4\" STYLE = \"ROUNDED\"\n"
            "\tCOLOR = \"%s\"\n"
            "\tBGCOLOR = \"%s\">\n"
            "\t<TR><TD COLSPAN = \"2\"><FONT COLOR = \"%s\">" TREE_TYPE_SPECIFIER "</FONT></TD></TR>\n"
            "\t</TABLE>\n"
        ">];\n\n",
        node, node_colors->line_color, node_colors->fill_color,
        node_colors->font_color, node->value);
}


void TreeCreateLinks(Tree_t* tree) {
    assert(tree != NULL);

    if (Tree_graphviz_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_GRAPHVIZ_FILE_NAME, __func__);
        return;
    }

    RecursiveCreatLinks(tree, tree->root, ROOT);
}

void RecursiveCreatLinks(Tree_t* tree, Tree_node_t* current_node, Walk_div_t div) {
    assert(tree != NULL);

    if (current_node != NULL) { // ANCHOR if recurse starts not from root it node won't have parent but parent != NULL, macros ROOT iz solution!
        if (current_node->parent != NULL)
            TreeCreateLink(tree, current_node, div);

        RecursiveCreatLinks(tree, current_node->left_branch, LEFT);
        RecursiveCreatLinks(tree, current_node->right_branch, RIGHT);
    }
}


void TreeCreateLink(Tree_t* tree, Tree_node_t* node, Walk_div_t div) {
    assert(tree != NULL); assert(node != NULL); assert(node->parent != NULL);

    if (Tree_graphviz_file == NULL) {
        printf("note from %s(), line %d: %s isn't open, %s was cancelled\n",
               __func__, __LINE__, TREE_GRAPHVIZ_FILE_NAME, __func__);
        return;
    }

    const char* color = NULL;

    if (div == LEFT)
        color = LINE_POSITIVE_COLOR;
    else if (div == RIGHT)
        color = LINE_NEGATIVE_COLOR; // FIXME red color if not double lnk with parent or not increase root distance
    else
        return;

    fprintf(Tree_graphviz_file, "node%p -> node%p [dir = forward, penwidth = %f, color = \"%s\"];\n",
            node->parent, node, DEFAULT_THICKNESS, color);
}
