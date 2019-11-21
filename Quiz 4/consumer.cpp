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
    KernelSemaphore ks("/my_kernel_sema", 0);
    ks.P();
    cout << "I can tell the producer is done" << endl;
}
