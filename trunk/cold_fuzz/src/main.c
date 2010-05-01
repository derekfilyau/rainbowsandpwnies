#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <pthread.h>

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>

#include "error.h"
#include "fuzzer.h"
#include "interface.h"
#include "network.h"
#include "options.h"
#include "results.h"
#include "thread.h"


#define DEBUG_MAIN 1



static pthread_mutex_t global_lock;

// these should all move to threadsafe_int_t
static int master_thread_running;
threadsafe_int_t master_thread_abort;
static int threads_running_n;

pthread_attr_t attr;



struct fuzzer_thread
{
	int thread_id;
	int data_len;
	pthread_t thread;
	char * hostname;
	char * port;
	unsigned char * data;
};



void threads_running_increment ()
{

	pthread_mutex_lock(&global_lock);
	threads_running_n++;
	pthread_mutex_unlock(&global_lock);
	
}



void threads_running_decrement ()
{

	pthread_mutex_lock(&global_lock);
	threads_running_n--;
	pthread_mutex_unlock(&global_lock);
	
}



void threads_running_reset ()
{

	pthread_mutex_lock(&global_lock);
	threads_running_n = 0;
	pthread_mutex_unlock(&global_lock);
	
}


int threads_running_get ()
{

	int result;
	pthread_mutex_lock(&global_lock);
	result = threads_running_n;
	pthread_mutex_unlock(&global_lock);
	return result;
	
}


void lock_and_append (char * textview, char * text)
{
	
	gdk_threads_enter();
	append_to_textview(textview, text);
	gdk_threads_leave();
	
}


struct fuzzer_thread * fuzzer_thread_create (int thread_id, char * hostname, char * port, unsigned char * data, int data_len)
{

	struct fuzzer_thread * t;

	//pthread_attr_setstacksize(&attr, 1024*1024*40);
	
	#if DEBUG_MAIN == 1
		printf("malloc fuzzer_thread *\n");
	#endif	
	t = (struct fuzzer_thread *) malloc(sizeof(struct fuzzer_thread));
	
	t->thread_id = thread_id;
	#if DEBUG_MAIN == 1
		printf("malloc t->hostname *\n");
	#endif
	t->hostname = (char *) malloc(strlen(hostname) + 1);
	if (t->hostname == NULL)
	{
		fuzzer_errno = FUZZER_ERROR_THREAD_MALLOC_FAIL;
		return NULL;
	}
	#if DEBUG_MAIN == 1
		printf("malloc t->post *\n");
	#endif
	t->port = (char *) malloc(strlen(port) + 1);
	if (t->port == NULL)
	{
		fuzzer_errno = FUZZER_ERROR_THREAD_MALLOC_FAIL;
		return NULL;
	}
	t->data_len = data_len;
	
	memset(t->hostname, 0, strlen(hostname) + 1);
	memset(t->port, 0, strlen(port) + 1);
	
	strncpy(t->hostname, hostname, strlen(hostname));
	strncpy(t->port, port, strlen(port));

	#if DEBUG_MAIN == 1
		printf("malloc t->data *\n");
	#endif
	t->data = (unsigned char *) malloc(data_len);
	if (t->data == NULL)
	{
		fuzzer_errno = FUZZER_ERROR_THREAD_MALLOC_FAIL;
		return NULL;
	}
	memcpy(t->data, data, data_len);
	
	return t;
	
}


void fuzzer_thread_destroy (struct fuzzer_thread * t)
{

	free(t->hostname);
	free(t->port);
	free(t->data);
	free(t);
	
}


