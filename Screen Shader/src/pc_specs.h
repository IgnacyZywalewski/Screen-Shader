#pragma once
#include "windows.h"
#include <string>
#include <thread>
#include <atomic>
#include "psapi.h"

#include "pdh.h"
#include <dxgi1_4.h>

#include <vector>

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;
static PDH_HQUERY diskQuery;
static PDH_HCOUNTER diskTotal;
static IDXGIAdapter3* g_adapter3 = nullptr;

std::atomic<double> g_CPUUsage(0.0);
std::atomic<double> g_CPUProcessUsage(0.0);
std::atomic<double> g_RAMUsage(0.0);
std::atomic<double> g_RAMProcessUsage(0.0);
std::atomic<double> g_DiskUsage(0.0);
std::atomic<double> g_DiskTotalGB(0.0);
std::atomic<double> g_DiskUsedGB(0.0);
std::atomic<double> g_VRAMUsedGB(0.0);


bool g_Running = true;
static std::thread thread;

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static int numProcessors;
static HANDLE self;

static ULONGLONG lastRead = 0;
static ULONGLONG lastWrite = 0;


void InitProcessCPU() {
    SYSTEM_INFO sysInfo;
    FILETIME ftime, fsys, fuser;

    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&lastCPU, &ftime, sizeof(FILETIME));

    self = GetCurrentProcess();
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
    memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}
double GetProcessCPUUsage() {
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now, sys, user;
    double percent;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));

    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);
    memcpy(&sys, &fsys, sizeof(FILETIME));
    memcpy(&user, &fuser, sizeof(FILETIME));

    percent = double(sys.QuadPart - lastSysCPU.QuadPart) + double(user.QuadPart - lastUserCPU.QuadPart);
    percent /= (now.QuadPart - lastCPU.QuadPart);
    percent /= numProcessors;

    lastCPU = now;
    lastUserCPU = user;
    lastSysCPU = sys;

    return percent * 100.0;
}
std::string GetCPUName() {
    HKEY hKey;
    char buffer[256];
    unsigned long bufferSize = sizeof(buffer);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)buffer, &bufferSize);
        RegCloseKey(hKey);
        return std::string(buffer);
    }
    else {
        return "Unknown CPU";
    }
}
double GetCPUUsage() { return g_CPUUsage.load(); }
double GetCPUProcessUsage() { return g_CPUProcessUsage.load(); }


double GetRAM() {
    MEMORYSTATUSEX memInfo{ sizeof(memInfo) };
    if (GlobalMemoryStatusEx(&memInfo)) return memInfo.ullTotalPhys / 1024.0 / 1024.0 / 1024.0;
    return 0;
}
double GetRAMUsage() { return g_RAMUsage.load(); }
double GetRAMProcessUsage() { return g_RAMProcessUsage.load(); }


double GetDiskUsage() { return g_DiskUsage.load(); }
double GetDiskTotalGB() { return g_DiskTotalGB.load(); }
double GetDiskUsedGB() { return g_DiskUsedGB.load(); }


std::string GetGPUName() {
    IDXGIFactory* factory = nullptr;
    IDXGIAdapter* adapter = nullptr;

    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
        return "Unknown GPU";

    if (factory->EnumAdapters(0, &adapter) != S_OK) {
        factory->Release();
        return "Unknown GPU";
    }

    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);

    adapter->Release();
    factory->Release();

    std::wstring wname(desc.Description);
    if (wname.empty()) 
        return "Unknown GPU";

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), (int)wname.size(), nullptr, 0, nullptr, nullptr);
    std::string name(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), (int)wname.size(), &name[0], size_needed, nullptr, nullptr);

    return name;
}
const char* GetOpenGLVersion() {
    const char* version = (const char*)glGetString(GL_VERSION);
    return version;
}


void MonitorThread() {
    PdhCollectQueryData(cpuQuery);
    PdhCollectQueryData(diskQuery);
    Sleep(1000);

    while (g_Running) {
        // CPU systemu
        PDH_FMT_COUNTERVALUE val;
        if (PdhCollectQueryData(cpuQuery) == ERROR_SUCCESS &&
            PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &val) == ERROR_SUCCESS) {
            g_CPUUsage = val.doubleValue;
        }

        // CPU procesu
        g_CPUProcessUsage = GetProcessCPUUsage();

        // RAM systemu
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            g_RAMUsage = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / 1024.0 / 1024.0 / 1024.0;
        }

        // RAM procesu
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        g_RAMProcessUsage = pmc.PrivateUsage / 1024.0 / 1024.0;

        // Dysk 
        ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
        if (GetDiskFreeSpaceExA("C:\\", &freeBytes, &totalBytes, &totalFreeBytes)) {
            g_DiskTotalGB = totalBytes.QuadPart / 1024.0 / 1024.0 / 1024.0;
            g_DiskUsedGB = (totalBytes.QuadPart - freeBytes.QuadPart) / 1024.0 / 1024.0 / 1024.0;
        }

        if (diskQuery) {
            PDH_FMT_COUNTERVALUE val;
            if (PdhCollectQueryData(diskQuery) == ERROR_SUCCESS &&
                PdhGetFormattedCounterValue(diskTotal, PDH_FMT_DOUBLE, NULL, &val) == ERROR_SUCCESS) {
                g_DiskUsage = val.doubleValue;
            }
        }

        Sleep(1000);
    }
}


void initThread() {
    g_Running = true;

    // CPU systemu
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);

    // CPU procesu
    InitProcessCPU();

    // Dysk
    PdhOpenQuery(NULL, NULL, &diskQuery);
    PdhAddEnglishCounter(diskQuery, L"\\PhysicalDisk(_Total)\\% Disk Time", NULL, &diskTotal);
    PdhCollectQueryData(diskQuery);

    thread = std::thread(MonitorThread);
}

void closeThread() {
    g_Running = false;

    if (thread.joinable()) 
        thread.join();

    if (cpuQuery) 
        PdhCloseQuery(cpuQuery);
    
    if (diskQuery) 
        PdhCloseQuery(diskQuery);

    if (g_adapter3) {
        g_adapter3->Release();
        g_adapter3 = nullptr;
    }
}
