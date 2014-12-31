#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>

#define NUM_CHANNELS 2
#define BIT_DEPTH 16 // keep this constant since we're using int16_t for now
#define SAMPLE_RATE 44100
#define FRAME_SIZE (sizeof(int16_t) * NUM_CHANNELS)

#define INIT_IPC_MSG_SIZE 4

typedef struct {
  size_t ipc_msg_size;
  char * ipc_msg;
  volatile int stdin_msgs;
  int16_t * stereo_frame;
  size_t frame_index;
  pthread_t * thread;
} pcm_thread_data_t;

pcm_thread_data_t * pcm_global_thread_data = NULL;

pcm_thread_data_t * make_pcm_thread_data() {
  pcm_thread_data_t * ptdt = malloc(sizeof(pcm_thread_data_t));
  ptdt->ipc_msg_size = INIT_IPC_MSG_SIZE;
  ptdt->ipc_msg = malloc(sizeof(char) * (ptdt->ipc_msg_size));
  ptdt->stdin_msgs = 0;
  ptdt->stereo_frame = malloc(FRAME_SIZE);
  ptdt->frame_index = 0;
  return ptdt;
}

void free_pcm_thread_data(pcm_thread_data_t * arg) {
  free(arg->ipc_msg);
  free(arg->stereo_frame);
}

// notifies that there is another line available on stdin
void catch_sigusr1(int sig __attribute__((unused))) {
  if (NULL != pcm_global_thread_data) {
    __sync_fetch_and_add(&pcm_global_thread_data->stdin_msgs, 1);
  }
}

void pcm_stream(pcm_thread_data_t * args) {
  pcm_global_thread_data = args;
  signal(SIGUSR1, catch_sigusr1);
  size_t freq = 50;
  size_t period = SAMPLE_RATE / freq;
  int16_t cur_wave_val;
  size_t period_index;
  while (true) {
    if (args->stdin_msgs > 0) {
      getline(&args->ipc_msg, &args->ipc_msg_size, stdin);
      fprintf(stderr, "%.*s", (int) args->ipc_msg_size, args->ipc_msg);
      if ('s' == args->ipc_msg[0]) {
        freq = 2 * freq;
        period = SAMPLE_RATE / freq;
      } else if ('e' == args->ipc_msg[0]) {
        freq = freq / 2;
        period = SAMPLE_RATE / freq;
      }
      __sync_fetch_and_sub(&args->stdin_msgs, 1);
    }
    period_index = args->frame_index % period;
    if (period_index < (period / 2)) {
      cur_wave_val = -20e3;
    } else {
      cur_wave_val = 20e3;
    }
    for (size_t j = 0; j < NUM_CHANNELS; ++j) {
      args->stereo_frame[j] = cur_wave_val;
    }
    fwrite(args->stereo_frame, FRAME_SIZE, 1, stdout);
    ++args->frame_index;
  }
}

int main() {
  pcm_thread_data_t * pthread_args = make_pcm_thread_data();
  pcm_stream(pthread_args);
  free_pcm_thread_data(pthread_args);
}
