#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
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

int main() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedData));

    SharedData* data = (SharedData*)mmap(nullptr, sizeof(SharedData),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&data->mutex, &mattr);

    data->writeIndex = 0;
    memset(data->messages, 0, sizeof(data->messages));

    cout << "Shared memory initialized.\n";

    munmap(data, sizeof(SharedData));
    close(fd);
    return 0;
}
