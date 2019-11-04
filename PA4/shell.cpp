#include <fcntl.h>
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

vector<string> split(string line, string separator = " ", vector<string> in = vector<string>(0, "")) {  //splits a string delimited by a separator into a vector
    //cout << in.size() << endl;
    vector<string> result = in;
    while (line.size()) {
        size_t found = line.find(separator);
        if (found == string::npos) {  //If separator not found
            string lastpart = trim(line);
            //cout << "Last part " << lastpart << endl;
            if (lastpart.size() > 0) {
                result.push_back(lastpart);
            }
            break;
        }
        size_t quotes_start = line.find("\"");  //ignore anything in quotes
        if (quotes_start != string::npos) {
            //cout << line.substr(quotes_start + 1) << endl;
            size_t quotes_end = line.substr(quotes_start + 1).find("\"") + quotes_start + 1;
            if (quotes_end != string::npos)
                //cout << "quotes in this bitch" << endl;
                if (found < quotes_start) {  //continue as normal
                    //cout << "< before quote" << endl;
                } else if (found > quotes_end) {  //continue as normal
                    //cout << "< after quotes" << endl;
                } else if ((found > quotes_start) && (found < quotes_end)) {  //if separator found within quotes ignore and look for seperator after the quotes
                    //cout << "quotes in middle" << endl;
                    found = line.substr(quotes_end + 1).find(separator) + quotes_end + 1;
                }
        }
        string segment = trim(line.substr(0, found));
        ////cout << "line: " << line << "found: " << found << endl;
        line = line.substr(found + 1);
        if (separator == ">") {
            string s2 = "@REDIROUTTO";
            line.insert(0, s2);
        } else if (separator == "<") {
            string s2 = "@REDIRINFROM";
            line.insert(0, s2);
        }
        //cout << "Segment: " << segment << endl;
        //cout << "line: " << line << endl;

        ////cout << "[" << segment << "]"<< endl;
        if (segment.size() != 0)
            result.push_back(segment);

        ////cout << line << endl;
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

vector<string> redirection(string line) {  //not actual logic but using split, creates a vector denoting redirection
    //cout << line << endl;
    vector<string> in_parts = split(line, "<");
    //cout << in_parts.size() << endl;
    vector<string> out_parts;
    for (string x : in_parts) {
        out_parts = split(x, ">", out_parts);
    }
    return out_parts;
}

int main() {
    int info = 0;
    vector<string> infoCommands;
    infoCommands.push_back("echo Working Directory:");
    infoCommands.push_back("pwd");
    infoCommands.push_back("echo User:");
    infoCommands.push_back("whoami");
    infoCommands.push_back("echo Date and Time:");
    infoCommands.push_back("date");

    while (true) {                         // repeat this loop until the user presses Ctrl + C
        int std_in = dup(STDIN_FILENO);    // need to keep original stdin so it can be reset after redirection
        int std_out = dup(STDOUT_FILENO);  // need to keep original stdout so it can be reset after redirection

        cout << "\nuser@shell $ ";

        string commandline = ""; /*get from STDIN, e.g., "ls  -la |   grep Jul  | grep . | grep .cpp" */
        getline(cin, commandline);
        // //cout << commandline << endl;
        // split the command by the "|", which tells you the pipe levels
        vector<string> piped_parts = split(commandline, "|");
        //cout << piped_parts.size() << endl;
        for (string x : piped_parts) {
            //cout << x << endl;
        }

        // for each pipe, do the following:
        for (int i = 0; i < piped_parts.size(); i++) {
            // make pipe
            int fd[2];
            pipe(fd);
            vector<string> redirected_parts = redirection(piped_parts.at(i));
            if (!fork()) {  //child; redirects output to parent and execute in here
                // redirect output to the next level unless this is the last level
                if (i < piped_parts.size() - 1) {
                    dup2(fd[1], STDOUT_FILENO);            // redirect STDOUT to fd[1], so that it can write to the other side
                    close(fd[1]);                          // STDOUT already points fd[1], which can be closed
                } else if (redirected_parts.size() > 1) {  //handles redirected output
                    //cout << "we got some redirections" << endl;
                    for (int j = 0; j < redirected_parts.size(); j++) {
                        string redirFile = redirected_parts.at(j);
                        size_t found = redirFile.find("@REDIROUTTO");
                        if (found != string::npos) {
                            //cout << "redirecting output" << endl;
                            redirFile = redirFile.substr(11);
                            redirFile = trim(redirFile);
                            int outFD = open(redirFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC,
                                             S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                            dup2(outFD, STDOUT_FILENO);
                            close(outFD);
                            piped_parts.at(i) = redirected_parts.at(0);
                        }
                        found = redirFile.find("@REDIRINFROM");
                        if (found != string::npos) {
                            //cout << "redirecting input" << endl;
                            redirFile = redirFile.substr(13);
                            redirFile = trim(redirFile);
                            int inFD = open(redirFile.c_str(), O_RDONLY);
                            dup2(inFD, STDIN_FILENO);
                            close(inFD);
                            piped_parts.at(i) = redirected_parts.at(0);
                        }
                    }
                } else if (commandline == "info") {
                    if (info < infoCommands.size()) {
                        execute(infoCommands.at(info));
                    }
                }
                /* executes function that can split the command by spaces to 
                find out all the arguments, see the definition*/
                if (commandline != "info") {
                    execute(piped_parts[i]);  // this is where you execute
                }

            } else {      //parent; if piped, redirects output from child in std input
                wait(0);  // wait for the child process
                if (i < piped_parts.size() - 1) {
                    dup2(fd[0], STDIN_FILENO);  // then do other redirects
                    close(fd[1]);
                } else if (commandline == "info") {
                    info++;
                    if (info < infoCommands.size()) {
                        i = i - 1;
                    }
                }
            }
        }

        dup2(std_in, STDIN_FILENO);  //reset std in
        close(std_in);
        dup2(std_out, STDOUT_FILENO);
        close(std_out);

        //cout << "Entering next iteration";
    }
}