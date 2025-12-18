#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

const char* SHM_NAME = "/chat_shm";
const int MAX_MSG = 50;
const int MSG_SIZE = 256;

struct Message {
    char text[MSG_SIZE];
};

struct SharedData {
    pthread_mutex_t mutex;
    int writeIndex;
    Message messages[MAX_MSG];
};

SharedData* data = nullptr;
int lastRead = 0;

void attachSharedMemory() {
    if (data) return;

    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    data = (SharedData*)mmap(nullptr, sizeof(SharedData),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
}

void writeMessageToSharedMemory(const string& msg) {
    attachSharedMemory();

    pthread_mutex_lock(&data->mutex);

    int index = data->writeIndex % MAX_MSG;
    strncpy(data->messages[index].text, msg.c_str(), MSG_SIZE);
    data->messages[index].text[MSG_SIZE - 1] = '\0';
    data->writeIndex++;

    pthread_mutex_unlock(&data->mutex);
}

vector<string> readMessagesFromSharedMemory() {
    attachSharedMemory();
    vector<string> newMessages;

    pthread_mutex_lock(&data->mutex);

    while (lastRead < data->writeIndex) {
        int index = lastRead % MAX_MSG;
        newMessages.push_back(data->messages[index].text);
        lastRead++;
    }

    pthread_mutex_unlock(&data->mutex);
    return newMessages;
}
