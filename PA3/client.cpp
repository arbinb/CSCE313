#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "FIFOreqchannel.h"
using namespace std;

clientArg_t* GlobalStruct;

void *patient_function(int patientNum, BoundedBuffer* buffer, HistogramCollection* hist)
{
    /* What will the patient threads do? */
    for(int i = 0; i < GlobalStruct->num_dp; i++) {
        datamsg msg(patientNum, i*.004, 1);
        vector<char> temp((char*) &msg, ((char*) &msg + sizeof(msg)));
        buffer->push(temp);
    }
}

void *fileProducer(int rem, BoundedBuffer* buffer) {
    //push file to buffer
    int offset = 0;
    while(rem > 0) {
        int buf = MAX_MESSAGE;
        if(rem < MAX_MESSAGE) {
            buf = rem;
        }
        filemsg filem(offset, buf);
        vector<char> temp((char*) &filem, ((char*) &filem + sizeof(filem)));
        copy(GlobalStruct->fileName.begin(), GlobalStruct->fileName.end(), back_inserter(temp));
        temp.push_back('\0');
        offset += buf;
        rem -= buf;
        buffer->push(temp);
    }
    
}

void *worker_function(FIFORequestChannel* worker, BoundedBuffer* buffer, HistogramCollection* hist)
{
    /*
		Functionality of the worker threads	
    */
   while(1) {
       vector<char> temp = buffer->pop();
       char* req = reinterpret_cast<char*>(temp.data());
       worker->cwrite((char*) req, temp.size());
       MESSAGE_TYPE m = *(MESSAGE_TYPE *) req;
       if(m == QUIT_MSG) {
           delete worker;
           break;
       } else if (m == DATA_MSG) {
           datamsg* data_req = (datamsg*) req;
           char* result = worker->cread();
           double entry = *(double*) result;
           hist->update(data_req->person, entry);
           delete[] result;
       } else if(m == FILE_MSG) {
           string output = "./received/" + GlobalStruct->fileName;
           filemsg *filem = (filemsg *) req;
           char* result = worker->cread();
           int fd = open(output.c_str(), O_RDWR, 0777);
           lseek(fd, filem->offset, SEEK_SET);
           write(fd, result, filem->length);
           close(fd);
       } else {
           break;
       }
   }
    
}
int main(int argc, char *argv[])
{
    //Hello
    int n = 200;            //default number of requests per "patient"
    int p = 2;             // number of patients [1,15]
    int w = 50;            //default number of worker threads
    int b = 30; 	        // default capacity of the request buffer, you should change this default
	int m = MAX_MESSAGE; 	// default capacity of the file buffer
    srand(time_t(NULL));
    
    int pid = fork(); //Honestly idk what this does
    if (pid == 0){
		//modify this to pass along m
        execl ("dataserver", "dataserver", (char *)NULL);
        
    } else {
        cout << "did i die here?" << endl;
        GlobalStruct = new clientArg_t(n,p,w,b);
        handleArgs(argc, argv, GlobalStruct);
        cout << GlobalStruct->num_dp << endl;
        cout << GlobalStruct->patients << endl;
        cout << GlobalStruct->workers << endl;
        cout << GlobalStruct->buffer_size << endl;

        FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
        BoundedBuffer* request_buffer= new BoundedBuffer(GlobalStruct->buffer_size);
        HistogramCollection* hc = new HistogramCollection();
        vector<thread> patients;
        vector<thread> workers;
        
        struct timeval start, end;
        gettimeofday (&start, 0);

        /* Start all threads here */
        if(GlobalStruct->fileReq) {
            cout << "in file" << endl;
            string output = "received/" + GlobalStruct->fileName;
            int size = sizeof(filemsg) + GlobalStruct->fileName.length() + 1;
            char package[size];
            filemsg *file_msg = new filemsg(0,0);
            *(filemsg*) package = *file_msg;
            strcpy(package + sizeof(filemsg), GlobalStruct->fileName.c_str());
            chan->cwrite((char*) &package, size);
            int length = *(int*) chan->cread();
            int fd = open(output.c_str(), O_RDWR | O_CREAT, 0777);
            close(fd);
            patients.push_back(thread(fileProducer, length, request_buffer));
        } else {
            //data
            for(int i = 0; i < GlobalStruct->patients; i++) {
                patients.push_back(thread(patient_function, i + 1, request_buffer, hc));
                hc->add(new Histogram(7, -4, 4));
            }
        }
        //consumers
        for(int i = 0; i < GlobalStruct->workers; i++) {
            MESSAGE_TYPE q = NEWCHANNEL_MSG;
            chan->cwrite((char*) &q, sizeof(MESSAGE_TYPE));
            char* temp = chan->cread();
            FIFORequestChannel* worker = new FIFORequestChannel(temp, FIFORequestChannel::CLIENT_SIDE);
            workers.push_back(thread(worker_function, worker, request_buffer, hc));
        }

        /* Join all threads here */
        //need to first join all patient threads, then push quit messages to all workers
        for(int i = 0; i < patients.size(); i++) {
            thread& t = patients.at(i);
            if(t.joinable()) {
                t.join();
            }
        }

        for(int i = 0; i < workers.size(); i++) {
            MESSAGE_TYPE q = QUIT_MSG;
            vector<char> temp((char*) &q, ((char*) &q + sizeof(q)));
            request_buffer->push(temp);
        }


        //finally join all worker threads
        for(int i = 0; i < workers.size(); i++) {
            thread& t = workers.at(i);
            if(t.joinable()) {
                t.join();
            }
        }

        gettimeofday (&end, 0);
        hc->print();
        int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
        int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
        cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

        MESSAGE_TYPE q = QUIT_MSG;
        chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
        cout << "All Done!!!" << endl;
        delete chan;
        
        delete request_buffer;
    }
    return 0;
}
