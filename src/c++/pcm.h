#ifndef ___PCM_STREAM_H___
#define ___PCM_STREAM_H___

#include <list>
#include <memory>
#include <mutex>

#define NUM_CHANNELS 2
#define BIT_DEPTH 16 // keep this constant since we're using int16_t for now
#define SAMPLE_RATE 44100
#define FRAME_SIZE (sizeof(int16_t) * NUM_CHANNELS)

namespace pcm {

typedef enum { SIN, SQUARE, TRIANGLE } Waveform;

struct Synth {
 public:
  Synth(Waveform waveForm, size_t frequency);
  Synth & doubleFreq();
  Synth & halveFreq();
  int16_t getOutput();

  Waveform wave;
  size_t freq;
  size_t frameIndex;
  size_t period;
};

class Thread {
 public:
  static Thread & getInstance();
  ~Thread();
  void beginStream();
  void pushFrontSynth(Synth s);
  Synth getFrontSynth();
  void replaceFrontSynth(Synth s);
  void stopStream();
  // because an external process can send the quit command via stdin
  void continueStreamUntilStopped();

 private:
  static std::mutex doubleCheckedLocker;

  static std::unique_ptr<Thread> singleton;
  Thread();
  static void catchSignal(int);
  void parseInput();
  void setFrameValues();
  static void * stream(void *);

  size_t ipcMsgSize;
  char * ipcMsg;
  volatile int stdinMsgs;
  std::unique_ptr<int16_t[]> stereoFrame;
  std::list<Synth> synths;
  volatile bool cancelStream;
  std::mutex synthAccessMutex;
  pthread_t thread;
};

void beginStream();

void pushFrontSynth(Synth s);

void replaceFrontSynth(Synth s);

void stopStream();

void continueStreamUntilStopped();
}

#endif
