#ifndef _COMMON_H_
#define _COMMON_H_
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#include <sys/time.h>
#include <cassert>
#include <assert.h>

#include <cmath>
#include <numeric>
#include <algorithm>

#include <list>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

#define NUM_PERSONS 15  // number of person to collect data for
#define MAX_MESSAGE 256  // maximum buffer size for each message

// different types of messages
enum MESSAGE_TYPE {DATA_MSG, FILE_MSG, NEWCHANNEL_MSG, QUIT_MSG, UNKNOWN_MSG};

class clientArg_t { 
public:
    int num_dp;
    int patients;
    int workers;
    int buffer_size;
    string fileName;
    MESSAGE_TYPE msg_t;
    bool fileReq = false;
    clientArg_t(int n, int p, int w, int b) {
        num_dp = n, patients = p, workers = w, buffer_size = b;
    }    
};


// message requesting a data point
class datamsg{
public:
    MESSAGE_TYPE mtype;
    int person;
    double seconds;
    int ecgno;
    datamsg (int _person, double _seconds, int _eno){
        mtype = DATA_MSG, person = _person, seconds = _seconds, ecgno = _eno;
    }
};

// message requesting a file
class filemsg{
public:
    MESSAGE_TYPE mtype;
    __int64_t offset;
    int length;
	    
    filemsg (__int64_t _offset, int _length){
        mtype = FILE_MSG, offset = _offset, length = _length;
    }
};

void EXITONERROR(string msg);
vector<string> split (string line, char separator);
__int64_t get_file_size (string filename);
void handleArgs(int argc, char *argv[], clientArg_t* obj);

#endif