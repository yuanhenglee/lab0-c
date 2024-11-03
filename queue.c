#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

inline element_t *q_new_element(char *s)
{
    element_t *e = malloc(sizeof(element_t));
    if (!e)
        return NULL;
    e->value = strdup(s);
    return e;
}

inline void q_free_element(element_t *e)
{
    if (!e)
        return;
    test_free(e->value);
    test_free(e);
}


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *it, *safe;
    list_for_each_entry_safe (it, safe, head, list)
        q_free_element(it);
    test_free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *e = q_new_element(s);
    if (!e)
        return false;
    list_add(&e->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *e = q_new_element(s);
    if (!e)
        return false;
    list_add_tail(&e->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *e = list_first_entry(head, element_t, list);
    list_del(&e->list);

    if (!sp || bufsize == 0)
        return e;

    strncpy(sp, e->value, bufsize);
    sp[bufsize - 1] = '\0';
    return e;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *e = list_last_entry(head, element_t, list);
    list_del(&e->list);

    if (!sp || bufsize == 0)
        return e;

    strncpy(sp, e->value, bufsize);
    sp[bufsize - 1] = '\0';
    return NULL;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *it;
    list_for_each (it, head)
        len++;

    return len;
}

static inline struct list_head *q_find_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *right = head, *left = head;
    do {
        right = right->prev;
        left = left->next;
    } while (right != head && right != left);
    return left;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;


    struct list_head *mid = q_find_mid(head);

    element_t *e = list_entry(mid, element_t, list);
    q_free_element(e);
    list_del(mid);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    element_t *it, *safe;
    list_for_each_entry_safe (it, safe, head, list) {
        element_t *next = list_entry(it->list.next, element_t, list);
        if (!next)
            break;
        if (!strcmp(it->value, next->value)) {
            list_del(&it->list);
            q_free_element(it);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *it, *safe;
    list_for_each_safe (it, safe, head)
        list_move(it, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || k <= 1)
        return;
    struct list_head *it, *safe;
    list_for_each_safe (it, safe, head) {
        struct list_head *end = it;
        for (int i = 0; i < k; i++) {
            if (!end)
                return;
            end = end->next;
        }
        if (!end)
            return;
        struct list_head *next = end->next;
        list_cut_position(it, end, next);
        q_reverse(it);
        list_splice_tail(it, next);
    }
}

void q_merge_two(struct list_head *head, struct list_head *head2, bool descend)
{
    if (!head || list_empty(head) || !head2 || list_empty(head2))
        return;
    LIST_HEAD(tmp);
    while (!list_empty(head) && !list_empty(head2)) {
        element_t *e1 = list_first_entry(head, element_t, list),
                  *e2 = list_first_entry(head2, element_t, list);
        bool e1_first = descend ^ (strcmp(e1->value, e2->value) < 0);
        list_move_tail(e1_first ? &e1->list : &e2->list, &tmp);
    }
    list_splice(&tmp, head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *mid = q_find_mid(head);

    LIST_HEAD(left);
    list_cut_position(&left, head, mid);

    q_sort(&left, descend);
    q_sort(mid, descend);

    q_merge_two(&left, mid, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere
 * to the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in
 * ascending/descending order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