void fuzzer_thread_instance (void * thread)
{

	struct fuzzer_thread * t = thread;
	struct network_info * info = NULL;
	#if DEBUG_MAIN == 1
		char debug_tmp[128];
		int thread_id = t->thread_id;
	#endif

	int error;

	#if DEBUG_MAIN == 1
		snprintf(debug_tmp, 128, "%d connecting", thread_id);
		printf("%s\n", debug_tmp);
	#endif
	error = network_connect(t->hostname, t->port, &info);
	if (error)
	{
		//lock_and_append("text_debug", fuzzer_error_string(error));
		debug_text_out(fuzzer_error_string(error));
	}
	else
	{
		#if DEBUG_MAIN == 1
			snprintf(debug_tmp, 128, "%d sending", thread_id);
			printf("%s\n", debug_tmp);
		#endif
		error = network_send(info, t->data, t->data_len);
		if (error)
		{
			//lock_and_append("text_debug", fuzzer_error_string(error));
			debug_text_out(fuzzer_error_string(error));
			threads_running_decrement();
			return;
		}
		else
		{
			#if DEBUG_MAIN == 1
				snprintf(debug_tmp, 128, "%d receiving", thread_id);
				printf("%s\n", debug_tmp);
			#endif	
			error = network_receive(info);
			if (error)
			{
				//lock_and_append("text_debug", fuzzer_error_string(error));
				debug_text_out(fuzzer_error_string(error));
				threads_running_decrement();
			}
			else
			{
				#if DEBUG_MAIN == 1
					snprintf(debug_tmp, 128, "%d sending results", thread_id);
					printf("%s\n", debug_tmp);
				#endif	
				// if master_thread_abort is 1, someone hit the abort switch, don't bother printing output
				if (get_threadsafe_int(&master_thread_abort) == 0)
				{
					gdk_threads_enter();
					results_append(t->thread_id, t->hostname, t->data_len, info->bytes_in_buffer, t->data, info->buffer);
					gdk_threads_leave();
				}			
			}
		}
	}

	network_close(info);
	fuzzer_thread_destroy(t);
	threads_running_decrement();
	
	#if DEBUG_MAIN == 1
		snprintf(debug_tmp, 128, "%d done", thread_id);
		printf("%s\n", debug_tmp);
	#endif	
	
}



gboolean key_press (GtkWidget * window, GdkEventKey * pKey, gpointer userdata)
{

	if (pKey->type == GDK_KEY_PRESS)
	{
		if (pKey->keyval == GDK_Escape)
			gtk_main_quit();
	}
	
	return FALSE;

}



char * replace_newlines (char * data)
{

	int data_i, new_data_i;
	char * new_data;
	
	new_data = (char *) malloc(strlen(data) * 2);
	if (new_data == NULL)
	{
		debug_text_out("malloc for replacement failed\n");
		return NULL;
	}
	memset(new_data, 0, strlen(data) * 2);
	
	new_data_i = 0;
	for (data_i = 0; data_i < strlen(data); data_i++)
	{
		if (data[data_i] == '\n')
		{
			new_data[new_data_i++] = '\r';
			new_data[new_data_i++] = '\n';
		}
		else
			new_data[new_data_i++] = data[data_i];
	}
	return new_data;
}



// function for convenience when we have to abort master_fuzzer_thread early
void master_fuzzer_thread_early_abort ()
{
	pthread_mutex_lock(&global_lock);
	master_thread_running = 0;
	pthread_mutex_unlock(&global_lock);	
}



