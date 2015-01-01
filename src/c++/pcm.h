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

typedef enum { sin, square, triangle, morphing } Waveform;

// oscillator
struct Osc {
 public:
  Osc(size_t frequency);
  Osc() = delete;
  Osc & doubleFreq();
  Osc & halveFreq();
  virtual int16_t getOutputAndAdvance() = 0;

  size_t freq;
  size_t frameIndex;
  size_t period;
};

struct SquareOsc : public Osc {
 public:
  SquareOsc(size_t frequency);
  SquareOsc() = delete;
  virtual int16_t getOutputAndAdvance();
};

Osc * makeOsc(Waveform wave, size_t frequency);

struct Note {
 public:
  Note(Osc * o, double seconds);
  Note() = delete;

  std::shared_ptr<Osc> osc;
  // TODO: make these bignums
  unsigned long long length; // in frames
  bool finished;
  unsigned long long frameIndex;
  int16_t getOutputAndAdvance();
};

class Thread {
 public:
  static Thread & getInstance();
  ~Thread();
  void beginStream();
  void pushNoteToStream(Note n);
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
  void removeFinishedNotes();
  static void * stream(void *) __attribute__((noreturn));

  size_t ipcMsgSize;
  char * ipcMsg;
  volatile int stdinMsgs;
  std::unique_ptr<int16_t[]> stereoFrame;
  std::list<Note> notes;
  volatile bool cancelStream;
  std::mutex noteAccessMutex;
  pthread_t thread;
};

void beginStream();

void pushNoteToStream(Note n);

void stopStream();

void continueStreamUntilStopped();
}

#endif
