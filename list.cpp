#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include "list.h"


static void dump_list_data(const List *list, FILE *output);

static int verify_loop(const List *list);

static int  generate_graph_code(const List *list);
static void generate_file_name(char *filename, const char *extension);


static const int max_file_with_graphviz_code_name_len = 30;
static const int max_generation_png_command_len = 100;
static const int max_png_file_name_len = 30;


int real_list_ctr(List *list, const char *file, const char *func, int line) {
    if (list == nullptr) {
        dump_list(list, "can't create list: nullptr to it\n");
        return NULLPTR_TO_LIST;
    }

    list->zero_elem = nullptr;
    list->cr_logs   = nullptr;

    list->zero_elem = (List_elem*) calloc(1, sizeof(List_elem));

    if (list->zero_elem == nullptr) {
        dump_list(list, "can't create list: not enough memory\n");
        list_dtor(list);
        return NOT_ENOUGTH_MEM;
    }

    list->cr_logs = (Creation_logs*) calloc(1, sizeof(Creation_logs));

    if (list->zero_elem == nullptr) {
        dump_list(list, "can't create list: not enough memory for logs\n");
        list_dtor(list);
        return NOT_ENOUGTH_MEM;
    }

    list->size = 0;

    list->zero_elem->val  = data_poison;
    list->zero_elem->prev = list->zero_elem;
    list->zero_elem->next = list->zero_elem;

    list->cr_logs->file_of_creation = file;
    list->cr_logs->func_of_creation = func;
    list->cr_logs->line_of_creation = line;

    return NO_LIST_ERRORS;
}

int list_dtor(List *list) {
    if (list == nullptr) {
        dump_list(list, "can't destruct list: pointer to it is nullptr\n");
        return NULLPTR_TO_LIST;
    }

    List_elem *next = list->zero_elem;

    for (size_t i = 0; i < list->size; ++i) {
        free(next->prev);
        next = next->next;
    }

    free(list->cr_logs);

    list->zero_elem = nullptr;
    list->cr_logs   = nullptr;

    list->size = 0;

    return NO_LIST_ERRORS;
}

List_elem* list_insert(List *list, List_elem* elem, Elem_t data) {
    int errors = NO_LIST_ERRORS;

    if (list == nullptr) {
        errors |= NULLPTR_TO_LIST;
    }

    if (elem == nullptr) {
        errors |= NULLPTR_TO_DATA;
    }

    errors |= list_verificator(list);

    if (errors != 0) {
        dump_list(list, "Can't insert because of errors. Error code: %d", errors);
        return nullptr;
    }

    List_elem *next = elem->next;
    List_elem *prev = elem;

    elem->next = (List_elem*) calloc(1, sizeof(List_elem));

    elem->next->next = next;
    elem->next->prev = prev;

    next->prev = elem->next;

    elem->next->val = data;

    ++list->size;

    return elem->next;
}

List_elem* list_insert_head(List *list, Elem_t elem) {
    int errors = NO_LIST_ERRORS;
    errors |= list_verificator(list);

    if (errors != 0) {
        dump_list(list, "Can't insert because of errors. Error code: %d", errors);
        return 0;
    }

    return list_insert(list, list->zero_elem, elem);
}

List_elem* list_insert_back(List *list, Elem_t elem) {
    int errors = NO_LIST_ERRORS;
    errors |= list_verificator(list);

    if (errors != 0) {
        dump_list(list, "Can't insert because of errors. Error code: %d", errors);
        return 0;
    }

    return list_insert(list, list->zero_elem->prev, elem);
}

Elem_t list_pop(List *list, List_elem *elem) {
    int errors = NO_LIST_ERRORS;

    if (list == nullptr) {
        errors |= NULLPTR_TO_LIST;
    }

    if (elem == nullptr) {
        errors |= NULLPTR_TO_DATA;
    }

    errors |= list_verificator(list);

    if (errors != 0) {
        dump_list(list, "Can't pop because of errors. Error code: %d", errors);
        return 0;
    }

    List_elem *prev = elem->prev;
    List_elem *next = elem->next;

    Elem_t popped = elem->val;

    prev->next = next;
    next->prev = prev;

    free(elem);

    --list->size;

    return popped;
}

Elem_t list_pop_head(List *list) {
    int errors = NO_LIST_ERRORS;
    errors |= list_verificator(list);

    if (errors != 0) {
        dump_list(list, "Can't pop because of errors. Error code: %d", errors);
        return 0;
    }

    return list_pop(list, list->zero_elem->next);
}

