#ifndef LIST
#define LIST

#include "Libs/logging.h"

#define LOGS_TO_HTML

typedef int Elem_t;

const Elem_t data_poison = 0x7011A;
const Elem_t prev_poison = 0x011A7;

struct List_elem {
    List_elem *prev = nullptr;
    List_elem *next = nullptr;
    Elem_t      val = 0;
};

struct List {
    Creation_logs* cr_logs = nullptr;
    List_elem*   zero_elem = nullptr;
    size_t            size = 0;
};

enum List_errors {
    NO_LIST_ERRORS      = 0,
    NULLPTR_TO_LIST     = 1 << 0,
    NULLPTR_TO_DATA     = 1 << 1,
    NULLPTR_TO_LOGS     = 1 << 2,
    NOT_ENOUGTH_MEM     = 1 << 3,
    POP_ZERO_ELEM       = 1 << 4,
    POS_DONT_EXIST      = 1 << 5,
    CANT_OPEN_FILE      = 1 << 6,
    CANT_GENER_PIC      = 1 << 7,
    BROKEN_IN_USE_LOOP  = 1 << 8,
};


static const char *RESERVED_FILL__COLOR = "#BEA0A0";
static const char *RESERVED_FRAME_COLOR = "#361C1C";

static const char *DATACELL_FILL__COLOR = "#9AA5BB";
static const char *DATACELL_FRAME_COLOR = "#232D42";

static const char *PREV_ARROW_COLOR = "#54303c";
static const char *NEXT_ARROW_COLOR = "#303C54";


#define RETURN_IF(error) if (error)      \
                            return errors;

#define CHECK_FOR_NULLPTR(ptr, errors, error_name) if(ptr == nullptr)       \
                                                        errors |= error_name;

#define CHECK_FOR_NULLPTR_WITH_MESSAGE(ptr, errors, error_name, message) \
        if (ptr == nullptr) {                                            \
            errors |= error_name;                                        \
            PrintToLogs(message);                                        \
            return errors;                                               \
        }

#define list_ctr(list) real_list_ctr(list, __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define dump_list(list, message, ...) real_dump_list(list, __FILE__, __PRETTY_FUNCTION__, __LINE__,\
                                                                             message, ##__VA_ARGS__)

#define Print_code(format, ...)                    \
        fprintf(code_output, format, ##__VA_ARGS__);


#define Print_busy(elem, i)                                                           \
        Print_code("node%p [label=\"node %zu|{val: %d|{prev: %p|adr : %p|next: %p}}\""\
                                                 ",fillcolor=\"%s\",color=\"%s\"];\n",\
                                     elem, i, elem->val, elem->prev, elem, elem->next,\
                                           DATACELL_FILL__COLOR, DATACELL_FRAME_COLOR);

#define Print_reserved(elem)                                                                                 \
        Print_code("node%p [label=\"reserved|prev: %p|adr : %p|next: %p\",fillcolor=\"%s\",color=\"%s\"];\n",\
                    elem, elem->prev, elem, elem->next, RESERVED_FILL__COLOR, RESERVED_FRAME_COLOR); 

#define Print_in_use_next_arrow(elem)                                                      \
        Print_code("node%p->node%p [color=\"%s\",constraint=false];\n", elem, elem->next,  \
                                                                          NEXT_ARROW_COLOR);

#define Print_in_use_prev_arrow(elem)                                                      \
        Print_code("node%p->node%p [color=\"%s\",constraint=false];\n", elem, elem->prev,  \
                                                                          PREV_ARROW_COLOR);

#define Print_connection__arrow(elem)                                               \
        Print_code("node%p->node%p [style=invis, weight = 100]\n", elem, elem->next);
        

int real_list_ctr(List *list, const char *file, const char *func, int line);
int list_dtor(List *list);

List_elem* list_insert(List *list, List_elem* elem, Elem_t data);

List_elem* list_insert_head(List *list, Elem_t elem);
List_elem* list_insert_back(List *list, Elem_t elem);

Elem_t list_pop(List *list, List_elem *elem);

Elem_t list_pop_head(List *list);
Elem_t list_pop_back(List *list);

List_elem* list_get_next(const List *list, List_elem *elem);
List_elem* list_get_prev(const List *list, List_elem *elem);

int list_verificator(const List *list);

int real_dump_list(const List *list, const char* file, const char* func, int line, const char *message, ...);

List_elem* get_pointer_by_index(List *list, size_t logical_index, int *errors = nullptr);

#endif