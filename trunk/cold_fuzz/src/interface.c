#include "interface.h"


// i've had to port everything once already *sigh* :(
GObject * get_ui_object (char * object_name)
{

	return gtk_builder_get_object(builder, object_name);
	
}



void debug_text_out (char * text)
{
	printf("%s\n", text);
}



void append_to_textview (char * textview_name, char * text)
{
	GObject * object;
	GtkTextBuffer * buffer;
	GtkTextIter end;
	
	object = get_ui_object(textview_name);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(object));
	
	gtk_text_buffer_get_end_iter(buffer, &end);
	
	gtk_text_buffer_insert(buffer, &end, text, strlen(text));
}



void append_bytes_to_textview (char * textview_name, char * bytes, int length)
{
	int i;
	GObject * object;
	GtkTextBuffer * buffer;
	GtkTextIter end;
	
	object = get_ui_object(textview_name);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(object));
	
	for (i = 0; i < length; i++)
	{
		if ((bytes[i] > 126)
			|| ((bytes[i] < 32) 
			    && ((bytes[i] != '\r') && (bytes[i] != '\n') && (bytes[i] != '\t'))))
			bytes[i] = '?';
	}
	
	gtk_text_buffer_get_end_iter(buffer, &end);
	
	gtk_text_buffer_insert(buffer, &end, bytes, length);
}



char * get_text_from_textview (char * textview_name)
{

	GObject * object;
	GtkTextBuffer * buffer;
	GtkTextIter start;
	GtkTextIter end;
	char * data;
	
	object = get_ui_object(textview_name);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(object));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	data = (char *) gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
	
	return data;

}



void set_status_bar (char * statusbar_name, char * text)
{

	GObject * object;
	
	object = get_ui_object(statusbar_name);
	gtk_statusbar_push(GTK_STATUSBAR(object),
	                   gtk_statusbar_get_context_id(GTK_STATUSBAR(object), "update"),
	                   (gchar *) text);
	                   
}



void clear_textview(char * textview_name)
{

	GObject * object;
	GtkTextBuffer * buffer;
	GtkTextIter start;
	GtkTextIter end;
	
	object = get_ui_object(textview_name);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(object));
	
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	
	gtk_text_buffer_delete(buffer, &start, &end);
	
}



char * get_textbox_text (char * textbox_name)
{

	GObject * object;
	
	object = get_ui_object(textbox_name);
	return (char *) gtk_entry_get_text(GTK_ENTRY(object));
	
}