Elem_t list_pop_back(List *list) {
    int errors = NO_LIST_ERRORS;
    errors |= list_verificator(list);

    if (errors != 0) {
        dump_list(list, "Can't pop because of errors. Error code: %d", errors);
        return 0;
    }

    return list_pop(list, list->zero_elem->prev);
}


List_elem* list_get_next(const List *list, List_elem *elem) {
    if (list == nullptr) {
        PrintToLogs("Can't return next element in non existing list (pointer to list is nullptr)\n");
        return 0;
    }

    if (elem == nullptr) {
        dump_list(list, "Can't return next for non-existing element. List:\n");
        return 0;
    }

    List_elem *next = elem->next;

    if (next == list->zero_elem) {
        next = list->zero_elem->next;
    }

    return next;
}

List_elem* list_get_prev(const List *list, List_elem *elem) {
    if (list == nullptr) {
        PrintToLogs("Can't return prev element in non existing list (pointer to list is nullptr)\n");
        return 0;
    }

    if (elem == nullptr) {
        dump_list(list, "Can't return next for non-existing element. List:\n");
        return 0;
    }

    List_elem *prev = elem->prev;

    if (prev == list->zero_elem) {
        prev = list->zero_elem->prev;
    }

    return prev;
}


int list_verificator(const List *list) {
    int errors = NO_LIST_ERRORS;

    CHECK_FOR_NULLPTR(list, errors, NULLPTR_TO_LIST);

    RETURN_IF(errors & NULLPTR_TO_LIST);

    CHECK_FOR_NULLPTR(list->cr_logs, errors, NULLPTR_TO_LOGS);
    CHECK_FOR_NULLPTR(list->zero_elem,    errors, NULLPTR_TO_DATA);

    errors |= verify_loop(list);

    return errors;
}

int real_dump_list(const List *list, const char* file, const char* func, int line, const char *message, ...) {
    int errors = list_verificator(list);

    FILE *output = GetLogStream();

    fprintf(output, "<b>List dump called in %s(%d), function %s: ", file, line, func);

    va_list ptr = {};
    va_start(ptr, message);
    vfprintf(output, message, ptr);
    va_end(ptr);

    fprintf(output, "\n</b>");
    
    if (errors & NULLPTR_TO_LIST) {
        fprintf(output, "Can't dump list from nullptr pointer\n");
        return errors;
    }

    fprintf(output, "List [%p] ", list);

    if (errors & NULLPTR_TO_LOGS) {
        fprintf(output, "without creation info (logs ptr in nullptr):\n");
    } else {
        fprintf(output, "created at %s(%d), function %s:\n", list->cr_logs->file_of_creation, 
                                                             list->cr_logs->line_of_creation, 
                                                             list->cr_logs->func_of_creation);
    }

    fprintf(output, "List info:\n");
    fprintf(output, "\tsize  : %zu\n", list->size);

    fflush(output);

    if (errors & NULLPTR_TO_DATA) {
        fprintf(output, "\tCan't print data: ptr to data is nullptr\n");
    } else {
        fprintf(output, "\tList data visualisation:\n");
        generate_graph_code(list);
    }

    fprintf(output, "\n");
    fprintf(output, "\n<hr>\n");
    fflush(output);

    return errors;
}

List_elem* get_pointer_by_index(List *list, size_t logical_index, int *errors) {

    if (list == nullptr) {
        if (errors == nullptr) {
            return nullptr;
        }

        *errors |= NULLPTR_TO_LIST;
        return nullptr;
    }

    List_elem *pointer = list->zero_elem;

    for (size_t i = 0; i < logical_index; ++i) {

        if (pointer == nullptr) {
            if (errors == nullptr) {
                return 0;
            }

            *errors |= NULLPTR_TO_DATA;
            return nullptr;
        }
        
        pointer = pointer->next;
    }

    return pointer;
}

