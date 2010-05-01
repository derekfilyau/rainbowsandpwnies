#include "options.h"



void options_set_entry_to_int (char * entry_name, int value)
{
	GObject * object;
	char tmp [32];
	
	memset(tmp, 0, 32);
	snprintf(tmp, 32, "%d", value);
	object = get_ui_object(entry_name);
	gtk_entry_set_text(GTK_ENTRY(object), tmp);
	
}



int options_get_int_from_entry (char * entry_name)
{

	GObject * object;
	char * text;
	int result;
	
	object = get_ui_object(entry_name);
	text = (char *) gtk_entry_get_text(GTK_ENTRY(object));
	result = atoi(text);
	
	return result;
	
}



void button_save_options_click ()
{

	GObject * object;
	
	set_threadsafe_int(&OPTIONS_THREADS_N, options_get_int_from_entry("box_number_of_threads"));
	set_threadsafe_int(&OPTIONS_RESULTS_MAX, options_get_int_from_entry("box_results_max"));
	
	object = get_ui_object("options_window");
	gtk_widget_hide(GTK_WIDGET(object));
	
}
	


void options_init ()
{

	GObject * object; 
	
	set_threadsafe_int(&OPTIONS_THREADS_N, 5);
	set_threadsafe_int(&OPTIONS_RESULTS_MAX, 100);
	
	object = get_ui_object("button_save_options");
	g_signal_connect(G_OBJECT(object), "clicked", G_CALLBACK(button_save_options_click), NULL);
	g_signal_connect(G_OBJECT(object), "activate", G_CALLBACK(button_save_options_click), NULL);
	
	object = get_ui_object("options_window");
	g_signal_connect(G_OBJECT(object), "delete_event", G_CALLBACK(button_save_options_click), NULL);
}



int options_window ()
{

	GObject * object;
	
	options_set_entry_to_int("box_number_of_threads", get_threadsafe_int(&OPTIONS_THREADS_N));
	options_set_entry_to_int("box_results_max", get_threadsafe_int(&OPTIONS_RESULTS_MAX));
	
	object = get_ui_object("options_window");
	
	gtk_widget_show(GTK_WIDGET(object));
	
	return 1;

}
