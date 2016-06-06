
/** @file apptree.h
 *  @author Dennis Law
 *  @date April 2016
 */

#ifndef APPTREE_H
#define APPTREE_H

#include <stdio.h>
#include <string.h>
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

enum apptree_mode {
	APPTREE_MODE_SIMPLE,
	APPTREE_MODE_SINGLE_SELECTION,
	APPTREE_MODE_MULTI_SELECTION
};

/* A single tree node */
struct apptree_node {
	char *title;
	char *info;
	
	struct apptree_node *parent;
	
	enum apptree_mode mode;
	struct list_head list_child;
	struct list_head list_parent;
	int num_child;
	
	bool selected;
	bool end;
	
	void (*function)(struct apptree_node *parent, int child_idx);
};

struct apptree_keybindings {
	char up;
	char down;
	char select;
	char back;
	char home;
};

/** @struct apptree_control
 *	@brief Keeps track of the current tree
 *	@var master Handle to the master node.
 *	@var current Handle to the current parent.
 *	@var picture Titles of all children for current master.
 *	@var picture_height Height of the picture.
 *	@var frame_pos Position of the frame in the picture.
 *	@var select_pos Position of the select arrow in the picture.
 *	@var enabled Set true when the enable function is called.
 *	@var keys Input key bindings.
 *	@var read_input Function for reading input. It returns 0 if successful
 *	and -1 if otherwise.
 */
struct apptree_control {
	struct apptree_node *master;
	struct apptree_node *current;
	
	char **picture;
	int picture_height;
	
	int frame_pos;
	int select_pos;
	
	bool enabled;
	
	struct apptree_keybindings *keys;
	int (*read_input)(char *input);
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
					int (*read_input)(char *input));
							
int apptree_enable(void);
int apptree_handle_input(void);
									
#endif	/* APPTREE_H */
