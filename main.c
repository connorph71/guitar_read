// Author: Connor Hoang 
#include <stdio.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/ioctl.h> 
#include <linux/i2c-dev.h> 
#include <math.h> 
#include <fftw3.h>

#include "rms.h"

#define ADC_ADDR 0x4b
#define ADC_MIDPOINT 128 
#define WINDOW_SIZE 2048
#define SAMPLE_RATE 4000.0f

int main() { 
	float samples_f[WINDOW_SIZE];
	fftwf_complex *fft_out = fftwf_malloc(sizeof(fftwf_complex) * (WINDOW_SIZE/2 + 1));
	fftwf_plan fft_plan;


	// open i2c
	int fd = open("/dev/i2c-1", O_RDWR); 
	if (fd < 0) { 
		perror("Failed to open I2C bus"); 
		return 1; 
	}
	
	// connect to ADC
	if (ioctl(fd, I2C_SLAVE, ADC_ADDR) < 0) { 
		perror("Failed to connect to ADC"); 
		return 1; 
	}

	fft_plan = fftwf_plan_dft_r2c_1d(WINDOW_SIZE, 
							samples_f, 
							fft_out, 
							FFTW_MEASURE);
	
	unsigned char control = 0x84; 
	unsigned char value; 
	float samples[WINDOW_SIZE]; 
	int index = 0; 
		
	while (1) { 
		write(fd, &control, 1); 
		read(fd, &value, 1); 
		read(fd, &value, 1); 

		// actual sample 
		samples[index++] = value; 

		/*RMS scaling
		float rms = compute_rms(samples, WINDOW_SIZE);
		if (index >= WINDOW_SIZE) {
			float rms = compute_rms(samples, WINDOW_SIZE); 
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
		

		//float rms = compute_rms(samples, WINDOW_SIZE);
		if (rms < 3.0f) {   // tune later
    		index = 0;
    		//continue;
		}
		*/

		

		// FFT		
		if (index >= WINDOW_SIZE) { 	

    		// 1. Compute mean (DC offset)
    		float mean = compute_mean(samples, WINDOW_SIZE);
		

    		// 2. Convert to float + Hann window
    		for (int i = 0; i < WINDOW_SIZE; i++) {
        		float centered = samples[i] - mean;
        		float w = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (WINDOW_SIZE - 1)));
        		samples_f[i] = centered * w;
    		}
		
			// 3. FFT
		    fftwf_execute(fft_plan);

			// 4. Magnitude Spectrum (peak detection) 
			
			int k_min = 70 * WINDOW_SIZE / SAMPLE_RATE;
			int k_max = 700 * WINDOW_SIZE / SAMPLE_RATE;
			//int k_min = 2;  // ignore DC + very low bins
			//int k_max = (int)(1000.0f * WINDOW_SIZE / SAMPLE_RATE);

			if (k_max > WINDOW_SIZE/2 - 1)
	    		k_max = WINDOW_SIZE/2 - 1;
	
			int max_bin = -1;
			float max_mag = 0.0f;

			for (int k = k_min; k <= k_max; k++) {
    			float re = fft_out[k][0];
    			float im = fft_out[k][1];
    			float mag = re*re + im*im;   // squared magnitude

			    if (mag > max_mag * 0.85) {
    	    		max_mag = mag;
        			max_bin = k;
    			}
			}


			//printf("sample[0]=%f\n", samples_f[0]);
			//printf("max_bin=%d max_mag=%f\n", max_bin, max_mag);
			
			// validate
			if (max_bin < 1 || max_bin >= WINDOW_SIZE/2 - 1) {
    			index = 0;
    			continue;
			}
			// 5. Parabolic Interpolation
			float alpha = fft_out[max_bin - 1][0]*fft_out[max_bin - 1][0] +
        	      fft_out[max_bin - 1][1]*fft_out[max_bin - 1][1];

			float beta  = max_mag;

			float gamma = fft_out[max_bin + 1][0]*fft_out[max_bin + 1][0] +
        	      fft_out[max_bin + 1][1]*fft_out[max_bin + 1][1];

			float denom = (alpha - 2.0f*beta + gamma);

			float delta = 0.0f;

			if (fabsf(denom) > 1e-6f)
			{
	 		   delta = 0.5f * (alpha - gamma) / denom;		
			}
			
			float freq = (max_bin + delta) * SAMPLE_RATE / WINDOW_SIZE;
			printf("%f = (%d + %f) * %f / %d\n", freq, max_bin, delta, SAMPLE_RATE, WINDOW_SIZE);
			if (!isfinite(freq) || freq < 50.0f || freq > 1000.0f) {
    			index = 0;
    			continue;
			}

			// 6. Harmonic Correction
			if(freq > 100) { 	
				float harm_ratios[] = {0.5f, 0.333f, 0.25f, 0.2f, 0.16};

				for(int i = 0; i < 5; i++) {
					//printf("ratio %2.2f: \n", harm_ratios[i]);
			    	float test_freq = freq * harm_ratios[i];
    				int test_bin = test_freq * WINDOW_SIZE / SAMPLE_RATE;
	
		    		float re = fft_out[test_bin][0];
    				float im = fft_out[test_bin][1];
    				float mag = sqrtf(re*re + im*im);
	
				    if(mag > 0.1f * max_mag) {
				        //printf("hit! %f -> %f\n", freq, test_freq);
						freq = test_freq;
						
        				break;
    				}
				}
			}

			// 7. Freq -> Notes
			float note_num = 69.0f + 12.0f * log2f(freq / 440.0f);

			if (!isfinite(note_num)) {
    			index = 0;
    			continue;
			}
	
			int midi = (int)roundf(note_num);
			float cents = (note_num - midi) * 100.0f;

			const char *names[] = {
    			"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
				};

			printf("\rFreq: %6.1f Hz | Note: %s | %+6.1f cents    \n",
    	   		freq,
       			names[midi % 12],
       			cents);

			//fflush(stdout);

			index = 0;
		}
	//
	usleep(250);
	} 

	fftwf_destroy_plan(fft_plan);

	close(fd); 
	return 0; 
}
