#include "pcm_stream.h"
#include <stdio.h>

int main(int argc __attribute__((unused)),
         char ** argv __attribute__((unused))) {
  pcm_thread_data_t * pthread_args = make_pcm_thread_data();
  pthread_join(pcm_begin_stream(pthread_args), NULL);
  fprintf(stderr, "WOAH\n");
  free_pcm_thread_data(pthread_args);
}
