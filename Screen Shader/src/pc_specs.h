#pragma once

#include <thread>
#include <atomic>
#include "TCHAR.h"
#include "pdh.h"
#pragma comment(lib, "pdh.lib")

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;
std::atomic<double> g_CPUUsage(0.0);
bool g_Running = true;
static std::thread cpuThread;


std::string GetProcessorName() {
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

double readCPU() {
    PDH_FMT_COUNTERVALUE val;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &val);
    return val.doubleValue;
}

void cpuMonitorThread() {
    PdhCollectQueryData(cpuQuery);
    Sleep(1000);

    while (g_Running) {
        PDH_FMT_COUNTERVALUE val;
        if (PdhCollectQueryData(cpuQuery) == ERROR_SUCCESS &&
            PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &val) == ERROR_SUCCESS) {
            g_CPUUsage = val.doubleValue;
        }
        Sleep(1000);
    }
}

double GetProcessorUsage() {
    return g_CPUUsage.load();
}

void initCPU() {
    g_Running = true;
    cpuThread = std::thread(cpuMonitorThread);

    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddEnglishCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

void closeCPUThread() {
    g_Running = false;
    if (cpuThread.joinable())
        cpuThread.join();

    PdhCloseQuery(cpuQuery);
}