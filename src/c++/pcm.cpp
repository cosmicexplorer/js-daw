#include <stdio.h>
#include <signal.h>
#include <pthread.h>

#include "pcm.h"

namespace pcm {

std::mutex Thread::doubleCheckedLocker;
std::unique_ptr<Thread> Thread::singleton;

Synth::Synth(Waveform waveForm, size_t frequency)
   : wave(waveForm),
     freq(frequency),
     frameIndex(0),
     period(SAMPLE_RATE / freq) {
}

Synth & Synth::doubleFreq() {
  freq *= 2;
  period = SAMPLE_RATE / freq;
  return *this;
}

Synth & Synth::halveFreq() {
  freq /= 2;
  period = SAMPLE_RATE / freq;
  return *this;
}

int16_t Synth::getOutput() {
  size_t periodIndex = frameIndex % period;
  if (periodIndex < (period / 2)) {
    return -5e3;
  } else {
    return 5e3;
  }
}

Thread & Thread::getInstance() {
  if (!singleton) {
    doubleCheckedLocker.lock();
    if (!singleton){
      singleton.reset(new Thread());
    }
    doubleCheckedLocker.unlock();
  }
  return *singleton;
}

Thread::~Thread() {
}

Thread::Thread()
   : ipcMsgSize(0),
     ipcMsg(nullptr),
     stdinMsgs(0),
     stereoFrame(new int16_t[FRAME_SIZE]),
     synths(),
     cancelStream(false),
     synthAccessMutex(),
     thread() {
}

// notifies that there is another line available on stdin
void Thread::catchSignal(int) {
  if (singleton) {
    __sync_fetch_and_add(&singleton->stdinMsgs, 1);
  }
}

void Thread::beginStream() {
  setvbuf(stdin, nullptr, _IONBF,
          0); // sets stdin unbuffered for immediate input
  setvbuf(stdout, nullptr, _IONBF, 0); // same with stdout
  signal(SIGUSR1, Thread::catchSignal);
  fprintf(stderr, "begin pcm\n");
  pthread_create(&thread, nullptr, stream, nullptr);
}

void Thread::parseInput(void) {
  __ssize_t chars_read = 0;
  chars_read = getline(&ipcMsg, &ipcMsgSize, stdin);
  fprintf(stderr, "%.*s", (int) (chars_read - 1), ipcMsg);
  if ('s' == ipcMsg[0]) {
    for (auto & synth : synths) {
      synth.doubleFreq();
    }
  } else if ('e' == ipcMsg[0]) {
    for (auto & synth : synths) {
      synth.halveFreq();
    }
  } else if ('q' == ipcMsg[0]) {
    cancelStream = true;
  }
  __sync_fetch_and_sub(&stdinMsgs, 1);
}

void Thread::setFrameValues(void) {
  int16_t cur_wave_val = 0;
  synthAccessMutex.lock();
  for (auto & synth : synths) {
    cur_wave_val += synth.getOutput();
    ++synth.frameIndex;
  }
  synthAccessMutex.unlock();
  for (size_t j = 0; j < NUM_CHANNELS; ++j) {
    stereoFrame[j] = cur_wave_val;
  }
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
  }
}

void Thread::pushFrontSynth(Synth s) {
  synthAccessMutex.lock();
  synths.push_front(s);
  synthAccessMutex.unlock();
}

void Thread::replaceFrontSynth(Synth s) {
  synthAccessMutex.lock();
  synths.front() = s;
  synthAccessMutex.unlock();
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

void pushFrontSynth(Synth s) {
  Thread::getInstance().pushFrontSynth(s);
}

void replaceFrontSynth(Synth s) {
  Thread::getInstance().replaceFrontSynth(s);
}

void stopStream() {
  Thread::getInstance().stopStream();
}

void continueStreamUntilStopped() {
  Thread::getInstance().continueStreamUntilStopped();
}
}
