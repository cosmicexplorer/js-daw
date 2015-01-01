#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <iostream>
#include <exception>

#include "pcm.h"

namespace pcm {

std::mutex Thread::doubleCheckedLocker;
std::unique_ptr<Thread> Thread::singleton;

Osc::Osc(size_t frequency)
   : freq(frequency),
     frameIndex(0),
     period(SAMPLE_RATE / freq) {
}

Osc & Osc::doubleFreq() {
  freq *= 2;
  period = SAMPLE_RATE / freq;
  return *this;
}

Osc & Osc::halveFreq() {
  freq /= 2;
  period = SAMPLE_RATE / freq;
  return *this;
}

SquareOsc::SquareOsc(size_t frequency) : Osc(frequency) {
}

int16_t SquareOsc::getOutputAndAdvance() {
  size_t periodIndex = frameIndex % period;
  ++frameIndex;
  if (periodIndex < (period / 2)) {
    return -5e3;
  } else {
    return 5e3;
  }
}

Osc * makeOsc(Waveform wave, size_t frequency) {
  if (wave == square) {
    return new SquareOsc(frequency);
  } else {
    throw std::invalid_argument("Unrecognized waveform.");
  }
}

Note::Note(Osc * o, double seconds)
   : osc(o),
     length(static_cast<unsigned long long>(seconds * SAMPLE_RATE)),
     finished(false),
     frameIndex(0) {
}

int16_t Note::getOutputAndAdvance() {
  ++frameIndex;
  if (frameIndex >= length) {
    finished = true;
  }
  return osc->getOutputAndAdvance();
}

Thread & Thread::getInstance() {
  if (!singleton) {
    doubleCheckedLocker.lock();
    if (!singleton) {
      singleton.reset(new Thread());
    }
    doubleCheckedLocker.unlock();
  }
  return *singleton;
}

Thread::~Thread() {
  // TODO: this isn't freeing mem
  free(ipcMsg); // since using getline
}

Thread::Thread()
   : ipcMsgSize(0),
     ipcMsg(nullptr),
     stdinMsgs(0),
     stereoFrame(new int16_t[FRAME_SIZE]),
     notes(),
     cancelStream(false),
     noteAccessMutex(),
     thread() {
}

// notifies that there is another line available on stdin
void Thread::catchSignal(int) {
  if (singleton) {
    __sync_fetch_and_add(&singleton->stdinMsgs, 1);
  }
}

void Thread::beginStream() {
  // sets stdin unbuffered for immediate input
  setvbuf(stdin, nullptr, _IONBF, 0);
  setvbuf(stdout, nullptr, _IONBF, 0); // same with stdout
  signal(SIGUSR1, Thread::catchSignal);
  std::cerr << "begin pcm" << std::endl;
  pthread_create(&thread, nullptr, stream, nullptr);
}

void Thread::parseInput(void) {
  __ssize_t chars_read = 0;
  chars_read = getline(&ipcMsg, &ipcMsgSize, stdin);
  fprintf(stderr, "%.*s", (int) (chars_read - 1), ipcMsg);
  if ('s' == ipcMsg[0]) {
    for (auto & note : notes) {
      note.osc->doubleFreq();
    }
  } else if ('e' == ipcMsg[0]) {
    for (auto & note : notes) {
      note.osc->halveFreq();
    }
  } else if ('f' == ipcMsg[0]) {
    pushNoteToStream(pcm::Note(pcm::makeOsc(pcm::square, 400), .2));
  } else if ('q' == ipcMsg[0]) {
    cancelStream = true;
  }
  __sync_fetch_and_sub(&stdinMsgs, 1);
}

void Thread::setFrameValues(void) {
  int16_t cur_wave_val = 0;
  noteAccessMutex.lock();
  for (auto & note : notes) {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wconversion"
    cur_wave_val += note.getOutputAndAdvance();
#pragma GCC diagnostic pop
#endif
  }
  noteAccessMutex.unlock();
  for (size_t j = 0; j < NUM_CHANNELS; ++j) {
    stereoFrame[j] = cur_wave_val;
  }
}

void Thread::removeFinishedNotes() {
  notes.remove_if([](Note & n) { return n.finished; });
}

void * Thread::stream(void * arg __attribute__((unused))) {
  Thread & t(Thread::getInstance());
  while (true) {
    if (t.cancelStream) {
      pthread_exit(nullptr);
    }
    if (t.stdinMsgs > 0) {
      t.parseInput();
    }
    t.setFrameValues();
    fwrite(t.stereoFrame.get(), FRAME_SIZE, 1, stdout);
    t.removeFinishedNotes();
  }
}

void Thread::pushNoteToStream(Note n) {
  noteAccessMutex.lock();
  notes.push_front(n);
  noteAccessMutex.unlock();
}

void Thread::stopStream(void) {
  cancelStream = true;
  pthread_join(thread, nullptr);
}

void Thread::continueStreamUntilStopped(void) {
  pthread_join(thread, nullptr);
}

void beginStream() {
  Thread::getInstance().beginStream();
}

void pushNoteToStream(Note n) {
  Thread::getInstance().pushNoteToStream(n);
}

void stopStream() {
  Thread::getInstance().stopStream();
}

void continueStreamUntilStopped() {
  Thread::getInstance().continueStreamUntilStopped();
}
}
