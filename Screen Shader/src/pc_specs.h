#pragma once
#include "windows.h"

#include <string>
#include <thread>
#include <atomic>
#include "TCHAR.h"
#include "pdh.h"
#pragma comment(lib, "pdh.lib")

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

std::atomic<double> g_CPUUsage(0.0);
std::atomic<double> g_RAMUsage(0.0);
bool g_Running = true;
static std::thread thread;


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
        return "Unknow CPU";
    }
}

void MonitorThread() {
    PdhCollectQueryData(cpuQuery);
    Sleep(1000);

    while (g_Running) {
        PDH_FMT_COUNTERVALUE val;
        if (PdhCollectQueryData(cpuQuery) == ERROR_SUCCESS && PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &val) == ERROR_SUCCESS) {
            g_CPUUsage = val.doubleValue;
        }

        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            g_RAMUsage = (memInfo.ullTotalPhys - memInfo.ullAvailPhys) / 1024.0 / 1024.0 / 1024.0;
        }
        Sleep(1000);
    }
}

double GetCPUUsage() {
    return g_CPUUsage.load();
}


double GetRAM() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullTotalPhys / 1024.0 / 1024.0 / 1024.0;
    }
    return 0;
}

double GetRAMUsage() {
    return g_RAMUsage.load();
}


void initThread() {
    g_Running = true;
    thread = std::thread(MonitorThread);

    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

void closeThread() {
    g_Running = false;
    if (thread.joinable())
        thread.join();

    PdhCloseQuery(cpuQuery);
}