static void dump_list_data(const List *list, FILE *output) {
    assert(list != nullptr);
    assert(output != nullptr);

    List_elem *next = list->zero_elem;

    fprintf(output, "\t\tindex: {");

    for (size_t i = 0; i <= list->size; ++i) {
        fprintf(output, "%zu ", i);
    }

    fprintf(output, "}\n\t\tdata:  {");

    for (size_t i = 0; i <= list->size; ++i) {
        fprintf(output, "%16d ", next->val);

        next = next->next;
    }

    next = list->zero_elem;

    fprintf(output, "}\n\t\tprev:  {");

    for (size_t i = 0; i <= list->size; ++i) {
        fprintf(output, "%p ", next->prev);

        next = next->next;
    }

    next = list->zero_elem;

    fprintf(output, "}\n\t\tnext:  {");

    for (size_t i = 0; i <= list->size; ++i) {
        fprintf(output, "%p ", next->next);

        next = next->next;
    }

    fprintf(output, "}\n");
}

static int generate_graph_code(const List *list) {
    char code_filename[max_file_with_graphviz_code_name_len] = {};
    generate_file_name(code_filename, "dot");

    FILE *code_output = fopen(code_filename, "w");

    int errors = NO_LIST_ERRORS;

    CHECK_FOR_NULLPTR_WITH_MESSAGE(code_output, errors, CANT_OPEN_FILE, 
                                   "Error: can't open file for graph generation\n");

    Print_code("digraph G{\n");
    Print_code("rankdir=LR;");
    Print_code("node [shape=record,style=\"filled\"];\n");
    Print_code("splines=ortho;\n");

    Print_code("info [label = \"List | size: %zu \"]", list->size);
    Print_code("info->node%p [style=invis, weight = 100]\n", list->zero_elem);

    Print_reserved(list->zero_elem);
    Print_connection__arrow(list->zero_elem);
    Print_in_use_next_arrow(list->zero_elem);
    Print_in_use_prev_arrow(list->zero_elem);

    if (list->size > 0) {
        List_elem *elem = list->zero_elem;

        size_t i = 1;

        for (; i < list->size; ++i) {
            elem = elem->next;

            Print_busy(elem, i);

            Print_connection__arrow(elem);
            Print_in_use_next_arrow(elem);
            Print_in_use_prev_arrow(elem);
        }

        elem = elem->next;

        Print_busy             (elem, i);
        Print_in_use_next_arrow(elem);
        Print_in_use_prev_arrow(elem);

    }
    
    Print_code("}");

    fclose(code_output);

    char command[max_generation_png_command_len] = {};
    char png_file_name[max_png_file_name_len] = {};
    generate_file_name(png_file_name, "png");
    sprintf(command, " c:\\GitHub\\ClassicList\\Libs\\Graphviz\\bin\\dot.exe %s -o %s -T png", 
                                                         code_filename, png_file_name);
    //printf("\n%s\n%s\n", code_filename, png_file_name);

    if (system(command) != 0) {
        PrintToLogs("Error: can't generate picture. Text dump:\n");
        dump_list_data(list, GetLogStream());
        return CANT_GENER_PIC;
    }

    #ifdef LOGS_TO_HTML
    fprintf(GetLogStream(), "\n<img src=\"%s\">\n", png_file_name);
    #else
    fprintf(GetLogStream(), "Picture is generated. You can find it by name %s.\n", png_file_name);
    #endif

    return NO_LIST_ERRORS;
}

static void generate_file_name(char *filename, const char *extension)  {
    static int file_with_graphviz_code_counter = 0;
    sprintf(filename, "Graphs/graph_%d.%s", file_with_graphviz_code_counter, extension);
    ++file_with_graphviz_code_counter;
}

static int verify_loop(const List *list) {
    int errors = 0;

    CHECK_FOR_NULLPTR(list, errors, NULLPTR_TO_LIST);
    CHECK_FOR_NULLPTR(list->zero_elem, errors, NULLPTR_TO_DATA);

    RETURN_IF(errors & NULLPTR_TO_LIST);
    RETURN_IF(errors & NULLPTR_TO_DATA);

    if (list->size == 0) {
        if (list->zero_elem->next == list->zero_elem && list->zero_elem->prev == list->zero_elem) {
            return NO_LIST_ERRORS;
        }
        return BROKEN_IN_USE_LOOP;
    }

    List_elem *next = list->zero_elem;

    for (size_t i = 0; i < list->size; ++i) {
        next = next->next; //next

        if (next == list->zero_elem) {
            return BROKEN_IN_USE_LOOP;
        }
    }

    next = next->next;

    if (next != list->zero_elem) {
        return BROKEN_IN_USE_LOOP;
    }

    return NO_LIST_ERRORS;
}
