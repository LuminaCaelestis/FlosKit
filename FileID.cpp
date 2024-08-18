
#include <iostream>
#include "FileIDGetter.h"

std::string helpInfo(
    "\n"
    "Usage: GetFileID [options]\n"
    "Options:\n"
    "  -h, --help     Show this help message and exit\n"
    "  -v, --version  Show version information and exit\n"
    "  -f, --file     Specify the file path or directory path to get the file ID\n"
    "Use double quotes: \"File Path\" to enclose the file path or directory path if it contains spaces.\n"
);

void printHelpInfo() {
    for(auto line : helpInfo) {
        std::cout << line;
    }
}

void Process(const char *optarg) {
    DWORD attributes = GetFileAttributesA(optarg);

    if (attributes == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "Invalid file or directory path: " << optarg << std::endl;

    } 
    
    else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
        try{
            using FileResultType = std::vector<FileIDGetter::FilenameWithID>;
            std::shared_ptr<FileResultType> FileInfomationList = FileIDGetter::GetFromDirectory(optarg);
            resultPrinter(FileInfomationList);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    } 

    else if (attributes & FILE_ATTRIBUTE_NORMAL) {
        try{
            FileIDGetter::FilenameWithID  fileInfo =  FileIDGetter::GetFileID(optarg);
            resultPrinter(fileInfo);
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

void resultPrinter(const FileIDGetter::FilenameWithID &fileInfo){
    std::cout <<  fileInfo.high << fileInfo.low << "\t" << fileInfo.FilePath << std::endl; 
}

void resultPrinter(const std::shared_ptr<std::vector<FileIDGetter::FilenameWithID>> &FileInfomationList) {
    for (auto fileInfo : *FileInfomationList) {
        resultPrinter(fileInfo);
    }
}


int main(int argc, char* argv[]) {
    int opt;
    int option_index = 0;
    
    struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 'v'},
        {"file",    required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hvf:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                printHelpInfo();
                break;
            case 'v':
                std::cout << "\nGetFileID   Version 1.0" << std::endl;
                break;
            case 'f':
                Process(optarg);
                break;
            default:
                std::cerr << "Unknown option\n" << std::endl;
                printHelpInfo();
                break;
        }
    }

    // 如果没有指定任何选项，显示帮助
    if (argc == 1) {
        printHelpInfo();
    }

    return 0;
}