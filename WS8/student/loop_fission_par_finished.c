#include <omp.h>

void compute(unsigned long **a, unsigned long **b, unsigned long **c, unsigned long **d, int N, int num_threads) {

	#pragma omp parallel
	// perform loop fission to transform this loop and parallelize it with OpenMP
	#pragma omp for
	for (int j = 1; j < N; j++) {
		for (int i = 1; i < N; i++) {
		//#pragma omp for
			c[i][j] = 3 * d[i][j];
			d[i][j]   = 2 * c[i + 1][j];
		}
		//#pragma omp for
//		for (int j = 1; j < N; j++) {
//			d[i][j]   = 2 * c[i + 1][j];
//		}
	}
	#pragma omp for
	for (int i = 1; i < N; i++) {
		//#pragma omp for
		for (int j = 1; j < N; j++) {
			a[i][j]   = a[i][j] * b[i][j];
//		}
		//#pragma omp for
//		for (int i = 1; i < N; i++) {
			b[i][j + 1] = 2 * a[i][j] * c[i - 1][j];
		}
	}
}
