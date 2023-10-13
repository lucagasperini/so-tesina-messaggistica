#include "stack.h"

#include "log.h"
#include <string.h> // memcpy, strcpy

matrix_stack* matrix_stack_push(matrix_stack* head, void* data, size_t len)
{
        matrix_stack* new_head;
        MATRIX_MALLOC(new_head, sizeof(matrix_stack));
        MATRIX_MALLOC(new_head->entry.data, len);
        new_head->next = NULL;

        memcpy(new_head->entry.data, data, len);
        new_head->entry.len = len;

        if (head != NULL) {
                new_head->next = head;
        }

        return new_head;
}

matrix_stack* matrix_stack_pop(matrix_stack* head, matrix_stack_entry* entry)
{
        if(head == NULL) {
                return NULL;
        }

        matrix_stack* new_head = head->next;
        entry->data = head->entry.data;
        entry->len = head->entry.len;
        MATRIX_FREE(head);
        return new_head;
}

matrix_stack* matrix_stack_next(matrix_stack* head, matrix_stack_entry* entry)
{
        if(head == NULL) {
                return NULL;
        }

        entry->data = head->entry.data;
        entry->len = head->entry.len;

        return head->next;
}

void matrix_stack_free(matrix_stack* head)
{
        if(head == NULL) {
                MATRIX_LOG_DEBUG("Try to free a nullptr stack");
                return;
        }
        matrix_stack* next_ptr;
        while(head != NULL) {
                next_ptr = head->next;
                MATRIX_FREE(head->entry.data);
                MATRIX_FREE(head);
                head = next_ptr;
        }
}

matrix_stack_str* matrix_stack_str_push(matrix_stack_str* head, char* data)
{
        matrix_stack_str* new_head;
        MATRIX_MALLOC(new_head, sizeof(matrix_stack_str));
        MATRIX_MALLOC(new_head->data, strlen(data) + 1);
        new_head->next = NULL;

        strcpy(new_head->data, data);

        if (head != NULL) {
                new_head->next = head;
        }

        return new_head;
}

matrix_stack_str* matrix_stack_str_pop(matrix_stack_str* head, char** entry)
{
        if(head == NULL) {
                return NULL;
        }

        matrix_stack_str* new_head = head->next;
        *entry =  head->data;
        MATRIX_FREE(head);
        return new_head;
}

matrix_stack_str* matrix_stack_str_next(matrix_stack_str* head, char** entry)
{
        if(head == NULL) {
                return NULL;
        }

        *entry = head->data;
        return head->next;
}

void matrix_stack_str_free(matrix_stack_str* head)
{
        if(head == NULL) {
                MATRIX_LOG_DEBUG("Try to free a nullptr stack string");
                return;
        }
        matrix_stack_str* next_ptr;
        while(head != NULL) {
                next_ptr = head->next;
                MATRIX_FREE(head->data);
                MATRIX_FREE(head);
                head = next_ptr;
        }
}