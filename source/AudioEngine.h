#ifndef SOUND_ENGINE
#define SOUND_ENGINE

#include "AudioDevice.h"
#include "MessageQueue.h"
#include "RingBuffer.h"
#include "Semaphore.h"
#include <math.h>

class AudioEngine
{
public:
    AudioEngine( int samplerate, int channels, int frames, int refills) 
    : mAudioBuffer(frames * channels, refills),
    mMessageQueue(64), mFrameMessages(0), mStopFlag(false), 
    mUpdateThread(&AudioEngine::process_messages, this),
    mAudioDevice(samplerate, channels, frames, callback, this) 
    {}

    ~AudioEngine()
    {
        mStopFlag.store(true);
        mSem.signal();
        mUpdateThread.join();
    }

    void update()
    {
        message frame_msg{message_t::frame};
        mMessageQueue.push(frame_msg);
        mFrameMessages.fetch_add(1);
    }

    void playSound(int sound_id)
    {
        message m;
        m.type = message_t::play_sound;
        m.params.play_params.sound_id = sound_id;
        mMessageQueue.push(m);
    }

    void stopSound(int sound_id)
    {
        message m;
        m.type = message_t::stop_sound;
        m.params.stop_params.sound_id = sound_id;
        mMessageQueue.push(m);
    }

    void quit()
    {
        message m;
        m.type = message_t::quit;
        mMessageQueue.push(m);
    }

private:

    static void callback(float* buffer, int channels, int frames, void* cookie)
    {
        AudioEngine* engine = (AudioEngine*)cookie;
        engine->write_data_to_device(buffer);
    }

    void write_data_to_device(float* buffer)
    {
        if(!mAudioBuffer.empty())
        {
            const float* read_buffer = mAudioBuffer.readPtr();
            int bytes = sizeof(float) * mAudioBuffer.blockSize();
            memcpy(buffer, read_buffer, bytes);
            mAudioBuffer.finishRead();
        }
        mSem.signal();
    }

    void process_messages()
    {
        for(;;)
        {
            mSem.wait();
            if(mStopFlag.load())
            {
                break;
            }
            while(!mAudioBuffer.full())
            {
                if(mStopFlag.load())
                {
                    break;
                }
                for(;;)
                {
                    int frame_messages = mFrameMessages.load();
                    if(frame_messages == 0)
                    {
                        break;
                    }
                    message msg;
                    if(!mMessageQueue.pop(msg))
                    {
                        break;
                    }
                    if(msg.type == message_t::frame)
                    {
                        mFrameMessages.fetch_sub(1);
                        break;
                    }
                    switch(msg.type)
                    {
                        case message_t::play_sound:
                        {
                            printf("playing sound\n");
                            int sound_id = msg.params.play_params.sound_id;
                            float* write = mAudioBuffer.writePtr();
                            // write a block of sine
                            const double PI = 3.141592653589793;
                            for(int i = 0; i < mAudioBuffer.blockSize(); i++)
                            {
                                write[i] = 0.1f * sinf(2.0 * PI * 500.0 * i/44100.0);
                            }
                            break;
                        }
                        case message_t::stop_sound:
                        {
                            int sound_id = msg.params.stop_params.sound_id;
                            printf("stop %d\n", sound_id);
                            break;
                        }
                        case message_t::quit:
                        {
                            printf("quitting\n");
                            mStopFlag.store(true);
                            mAudioDevice.quit();
                            break;
                        }
                        default: {break;}
                    }
                    mAudioBuffer.finishWrite();
                }
            }
        }
    }

    // members
    LinuxSemaphore mSem;
    RingBuffer mAudioBuffer;
    MessageQueue mMessageQueue;
    AudioDevice mAudioDevice;
    std::atomic<int> mFrameMessages;
    std::atomic<bool> mStopFlag;
    std::thread mUpdateThread;
    
};

#endif