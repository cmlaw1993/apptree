
/** @file apptree.h
 *  @author Dennis Law
 *  @date April 2016
 */

#ifndef APPTREE_H
#define APPTREE_H

#include <stdlib.h>
#include <stdbool.h>

#include "list.h"


#define TERMINAL_HEIGHT					24
#define TERMINAL_WIDTH					80

#define FRAME_HEIGHT					18
#define FRAME_WIDTH						80

#define MAX_TITLE_WIDTH					74
#define MAX_INFO_WIDTH					78


struct apptree_node;


/** @enum apptree_mode
 *	@brief Defines modes for nodes.
 */
enum apptree_mode {
	/** Simple mode */
	APPTREE_MODE_SIMPLE,
	/** Single Selection mode */
	APPTREE_MODE_SINGLE_SELECTION,
	/** Multi Selection mode */
	APPTREE_MODE_MULTI_SELECTION
};

/** @struct apptree_node
 *	@brief A single tree node
 */
struct apptree_node {
	/** Node title */
	char *title;
	/** Node info */
	char *info;
	
	/** Parent of the node */
	struct apptree_node *parent;
	
	/** Mode of the node */
	enum apptree_mode mode;
	/** List head for inserting the node as a children of another node */
	struct list_head list_child;
	/** List head for inserting the node as a parent of another node */
	struct list_head list_parent;
	/** Number of children in this node */
	int num_child;
	
	/** Determines if this node is selected */
	bool selected;
	/** Determines if this is an end node */
	bool end;
	
	/** Function called when the node is selected
	 *	@param parent Parent of this node.
	 *	@param child_idx The position of this node as a child to its parent.
	 */
	void (*function)(struct apptree_node *parent, int child_idx);
};

/** @struct apptree_keybindings
 *	@brief Structure for holding key binding information
 */
struct apptree_keybindings {
	char up;
	char down;
	char select;
	char back;
	char home;
};

/** @struct apptree_control
 *	@brief Keeps track of the apptree
 */
struct apptree_control {
	/** Handle to the master node */
	struct apptree_node *master;
	/** Handle to the current parent */
	struct apptree_node *current;
	
	/** Titles of all children for current master */
	char **picture;
	/** Height of the picture. */
	int picture_height;
	
	/** Position of the frame in the picture */
	int frame_pos;
	/** Position of the select arrow in the picture. */
	int select_pos;
	
	/** Set as true when the enable function is called. */
	bool enabled;
	
	/** Input key bindings. */
	struct apptree_keybindings *keys;
	

};


int apptree_create_node(struct apptree_node **new_node,
		struct apptree_node *parent,
		char *title,
		char *info,
		enum apptree_mode mode,
		bool selected,
		void (*function)(struct apptree_node *parent, int child_idx));

int apptree_init(struct apptree_node **master,
					char *master_title,
					enum apptree_mode master_mode,
					struct apptree_keybindings *key,
					int (*read_input)(char *input),
					void (*write_output)(char output));

int apptree_enable(void);
int apptree_handle_input(void);

#endif	/* APPTREE_H */
