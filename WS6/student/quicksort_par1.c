#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "quicksort.h"
#include "helper.h"
#include <omp.h>

void engine (int *a, int left, int right, int num_threads){
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

	if (omp_get_active_level() < 3){
		#pragma omp parallel sections
		{
	  		#pragma omp section
			engine(a, left, swapIdx - 1, num_threads);
			#pragma omp section
			engine(a, swapIdx + 1, right, num_threads);
		}
	} else {
		engine(a, left, swapIdx - 1, num_threads);
		engine(a, swapIdx + 1, right, num_threads);

	  }
	}
}


void quicksort(int *a, int left, int right, int num_threads){
	omp_set_nested(1);
	omp_set_max_active_levels(27);
	omp_set_num_threads(num_threads);
	engine(a, left, right, num_threads);
}
