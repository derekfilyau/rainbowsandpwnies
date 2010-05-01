#ifndef results_HEADER
#define results_HEADER

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "thread.h"
#include "interface.h"
#include "options.h"

typedef struct
{
	int bytes_sent;
	int bytes_recv;
	
	char * data_sent;
	char * data_recv;
} result_t;


// these are defined in interface.gtk
#define COLUMN_THREAD_ID  0
#define COLUMN_HOSTNAME   1
#define COLUMN_BYTES_SENT 2
#define COLUMN_BYTES_RECV 3
#define COLUMN_DATA_SENT  4
#define COLUMN_DATA_RECV  5

threadsafe_int_t results_max;
threadsafe_int64_t results_memory;


void results_init ();
void results_remove_first ();
void results_row_activated (GtkTreeView * tree_view, GtkTreePath * tree_path, GtkTreeViewColumn * column, gpointer user_data);
void results_append (int thread_id, char * hostname, int bytes_sent, int bytes_recv, unsigned char * data_sent, char * data_recv);
gint results_compare_rows (GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b, gpointer useradata);
void results_update_status_bar ();
void results_test ();


#endif
