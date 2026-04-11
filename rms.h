#ifndef RMS_H
#define RMS_H

float compute_mean(float* buffer, int write_index, int window_size, int buffer_size);
float compute_rms(float *samples, int n);
void print_rms(float *samples, int window_size, int index);

#endif