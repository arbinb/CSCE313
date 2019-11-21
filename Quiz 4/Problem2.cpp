#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <locale>
#include <vector>

using namespace std;

class KernelSemaphore {
    string name;

   public:
    KernelSemaphore(string _name, int _intit_value);
    void P();
    void V();
    ~KernelSemaphore();
};
//producer.cpp
int main() {
    cout << "This program will create the semaphore, initialize it to 0, ";
    cout << "then produce some data and finally V() the semaphore" << endl;
    KernelSemaphore ks("/my_kernel_sema", 0);
    sleep(rand() % 10);
    ks.V();
}
//consumer.cpp
int main() {
    KernelSemaphore ks("/my_kernal_sema", 0);
    ks.P();
    cout << "I can tell the producer is done " << endl;
}

// void signal_handler(int signo) {
//     cout << "Got SIGUSR1" << endl;
// }
// int main() {
//     //signal(SIGUSR1, signal_handler);  //comment out for b)
//     int pid = fork();
//     if (pid == 0) {  // chilld process
//         for (int i = 0; i < 5; i++) {
//             kill(getppid(), SIGUSR1);
//             sleep(1);
//         }

//     } else {  // parent process
//         wait(0);
//     }
// }
