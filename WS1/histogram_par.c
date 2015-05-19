#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "histogram.h"

struct thread_data{
	unsigned int nBlocks;
	block_t *blocks;
	unsigned int num_threads;
	int id;
	unsigned int *histo;

};

void * parallel(void *thrArg){
	struct thread_data *data = thrArg;
	int tred = data->id;
	unsigned int nB = data->nBlocks;
	block_t *blocks = data->blocks;
	unsigned int *histogram;
	histogram = calloc(26, sizeof(*histogram));
	unsigned int tredov = data->num_threads;
	
	int i;
	int eltov = nB/tredov; //tu ze dobis nBlocks = nBlocks*(rep+1) pri klicu fje v mainu

	// build histogram
        for (i=tred*eltov; i<eltov*(tred+1); i++) { //tu obdela sam tok eltov, da se izide deljenje s st. tredov
                for (int j=0; j<BLOCKSIZE; j++) {
                        if (blocks[i][j] >= 'a' && blocks[i][j] <= 'z')
                                histogram[blocks[i][j]-'a']++;
                        else if(blocks[i][j] >= 'A' && blocks[i][j] <= 'Z')
                                histogram[blocks[i][j]-'A']++;
                }
        }
        for (i=eltov*tredov+tred; i<nB; i+=tredov){ //obdelava preostalih
                for (int j=0; j<BLOCKSIZE; j++) {
                        if (blocks[i][j] >= 'a' && blocks[i][j] <= 'z')
                                histogram[blocks[i][j]-'a']++;
                        else if(blocks[i][j] >= 'A' && blocks[i][j] <= 'Z')
                                histogram[blocks[i][j]-'A']++;
                }
        }
	data->histo = histogram;
	return NULL;
}


void get_histogram(unsigned int nBlck,
			block_t *blockz,
			unsigned int* histogram,
			unsigned int num_thread){

	struct thread_data *seznam;
	seznam = malloc(num_thread * sizeof(*seznam));

	pthread_t *thread;
	thread = malloc(num_thread * sizeof(*thread));

	for (int i=0; i<num_thread; i++){
		seznam[i].id = i;
		seznam[i].nBlocks = nBlck;
		seznam[i].blocks = blockz;
		seznam[i].num_threads = num_thread;
		pthread_create(thread+i, NULL, &parallel, seznam+i);
	}

	//join threads at the end
	for(int i=0; i<num_thread; i++){
		pthread_join(*(thread+i), NULL);
	}

	for (int i=0; i<num_thread; i++){
		for (int j=0; j<26; j++){
			histogram[j] += seznam[i].histo[j];
		}
		free(seznam[i].histo);
	}


	free(seznam);
	free(thread);
}

