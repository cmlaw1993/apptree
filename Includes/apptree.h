
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

#define FRAME_HEIGHT					19
#define FRAME_WIDTH						80

#define MAX_TITLE_WIDTH					74
#define MAX_INFO_WIDTH					78

struct apptree_node;

/* A single tree node */
struct apptree_node {
	char *title;
	char *info;
	
	struct apptree_node *parent;
	
	struct list_head list_child;
	struct list_head list_parent;
	int num_child;
	
	void (*function)(void);
};

/** Keeps track of the current tree */
struct apptree_control {
	struct apptree_node *master;
	struct apptree_node *current;
	
	char **picture;
	int picture_size;
	int frame_pos;
	
	bool enabled;
};

int apptree_create_node(struct apptree_node **new_node,
							struct apptree_node *parent,
							char *title,
							char *info,
							void (*function)(void));
int apptree_init(struct apptree_node **master, char *title);
							
int apptree_enable(void);
									
#endif	/* APPTREE_H */
