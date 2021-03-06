
/** @file apptree.c
 *  @brief A tree-based application display framework for microcontrollers.
 *  @author Dennis Law
 *  @date April 2016
 */

#include "apptree_io.h"
#include "apptree.h"


static int apptree_bind_keys(struct apptree_keybindings *key);
static int apptree_create_master(struct apptree_node **master,
									char *title,
									enum apptree_mode mode);
									
static void apptree_populate_picture(void);
static int apptree_resize_picture(void);
static void apptree_print_keybindings(void);
static void apptree_print_info(void);
static void apptree_print_frame(void);
static void apptree_print_title(void);
static void apptree_print_blank(void);
static void apptree_print_select(int index);
static void apptree_print_selected(struct apptree_node *parent,
									int child_index);
static void apptree_print_menu(void);

static int apptree_validate_node(struct apptree_node *block);

static void apptree_adjust_frame_pos(void);
static void apptree_increase_select_pos(void);
static void apptree_decrease_select_pos(void);
static void apptree_update_selected(struct apptree_node *parent,
									int child_index);

static void apptree_handle_up_input(void);
static void apptree_handle_down_input(void);
static void apptree_handle_select_input(void);
static void apptree_handle_back_input(void);
static void apptree_handle_home_input(void);


static struct apptree_control control;


/** @name Initialization Functions
 *	The initialization functions are used to initialize the configuration
 *	variables used by the apptree. It also creates a master node which
 *	subsequent nodes will grow from and binds the key inputs.
 *
 *	@note The apptree uses the standard C library (printf) for printing its
 *	output. The standard ouput (serial, LCD, etc) is expected to be properly
 *	configured and binded to the printf function prior to calling the init
 *	function.
 */
/** @{*/

/** @brief Binds input keys
 *	@param key Pointer to key binding struct.
 *	@returns 0 if successful and -1 if otherwise.
 */
static int apptree_bind_keys(struct apptree_keybindings *key)
{
	if (key == NULL)
		return -1;
	
	control.keys = key;
	return 0;
}

/**	@brief Creates a master node.
 *	@param master Handle for holoding the master node.
 *	@param title Title for the master node.
 *	@param mode Mode of the master node.
 *	@returns 0 if successful and -1 if otherwise.
 */
static int apptree_create_master(struct apptree_node **master,
									char *title,
									enum apptree_mode mode)
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
	node->mode		= mode;
	node->num_child = 0;
	node->selected	= false;
	node->end		= false;
	node->function 	= NULL;
	
	*master = node;
	
	return 0;
}

/** @brief Initializes the apptree and creates a master node.
 *	@param master Handle for holding the master node.
 *	@param master_title Title for the master node.
 *	@param mode Mode of the master node.
 *	@param key Key binding for the apptree.
 *	@param read_input Non-blocking function for reading user input.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	@note This function should be called before any nodes are added to
 *	the tree.
 */
int apptree_init(struct apptree_node **master,
					char *master_title,
					enum apptree_mode master_mode,
					struct apptree_keybindings *key,
					int (*read_input)(char *input),
					void (*write_output)(char output))
{
	if ((read_input == NULL) || (write_output == NULL))
		return -1;
	
	if (apptree_bind_keys(key))
		return -1;
	
	if (apptree_create_master(master, master_title, master_mode))
		return -1;
	
	apptree_io_init(read_input, write_output);
	
	control.master 			= *master;
	control.current			= *master;
	control.picture 		= NULL,
	control.picture_height 	= 0;
	control.frame_pos 		= 0;
	control.select_pos 		= 0;
	control.enabled 		= 0;
	
	return 0;
}

/** @}*/

/* -------------------------------------------------------------------------- */
/** @name Printing Functions
 *	Handles the printing of the apptree to the output media.
 */
/** @{*/

/** @brief Populates the picture
 *	
 *	The picture is populated with the titles of all the children belonging
 *	to the current node.
 *
 *	@note Ensure that the picture size is the same as the number of children in
 *	the current node before calling. Call the function apptree_resize_picture
 *	if the size is incorrect.
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
	
	temp = realloc(control.picture, control.picture_height * sizeof(char **));
	if (temp == NULL)
		return -1;
	
	control.picture = temp;
	return 0;
}

/** @brief Prints keybindings
 *	@note This function should only be called by apptree_print_menu.
 */
static void apptree_print_keybindings(void)
{
	apptree_print("KEY BINDINGS => UP:[%c]  DOWN:[%c]  SELECT:[%c]  BACK:[%c]  HOME:[%c]\r\n",
		control.keys->up, control.keys->down, control.keys->select,
		control.keys->back, control.keys->home);
}

/** @brief Prints the info of a pointed item
 */
static void apptree_print_info(void)
{
	struct list_head *head;
	struct apptree_node *node;

	head = list_travese_to_index(&control.current->list_parent, control.select_pos);
	node = container_of(head, struct apptree_node, list_child);
	
	apptree_print("< %s >\r\n", node->info);
}

/** @brief Prints the select arrow
 *	@param index Index of the item which the arrow is pointed on.
 *
 *	@note This function should only be called by apptree_print_menu.
 */
