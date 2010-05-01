#include "results.h"


void results_init ()
{
	GObject * object;
	
	set_threadsafe_int(&results_max, 0);
	set_threadsafe_int64(&results_memory, 0);
	
	object = get_ui_object("tree_results");
	g_signal_connect(object, "row-activated", G_CALLBACK(results_row_activated), NULL);
	
	object = get_ui_object("sort_results");
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(object), COLUMN_THREAD_ID, results_compare_rows, GINT_TO_POINTER(COLUMN_THREAD_ID), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(object), COLUMN_HOSTNAME, results_compare_rows, GINT_TO_POINTER(COLUMN_HOSTNAME), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(object), COLUMN_BYTES_SENT, results_compare_rows, GINT_TO_POINTER(COLUMN_BYTES_SENT), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(object), COLUMN_BYTES_RECV, results_compare_rows, GINT_TO_POINTER(COLUMN_BYTES_RECV), NULL);
	
	results_update_status_bar();
}



void results_update_status_bar ()
{
	
	char status_bar_text[128];
	
	snprintf(status_bar_text, 128, "results size: %d bytes", get_threadsafe_int64(&results_memory));
	set_status_bar("status_results", status_bar_text);
	
}



void results_test ()
{
	results_append(0, "test", 7, 6, (unsigned char *) "testout", "testin");
	results_remove_first();
}



void results_remove_first ()
{

	GtkListStore * liststore;
	GtkTreeIter iter;
	int bytes_sent, bytes_recv;
	char * hostname;
	char * data_sent_pointer = NULL;
	char * data_recv_pointer = NULL;
	
	subtract_threadsafe_int(&results_max, 1);
	
	liststore = GTK_LIST_STORE(get_ui_object("list_results"));
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore), &iter))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(liststore), &iter,
		                   COLUMN_BYTES_SENT, &bytes_sent,
		                   COLUMN_BYTES_RECV, &bytes_recv,
		                   COLUMN_HOSTNAME, &hostname,
		                   COLUMN_DATA_SENT, &data_sent_pointer,
		                   COLUMN_DATA_RECV, &data_recv_pointer,
		                   -1);
		gtk_list_store_remove(liststore, &iter);
	}
	
	subtract_threadsafe_int64(&results_memory, ((int64_t) bytes_sent)
	                                       + ((int64_t) bytes_recv)
	                                       + ((int64_t) strlen(hostname))
	                                       + ((int64_t) sizeof(gint) * 3)
	                                       + ((int64_t) sizeof(gpointer) * 2));
	                                       
	results_update_status_bar();
	
	free(data_sent_pointer);
	free(data_recv_pointer);
	
}



void results_row_activated (GtkTreeView * tree_view, GtkTreePath * tree_path, GtkTreeViewColumn * column, gpointer user_data)
{

	GtkTreeModel * tree_model;
	GtkTreeIter iter;
	
	int bytes_sent;
	int bytes_recv;
	char * data_sent;
	char * data_recv;
	
	tree_model = gtk_tree_view_get_model(tree_view);
	if (gtk_tree_model_get_iter(tree_model, &iter, tree_path))
	{
		gtk_tree_model_get(tree_model, &iter,
		                   COLUMN_BYTES_SENT, &bytes_sent,
		                   COLUMN_BYTES_RECV, &bytes_recv,
		                   COLUMN_DATA_SENT, &data_sent,
		                   COLUMN_DATA_RECV, &data_recv,
		                   -1);
		clear_textview("text_sent");
		clear_textview("text_recv");
		append_bytes_to_textview("text_sent", data_sent, bytes_sent);
		append_bytes_to_textview("text_recv", data_recv, bytes_recv);
	}
	
}
		


void results_append (int thread_id, char * hostname, int bytes_sent, int bytes_recv, unsigned char * data_sent, char * data_recv)
{

	GtkListStore * liststore;
	GtkTreeIter iter;
	
	unsigned char * data_sent_pointer;
	char * data_recv_pointer;
	
	add_threadsafe_int(&results_max, 1);
	// if we have too many items in list, get rid of one
	if (get_threadsafe_int(&results_max) > OPTIONS_RESULTS_MAX)
		results_remove_first();
		
	if (OPTIONS_RESULTS_MAX == 0)
		return;
	
	data_sent_pointer = (unsigned char *) malloc(bytes_sent);
	data_recv_pointer = (char *) malloc(bytes_recv);
	
	memcpy(data_sent_pointer, data_sent, bytes_sent);
	memcpy(data_recv_pointer, data_recv, bytes_recv);

	
	liststore = GTK_LIST_STORE(get_ui_object("list_results"));

	gtk_list_store_append(liststore, &iter);

	gtk_list_store_set(liststore, &iter,
	                   COLUMN_THREAD_ID, thread_id,
	                   COLUMN_HOSTNAME, hostname,
	                   COLUMN_BYTES_SENT, bytes_sent,
	                   COLUMN_BYTES_RECV, bytes_recv,
	                   COLUMN_DATA_SENT, data_sent_pointer,
	                   COLUMN_DATA_RECV, data_recv_pointer,
	                   -1);
	
	add_threadsafe_int64(&results_memory, ((int64_t) bytes_sent)
	                                       + ((int64_t) bytes_recv)
	                                       + ((int64_t) strlen(hostname))
	                                       + ((int64_t) sizeof(gint) * 3)
	                                       + ((int64_t) sizeof(gpointer) * 2));
	results_update_status_bar();
}


// http://scentric.net/tutorial/sec-sorting.html
gint results_compare_rows (GtkTreeModel * tree_model, GtkTreeIter * a, GtkTreeIter * b, gpointer userdata)
{

	gint column;
	gint column_a, column_b;
	gchar * hostname_a, * hostname_b;
	
	column = GPOINTER_TO_INT(userdata);
	
	switch (column)
	{
	
		case COLUMN_THREAD_ID :
		case COLUMN_BYTES_SENT :
		case COLUMN_BYTES_RECV :
		{
		
			gtk_tree_model_get(tree_model, a, column, &column_a, -1);
			gtk_tree_model_get(tree_model, b, column, &column_b, -1);
				
			if (column_a > column_b)
				return 1;
			else if (column_a < column_b)
				return -1;
			else
				return 0;
				
			break;
			
		}
		case COLUMN_HOSTNAME :
		{
		
			gtk_tree_model_get(tree_model, a, column, &hostname_a, -1);
			gtk_tree_model_get(tree_model, b, column, &hostname_b, -1);
			
			if ((hostname_a == NULL) || hostname_b == NULL)
			{
				if ((hostname_a == NULL) && (hostname_b == NULL))
					return 0;
				return ((hostname_a == NULL) ? -1 : 1);
			}
			
			return g_utf8_collate(hostname_a, hostname_b);
			
			break;
		}
		
	}

	return 0;
	
}