void master_fuzzer_thread ()
{

	int output_len;
	int thread_id;

	char * hostname;
	char * port;
	char * input;
	
	unsigned char * output;
	
	fuzzer_engine_t * engine;
	struct fuzzer_thread * thread;
	GObject * send_button;
	
	char status_text[128];
	
	
	hostname = get_textbox_text("box_host");
	port = get_textbox_text("box_port");
	
	if (strlen(hostname) == 0)
	{
		debug_text_out("you must enter a hostname\n");
		master_fuzzer_thread_early_abort();
		return;
	}
	if (strlen(port) == 0)
	{
		debug_text_out("you must enter a port\n");
		master_fuzzer_thread_early_abort();
		return;
	}
	
	gdk_threads_enter();
	input = get_text_from_textview("text_input");
	gdk_threads_leave();
	
	if (OPTIONS & OPTIONS_NEWLINE)
	{
		debug_text_out("replacing \\n with \\r\\n\n");
		master_fuzzer_thread_early_abort();
		input = replace_newlines(input);
	}
	
	#if DEBUG_MAIN == 1
		printf("creating fuzzer_engine\n");
	#endif
	engine = fuzzer_engine_create();
	
	// fuzzer_engine_create will generate error output for us
	// FIX THIS
	if (engine == NULL)
	{
		master_fuzzer_thread_early_abort();
		debug_text_out(fuzzer_error_string(fuzzer_errno));
		return;
	}
	
	if (fuzzer_engine_load(engine, input))
	{
		master_fuzzer_thread_early_abort();
		debug_text_out(fuzzer_error_string(fuzzer_errno));
		return;
	}

	gdk_threads_enter();
	send_button = get_ui_object("button_send");
	g_object_set(GTK_OBJECT(send_button), "label", "Abort", NULL);
	gdk_threads_leave();

	threads_running_reset();
	
	thread_id = 0;
	
	while ((output = fuzzer_engine_execute(engine, &output_len)) != NULL)
	{
		// this loop is set up this way to for at least SOME pause inbetween launching threads.
		// launching threads too quickly was causing connection fails as the accept() queue filled
		// up on the other end
		while (1)
		{
			usleep(20000);
			if ((threads_running_get() < get_threadsafe_int(&OPTIONS_THREADS_N)) || (get_threadsafe_int(&master_thread_abort)))
				break;
		}
		
		if (get_threadsafe_int(&master_thread_abort))
			break;
		
		#if DEBUG_MAIN == 1
			printf("fuzzer_thread_create\n");
		#endif
		thread = fuzzer_thread_create(thread_id++, hostname, port, output, output_len);
		if (thread == NULL)
		{
			debug_text_out(fuzzer_error_string(errno));
			break;
		}
		
		#if DEBUG_MAIN == 1
			printf("launching thread\n");
		#endif
		pthread_create(&(thread->thread), NULL, (void *) fuzzer_thread_instance, thread);
		threads_running_increment();
		
		memset(status_text, 0, 128);
		snprintf(status_text, 128, "%d threads active.", threads_running_get());
		gdk_threads_enter();
		set_status_bar("status_threads", status_text);
		gdk_threads_leave();
	}
	
	
	#if DEBUG_MAIN == 1
		printf("no more thread iterations left\n");
	#endif
	
	while (threads_running_get() > 0)
	{
		if (get_threadsafe_int(&master_thread_abort))
			break;
		usleep(100000);
		snprintf(status_text, 128, "%d threads active.", threads_running_get());
		gdk_threads_enter();
		set_status_bar("status_threads", status_text);
		gdk_threads_leave();
	}
	if (get_threadsafe_int(&master_thread_abort))
	{
		snprintf(status_text, 128, "master thread aborted");
		gdk_threads_enter();
		set_status_bar("status_threads", status_text);
		gdk_threads_leave();
	}
	else
	{
		snprintf(status_text, 128, "%d threads active.", threads_running_get());
		gdk_threads_enter();
		set_status_bar("status_threads", status_text);
		gdk_threads_leave();
	}
	
	if (OPTIONS & OPTIONS_NEWLINE)
		free(input);
	
	fuzzer_engine_destroy(engine);
	
	pthread_mutex_lock(&global_lock);
	master_thread_running = 0;
	pthread_mutex_unlock(&global_lock);
	
	gdk_threads_enter();
	g_object_set(GTK_OBJECT(send_button), "label", "Send", NULL);
	gdk_threads_leave();
	
}



void button_send_click ()
{

	pthread_t thread;

	pthread_mutex_lock(&global_lock);
	if (master_thread_running == 0)
	{
		master_thread_running = 1;
		set_threadsafe_int(&master_thread_abort, 0);
		pthread_mutex_unlock(&global_lock);
		pthread_create(&thread, NULL, (void *) master_fuzzer_thread, NULL);
	}
	else
	{
		set_threadsafe_int(&master_thread_abort, 1);
		pthread_mutex_unlock(&global_lock);
	}
	
}


void show_window_about ()
{

	GObject * object;
	
	object = get_ui_object("window_about");

	gtk_widget_show(GTK_WIDGET(object));
	
}




int main (int argc, char * argv[])
{

	GObject * object;

	gtk_init(&argc, &argv);
	g_thread_init(NULL);
	gdk_threads_init();
	
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "interface.gtk", NULL);

	master_thread_running = 0;

	options_init();
	results_init();
	
	object = get_ui_object("menu_options_options");
	g_signal_connect(object, "activate", G_CALLBACK(options_window), NULL);
	
	object = get_ui_object("menu_options_about");
	g_signal_connect(object, "activate", G_CALLBACK(show_window_about), NULL);
	
	object = get_ui_object("button_send");
	g_signal_connect(object, "clicked", G_CALLBACK(button_send_click), NULL);
	
	object = get_ui_object("mainWindow");
	g_signal_connect(object, "delete_event", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect (object, "key-press-event", G_CALLBACK (key_press), NULL);	

	gtk_widget_show(GTK_WIDGET(object));
	
	results_test();
	
	pthread_mutex_init(&global_lock, NULL);

	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
	
	pthread_mutex_destroy(&global_lock);
	
	return 1;

}
