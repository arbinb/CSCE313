#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include "Semaphore.h"

using namespace std;

Semaphore a(0);
Semaphore b(0);
Semaphore c(1);
Semaphore bmtx(1);

int bCount = 0;

void aFunction() {
    while (true) {
        c.P();
        cout << "A" << endl;
        a.V();
        a.V();
    }
}

void bFunction() {
    while (true) {
        a.P();
        bmtx.P();
        cout << "B" << endl;
        bCount++;
        bmtx.V();

        bmtx.P();
        if (bCount == 2) {
            bCount = 0;
            b.V();
        }
        bmtx.V();
    }
}

void cFunction() {
    while (true) {
        b.P();
        cout << "C" << endl;
        c.V();
    }
}

int main(int argc, char const *argv[]) {
    vector<thread> a_Thread;
    vector<thread> b_Thread;
    vector<thread> c_Thread;

    for (int i = 0; i < 20; i++)
        a_Thread.push_back(thread(aFunction));
    for (int i = 0; i < 20; i++)
        b_Thread.push_back(thread(bFunction));
    for (int i = 0; i < 20; i++)
        c_Thread.push_back(thread(cFunction));

    for (int i = 0; i < a_Thread.size(); i++)
        a_Thread[i].join();
    for (int i = 0; i < b_Thread.size(); i++)
        b_Thread[i].join();
    for (int i = 0; i < c_Thread.size(); i++)
        c_Thread[i].join();
    return 0;
}
