#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>

#define NUM_CHANNELS 2
#define BIT_DEPTH 16 // keep this constant since we're using int16_t for now
#define SAMPLE_RATE 44100
#define FRAME_SIZE (sizeof(int16_t) * NUM_CHANNELS)

#define INIT_IPC_MSG_SIZE 4

size_t * cur_ipc_msg_size;

volatile int stdin_msgs = 0;

// notifies that there is another line available on stdin
void catch_sigusr1(int sig __attribute__((unused))) {
  __sync_fetch_and_add(&stdin_msgs, 1);
}

int main() {
  signal(SIGUSR1, catch_sigusr1);
  cur_ipc_msg_size = malloc(sizeof(size_t));
  *cur_ipc_msg_size = INIT_IPC_MSG_SIZE;
  char * ipc_msg = malloc(sizeof(char) * (*cur_ipc_msg_size));
  int16_t * stereo_frame = malloc(FRAME_SIZE);
  size_t freq = 50;
  size_t period = SAMPLE_RATE / freq;
  size_t frame_index = 0;
  int16_t cur_wave_val;
  size_t period_index;
  while (true) {
    if (__sync_fetch_and_sub(&stdin_msgs, 1) > 0) {
      getline(&ipc_msg, cur_ipc_msg_size, stdin);
      fprintf(stderr, "%.*s", (int) *cur_ipc_msg_size, ipc_msg);
      if ('s' == ipc_msg[0]) {
        freq = 2 * freq;
        period = SAMPLE_RATE / freq;
      } else if ('q' == ipc_msg[0]) {
        freq = freq / 2;
        period = SAMPLE_RATE / freq;
      }
    } else {
      __sync_fetch_and_add(&stdin_msgs, 1);
    }
    period_index = frame_index % period;
    if (period_index < (period / 2)) {
      cur_wave_val = -20e3;
    } else {
      cur_wave_val = 20e3;
    }
    for (size_t j = 0; j < NUM_CHANNELS; ++j) {
      stereo_frame[j] = cur_wave_val;
    }
    fwrite(stereo_frame, FRAME_SIZE, 1, stdout);
    ++frame_index;
  }
  free(stereo_frame);
}
