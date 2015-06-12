#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "histogram.h"
#include <unistd.h>
#include <string.h>

struct thread_data{
        char * chunk;
        int *ch_size;
        unsigned int *histo;
        pthread_mutex_t * mutex;
	pthread_cond_t * cond_cons;
	pthread_cond_t * cond_prod;
	int id;
};


void processing(char * chunk, unsigned int* histogram, const int sajz){
        for (int i=0; i<sajz; i++) {
                if (chunk[i] >= 'a' && chunk[i] <= 'z')
                        histogram[chunk[i]-'a']++;
                else if(chunk[i] >= 'A' && chunk[i] <= 'Z')
                        histogram[chunk[i]-'A']++;
        }
}


void* parallel(void* threadArg){
        /*getting arguments*/
        struct thread_data *data = threadArg;
        unsigned int *histogram;
        histogram = calloc(26, sizeof(*histogram));

	char * mojmoj = calloc(CHUNKSIZE, sizeof(*mojmoj));
	int size;

	do {
		pthread_mutex_lock(data->mutex);
		pthread_cond_wait(data->cond_cons, data->mutex);
//		printf("thread %d got signal, works.\n", data->id);
	        size = *data->ch_size;
		if(size==0) {
//			printf("slave zaznal size=0");
	//		pthread_cond_signal(data->cond_prod);
			pthread_mutex_unlock(data->mutex);
			break;
		}
		memcpy(mojmoj, data->chunk, size);
//		printf("thread %d signals producer.\t", data->id);
//		usleep(100);
		pthread_cond_signal(data->cond_prod);
		pthread_mutex_unlock(data->mutex);

	        processing(mojmoj, histogram, size);

	} while (size!=0);

        /*shranis local histogram in koncas*/
        data->histo = histogram;

	free(mojmoj);
//	printf("thread %d exit.\n", data->id);
	//pthread_exit(NULL);
        return NULL;
}


void* kopiraj(void* threadArg){
        /*getting arguments*/
        struct thread_data *data = threadArg;
        //int *size;   //probi sam data->size = get_chunk...
	//size = data->ch_size;

	usleep(100);
	pthread_mutex_lock(data->mutex);
	do {
		// g*size = get_chunk(data->chunk);
		*data->ch_size = get_chunk(data->chunk);
//		printf("size= %d \n", *data->ch_size);
		usleep(100000);
		pthread_cond_broadcast(data->cond_cons);
//		printf("signaliziral,   ");
//		printf("unlocking mutex.\t");
//		pthread_mutex_unlock(data->mutex);
//		usleep(100);
//		printf("Master going to hibernate...\n");
		if(*data->ch_size==0){break;};
		pthread_cond_wait(data->cond_prod, data->mutex);
//		printf("master awoken.\n");
	} while (*data->ch_size > 0);

//	printf("signaliziram vsem\n");
	usleep(1000);
	pthread_cond_broadcast(data->cond_cons);
	pthread_mutex_unlock(data->mutex);

//	printf("zakljucujem...\n");
	//pthread_exit(NULL);
	return  NULL;
}


void get_histogram(unsigned int* histogram,
                   unsigned int num_threads) {

//	printf("hello, jaz sem thread");

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

/*        seznam[1].chunk = chunk;
        seznam[1].ch_size = &size;
        seznam[1].mutex = &mutex;
	seznam[1].cond_cons = &cond_cons;
	seznam[1].cond_prod = &cond_prod;
	seznam[1].id = 1;
        pthread_create(thread+1, NULL, &parallel, seznam+1);*/



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
                pthread_join(thread[i], NULL);
                for(int j = 0; j < NALPHABET; j++) {
                        histogram[j] += seznam[i].histo[j];
                }
        }

        pthread_join(thread[0], NULL);
	pthread_join(thread[1],NULL);

        //zbrises vse mutexe
	free(chunk);
        free(thread);
        free(seznam);
}
