#define _GNU_SOURCE // for getline: https://stackoverflow.com/questions/8480929/
#include "pcm_stream.h"
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>

pcm_thread_data_t * make_pcm_thread_data(void) {
  pcm_thread_data_t * ptdt = malloc(sizeof(pcm_thread_data_t));
  ptdt->ipc_msg_size = 0;
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
void pcm_catch_sigusr1(int sig __attribute__((unused))) {
  if (NULL != pcm_glob) { // will never happen, but here for safety
    __sync_fetch_and_add(&pcm_glob->stdin_msgs, 1);
  }
}

void * pcm_stream(void * arg __attribute__((unused))) {
  size_t freq = 50;
  size_t period = SAMPLE_RATE / freq;
  int16_t cur_wave_val;
  size_t period_index;
  while (true) {
    if (pcm_glob->stdin_msgs > 0) {
      getline(&pcm_glob->ipc_msg, &pcm_glob->ipc_msg_size, stdin);
      fprintf(stderr, "%.*s", (int) pcm_glob->ipc_msg_size, pcm_glob->ipc_msg);
      if ('s' == pcm_glob->ipc_msg[0]) {
        freq = 2 * freq;
        period = SAMPLE_RATE / freq;
      } else if ('e' == pcm_glob->ipc_msg[0]) {
        freq = freq / 2;
        period = SAMPLE_RATE / freq;
      } else if ('q' == pcm_glob->ipc_msg[0]) {
        pthread_exit(NULL);
      }
      __sync_fetch_and_sub(&pcm_glob->stdin_msgs, 1);
    }
    period_index = pcm_glob->frame_index % period;
    if (period_index < (period / 2)) {
      cur_wave_val = -20e3;
    } else {
      cur_wave_val = 20e3;
    }
    for (size_t j = 0; j < NUM_CHANNELS; ++j) {
      pcm_glob->stereo_frame[j] = cur_wave_val;
    }
    fwrite(pcm_glob->stereo_frame, FRAME_SIZE, 1, stdout);
    ++pcm_glob->frame_index;
  }
}

pthread_t pcm_begin_stream(pcm_thread_data_t * args) {
  pcm_glob = args;
  signal(SIGUSR1, pcm_catch_sigusr1);
  fprintf(stderr, "begin pcm\n");
  pthread_create(&args->thread, NULL, pcm_stream, NULL);
  return args->thread;
}