static void apptree_print_select(int index)
{
	if (index == control.select_pos)
		apptree_print(" -> ");
	else
		apptree_print("    ");
}

/** @brief Print the selected marker of a node
 *	@param parent The parent of the node.
 *	@param child_index The position of the node as a child to its parent.
 *	
 *	This function prints out the selected marker of a node if its parent is not
 *	Simple.
 */
static void apptree_print_selected(struct apptree_node *parent,
									int child_index)
{
	struct list_head *head;
	struct apptree_node *node;

	head = list_travese_to_index(&parent->list_parent, child_index);
	node = container_of(head, struct apptree_node, list_child);
	
	if (parent->mode == APPTREE_MODE_SIMPLE)
		return;
	
	if (node->selected)
		apptree_print("[*] ");
	else
		apptree_print("[ ] ");
}

/** @brief Prints a frame
 */
static void apptree_print_frame(void)
{
	int end, start = control.frame_pos;
	int i, j;
	
	if (control.picture_height <= FRAME_HEIGHT) {
		
		for (i = start; i < control.picture_height; i++) {
			apptree_print_select(i);
			apptree_print_selected(control.current, start + i);
			apptree_print("%2d. %s\r\n", i+1, control.picture[i]);
		}

		for (j = i; j < FRAME_HEIGHT; j++)
			apptree_print("\r\n");
	} else {
		end = control.frame_pos + FRAME_HEIGHT;
		
		for (i = start; i < end; i++)
		{
			apptree_print_select(i);
			apptree_print("%2d. %s\r\n", i+1, control.picture[i]);
		}
	}
}

/**	@brief Prints the title of the current parent node
 */
static void apptree_print_title(void)
{
	apptree_print("%s\r\n", control.current->title);
}

/** @brief Prints a blank line.
 */
static void apptree_print_blank(void)
{
	apptree_print("\r\n");
}

/**	@brief Prints the menu.
 *	Prints the menu which consists of the title, info and frame in that order.
 */
static void apptree_print_menu(void)
{
	apptree_print_blank();
	apptree_print_title();
	apptree_print_blank();
	apptree_print_frame();
	apptree_print_blank();
	apptree_print_info();
	apptree_print_keybindings();
}

/** @}*/

/* -------------------------------------------------------------------------- */
/** @name Setup Functions
 *	Handles node creation. Before any nodes can be added into the tree, the
 *	initialization function has to be called.
 */
/** @{*/

/** @brief Checks if a node is attached to the tree
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
 *	@param mode Mode of the node.
 *	@param selected Indicated whether this node should be set as selected.
 *	@param function Function to bind to this node.
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	This function dynamically allocates memory to create a new node and
 *	subsequently attaches it to an existing parent in the tree. This function
 *	will fail under two circumstances:
 *	
 *		1. The apptree_enable function has been called.
 *		2. The parent function is an end node.
 *
 *	@note The children of a node which is not Simple (either Single Selection
 *	or Multi Selection) is automatically set as an end node. An end node will
 *	not be able to have children. Also, if a parent node is set to Single
 *	Selection, only one of its children can be set as selected.
 */
int apptree_create_node(struct apptree_node **new_node,
		struct apptree_node *parent,
		char *title,
		char *info,
		enum apptree_mode mode,
		bool selected,
		void (*function)(struct apptree_node *parent, int child_idx))
{	
	struct apptree_node *node;
	
	if (control.enabled)
		return -1;
	
	if (parent->end)
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
	
	if (parent->mode != APPTREE_MODE_SIMPLE)
		node->end = true;
	else
		node->end = false;
	
	node->title	 	= title;
	node->info	 	= info;
	node->mode		= mode;
	node->num_child = 0;
	node->selected	= selected;
	node->function	= function;

	*new_node = node;
	
	return 0;
}

/** @brief Enables the apptree
 *	@returns 0 if successful and -1 if otherwise.
 *
 *	This function is called at the end of the setup phase (after all nodes have
 *	been added. It enables the apptree and prints the menu with the master node
 *	as the current node. The enabled flag is also set to prevent changes in the
 *	tree structure.
 */
int apptree_enable(void)
{
	if (control.master == NULL)
		return -1;
	
	if (control.keys == NULL)
		return -1;
	
	control.current = control.master;
	control.picture_height = control.master->num_child;
	
	control.enabled = true;
	
	apptree_resize_picture();
	apptree_populate_picture();
	apptree_print_menu();
	
	return 0;
}

/** @}*/


/* -------------------------------------------------------------------------- */
/** @name Input Handling Functions
 *	Handles the user input as well as any subsequent results from the input.
 */
/** @{*/

/** @brief Adjust the value of frame_pos
 *	
 *	The value of frame_pos is adjusted based on the value of select_pos.
 *	Therefore, this function is called after every update to the value of
 *	select_pos. It helps to handle the following 4 conditions.
 *
 *		1. Select arrow loops from bottom to top. 
 *		2. Select arrow loops from top to bottom.
 *		3. Select arrow moves downward beyond current frame.
 *		4. Select arrow moves upward beyond current frame.
 */
