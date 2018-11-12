#include <alsa/asoundlib.h>
#include <atomic>
#include <cstdlib>
#include <pthread.h>
#include <thread>

class AudioDevice
{
public:
    AudioDevice(int sample_rate, 
                int channels, 
                int frames, 
                void (*func)(float*,int,int,void*), 
                void* cookie) : mStopFlag(false)
    {
        printf("initializing audio device\n");
        mAudioBuffer = (float*)malloc(sizeof(float*) * channels * frames);
        snd_pcm_open(&mDeviceHandle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK | SND_PCM_ASYNC);
        snd_pcm_hw_params_t* hardware_params;
        snd_pcm_hw_params_alloca(&hardware_params);
        snd_pcm_hw_params_any(mDeviceHandle, hardware_params);
        snd_pcm_hw_params_set_access(mDeviceHandle, hardware_params, SND_PCM_ACCESS_RW_INTERLEAVED);


        snd_pcm_hw_params_set_format(mDeviceHandle, hardware_params, SND_PCM_FORMAT_FLOAT);
        snd_pcm_hw_params_set_rate(mDeviceHandle, hardware_params, sample_rate, 0);
        snd_pcm_hw_params_set_channels(mDeviceHandle, hardware_params, channels);
        snd_pcm_hw_params(mDeviceHandle, hardware_params);

        snd_pcm_sw_params_t* software_params;
        snd_pcm_sw_params_alloca(&software_params);
        snd_pcm_sw_params_current(mDeviceHandle, software_params);

        snd_pcm_sw_params_set_avail_min(mDeviceHandle, software_params, frames);

        snd_pcm_sw_params_set_start_threshold(mDeviceHandle, software_params, 0);

        snd_pcm_sw_params(mDeviceHandle, software_params);
        snd_pcm_prepare(mDeviceHandle);

        mAudioThread = std::thread(
            [this, func, cookie, channels, frames]()
            {
                while(!mStopFlag)
                {
                    snd_pcm_wait(mDeviceHandle, -1);
                    func((float*)mAudioBuffer, channels, frames, cookie);
                    snd_pcm_writei(mDeviceHandle, mAudioBuffer, frames);
                }
            }
        );

        sched_param sch;
        sch.__sched_priority = 20;
        pthread_setschedparam(mAudioThread.native_handle(), SCHED_FIFO, &sch);
    }

    ~AudioDevice()
    {
        printf("quitting audio device");
        mStopFlag = true;
        mAudioThread.join();
        snd_pcm_close(mDeviceHandle);
        free(mAudioBuffer);
    }

    void quit()
    {
        mStopFlag.store(true);
    };

private:
    float* mAudioBuffer;
    snd_pcm_t* mDeviceHandle;
    std::atomic<bool> mStopFlag;
    std::thread mAudioThread;

};
