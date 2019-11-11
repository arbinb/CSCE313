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
    vector<string> result = in;
    while (line.size()) {
        int length = line.length();
        size_t found = line.find(separator);
        if (found == string::npos) {  //If separator not found
            string lastpart = trim(line);
            if (lastpart.size() > 0) {
                result.push_back(lastpart);
            }
            break;
        }
        size_t quotes_start = line.find("\"");  //ignore anything in quotes
        if (quotes_start != string::npos) {
            size_t quotes_end = line.substr(quotes_start + 1).find("\"") + quotes_start + 1;
            if (quotes_end != string::npos)
                if ((found > quotes_start) && (found < quotes_end)) {  //if separator found within quotes ignore and look for seperator after the quotes
                    found = line.substr(quotes_end + 1).find(separator) + quotes_end + 1;
                }
        }
        quotes_start = line.find("'");  //ignore anything in quotes
        if (quotes_start != string::npos) {
            size_t quotes_end = line.substr(quotes_start + 1).find("'") + quotes_start + 1;
            if (quotes_end != string::npos)
                if ((found > quotes_start) && (found < quotes_end)) {  //if separator found within quotes ignore and look for seperator after the quotes
                    found = line.substr(quotes_end + 1).find(separator) + quotes_end + 1;
                }
        }
        if (found >= line.length() - 1) {
            found += 1;
            separator = "!@#$%^";
        }
        string segment = trim(line.substr(0, found));
        if (found != length) {
            line = line.substr(found + 1);
        } else {
            line = "";
        }
        if (separator != " ") {
            if (separator == ">") {
                string s2 = "@REDIROUTTO";
                line.insert(0, s2);
            } else if (separator == "<") {
                string s2 = "@REDIRINFROM";
                line.insert(0, s2);
            }
        }
        if (segment.size() != 0) {
            result.push_back(segment);
        }
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
    vector<string> argstrings = split(command, " ");
    // split the command into space-separated parts
    for (int i = 0; i < argstrings.size(); i++) {
        argstrings.at(i).erase(std::remove(argstrings.at(i).begin(), argstrings.at(i).end(), '"'), argstrings.at(i).end());
        argstrings.at(i).erase(std::remove(argstrings.at(i).begin(), argstrings.at(i).end(), '\''), argstrings.at(i).end());
    }
    char** args = vec_to_char_array(argstrings);  // convert vec<string> into an array of char*
    execvp(args[0], args);
}

vector<string> redirection(string line) {  //not actual logic but using split, creates a vector denoting redirection
    vector<string> in_parts = split(line, "<");
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

    char cwd[100];
    char pwd[100];

    getcwd(cwd, 100);
    getcwd(pwd, 100);

    vector<int> processes_pid;
    vector<string> processes_name;
    while (true) {                       // repeat this loop until the user presses Ctrl + C
        int std_in = dup(STDIN_FILENO);  // need to keep original stdin so it can be reset after redirection
        int std_out = dup(STDOUT_FILENO);
        bool backProcess = false;  // need to keep original stdout so it can be reset after redirection

        for (int x = 0; x < processes_pid.size(); x++) {
            int status = waitpid(processes_pid.at(x), 0, WNOHANG);
            if (status > 0) {
                processes_pid.erase(processes_pid.begin() + x);
                processes_name.erase(processes_name.begin() + x);
            }
        }

        cout << "user@shell $ ";

        string commandline = ""; /*get from STDIN, e.g., "ls  -la |   grep Jul  | grep . | grep .cpp" */
        getline(cin, commandline);
        string checkExit = commandline;
        transform(checkExit.begin(), checkExit.end(), checkExit.begin(), ::tolower);
        if (checkExit == "exit") {
            exit(-1);
        }

        size_t cd = commandline.find("cd");  //handles cd
        if (cd != string::npos) {
            char cd_arg[100];
            strcpy(cd_arg, commandline.substr(3).c_str());
            char old[] = "-";
            if (cd_arg[0] == old[0]) {
                char tmp[100];
                memcpy(tmp, pwd, sizeof(pwd));
                memcpy(pwd, cwd, sizeof(cwd));
                memcpy(cwd, tmp, sizeof(tmp));
                chdir(cwd);
            } else {
                memcpy(pwd, cwd, sizeof(cwd));
                chdir(cd_arg);
                getcwd(cwd, 100);
            }
            char x[100];
            cout << getcwd(x, 100) << endl;
            continue;
        }

        if (commandline == "jobs") {
            for (int yy = 0; yy < processes_name.size(); yy++) {
                cout << processes_pid.at(yy) << "       " << processes_name.at(yy) << endl;
            }
            continue;
        }
        // split the command by the "|", which tells you the pipe levels
        vector<string> piped_parts = split(commandline, "|");

        // for each pipe, do the following:
        for (int i = 0; i < piped_parts.size(); i++) {
            // make pipe
            int fd[2];
            pipe(fd);
            vector<string> redirected_parts = redirection(piped_parts.at(i));
            size_t background = piped_parts.at(i).find("&");
            if (background != string::npos) {
                piped_parts.at(i) = piped_parts.at(i).substr(0, background);
                processes_name.push_back(piped_parts.at(i));
                backProcess = true;
            }

            int pid = fork();
            if (pid == 0) {  //child; redirects output to parent and execute in here
                // redirect output to the next level unless this is the last level
                if (backProcess) {
                    processes_pid.push_back((int)getpid());
                }
                if (i < piped_parts.size() - 1) {
                    if (redirected_parts.size() > 1) {
                        for (int k = 0; k < redirected_parts.size(); k++) {
                            string redirFile = redirected_parts.at(k);
                            size_t found = redirFile.find("@REDIRINFROM");
                            if (found != string::npos) {
                                redirFile = redirFile.substr(13);
                                redirFile = trim(redirFile);
                                int inFD = open(redirFile.c_str(), O_RDONLY);
                                dup2(inFD, STDIN_FILENO);
                                close(inFD);
                                piped_parts.at(i) = redirected_parts.at(0);
                            }
                            found = redirFile.find("@REDIROUTTO");
                            if (found != string::npos) {
                                redirFile = redirFile.substr(11);
                                redirFile = trim(redirFile);
                                int outFD = open(redirFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC,
                                                 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                                dup2(outFD, STDOUT_FILENO);
                                close(outFD);
                                piped_parts.at(i) = redirected_parts.at(0);
                            }
                        }
                    }
                    dup2(fd[1], STDOUT_FILENO);            // redirect STDOUT to fd[1], so that it can write to the other side
                    close(fd[1]);                          // STDOUT already points fd[1], which can be closed
                } else if (redirected_parts.size() > 1) {  //handles redirected output
                    for (int j = 0; j < redirected_parts.size(); j++) {
                        string redirFile = redirected_parts.at(j);
                        size_t found = redirFile.find("@REDIROUTTO");
                        if (found != string::npos) {
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

            } else {
                //parent; if piped, redirects output from child in std input
                if (backProcess) {
                    processes_pid.push_back(pid);
                    continue;
                }
                wait(NULL);  // wait for the child process
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
    }
}