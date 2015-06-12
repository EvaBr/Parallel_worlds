#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "quicksort.h"
#include "helper.h"
#include <omp.h>

#define OMP_LEVELS 5


void qs (int *a, int left, int right, int num_threads);

void quicksort(int *a, int left, int right, int num_threads){
	omp_set_nested(1);
	omp_set_max_active_levels(33);
	omp_set_num_threads(num_threads);
	qs(a, left, right, num_threads);
}

void qs (int *a, int left, int right, int num_threads){
	if(left < right)
	{
		int x = left, y = (left+right)/2, z =right;
		int pivotIdx = (a[x] <= a[y])
		    ? ((a[y] <= a[z]) ? y : ((a[x] < a[z]) ? z : x))
		    : ((a[x] <= a[z]) ? x : ((a[y] < a[z]) ? z : y));

	  int pivotVal = a[pivotIdx];
	  swap(a + pivotIdx, a + right);

	  int swapIdx = left;


	  for(int i=left; i < right; i++)
	  {
		if(a[i] <= pivotVal)
		{
			swap(a + swapIdx, a + i);
		  	swapIdx++;
		}
	  }
	  swap(a + swapIdx, a + right);

//ce dosezen max level se poklici sekvencno, sicer pa s pragmo parallel.
	  if (omp_get_active_level() > OMP_LEVELS){
		qs(a, left, swapIdx - 1, num_threads);
		qs(a, swapIdx + 1, right, num_threads);
	  }
	  else {
	  #pragma omp parallel sections
	  {
	  	#pragma omp section
		  	qs(a, left, swapIdx - 1, num_threads);
		#pragma omp section
			qs(a, swapIdx + 1, right, num_threads);

	  }
	  }
	}
}
// za prskusat ce dela zazeni s flagi -e 35 -d 1 ; 35 je dolzina arraya, -d 1 pa mu pove da naj sprinta, da loh vidš če je vse okidoks.
