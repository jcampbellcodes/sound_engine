#ifndef SOUND_ENGINE
#define SOUND_ENGINE

#include "audio_device.h"
#include "message_queue.h"
#include "ring_buffer.h"
#include "update_event.h"
#include <math.h>

const double PI = 3.141592653589793;

class sound_engine
{

public:
    sound_engine(
        int samplerate, int channels, int frames, int refills
    ) : buffers_(frames * channels, refills),
    queue_(64), frame_messages_(0), stop_(false), 
    update_thread_(&sound_engine::process_messages, this),
    device_(samplerate, channels, frames, callback, this) {}

    ~sound_engine()
    {
        stop_.store(true);
        event_.signal();
        update_thread_.join();
    }

    void update()
    {
        message frame_msg{message_t::frame};
        queue_.push(frame_msg);
        frame_messages_.fetch_add(1);
    }

    void play_sound(int sound_id)
    {
        message m;
        m.type = message_t::play_sound;
        m.params.play_params.sound_id = sound_id;
        queue_.push(m);
    }

    void stop_sound(int sound_id)
    {
        message m;
        m.type = message_t::stop_sound;
        m.params.stop_params.sound_id = sound_id;
        queue_.push(m);
    }

    void quit()
    {
        message m;
        m.type = message_t::quit;
        queue_.push(m);
    }

private:

    static void callback(float* buffer, int channels, int frames, void* cookie)
    {
        sound_engine* engine = (sound_engine*)cookie;
        engine->write_data_to_device(buffer);
    }

    void write_data_to_device(float* buffer)
    {
        if(buffers_.can_read())
        {
            float* read_buffer = buffers_.read_buffer();
            int bytes = sizeof(float) * buffers_.samples_;
            memcpy(buffer, read_buffer, bytes);
            buffers_.finish_read();
        }
        event_.signal();
    }

    void process_messages()
    {
        for(;;)
        {
            printf("processing messages\n");
            event_.wait();
            if(stop_.load())
            {
                printf("STOP!\n");
                break;
            }
            while(buffers_.can_write())
            {
                if(stop_.load())
                {
                    break;
                }
                for(;;)
                {
                    int frame_messages = frame_messages_.load();
                    if(frame_messages == 0)
                    {
                        //printf("framemessages is 0!\n");
                        break;
                    }
                    message msg;
                    if(!queue_.pop(msg))
                    {
                        //printf("no more messages to read\n");
                        break;
                    }
                    if(msg.type == message_t::frame)
                    {
                        frame_messages_.fetch_sub(1);
                        //printf("got a frame message!\n");
                        break;
                    }
                    switch(msg.type)
                    {
                        case message_t::play_sound:
                        {
                            printf("playing sound\n");
                            int sound_id = msg.params.play_params.sound_id;
                            float* write = buffers_.write_buffer();
                            // initialize floats to 0
                            for(int i = 0; i < buffers_.samples_ * buffers_.max_frames_; i++)
                            {
                                write[i] = sinf(2.0 * PI * 1000.0 * i/44100.0);
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
                            stop_.store(true);
                            device_.stop_.store(true);
                            break;
                        }
                        default: {break;}
                    }
                    buffers_.finish_write();
                }
            }
        }
    }

    update_event event_;
    ring_buffer buffers_;
    message_queue queue_;
    std::atomic<int> frame_messages_;
    std::atomic<bool> stop_;
    std::thread update_thread_;
    audio_device device_;
};

#endif