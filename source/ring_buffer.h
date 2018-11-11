#ifndef RING_BUFFER
#define RING_BUFFER

#include <atomic>
#include <cstdlib>
#include <cstring>
// ring buffer to hold audio samples
struct ring_buffer
{

    ring_buffer(int samples, int frames)
    : buffer_(nullptr), samples_(samples),
      max_frames_(frames), read_(0),
      write_(0), num_frames_(frames)
    {
        // get a buffer of floats that is the number of samples * number of frames
        size_t alloc_size = samples_ * max_frames_ * sizeof(float);
        buffer_ = (float*)malloc(alloc_size);

        // initialize floats to 0
        memset(buffer_, 0, alloc_size);
    }

    ~ring_buffer() { free(buffer_); }

    // atomic writing 
    // check if there are open audio frames to write to
    bool can_write() { return num_frames_.load() != max_frames_; }

    // get the next free audio frame for writing
    float* write_buffer() { return buffer_ + (write_ * samples_); }

    // when write is finished, increment to the next write index
    // and atomically increase the marker for whether reads and writes are possible
    void finish_write()
    {
        write_ = (write_ + 1) % max_frames_;
        num_frames_.fetch_add(1);
    }


    // atomic reading

    // be sure there are actually frames to read from
    bool can_read() { return num_frames_.load() != 0; }

    // get the address of the first float to be read from
    float* read_buffer() { return buffer_ + (read_ * samples_); }

    // decrement number of available frames
    void finish_read() 
    {
        read_ = (read_ + 1) % max_frames_;
        num_frames_.fetch_sub(1);
    }

    // array of the samples
    float* buffer_;

    // number of samples in a frame
    int samples_;

    // maximum number of frames to be held in this buffer
    int max_frames_;

    // position in the audio buffer to read from next
    int read_;

    // position in the audio buffer to write to next
    int write_;

    // current number of frames with unread audio data
    std::atomic<int> num_frames_;


};
#endif
