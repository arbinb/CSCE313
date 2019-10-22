#include "common.h"

void EXITONERROR(string msg)
{
    perror(msg.c_str());
    exit(-1);
}

vector<string> split (string line, char separator){
	vector<string> result;
	while (line.size()){
		size_t found = line.find_first_of (separator);
		if (found!=std::string::npos){
			string part = line.substr(0, found);
			result.push_back(part);
			line = line.substr (found+1);
		}
		else{
			result.push_back (line);
			break;
		}
	}
	return result;
}

__int64_t get_file_size (string filename){
    struct stat buf;
    int fd = open (filename.c_str (), O_RDONLY);
    fstat(fd, &buf);
    __int64_t size = (__int64_t) buf.st_size;
    close (fd);
    return size;
}

void handleArgs(int argc, char *argv[], clientArg_t* obj) {
	int c;
	while((c = getopt (argc, argv, "n:p:w:b:f")) != -1) {
		switch(c) {
		case 'n':
			if(obj->msg_t == FILE_MSG) {
				cerr << "ERROR: CANNOT MAKE MULTIPLE REQUEST TYPES; MAKE SURE FLAGS ARE CORRECT" << endl;
				exit(-1);
			} else {
				if(atoi(optarg) > 0 && atoi(optarg) <= 15000) {
					obj->num_dp = atoi(optarg);
					obj->msg_t = DATA_MSG;
				} else {
					cerr << "INVALID AMOUNT OF DATA POINTS" << endl;
					exit(-1);
				}
				//cout << "n: " << obj->num_dp << endl;
			}
			break;
		case 'p':
			if(obj->msg_t == FILE_MSG) {
				cerr << "ERROR: CANNOT MAKE MULTIPLE REQUEST TYPES; MAKE SURE FLAGS ARE CORRECT" << endl;
				exit(-1);
			} else {
				if(atoi(optarg) > 0 && atoi(optarg) <= 15) {
					obj->patients = atoi(optarg);					
					obj->msg_t = DATA_MSG;
				} else {
					cerr << "INVALID AMOUNT OF PATIENTS" << endl;
					exit(-1);
				}
				//cout << "p: " << obj->patients << endl;
			}
			break;
		case 'w':
			if(obj->msg_t == FILE_MSG) {
				cerr << "ERROR: CANNOT MAKE MULTIPLE REQUEST TYPES; MAKE SURE FLAGS ARE CORRECT" << endl;
				exit(-1);
			} else {
				if(atoi(optarg) >= 50 && atoi(optarg) <= 5000) {
					obj->workers = atoi(optarg);					
					obj->msg_t = DATA_MSG;
				} else {
					cerr << "INVALID AMOUNT OF WORKERS" << endl;
					exit(-1);
				}
				//cout << "w: " << obj->workers << endl;
			}
			break;
		case 'b':
			if(obj->msg_t == FILE_MSG) {
				cerr << "ERROR: CANNOT MAKE MULTIPLE REQUEST TYPES; MAKE SURE FLAGS ARE CORRECT" << endl;
				exit(-1);
			} else {
				if(atoi(optarg) > 0 && atoi(optarg) <= 300) {
					obj->buffer_size = atoi(optarg);					
					obj->msg_t = DATA_MSG;
				} else {
					cerr << "INVALID BUFFER SIZE" << endl;
					exit(-1);
				}
				//cout << "b: " << obj->buffer_size << endl;
			}
			break;
		case 'f':
			if(obj->msg_t == DATA_MSG) {
				cerr << "ERROR: CANNOT MAKE MULTIPLE REQUEST TYPES; MAKE SURE FLAGS ARE CORRECT" << endl;
				exit(-1);
			} else {
				obj->fileName = optarg;
				cout << "f: " << obj->fileName << endl;
				obj->msg_t = FILE_MSG;
			}
			break;	
		}
	}
}
