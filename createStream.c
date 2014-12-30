#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>

#define NUM_CHANNELS 2
#define BIT_DEPTH 16 // keep this constant
#define SAMPLE_RATE 44100
#define FRAME_SIZE (sizeof(int16_t) * NUM_CHANNELS)

#define INIT_IPC_MSG_SIZE 4

size_t * cur_ipc_msg_size;

volatile bool stdin_avail = false;

void catch_sigusr1(int sig __attribute__((unused))) {
  stdin_avail = true;
}

int main() {
  cur_ipc_msg_size = malloc(sizeof(size_t));
  *cur_ipc_msg_size = INIT_IPC_MSG_SIZE;
  signal(SIGUSR1, catch_sigusr1);
  int16_t * stereo_frame = malloc(FRAME_SIZE);
  size_t freq = 50;
  size_t period = SAMPLE_RATE / freq;
  size_t frame_index = 0;
  char * ipc_msg = malloc(sizeof(char) * (*cur_ipc_msg_size));
  while (true) {
    if (stdin_avail) {
      getline(&ipc_msg, cur_ipc_msg_size, stdin);
      fprintf(stderr, "%.*s", (int) *cur_ipc_msg_size, ipc_msg);
      stdin_avail = false; // VERY IMPORTANT
      if ('f' == ipc_msg[0]) {
        freq = 2 * freq;
        period = SAMPLE_RATE / freq;
      } else if ('q' == ipc_msg[0]) {
        freq = freq / 2;
        period = SAMPLE_RATE / freq;
      }
    }
    int16_t cur_wave_val;
    size_t period_index = frame_index % period;
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
