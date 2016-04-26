
/** @file list.h
 *	@brief Implements doubly linked-list	
 *
 *	This file is shortened and modified from list.h of the Linux
 *	kernel source code. The file was modified to include a container_of macro
 *	and a function for node traversing.
 *
 *  @author Dennis Law
 *  @date April 2016
 */

#ifndef LIST_H
#define LIST_H

/** @brief Container of macro */
#define container_of(ptr, type, member)		\
	((type *)								\
	(((char *)(ptr))						\
	- ((char *)(&((type*)0)->member))))

/** @brief Simple doubly linked list implementation.
 */
struct list_head {
	struct list_head *next, *prev;
	int id;
};

/** @brief Initilizes a list_head structure */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/** @brief Insert a new entry between two known consecutive entries.
 *	
 *	This is only for internal list manipulation where we know
 * 	the prev/next entries already!
 */
static inline void do_list_add(struct list_head *new_item,
								struct list_head *prev,
								struct list_head *next)
{
	next->prev		= new_item;
	new_item->next	= next;
	new_item->prev	= prev;
	prev->next		= new_item;
}

/** @brief Add a new entry
 * 	@param new new entry to be added
 * 	@param head list head to add it after
 *
 *  Insert a new entry after the specified head.
 * 	This is good for implementing stacks.
 */
static inline void list_add(struct list_head *new_item, struct list_head *head)
{
	do_list_add(new_item, head, head->next);
}

/**	@brief add a new entry
 *	@param new new entry to be added
 *	@param head list head to add it before
 *
 *	Insert a new entry before the specified head.
 *	This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new_item, struct list_head *head)
{
	do_list_add(new_item, head->prev, head);
}

/** @brief Delete a list entry by making the prev/next entries point to each other.
 *
 *	This is only for internal list manipulation where we know the prev/next
 *	entries already!
 */
static inline void do_list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/** @brief deletes entry from list.
 *  @pram entry the element to delete from the list.
 *  
 *	list_empty() on entry does not return true after this, the entry is
 *  in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{
	do_list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

/** @brief get the struct for this entry
 *	@param ptr the &struct list_head pointer.
 *	@param type the type of the struct this is embedded in.
 *	@param member the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member)	\
	container_of(ptr, type, member)

/** @brief travese down the list equal to index
 * 	@param head the &struct list_head pointer.
 *  @param index the index of the item in the list
 */
static inline struct list_head *list_travese_to_index(struct list_head *head, int index)
{
	int i; struct list_head *ptr;
	ptr = head->next;
	for (i = 0; i < index; i++)
		ptr = ptr->next;

	return ptr;
}

#endif	/* LIST_H */
