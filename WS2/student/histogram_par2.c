#include <pthread.h>
#include <stdio.h>
#include "histogram.h"
#include <stdlib.h>

struct thread_data{
	char * buffer;
        unsigned int chunk_size;
//        unsigned int num_threads;
        int id;
        unsigned int *histo;
	int* howfar;
	pthread_mutex_t * mutex;
};


void* parallel(void* threadArg){
	/*getting arguments*/
	struct thread_data *data = threadArg;
	//int id = data->id;
        unsigned int chunk_size = data->chunk_size;
        char *buffer = data->buffer;

        unsigned int *histogram;
        histogram = calloc(26, sizeof(*histogram));

  //      unsigned int num_threads = data->num_threads;
	int* howfar = data->howfar;
	pthread_mutex_t * mutex = data->mutex;

	int where=0;

	while(where!=-1){
		pthread_mutex_lock(mutex);
		where = *howfar;
		pthread_mutex_unlock(mutex);

		pthread_mutex_lock(mutex);
		char znak = buffer[where+1];
		pthread_mutex_unlock(mutex);

		if (znak==TERMINATOR){
			pthread_mutex_lock(mutex);
			*howfar = -1; // povemo, da smo cist na konc in naj se vsi tredi ustavjo.
			pthread_mutex_unlock(mutex);
			continue;
		}
		pthread_mutex_lock(mutex);
		*howfar = *howfar+chunk_size-1;
		pthread_mutex_unlock(mutex);
		
		
		//zaklenes stvar
		pthread_mutex_lock(mutex);
		for (int i=where; i<chunk_size && i!=TERMINATOR; i++) {
			if (buffer[i] >= 'a' && buffer[i] <= 'z')
				histogram[buffer[i]-'a']++;
			else if(buffer[i] >= 'A' && buffer[i] <= 'Z')
				histogram[buffer[i]-'A']++;
	        }
	        pthread_mutex_unlock(mutex);
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

	int howfar = 0;

	for (int i=0; i<num_threads; i++){
		seznam[i].id = i;
		seznam[i].buffer = buffer;
		seznam[i].chunk_size = chunk_size;
		//seznam[i].num_threads = num_threads;
		seznam[i].mutex = &mutex;
		seznam[i].howfar = &howfar;
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
