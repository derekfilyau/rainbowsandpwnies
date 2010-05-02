#ifndef interface_HEADER
#define interface_HEADER

#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>



GtkBuilder * builder;


GObject * get_ui_object            (char * object_name);

void      debug_text_out           (char * text);

// for null-terminated strings
void      append_to_textview       (char * textview_name, char * text);
// for strings which may not be null-terminated (but this sometimes breaks things)
void      append_bytes_to_textview (char * textview_name, char * bytes, int length);
char *    get_text_from_textview   (char * textview_name);
void      clear_textview           (char * textview_name);

void      set_status_bar           (char * statusbar_name, char * text);

char *    get_textbox_text         (char * textbox_name); // needs to be changed to set_entry_text
void      set_entry_text           (char * entry_name, char * text);



#endif
