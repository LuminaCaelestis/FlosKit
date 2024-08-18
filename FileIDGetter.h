/*
 * 用于获取文件ID, 适用于唯一标识文件，且不希望依赖文件名的场合。
 *
 * 异常：
 * - 当路径无效或无法访问时，抛出 std::runtime_error 异常
 * - 当无法打开文件或获取文件信息时，抛出 std::runtime_error 异常
 */

#ifndef FLOS_FILE_ID_GETTER_H
#define FLOS_FILE_ID_GETTER_H

#include <windows.h>
#include <unistd.h>
#include <getopt.h>
#include <stack>
#include <vector>
#include <memory>
#include <stdexcept>

class FileIDGetter {

public:

    struct FilenameWithID {
        std::string FilePath;
        DWORD high;
        DWORD low;
    };

    static std::shared_ptr<std::vector<FilenameWithID>> GetFromDirectory(const char*);
    static FilenameWithID GetFileID(const char*);
};


FileIDGetter::FilenameWithID FileIDGetter::GetFileID(const char *filePath) {
    HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        throw std::runtime_error("Failed to open file with error code: " + std::to_string(errorCode));
    }
    // Get file information
    BY_HANDLE_FILE_INFORMATION fileInfo;
    if (GetFileInformationByHandle(hFile, &fileInfo)) {
        FilenameWithID fileID = {filePath, fileInfo.nFileIndexHigh, fileInfo.nFileIndexLow};
        CloseHandle(hFile);
        return fileID;
    } else {
        DWORD errorCode = GetLastError();
        CloseHandle(hFile);
        throw std::runtime_error("Failed to get file information with error code: " + std::to_string(errorCode));
    }
}

std::shared_ptr<std::vector<FileIDGetter::FilenameWithID>> FileIDGetter::GetFromDirectory(const char *directoryPath) {

    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    std::stack<std::string> directoriesToProcess;
    directoriesToProcess.push(directoryPath);
    auto result = std::make_shared<std::vector<FilenameWithID>>();

    while (!directoriesToProcess.empty()) {

        const std::string currentDirectory = directoriesToProcess.top();
        directoriesToProcess.pop();
        std::string searchPath = currentDirectory + "\\*";
        hFind = FindFirstFileA(searchPath.c_str(), &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to open directory");
        }
        do {
            if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0) {
                continue;
            }

            std::string fullPath = currentDirectory + "\\" + findFileData.cFileName;

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                directoriesToProcess.push(fullPath);
            } else {
                result->push_back(GetFileID(fullPath.c_str()));
            }
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);
    }
    
    if (result->empty()) {
        throw std::runtime_error("No valid file found in the directory");
    }

    return result;
}

#endif // FLOS_FILE_ID_GETTER_H