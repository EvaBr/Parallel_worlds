#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "histogram.h"


struct thread_data{
        char * chunk;
        int *ch_size;
        unsigned int *histo;
        pthread_mutex_t * mutex;
	pthread_cond_t * cond_cons;
	pthread_cond_t * cond_prod;
	int id;
};


void processing(char * chunk, unsigned int* histogram, int *ch_size){
        for (int i=0; i<*ch_size; i++) {
                if (chunk[i] >= 'a' && chunk[i] <= 'z')
                        histogram[chunk[i]-'a']++;
                else if(chunk[i] >= 'A' && chunk[i] <= 'Z')
                        histogram[chunk[i]-'A']++;
        }
}


void* parallel(void* threadArg){
        /*getting arguments*/
        struct thread_data *data = threadArg;
        int *size = data->ch_size;
        unsigned int *histogram;
        histogram = calloc(26, sizeof(*histogram));

	pthread_mutex_lock(data->mutex);
	pthread_cond_wait(data->cond_cons, data->mutex);
	printf("thread %d prejel signal, delam.\n", data->id);
	while (*size!=0){
	        processing(data->chunk, histogram, size);
		pthread_cond_signal(data->cond_prod);
//		printf("thread %d dajem signal produceru.\n", data->id);

		pthread_cond_wait(data->cond_cons, data->mutex);
	}

	printf("thread %d dajem signal produceru.\n", data->id);
	pthread_cond_signal(data->cond_prod);
	pthread_mutex_unlock(data->mutex);

        /*shranis local histogram in koncas*/
        data->histo = histogram;

//	printf("thread %d exit.\n", data->id);
	//pthread_exit(NULL);
        return NULL;
}



void* kopiraj(void* threadArg){
        /*getting arguments*/
        struct thread_data *data = threadArg;
        int *size;
	size = data->ch_size;


	pthread_mutex_lock(data->mutex);
	do {
		*size = get_chunk(data->chunk);
		pthread_cond_signal(data->cond_cons);
		printf("signaliziram \n");
		pthread_cond_wait(data->cond_prod, data->mutex);
	} while (*size > 0);
	printf("signaliziram vsem\n");
	pthread_cond_broadcast(data->cond_cons);
	pthread_mutex_unlock(data->mutex);

	printf("zakljucujem...\n");
	//pthread_exit(NULL);
	return  NULL;
}


void get_histogram(unsigned int* histogram,
                   unsigned int num_threads) {

	struct thread_data *seznam;
        seznam = malloc(num_threads * sizeof(*seznam));
        pthread_t *thread;
        thread = malloc(num_threads * sizeof(*thread));

        /*ustvaris mutex in cond. variables*/
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond_cons = PTHREAD_COND_INITIALIZER;
	pthread_cond_t cond_prod = PTHREAD_COND_INITIALIZER;


	char *chunk = malloc( CHUNKSIZE );
	int size = 0;


	seznam[0].chunk = chunk;
	seznam[0].ch_size = &size;
	seznam[0].mutex = &mutex;
	seznam[0].cond_cons = &cond_cons;
	seznam[0].cond_prod = &cond_prod;
	seznam[0].id = 0;
	pthread_create(thread, NULL, &kopiraj, seznam);
	
        for (int i=1; i<num_threads; i++){
                seznam[i].chunk = chunk;
                seznam[i].ch_size = &size;
                seznam[i].mutex = &mutex;
		seznam[i].cond_cons = &cond_cons;
		seznam[i].cond_prod = &cond_prod;
		seznam[i].id = i;
                pthread_create(thread+i, NULL, &parallel, seznam+i);
        }



        for(int i = 1; i < num_threads; ++i) {
	        pthread_join(thread[0], NULL);
                pthread_join(thread[i], NULL);
                for(int j = 0; j < NALPHABET; j++) {
                        histogram[j] += seznam[i].histo[j];
                }
        }


        //zbrises vse mutexe 
	free(chunk);
        free(thread);
        free(seznam);
}
