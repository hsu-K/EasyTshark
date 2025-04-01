#pragma once
#include "MiscUtil.hpp"
#include <io.h>
#include <fcntl.h>

class ProcessUtil {
public:
    static FILE* PopenEx(std::string command, PID_T* pidOut = nullptr)
    {
        HANDLE hReadPipe, hWritePipe;
        SECURITY_ATTRIBUTES saAttr;
        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;
        FILE* pipeFp = nullptr;

        // 設置安全屬性，允許管道句柄繼承
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = nullptr;

        // 創建匿名管道
        if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0)) {
            perror("CreatePipe");
            return nullptr;
        }

        if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0)) {
            perror("SetHandleInformation");
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            return nullptr;
        }

        //初始化STARTUPINFO 結構體
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = hWritePipe;
        siStartInfo.hStdOutput = hWritePipe;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        std::wstring wCommand = MiscUtil::stringToWString(command);
        // 創建子進程
        if (!CreateProcess(
            nullptr,                        // No module name (use command line)
            (LPWSTR)wCommand.data(),          // Command line
            nullptr,                        // Process handle not inheritable
            nullptr,                        // Thread handle not inheritable
            TRUE,                           // Set handle inheritance
            CREATE_NO_WINDOW,               // No window
            nullptr,                        // Use parent's environment block
            nullptr,                        // Use parent's starting directory 
            &siStartInfo,                   // Pointer to STARTUPINFO structure
            &piProcInfo                     // Pointer to PROCESS_INFORMATION structure
        )) {
            perror("CreateProcess");
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            return nullptr;
        }

        // 關閉寫端句柄(父進程不使用)
        CloseHandle(hWritePipe);

        // 返回子進程 PID
        if (pidOut) {
            *pidOut = piProcInfo.dwProcessId;
        }

        // 將管道的讀端轉換為FILE* 並返回
        // _open_osfhandle()：將 Windows HANDLE 轉換為文件描述符（fd）
        // _fdopen()：將 fd 轉換為標準 C FILE*，可用於 fgets() 讀取子進程的輸出
        pipeFp = _fdopen(_open_osfhandle(reinterpret_cast<intptr_t>(hReadPipe), _O_RDONLY), "r");
        if (!pipeFp) {
            CloseHandle(hReadPipe);
        }
        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        return pipeFp;
    }

    static int Kill(PID_T pid) {

        // 打開指定進程
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess == nullptr) {
            cout << "Failed to open process with PID " << pid << ", error: " << GetLastError() << endl;
            return -1;
        }

        // 終止進程
        if (!TerminateProcess(hProcess, 0)) {
            cout << "Failed to terminate process with PID" << pid << ", error: " << GetLastError() << endl;
            return -1;
        }

        CloseHandle(hProcess);
        return 0;
    }

    // 封裝一個不會產生畫面的子進程
    static bool Exec(std::string cmdline) {
#ifdef _WIN32
        PROCESS_INFORMATION piProcInfo;
        STARTUPINFO siStartInfo;

        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

        wstring wcommand = MiscUtil::stringToWString(cmdline);
        if (CreateProcess(
            nullptr,
            (LPWSTR)wcommand.data(),
            nullptr,
            nullptr,
            TRUE,
            CREATE_NO_WINDOW,
            nullptr,
            nullptr,
            &siStartInfo,
            &piProcInfo
        )) {
            WaitForSingleObject(piProcInfo.hProcess, INFINITE);
            CloseHandle(piProcInfo.hProcess);
            CloseHandle(piProcInfo.hThread);
            return true;
        }
        else {
            return false;
        }
#else
        return std::system(cmdline.c_str()) == 0;
#endif
    }

};