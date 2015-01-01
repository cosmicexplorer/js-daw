#include <iostream>
#include "pcm.h"

int main() {
  pcm::beginStream();
  pcm::pushNoteToStream(pcm::Note(pcm::makeOsc(pcm::square, 50), 1.5));
  pcm::pushNoteToStream(pcm::Note(pcm::makeOsc(pcm::square, 200), 1.2));
  pcm::continueStreamUntilStopped(); // exits when thread exits
  std::cerr << "WOAH" << std::endl;
}
