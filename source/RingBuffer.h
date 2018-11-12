#ifndef RING_BUFFER
#define RING_BUFFER

#include <atomic>
#include <cstdlib>
#include <cstring>
// ring buffer to hold audio samples
class RingBuffer
{
public:
    RingBuffer(int inBlockSize, int inNumBlocks)
    : mBuffer(nullptr), mBlockSize(inBlockSize),
      mNumBlocksAvailable(inNumBlocks), mReadPos(0),
      mWritePos(0), mNumBlocksFull(inNumBlocks)
    {
        // get a buffer of floats that is the number of samples * number of frames
        size_t alloc_size = mBlockSize * mNumBlocksAvailable * sizeof(float);
        mBuffer = (float*)malloc(alloc_size);

        // initialize floats to 0
        memset(mBuffer, 0, alloc_size);
    }

    ~RingBuffer() { free(mBuffer); }

    // atomic writing 
    // check if there are open audio frames to write to
    bool full() const { return mNumBlocksFull.load() == mNumBlocksAvailable; }

    // get the next free audio frame for writing
    float* writePtr() { return mBuffer + (mWritePos * mBlockSize); }

    // when write is finished, increment to the next write index
    // and atomically increase the marker for whether reads and writes are possible
    void finishWrite()
    {
        mWritePos = (mWritePos + 1) % mNumBlocksAvailable;
        mNumBlocksFull.fetch_add(1);
    }


    // atomic reading
    // be sure there are actually frames to read from
    bool empty() const { return mNumBlocksFull.load() == 0; }

    // get the address of the first float to be read from
    float const* readPtr() const { return mBuffer + (mReadPos * mBlockSize); }

    // decrement number of available frames
    void finishRead()
    {
        mReadPos = (mReadPos + 1) % mNumBlocksAvailable;
        mNumBlocksFull.fetch_sub(1);
    }

    // accessors
    int32_t blockSize() const { return mBlockSize; };

private:
    // array of the samples
    float* mBuffer;

    // number of samples in a frame
    int mBlockSize;

    // maximum number of frames to be held in this buffer
    int mNumBlocksAvailable;

    // position in the audio buffer to read from next
    int mReadPos;

    // position in the audio buffer to write to next
    int mWritePos;

    // current number of frames with unread audio data
    std::atomic<int> mNumBlocksFull;
};
#endif
