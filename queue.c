#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (q == NULL)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (l == NULL)
        return;
    element_t *e, *s;
    list_for_each_entry_safe (e, s, l, list) {
        q_release_element(e);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (new == NULL)
        return false;
    new->value = strdup(s);
    if (new->value == NULL) {
        free(new);
        return false;
    }
    list_add(&new->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *new = malloc(sizeof(element_t));
    if (new == NULL)
        return false;
    new->value = strdup(s);
    if (new->value == NULL) {
        free(new);
        return false;
    }
    list_add_tail(&new->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *ele = list_entry(head->next, element_t, list);
    list_del_init(&ele->list);
    if (sp != NULL) {
        strncpy(sp, ele->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';
    }
    return ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *ele = list_entry(head->prev, element_t, list);
    list_del_init(&ele->list);
    if (sp != NULL) {
        strncpy(sp, ele->value, bufsize - 1);
        *(sp + bufsize - 1) = '\0';
    }
    return ele;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return 0;
    int count = 0;
    struct list_head *n;
    list_for_each (n, head) {
        count++;
    }
    return count;
}

struct list_head *q_get_mid(struct list_head *head)
{
    if (list_is_singular(head))
        return head->next;
    struct list_head *forward, *backward;
    forward = head->next;
    backward = head->prev;
    while (forward != backward && forward->next != backward) {
        forward = forward->next;
        backward = backward->prev;
    }
    return forward;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return false;
    struct list_head *mid = q_get_mid(head);
    list_del(mid);
    q_release_element(list_entry(mid, element_t, list));
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    if (head == NULL)
        return false;
    element_t *e, *s;
    list_for_each_entry_safe (e, s, head, list)
        if (&s->list != head && strcmp(e->value, s->value) == 0) {
            list_del(&e->list);
            q_release_element(e);
        }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    struct list_head *node1, *node2;
    node1 = head->next;
    node2 = node1->next;
    while (node1 != head && node2 != head) {
        node1->prev->next = node2;
        node2->prev = node1->prev;
        node2->next->prev = node1;
        node1->next = node2->next;
        node2->next = node1;
        node1->prev = node2;
        node1 = node1->next;
        node2 = node1->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return;
    struct list_head *n, *s;
    list_for_each_safe (n, s, head) {
        n->next = n->prev;
        n->prev = s;
    }
    struct list_head *tmp;
    tmp = head->next;
    head->next = head->prev;
    head->prev = tmp;
}

void merge(struct list_head *H1, struct list_head *H2)
{
    element_t *E1, *E2;
    struct list_head *node1 = H1->next, *node2 = H2->next, *next_node;
    for (; !list_empty(H2); node2 = next_node) {
        while (node1 != H1) {
            E1 = list_entry(node1, element_t, list);
            E2 = list_entry(node2, element_t, list);
            if (strcmp(E1->value, E2->value) < 0)
                node1 = node1->next;
            else
                break;
        }
        if (node1 == H1)
            list_splice_tail_init(H2, H1);
        else {
            next_node = node2->next;
            list_del_init(node2);
            list_add_tail(node2, node1);
        }
    }
}
// void merge(struct list_head *head, struct list_head *head2)
// {
//     struct list_head *i_head = head->next, *i_head2, *next;
//     for (i_head2 = head2->next; !list_empty(head2); i_head2 = next) {
//         while (i_head != head &&
//                strcmp(list_entry(i_head, element_t, list)->value,
//                       list_entry(i_head2, element_t, list)->value) < 0) {
//             i_head = i_head->next;
//         }
//         if (i_head == head)
//             list_splice_tail_init(head2, i_head);
//         else {
//             next = i_head2->next;
//             list_del_init(i_head2);
//             list_add_tail(i_head2, i_head);
//         }
//     }
// }

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;

    LIST_HEAD(head2);
    list_cut_position(&head2, head, q_get_mid(head));
    q_sort(head);
    q_sort(&head2);
    merge(head, &head2);
}
