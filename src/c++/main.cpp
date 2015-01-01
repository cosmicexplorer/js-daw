#include "pcm.h"
#include <iostream>

int main() {
  pcm::beginStream();
  pcm::pushFrontSynth(pcm::Synth(pcm::SQUARE, 50));
  pcm::pushFrontSynth(pcm::Synth(pcm::SQUARE, 200));
  pcm::continueStreamUntilStopped(); // exits when thread exits
  std::cerr << "WOAH" << std::endl;
}
