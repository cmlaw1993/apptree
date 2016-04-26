
/** @file apptree.c
 *  @brief A tree-based application display framework.
 *  @author Dennis Law
 *  @date April 2016
 */

#include "apptree.h"

static void apptree_populate_picture(void);
static int apptree_resize_picture(void);

static int apptree_validate_node(struct apptree_node *block);
static int apptree_create_master(struct apptree_node **master, char *title);

static void apptree_print_info(void);
static void apptree_print_frame(void);
static void apptree_print_title(void);
static void apptree_print_blank(void);
static void apptree_print_select(int index);
static void apptree_print_menu(void);

/* Initialize control structure */
static struct apptree_control control = {
	NULL,		/* master */
	NULL,		/* current */
	NULL,		/* picture */
	0,			/* picture_size */
	0,			/* frame_pos */
	0,			/* select_pos */
	false		/* enabled */
};

/** @brief Populates the picture
 *	
 *	The picture is populated with the titles of all the children belonging
 *	to the current node.
 *
 *	@note Ensure that the picture size is the same as the number of children in
 *	the current node. Call the function apptree_resize_picture if the size is
 *	incorrect.
 */
static void apptree_populate_picture(void)
{
	struct list_head *head;
	struct apptree_node *node;
	int i;
	
	for (i = 0; i < control.current->num_child; i++) {
		head = list_travese_to_index(&control.current->list_parent, i);
		node = container_of(head, struct apptree_node, list_child);
		control.picture[i] = node->title;
	}
}

/** @brief Resizes the picture
 *	@returns 0 if sucessful and -1 if otherwise.
 *
 *	The picture is dynamically resized using realloc according to the number of
 *	children of the current node.
 */
static int apptree_resize_picture(void)
{	
	char **temp;
	
	temp = realloc(control.picture, control.picture_size * sizeof(char **));
	if (temp == NULL)
		return -1;
	
	control.picture = temp;
	return 0;
}

/** @brief Checks if a node is atteched to the tree
 *	@returns 0 if yes and -1 if otherwise
 *	
 *	A node is attached to the tree if it has the master node as its encestor.
 */
static int apptree_validate_node(struct apptree_node *block)
{
	struct apptree_node *node = block;
	
	do {
		node = node->parent;
	} while (node->parent != NULL);
	
	if (node != control.master)
		return -1;
	else
		return 0;
}

/** @brief Creates a node and attaches it to the tree
 *	@param new_node Handle for holding the new node.
 *	@param parent Parent node to attach the new node to.
 *	@param title Title message of the new node.
 *	@param info Info message of the new node.
 *	@param function Function to bind to this node.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	This fucntion dynamically allocates memory to create a new node and
 *	subsequently attaches it to an existing parent in the tree.
 */
int apptree_create_node(struct apptree_node **new_node,
							struct apptree_node *parent,
							char *title,
							char *info,
							void (*function)(void))
{	
	struct apptree_node *node;
	
	if (control.enabled)
		return -1;
	
	node = (struct apptree_node *)calloc(1, sizeof(struct apptree_node));
	if (node == NULL)
		return -1;

	node->parent = parent;
	if (apptree_validate_node(node)) {
		free(node);
		return -1;
	}
	
	INIT_LIST_HEAD(&node->list_parent);
	
	INIT_LIST_HEAD(&node->list_child);
	list_add_tail(&node->list_child, &parent->list_parent);
	parent->num_child++;
	
	node->title	 	= title;
	node->info	 	= info;
	node->num_child = 0;
	node->function	= function;

	*new_node = node;
	
	return 0;
}

/**	@brief Creates a master node.
 *	@param master Handle for holoding the master node.
 *	@param title Title for the master node.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	@note This function should only be called by the apptree_init function.
 */
static int apptree_create_master(struct apptree_node **master, char *title)
{
	struct apptree_node *node;
	
	node = (struct apptree_node *)calloc(1, sizeof(struct apptree_node));
	if (node == NULL)
		return -1;
	
	INIT_LIST_HEAD(&node->list_parent);
	INIT_LIST_HEAD(&node->list_child);
	
	node->title 	= title;
	node->info 		= NULL;
	node->parent	= NULL;
	node->num_child = 0;
	node->function 	= NULL;
	
	*master = node;
	
	return 0;
}

/** @brief Initializes the apptree and creates a master node.
 *	@param master Handle for holding the master node.
 *	@param title Title for the master node.
 *	@returns 0 if successful and -1 if otherwise.
 */
int apptree_init(struct apptree_node **master, char *title)
{
	if (apptree_create_master(master, title))
		return -1;
	
	control.master 	= *master;
	control.current	= *master;
	
	control.frame_pos = 0;
	control.picture_size = 0;
	control.enabled = false;
	
	return 0;
}

/** @brief Prints the info of a pointed item
 */
static void apptree_print_info(void)
{
	printf("info\r\n");
}

/** @brief prints the select arrow
 *	@index Index of the item in the picture.
 *
 *	@note This function should only be called by apptree_print_menu.
 */
static void apptree_print_select(int index)
{
	if (index == control.select_pos)
		printf("-> ");
	else
		printf("   ");
}

/** @brief Prints a frame
 */
static void apptree_print_frame(void)
{
	int end, start = control.frame_pos;
	int i, j;
	
	if (control.picture_size <= FRAME_HEIGHT) {
		
		for (i = start; i < control.picture_size; i++) {
			apptree_print_select(i);
			printf("%2d.%s\r\n", i+1, control.picture[i]);
		}

		for (j = i; j < FRAME_HEIGHT; j++)
			printf("\r\n");
	} else {
		end = control.frame_pos + FRAME_HEIGHT;
		
		for (i = start; i < end; i++)
		{
			apptree_print_select(i);
			printf("%2d.%s\r\n", i+1, control.picture[i]);
		}
	}
}

/**	@brief Prints the title of the current parent node
 */
static void apptree_print_title(void)
{
	printf("%s\r\n", control.current->title);
}

/** @brief Prints a blank line.
 */
static void apptree_print_blank(void)
{
	printf("\r\n");
}

/**	@brief Prints the menu
 *	Prints the menu which consists of the title, info and frame in that order.
 */
static void apptree_print_menu(void)
{
	apptree_print_title();
	apptree_print_blank();
	apptree_print_frame();
	apptree_print_blank();
	apptree_print_info();
}

/** @brief Enables the apptree
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	Enables the apptree and prints the menu with the master node as the current
 *	node. The enabled flag is also set to prevent changes in the tree structure.
 */
int apptree_enable(void)
{
	if (control.master == NULL)
		return -1;
	
	control.current = control.master;
	control.picture_size = control.master->num_child;
	
	control.enabled = true;
	
	apptree_resize_picture();
	apptree_populate_picture();
	apptree_print_menu();
	
	return 0;
}
