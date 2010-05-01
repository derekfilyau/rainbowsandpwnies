#ifndef thread_HEADER
#define thread_HEADER

#include <pthread.h>
#include <inttypes.h>

typedef struct
{
	pthread_mutex_t lock;
	int value;
} threadsafe_int_t;



typedef struct
{
	pthread_mutex_t lock;
	int64_t value;
} threadsafe_int64_t;



int  get_threadsafe_int      (threadsafe_int_t * t);
void set_threadsafe_int      (threadsafe_int_t * t, int value);
void add_threadsafe_int      (threadsafe_int_t * t, int operand);
void subtract_threadsafe_int (threadsafe_int_t * t, int operand);

int  get_threadsafe_int64      (threadsafe_int64_t * t);
void set_threadsafe_int64      (threadsafe_int64_t * t, int64_t value);
void add_threadsafe_int64      (threadsafe_int64_t * t, int64_t operand);
void subtract_threadsafe_int64 (threadsafe_int64_t * t, int64_t operand);


#endif
