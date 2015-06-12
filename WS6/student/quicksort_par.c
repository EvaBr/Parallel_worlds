#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "quicksort.h"
#include "helper.h"
#include <omp.h>

#define TR 100

void qs (int *a, int left, int right, int num_threads);

void quicksort(int *a, int left, int right, int num_threads){
	omp_set_nested(1);
	omp_set_dynamic(0);
	omp_set_max_active_levels(70);
	omp_set_num_threads(num_threads);

	#pragma omp parallel //shared(a, num_threads)
	{
		#pragma omp single
		qs(a, left, right, num_threads);
	}
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

	  #pragma omp task //shared(num_threads, swapIdx) //firstprivate(left) //final (omp_get_active_level() < TR)
		qs(a, left, swapIdx - 1, num_threads);
	  #pragma omp task //shared(num_threads, swapIdx) //firstprivate(right) //final (omp_get_active_level() > TR)
		qs(a, swapIdx + 1, right, num_threads);
//	  #pragma omp taskwait
	}
}
// za prskusat ce dela zazeni s flagi -e 35 -d 1 ; 35 je dolzina arraya, -d 1 pa mu pove da naj sprinta, da loh vidš če je vse okidoks.
