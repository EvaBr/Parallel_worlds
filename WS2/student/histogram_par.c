#include <pthread.h>
#include <stdio.h>
#include "histogram.h"
#include <stdlib.h>

struct thread_data{
	char ** buffer;
        unsigned int chunk_size;
        unsigned int *histo;
	pthread_mutex_t * mutex;
};


void processing(const char * delcek, unsigned int* histogram, unsigned int chunk_size){
	for (int i=0; i<chunk_size; i++) {
		if (delcek[i] >= 'a' && delcek[i] <= 'z')
			histogram[delcek[i]-'a']++;
		else if(delcek[i] >= 'A' && delcek[i] <= 'Z')
			histogram[delcek[i]-'A']++;
        }
}


char * update_ptr(struct thread_data* data){
	pthread_mutex_lock(data->mutex);
	char* delec = *data->buffer;
	if (*delec==TERMINATOR){
		delec = NULL;}
	else{
		*data->buffer += data->chunk_size;}

	pthread_mutex_unlock(data->mutex);
	return  delec;
}


void* parallel(void* threadArg){
	/*getting arguments*/
	struct thread_data *data = threadArg;
        unsigned int chunk_size = data->chunk_size;
        unsigned int *histogram;
        histogram = calloc(26, sizeof(*histogram));
	const char* delec = update_ptr(threadArg);

	while(delec){
		processing(delec, histogram, chunk_size);
		delec = update_ptr(threadArg);
	}

	/*shranis local histogram in koncas*/
        data->histo = histogram;
        return NULL;
}


void get_histogram(char *buffer,
		   			 unsigned int* histogram,
		   			 unsigned int num_threads,
						 unsigned int chunk_size) {


	struct thread_data *seznam;
	seznam = malloc(num_threads * sizeof(*seznam));
	pthread_t *thread;
	thread = malloc(num_threads * sizeof(*thread));

	/*ustvaris mutex*/
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	//int pthread_mutex_init( pthread_mutex_t *mutex, pthread_mutexattr_t *attr ); <- to je dinamicno ustvar.


	for (int i=0; i<num_threads; i++){
		seznam[i].buffer = &buffer;
		seznam[i].chunk_size = chunk_size;
		seznam[i].mutex = &mutex;
		pthread_create(thread+i, NULL, &parallel, seznam+i);
	}

	for(int i = 0; i < num_threads; ++i) {
		pthread_join(thread[i], NULL);
		for(int j = 0; j < NALPHABET; j++) {
	        	histogram[j] += seznam[i].histo[j];
		}
		free(seznam[i].histo);
	}

	//zbrises vse mutexe 
	//int pthread_mutex_destroy( pthread_mutex_t *mutex );
	free(thread);
	free(seznam);
}