static void apptree_adjust_frame_pos(void)
{
	if (control.select_pos == 0) {
		control.frame_pos = 0;
	} else if (control.select_pos == (control.picture_height - 1)) {
		control.frame_pos = control.picture_height - FRAME_HEIGHT;
		
		if(control.frame_pos < 0)
			control.frame_pos = 0;
		
	} else if (control.select_pos >= (control.frame_pos + FRAME_HEIGHT)) {
		control.frame_pos++;
	} else if (control.select_pos < control.frame_pos) {
		control.frame_pos--;
	}
}

/** @brief Increase the value of select_pos
 *
 *	This function increases the value of select_pos and also helps to reposition
 *	it should the value reach the end of the picture_length.
 */
static void apptree_increase_select_pos(void)
{
	if (control.select_pos == (control.picture_height - 1))
		control.select_pos = 0;
	else
		control.select_pos++;
}

/** @brief Decreases the value of select_pos
 *
 *	This function decreases the value of select_pos and also helps to reposition
 *	it should the value reach the end of the picture_length.
 */
static void apptree_decrease_select_pos(void)
{
	if (control.select_pos == 0)
		control.select_pos = control.picture_height - 1;
	else
		control.select_pos--;
}

/** @brief Update the selected field of a node's children
 *	@param parent The parent node.
 *	@param child_index The index of the child that has been selected.
 *
 *	This function updates the selected field of a node's children if the node
 *	is not Simple. This function should only be called after a recent selection
 *	has been made by the user.
 */
static void apptree_update_selected(struct apptree_node *parent,
									int child_index)
{
	struct list_head *head;
	struct apptree_node *child;
	int i;
	
	switch (parent->mode)
	{
	case APPTREE_MODE_SIMPLE:
		/* do nothing */
		break;
	
	case APPTREE_MODE_SINGLE_SELECTION:
		for (i = 0; i < parent->num_child; i++) {
			head = list_travese_to_index(&parent->list_parent, i);
			child = container_of(head, struct apptree_node, list_child);
			
			if (i == child_index)
				child->selected = true;
			else
				child->selected = false;
		}
		break;
		
	case APPTREE_MODE_MULTI_SELECTION:
		head = list_travese_to_index(&parent->list_parent, child_index);
		child = container_of(head, struct apptree_node, list_child);
	
		if (child->selected)
			child->selected = false;
		else
			child->selected = true;
		
		break;
	}		
}

/** @brief Handles an "up" input
 */
static void apptree_handle_up_input(void)
{
	apptree_decrease_select_pos();
	apptree_adjust_frame_pos();
	apptree_print_menu();
}

/** @brief Handles a "down" input
 */
static void apptree_handle_down_input(void)
{
	apptree_increase_select_pos();
	apptree_adjust_frame_pos();
	apptree_print_menu();
}

/** @brief Handles a "select" input
 */
static void apptree_handle_select_input(void)
{
	struct list_head *head;
	struct apptree_node *child;
	
	head = list_travese_to_index(&control.current->list_parent,
									control.select_pos);
	child = container_of(head, struct apptree_node, list_child);

	if (child->num_child > 0) {	
		control.current = child;
		
		control.picture_height = control.current->num_child;
		control.frame_pos = 0;
		control.select_pos = 0;
		
		apptree_resize_picture();
		apptree_populate_picture();
		apptree_print_menu();
	} else {
		if(child->function) {
			child->function(control.current, control.select_pos);
			apptree_update_selected(control.current, control.select_pos);
			apptree_print_menu();
		}
	}
}

/** @brief Handles a "back" input.
 */
static void apptree_handle_back_input(void)
{
	if (control.current == control.master)
		return;
	
	control.current = control.current->parent;
	
	control.picture_height = control.current->num_child;
	control.frame_pos = 0;
	control.select_pos = 0;

	apptree_resize_picture();
	apptree_populate_picture();
	apptree_print_menu();
}

/** @brief Handles a "home" input.
 */
static void apptree_handle_home_input(void)
{
	if (control.current == control.master)
		return;
	
	control.current = control.master;
	
	control.picture_height = control.current->num_child;
	control.frame_pos = 0;
	control.select_pos = 0;

	apptree_resize_picture();
	apptree_populate_picture();
	apptree_print_menu();
}

/** @brief Handles user inputs
 *	@returns 0 if a new input is detected and -1 if otherwise.
 *
 *	This function checks for a user input and handles it according to the
 *	binded key values.
 */
int apptree_handle_input(void)
{
	char input;
	
	if (!control.enabled)
		return -1;
	
	if (apptree_read(&input))
		return -1;
	
	if (input == control.keys->up) {
		apptree_handle_up_input();
	} else if (input == control.keys->down) {
		apptree_handle_down_input();
	} else if (input == control.keys->select) {
		apptree_handle_select_input();
	} else if (input == control.keys->back) {
		apptree_handle_back_input();
	} else if (input == control.keys->home) {
		apptree_handle_home_input();
	}
	
	return 0;
}

/** @}*/
