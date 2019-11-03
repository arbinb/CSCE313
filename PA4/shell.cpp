#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <vector>

using namespace std;

string trim(string input) {  //trims anything before and including leading spaces
    int i = 0;
    while (i < input.size() && input[i] == ' ')
        i++;
    if (i < input.size())
        input = input.substr(i);
    else {
        return "";
    }

    i = input.size() - 1;
    while (i >= 0 && input[i] == ' ')
        i--;
    if (i >= 0)
        input = input.substr(0, i + 1);
    else
        return "";

    return input;
}

vector<string> split(string line, string separator = " ") {  //splits a string delimited by a separator into a vector
    vector<string> result;
    while (line.size()) {
        size_t found = line.find(separator);
        if (found == string::npos) {
            string lastpart = trim(line);
            if (lastpart.size() > 0) {
                result.push_back(lastpart);
            }
            break;
        }
        string segment = trim(line.substr(0, found));
        //cout << "line: " << line << "found: " << found << endl;
        line = line.substr(found + 1);

        //cout << "[" << segment << "]"<< endl;
        if (segment.size() != 0)
            result.push_back(segment);

        //cout << line << endl;
    }
    return result;
}

char** vec_to_char_array(vector<string> parts) {
    char** result = new char*[parts.size() + 1];  // add 1 for the NULL at the end
    for (int i = 0; i < parts.size(); i++) {
        // allocate a big enough string
        result[i] = new char[parts[i].size() + 1];  // add 1 for the NULL byte
        strcpy(result[i], parts[i].c_str());
    }
    result[parts.size()] = NULL;
    return result;
}

void execute(string command) {
    vector<string> argstrings = split(command, " ");  // split the command into space-separated parts
    char** args = vec_to_char_array(argstrings);      // convert vec<string> into an array of char*
    execvp(args[0], args);
}

int main() {
    while (true) {                       // repeat this loop until the user presses Ctrl + C
        int std_in = dup(STDIN_FILENO);  // need to keep original stdin so it can be reset after redirection

        cout << "\nuser@shell $ ";

        string commandline = ""; /*get from STDIN, e.g., "ls  -la |   grep Jul  | grep . | grep .cpp" */
        getline(cin, commandline);
        // cout << commandline << endl;
        // split the command by the "|", which tells you the pipe levels
        vector<string> piped_parts = split(commandline, "|");
        cout << piped_parts.size() << endl;
        for (string x : piped_parts) {
            cout << x << endl;
        }

        // for each pipe, do the following:
        for (int i = 0; i < piped_parts.size(); i++) {
            // make pipe
            int fd[2];
            pipe(fd);
            if (!fork()) {  //child; redirects output to parent and execute in here
                // redirect output to the next level
                // unless this is the last level
                if (i < piped_parts.size() - 1) {
                    dup2(fd[1], STDOUT_FILENO);  // redirect STDOUT to fd[1], so that it can write to the other side
                    close(fd[1]);                // STDOUT already points fd[1], which can be closed
                }
                /* executes function that can split the command by spaces to 
                find out all the arguments, see the definition*/
                execute(piped_parts[i]);  // this is where you execute
            } else {                      //parent; if piped, redirects output from child in std input
                wait(0);                  // wait for the child process
                if (i < piped_parts.size() - 1) {
                    dup2(fd[0], STDIN_FILENO);  // then do other redirects
                    close(fd[1]);
                }
            }
        }

        dup2(std_in, STDIN_FILENO);  //reset std in
        close(std_in);

        cout << "Entering next iteration";
    }
}