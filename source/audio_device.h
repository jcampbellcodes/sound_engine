#include <alsa/asoundlib.h>
#include <atomic>
#include <cstdlib>
#include <pthread.h>
#include <thread>

struct audio_device
{
    audio_device(int sample_rate, 
                 int channels, 
                 int frames, 
                 void (*func)(float*,int,int,void*), 
                 void* cookie) : stop_(false)
    {
        printf("initializing audio device\n");
        buffer_ = (float*)malloc(sizeof(float*) * channels * frames);
        snd_pcm_open(&handle_, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK | SND_PCM_ASYNC);
        snd_pcm_hw_params_t* hardware_params;
        snd_pcm_hw_params_alloca(&hardware_params);
        snd_pcm_hw_params_any(handle_, hardware_params);
        snd_pcm_hw_params_set_access(handle_, hardware_params, SND_PCM_ACCESS_RW_INTERLEAVED);


        snd_pcm_hw_params_set_format(handle_, hardware_params, SND_PCM_FORMAT_FLOAT);
        snd_pcm_hw_params_set_rate(handle_, hardware_params, sample_rate, 0);
        snd_pcm_hw_params_set_channels(handle_, hardware_params, channels);
        snd_pcm_hw_params(handle_, hardware_params);

        snd_pcm_sw_params_t* software_params;
        snd_pcm_sw_params_alloca(&software_params);
        snd_pcm_sw_params_current(handle_, software_params);

        snd_pcm_sw_params_set_avail_min(handle_, software_params, frames);

        snd_pcm_sw_params_set_start_threshold(handle_, software_params, 0);

        snd_pcm_sw_params(handle_, software_params);
        snd_pcm_prepare(handle_);

        audio_thread_ = std::thread(
            [this, func, cookie, channels, frames]()
            {
                while(!stop_)
                {
                    //printf("processing audio");
                    snd_pcm_wait(handle_, -1);
                    func((float*)buffer_, channels, frames, cookie);
                    snd_pcm_writei(handle_, buffer_, frames);
                }
            }
        );

        sched_param sch;
        sch.__sched_priority = 20;
        pthread_setschedparam(audio_thread_.native_handle(), SCHED_FIFO, &sch);
    }

    ~audio_device()
    {
        printf("quitting audio device");
        stop_ = true;
        audio_thread_.join();
        snd_pcm_close(handle_);
        free(buffer_);
    }


    float* buffer_;
    snd_pcm_t* handle_;
    std::atomic<bool> stop_;
    std::thread audio_thread_;

};
