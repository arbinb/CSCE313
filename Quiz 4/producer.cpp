#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include "KernelSemaphore.h"

using namespace std;

int main() {
    cout << "this program will create the semaphore, initialize it to 0, then produce some data and finally V() the semaphore" << endl;
    KernelSemaphore ks("/my_kernel_sema", 0);
    sleep(rand() % 10);  //sleep a random amount of seconds
    ks.V();
}
