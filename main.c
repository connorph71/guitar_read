// Author: Connor Hoang 
#include <stdio.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <sys/ioctl.h> 
#include <linux/i2c-dev.h> 
#include <math.h> 
#include <fftw3.h>
#include <pthread.h> //multithread
#include <unistd.h>

#include "mic_read.h"
#include "rms.h"

#define ADC_ADDR 0x4b
#define ADC_MIDPOINT 128 
#define WINDOW_SIZE 2048
#define SAMPLE_RATE 4000.0f

//multithreading
#define BUFFER_SIZE 1024
float buffer[BUFFER_SIZE];
int write_index = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//i2c
int fd;

//json
float raw = 0;


int setup_i2c(){
	// open i2c
	fd = open("/dev/i2c-1", O_RDWR); 
	if (fd < 0) { 
		perror("Failed to open I2C bus"); 
		return 1; 
	}
	
	// connect to ADC
	if (ioctl(fd, I2C_SLAVE, ADC_ADDR) < 0) { 
		perror("Failed to connect to ADC"); 
		return 1; 
	}

	return fd;
}

void* mic_thread(void* arg) {
	while (1) {
        raw = mic_read(fd);

        buffer[write_index] = raw;
        write_index = (write_index + 1) % BUFFER_SIZE;
		usleep(1000);
    }

    return NULL;
}

void* sender_thread(void* arg) {
    while (1) {
        printf("%f\n", raw);
		fflush(stdout);

		usleep(1000);
    }

    return NULL;
}

int main() {
    pthread_t mic_t, send_t;
	fd = setup_i2c();

    pthread_mutex_init(&lock, NULL);

    pthread_create(&mic_t, NULL, mic_thread, NULL);
    pthread_create(&send_t, NULL, sender_thread, NULL);

    pthread_join(mic_t, NULL);
    pthread_join(send_t, NULL);

    return 0;
}


/*
	setvbuf(stdout, NULL, _IONBF, 0);
	float samples_f[WINDOW_SIZE];
	fftwf_complex *fft_out = fftwf_malloc(sizeof(fftwf_complex) * (WINDOW_SIZE/2 + 1));
	fftwf_plan fft_plan;

	fft_plan = fftwf_plan_dft_r2c_1d(WINDOW_SIZE, 
							samples_f, 
							fft_out, 
							FFTW_MEASURE);
	
	float samples[WINDOW_SIZE]; 
	int index = 0; 
	int value = 0;
		
	while (1) { 
		// raw sampled data 
		raw = mic_read(ADC_ADDR);
		
		//int raw = value;
		printf("%d\n", value);
		fflush(stdout);

		index++;

		float centered_data;

		//FFT		
		if (index >= WINDOW_SIZE) { 	

    		// 1. Compute mean (DC offset)
    		float mean = compute_mean(samples, WINDOW_SIZE);
			
			// 2. Convert to float + Hann window
			for (int i = 0; i < WINDOW_SIZE; i++) {
        		float centered = samples[i] - mean;
				
				float w = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (WINDOW_SIZE - 1)));
        		samples_f[i] = centered * w;
				centered_data = samples_f[i];
    		}

			// 3. FFT
		    fftwf_execute(fft_plan);

			// 4. Magnitude Spectrum (peak detection) 
			//fit guitar freq spectrum
			int k_min = (int)(70.0f * WINDOW_SIZE / SAMPLE_RATE);
			int k_max = (int)(350.0f * WINDOW_SIZE / SAMPLE_RATE);

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
			//printf("%f = (%d + %f) * %f / %d\n", freq, max_bin, delta, SAMPLE_RATE, WINDOW_SIZE);
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
	
				    if(mag > 0.15f * max_mag) {
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

			//printf("Centered: %f Freq: %f\n", centered_data, freq);
			
			const char *names[] = {
    			"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
				};

			
			printf("\rFreq: %6.1f Hz | Note: %s | %+6.1f cents    ",
    	   		freq,
       			names[midi % 12],
       			cents);

			fflush(stdout);
			
			index = 0;
		}
	
	} 

	fftwf_destroy_plan(fft_plan);

	return 0; 
}
*/