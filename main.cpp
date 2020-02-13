#include <iostream>
#include <stdio.h>
#include <string>
#include <dirent.h>
#include <thread>
#include <fstream>
#include <sstream>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <windows.h>

using namespace std;

bool isRunning = false;
bool hasAcFile = false;
string projectFileName;
string outputName;


struct AddedFiles{
    string fileName;
    time_t modTime;
};

// This function chescks the name of a file passed as parameter "name" and determines if it is of type .ac
bool isAutoFile(char *name, string directoryName){

    int i = 0;

    while(name[i] != '\0'){
        i++;
    }

    if(name[i-3] == '.'){
        if(name[i-2] == 'a' && name[i-1] == 'c'){
            cout << "\n-------------------------\nAuto Compile Program already exists\n-------------------------" << endl;
            hasAcFile = true;
            projectFileName = directoryName + "\\" + name;
            cout << "PROJECT NAME IS: " << projectFileName << endl;
            return true;
        }
    }

    return false;
}


// Takes a directory name in "directoryName" and returns a vector of the names of all files in that directory
std::vector<string> filesInDirectory(string directoryName){

    struct dirent *file;
    DIR *directory;

    std::vector<string> files;
    int i = 0;

    if((directory = opendir(directoryName.c_str())) != NULL)
    {
        while((file = readdir(directory)) != NULL)
        {
            if(i > 1)
                files.push_back(file->d_name);
            i++;
        }
        return files;
    }
}


//  Prompts user to enter a project name and creates a .ac file with the project name in the folder "directoryName"
void createAcFile(string directoryName){

    string projectName;
    cout << "No AutoCompile file dectected, creating new file" << endl << "\nEnter project name: ";
    cin >> projectName;
    outputName = projectName;

    projectName = directoryName + "\\" + projectName + ".ac";
    projectFileName = projectName;
    
    cout << "PROJECT FILE NAME IS: "  << projectFileName << endl;

    std::ofstream fs (projectName);
    fs.close();

    cout << "\n************************\nNew Auto Compile program created\n************************" << endl;
}


// Started in a worker thread. listens for command input from the terminal and responds
void commandListen(){
    
    string command;

    cin >> command;

    if(command == "exit"){
        isRunning = false;
    }else if(command == "add"){
        
    }
}


// Main loop of program. runs while the isRunning bool is set to true in the directory "directoryName"
void commandLoop(string directoryName){

    isRunning = true;

    std::thread commandThread(commandListen);
    commandThread.detach();

    std::vector<string> files = filesInDirectory(directoryName);
    std::vector<AddedFiles> addedFilesArray;
    std::vector<string> lines;

    std::ifstream acFile(projectFileName);

    string line;
    while(std::getline(acFile, line))
    {
        lines.push_back(line);
    }

    for(int i = 0; i < files.size(); i++){
        cout << files[i] << endl;

        struct stat fileInfo;
        struct AddedFiles addedFile;
        stat((directoryName + "/" + files[i]).c_str(), &fileInfo);
        
        addedFile.fileName = files[i];
        addedFile.modTime = fileInfo.st_mtime;
        addedFilesArray.push_back(addedFile);
    }

    while(isRunning){
        for(int i = 0; i < addedFilesArray.size(); i++){
            struct stat fileInfo;
            stat((directoryName + "/" + addedFilesArray[i].fileName).c_str(), &fileInfo);

            if(difftime(fileInfo.st_mtime, addedFilesArray[i].modTime) > 0){
                cout << "File " << addedFilesArray[i].fileName << " has been changed" << endl << "Compiling Program..." << endl;
                addedFilesArray[i].modTime = fileInfo.st_mtime;
                string compileString = "g++ " + directoryName + "/" + addedFilesArray[1].fileName + " -o " + directoryName + "/" + outputName;
                cout << "string: " << compileString.c_str() << endl;
                system(compileString.c_str());
            }
        }
    }
}


int main(int argc, char **argv)
{
    string directoryName;
    cout << "Enter project directory: ";
    cin >> directoryName;

    struct dirent *file;
    DIR *directory;

    if((directory = opendir(directoryName.c_str())) != NULL){
        while((file = readdir(directory)) != NULL){
            if(isAutoFile(file->d_name, directoryName)){
                commandLoop(directoryName);
            }
        }
        if(!hasAcFile){
            createAcFile(directoryName);
            commandLoop(directoryName);
        }
        closedir(directory);
    }else{
        cout << "could not open" << endl;
    }
}

