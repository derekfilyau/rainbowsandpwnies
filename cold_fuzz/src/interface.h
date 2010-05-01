#ifndef interface_HEADER
#define interface_HEADER

#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>



GtkBuilder * builder;


GObject * get_ui_object (char * object_name);
void debug_text_out (char * text);
void append_to_textview (char * textview_name, char * text);
void append_bytes_to_textview (char * textview_name, char * bytes, int length);
char * get_text_from_textview (char * textview_name);
void set_status_bar (char * statusbar_name, char * text);
void clear_textview(char * textview_name);
char * get_textbox_text (char * textbox_name);



#endif
