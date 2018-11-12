#ifndef MESSAGE_BUFFER
#define MESSAGE_BUFFER

#include <atomic>
#include <cstdlib>

// put messages here
struct play_msg
{
    int sound_id;
};

struct stop_msg
{
    int sound_id;
};

enum class message_t { play_sound, stop_sound, frame, quit };

union message_params
{
    play_msg play_params;
    stop_msg stop_params;
};

struct message
{
    message_t type;
    message_params params;
};

class MessageQueue
{
public:
    MessageQueue(int max_messages) :
        messages_(nullptr), max_messages_(max_messages),
        num_messages_(0), head_(0), tail_(0)
    {
        size_t alloc_size = max_messages_ * sizeof(message);
        messages_ = (message *)malloc(alloc_size);
    }

    ~MessageQueue() { free(messages_); }

    bool push(const message& msg)
    {
        if(num_messages_.load() != max_messages_)
        {
            int new_head = (head_ + 1) % max_messages_;
            messages_[head_] = msg;
            head_ = (head_ + 1) % max_messages_;
            num_messages_.fetch_add(1);
            return true;
        }
        return false;
    } 

    bool pop(message& msg)
    {
        if(num_messages_.load() != 0)
        {
            msg = messages_[tail_];
            tail_ = (tail_ + 1) % max_messages_;
            num_messages_.fetch_sub(1);
            return true;
        }
        return false;
    }

private:
    message* messages_;
    int max_messages_;
    int head_;
    int tail_;
    std::atomic<int> num_messages_;
};

#endif
