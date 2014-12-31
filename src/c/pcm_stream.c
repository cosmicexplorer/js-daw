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

pthread_t pcm_begin_stream(pcm_thread_data_t * args) {
  pcm_glob = args;
  setvbuf(stdin, NULL, _IONBF, 0);  // sets stdin unbuffered for immediate input
  setvbuf(stdout, NULL, _IONBF, 0); // same with stdout
  signal(SIGUSR1, pcm_catch_sigusr1);
  fprintf(stderr, "begin pcm\n");
  pthread_create(&args->thread, NULL, pcm_stream, NULL);
  return args->thread;
}

void pcm_parse_input(void) {
  size_t chars_read = 0;
  chars_read = getline(&pcm_glob->ipc_msg, &pcm_glob->ipc_msg_size, stdin);
  fprintf(stderr, "%.*s", (int) (chars_read - 1), pcm_glob->ipc_msg);
  if ('s' == pcm_glob->ipc_msg[0]) {
    pcm_glob->freq = 2 * pcm_glob->freq;
    pcm_glob->period = SAMPLE_RATE / pcm_glob->freq;
  } else if ('e' == pcm_glob->ipc_msg[0]) {
    pcm_glob->freq = pcm_glob->freq / 2;
    pcm_glob->period = SAMPLE_RATE / pcm_glob->freq;
  } else if ('q' == pcm_glob->ipc_msg[0]) {
    pthread_exit(NULL);
  }
  __sync_fetch_and_sub(&pcm_glob->stdin_msgs, 1);
}

void * pcm_stream(void * arg __attribute__((unused))) {
  pcm_glob->freq = 50;
  pcm_glob->period = SAMPLE_RATE / pcm_glob->freq;
  int16_t cur_wave_val;
  size_t period_index;
  while (true) {
    if (pcm_glob->stdin_msgs > 0) {
      pcm_parse_input();
    }
    period_index = pcm_glob->frame_index % pcm_glob->period;
    if (period_index < (pcm_glob->period / 2)) {
      cur_wave_val = -5e3;
    } else {
      cur_wave_val = 5e3;
    }
    for (size_t j = 0; j < NUM_CHANNELS; ++j) {
      pcm_glob->stereo_frame[j] = cur_wave_val;
    }
    fwrite(pcm_glob->stereo_frame, FRAME_SIZE, 1, stdout);
    ++pcm_glob->frame_index;
  }
}
