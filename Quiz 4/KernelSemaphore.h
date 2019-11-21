#ifndef Kernel
#define Kernel
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <string>

class KernelSemaphore {
   private:
    std::string name;
    sem_t* sema;

   public:
    KernelSemaphore(std::string _name, int _init_value) {
        this->name = _name;
        sema = sem_open(name.c_str(), O_CREAT, 0600, _init_value);
    }
    void P() {
        sem_wait(sema);
    }
    void V() {
        sem_post(sema);
    }
    ~KernelSemaphore() {
        sem_close(sema);
    }
};

#endif