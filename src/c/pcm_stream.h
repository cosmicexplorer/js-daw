#ifndef ___PCM_STREAM_H___
#define ___PCM_STREAM_H___

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#define NUM_CHANNELS 2
#define BIT_DEPTH 16 // keep this constant since we're using int16_t for now
#define SAMPLE_RATE 44100
#define FRAME_SIZE (sizeof(int16_t) * NUM_CHANNELS)

typedef struct {
  size_t ipc_msg_size;
  char * ipc_msg;
  volatile int stdin_msgs;
  int16_t * stereo_frame;
  size_t frame_index;
  size_t freq;
  size_t period;
  pthread_t thread;
} pcm_thread_data_t;

// defined to be null at start
static pcm_thread_data_t * pcm_glob;

pcm_thread_data_t * make_pcm_thread_data(void);

void free_pcm_thread_data(pcm_thread_data_t * arg);

void pcm_catch_sigusr1(int sig);

pthread_t pcm_begin_stream(pcm_thread_data_t * args);

void pcm_parse_input(void);

void * pcm_stream(void * arg);

#endif
