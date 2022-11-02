#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include "ShObjIdl.h"
using namespace std;

#pragma comment( lib, "psapi" )

enum THREADINFOCLASS
{
    ThreadQuerySetWin32StartAddress = 9,
};

typedef NTSTATUS(__stdcall* f_NtQueryInformationThread)(HANDLE, THREADINFOCLASS, void*, ULONG_PTR, ULONG_PTR*);

ULONG_PTR GetThreadStartAddress(HANDLE hThread)
{
    auto NtQueryInformationThread = reinterpret_cast<f_NtQueryInformationThread>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationThread"));
    if (!NtQueryInformationThread)
        return 0;

    ULONG_PTR ulStartAddress = 0;
    NTSTATUS Ret = NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &ulStartAddress, sizeof(ULONG_PTR), nullptr);

    if (Ret)
        return 0;

    return ulStartAddress;
}


bool SuspendThreadByStartaddress(ULONG_PTR StartAddress, DWORD dwProcId)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (!hSnap)
        return false;

    THREADENTRY32 TE32 = { 0 };
    TE32.dwSize = sizeof(THREADENTRY32);

    BOOL Ret = Thread32First(hSnap, &TE32);
    while (Ret)
    {
        if (TE32.th32OwnerProcessID == dwProcId)
        {
            HANDLE hTempThread = OpenThread(THREAD_ALL_ACCESS, FALSE, TE32.th32ThreadID);
            if (!hTempThread)
                continue;

            if (StartAddress == GetThreadStartAddress(hTempThread))
            {
                SuspendThread(hTempThread);
                CloseHandle(hTempThread);
                CloseHandle(hSnap);
                return true;
            }
        }
        Ret = Thread32Next(hSnap, &TE32);
    }

    CloseHandle(hSnap);

    return false;
}

bool TerminateThreadByStartaddress(ULONG_PTR StartAddress, DWORD dwProcId)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (!hSnap)
        return false;

    THREADENTRY32 TE32 = { 0 };
    TE32.dwSize = sizeof(THREADENTRY32);

    BOOL Ret = Thread32First(hSnap, &TE32);
    while (Ret)
    {
        if (TE32.th32OwnerProcessID == dwProcId)
        {
            HANDLE hTempThread = OpenThread(THREAD_ALL_ACCESS, FALSE, TE32.th32ThreadID);
            if (!hTempThread)
                continue;

            if (StartAddress == GetThreadStartAddress(hTempThread))
            {
                TerminateThread(hTempThread, 0);
                CloseHandle(hTempThread);
                CloseHandle(hSnap);
                return true;
            }
        }
        Ret = Thread32Next(hSnap, &TE32);
    }

    CloseHandle(hSnap);

    return false;
}

bool ResumeThreadByStartaddress(ULONG_PTR StartAddress, DWORD dwProcId)
{
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (!hSnap)
        return false;

    THREADENTRY32 TE32 = { 0 };
    TE32.dwSize = sizeof(THREADENTRY32);

    BOOL Ret = Thread32First(hSnap, &TE32);
    while (Ret)
    {
        if (TE32.th32OwnerProcessID == dwProcId)
        {
            HANDLE hTempThread = OpenThread(THREAD_ALL_ACCESS, FALSE, TE32.th32ThreadID);
            if (!hTempThread)
                continue;

            if (StartAddress == GetThreadStartAddress(hTempThread))
            {
                ResumeThread(hTempThread);
                CloseHandle(hTempThread);
                CloseHandle(hSnap);
                return true;
            }
        }
        Ret = Thread32Next(hSnap, &TE32);
    }

    CloseHandle(hSnap);

    return false;
}


uintptr_t dwGetModuleBaseAddress(DWORD procId, const char* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (strcmp(modEntry.szModule, modName) == 0)
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));

        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

int main()
{
	HWND peepoo;
	HANDLE hProcess;
	DWORD PID;
    	string str("NIF Bypass");
    	SetConsoleTitle(str.c_str());
    	peepoo = FindWindow(NULL, "BlackShot");
	if (!peepoo) {
		cout << "Process is not running.\n";
	}
	else {
        while (true)
        {
            
            GetWindowThreadProcessId(peepoo, &PID);
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
            uintptr_t base = dwGetModuleBaseAddress(PID, "BlackShot");
            Sleep(100);
            TerminateThreadByStartaddress(0xADDRESS, PID);
            cout << "[+] Done.\n";
            
            Sleep(1500);
            return 1;
            
        }
        
        
	}
    
}

	