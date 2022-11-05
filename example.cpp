#include <stdio.h>

#include "list.h"
#include "Libs/logging.h"

int main() {
    FILE *output = CreateLogFile("logs.html");

    List list1 = {};
    list_ctr(&list1);

    dump_list(&list1, "Freshy created list:\n");

    for (int i = 1; i < 5; ++i) {
        list_insert(&list1, list1.zero_elem, i);
        dump_list(&list1, "List after adding element %d to head\n", i);
    }

    for (int i = 6; i < 10; ++i) {
        list_insert_back(&list1, i);
        dump_list(&list1, "List after adding element %d to back\n", i);
    }

    dump_list(&list1, "Poped first element: %d. List after popping:\n", list_pop_head(&list1));

    dump_list(&list1, "Poped last element: %d. List after popping:\n", list_pop_back(&list1));

    list_insert_head(&list1, 14);

    dump_list(&list1, "List after inserting element to the first position:\n");

    list_insert_back(&list1, 17);

    dump_list(&list1, "List after inserting element to the last position:\n");
    
    list_dtor(&list1);

    dump_list(&list1, "List after destruction:\n");

    fclose(output);

    printf("finished\n");

    return 0;
}