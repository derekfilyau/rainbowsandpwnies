#ifndef options_HEADER
#define options_HEADER

#include <stdlib.h>

#include "interface.h"
#include "thread.h"

threadsafe_int_t OPTIONS_THREADS_N;
threadsafe_int_t OPTIONS_RESULTS_MAX;



void options_set_entry_to_int (char * entry_name, int value);
int options_get_int_from_entry (char * entry_name);
void button_save_options_click ();
void options_init ();
int options_window ();


#endif
