#include "thread.h"

int get_threadsafe_int (threadsafe_int_t * t)
{
	int result;
	pthread_mutex_lock(&(t->lock));
	result = t->value;
	pthread_mutex_unlock(&(t->lock));
	return result;
}



void set_threadsafe_int (threadsafe_int_t * t, int value)
{
	pthread_mutex_lock(&(t->lock));
	t->value = value;
	pthread_mutex_unlock(&(t->lock));
}



void add_threadsafe_int (threadsafe_int_t * t, int operand)
{
	pthread_mutex_lock(&(t->lock));
	t->value += operand;
	pthread_mutex_unlock(&(t->lock));
}



void subtract_threadsafe_int (threadsafe_int_t * t, int operand)
{
	pthread_mutex_lock(&(t->lock));
	t->value -= operand;
	pthread_mutex_unlock(&(t->lock));
}




int get_threadsafe_int64 (threadsafe_int64_t * t)
{
	int result;
	pthread_mutex_lock(&(t->lock));
	result = t->value;
	pthread_mutex_unlock(&(t->lock));
	return result;
}



void set_threadsafe_int64 (threadsafe_int64_t * t, int64_t value)
{
	pthread_mutex_lock(&(t->lock));
	t->value = value;
	pthread_mutex_unlock(&(t->lock));
}



void add_threadsafe_int64 (threadsafe_int64_t * t, int64_t operand)
{
	pthread_mutex_lock(&(t->lock));
	t->value += operand;
	pthread_mutex_unlock(&(t->lock));
}



void subtract_threadsafe_int64 (threadsafe_int64_t * t, int64_t operand)
{
	pthread_mutex_lock(&(t->lock));
	t->value -= operand;
	pthread_mutex_unlock(&(t->lock));
}
