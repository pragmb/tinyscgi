#include <stdio.h>
#include <stdlib.h>
#include "tscgi/request.h"

struct header_list * create_header_list()
{
    struct header_list *list;

    list = (struct header_list *) malloc(sizeof(struct header_list));
    if (!list)
    {
        perror("could not create header list");
        abort();
    }

    list->item.name = NULL;
    list->item.value = NULL;
    list->next = NULL;

    return list;
}

void destory_header_list(struct header_list *list)
{
    struct header_list *current, *next;

    current = list;
    while (current)
    {
        next = current->next;
        free(current);
        current = next;
    }
}

int append_header_list(struct header_list *list, struct header *item)
{
    while (list->next)
        list = list->next;

    list->next = create_header_list();
    list->next->item = *item;

    return 0;
}

int parse_headers(const char *buffer, size_t len, struct header_list *headers)
{
    size_t i;
    enum { name_ptr, name, value_ptr, value } s;
    struct header_list *n;

    for (i=0, s=name_ptr, n=headers; i<len; i++)
    {
        switch (s)
        {
            case name_ptr:
                n->item.name = buffer[i] ? (char *) &buffer[i] : NULL;
                s = name;
                break;
            case name:
                if (buffer[i] == '\0')
                    s = value_ptr;
                break;
            case value_ptr:
                n->item.value = buffer[i] ? (char *) &buffer[i] : NULL;
                s = value;
                break;
            case value:
                if (buffer[i] == '\0' && (i + 1) < len)
                {
                    n->next = create_header_list();
                    n = n->next;
                    s = name_ptr;
                }
                break;
        }
    }

    return 0;
}
