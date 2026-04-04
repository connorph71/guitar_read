// Author: Connor Hoang 
#include <math.h>
#include <stdio.h>
#include "rms.h"

float compute_mean(float *samples, int n) { 
	float sum = 0.0f; 
	for (int i = 0; i < n; i++) { 
		sum += samples[i]; 
	} 
	return sum / n; 
} 

float compute_rms(float *samples, int n) { 
    int min = 255, max = 0;

    for (int i = 0; i < n; i++) {
        if (samples[i] < min) min = samples[i];
        if (samples[i] > max) max = samples[i];
    }

    float mean = compute_mean(samples, n); 
    float sum = 0.0f; 

    for (int i = 0; i < n; i++) { 
        float centered = samples[i] - mean; 
        sum += centered * centered; 
    }

    float rms = sqrt(sum / n);

    /*printf("\rmin=%3d max=%3d span=%3d rms=%6.2f   \n",
           min, max, max - min, rms);
	*/

    return rms; 
}

void print_rms(float *samples, int window_size, int index){
	float rms = compute_rms(samples, window_size);
	if (index >= window_size) {
		rms = compute_rms(samples, window_size); 
		int bars = rms / 2; // scale factor 
		
		if (bars > 40) 
			bars = 40; 
		
		printf("\rRMS: %3.0f |", rms); 
			
		for (int i = 0; i < bars; i++) 
			printf("¦"); 
				
		for (int i = bars; i < 40; i++) 
			printf(" "); 
			
		printf("|"); 
		fflush(stdout); 
		index = 0; 	
	} 
}
