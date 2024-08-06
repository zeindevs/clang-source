#include <windows.h>

#include <Wbemidl.h>
#include <comdef.h>
#include <psapi.h>

#include <iomanip>
#include <iostream>

#pragma comment(lib, "wbemuuid.lib")

void GetCpuUsage(float &cpuUsage, FILETIME &lastSysIdle, FILETIME &lastSysKernel, FILETIME &lastSysUser,
                 FILETIME &lastProcKernel, FILETIME &lastProcUser)
{
    FILETIME sysIdle, sysKernel, sysUser;
    FILETIME procCreation, procExit, procKernel, procUser;

    // Get system times
    GetSystemTimes(&sysIdle, &sysKernel, &sysUser);

    // Get process times
    GetProcessTimes(GetCurrentProcess(), &procCreation, &procExit, &procKernel, &procUser);

    ULONGLONG sysIdleTime = ((ULONGLONG)sysIdle.dwLowDateTime) | (((ULONGLONG)sysIdle.dwHighDateTime) << 32);
    ULONGLONG sysKernelTime = ((ULONGLONG)sysKernel.dwLowDateTime) | (((ULONGLONG)sysKernel.dwHighDateTime) << 32);
    ULONGLONG sysUserTime = ((ULONGLONG)sysUser.dwLowDateTime) | (((ULONGLONG)sysUser.dwHighDateTime) << 32);
    ULONGLONG procKernelTime = ((ULONGLONG)procKernel.dwLowDateTime) | (((ULONGLONG)procKernel.dwHighDateTime) << 32);
    ULONGLONG procUserTime = ((ULONGLONG)procUser.dwLowDateTime) | (((ULONGLONG)procUser.dwHighDateTime) << 32);

    // Calculate differences
    ULONGLONG sysIdleDiff =
        sysIdleTime - ((ULONGLONG)lastSysIdle.dwLowDateTime | (((ULONGLONG)lastSysIdle.dwHighDateTime) << 32));
    ULONGLONG sysKernelDiff =
        sysKernelTime - ((ULONGLONG)lastSysKernel.dwLowDateTime | (((ULONGLONG)lastSysKernel.dwHighDateTime) << 32));
    ULONGLONG sysUserDiff =
        sysUserTime - ((ULONGLONG)lastSysUser.dwLowDateTime | (((ULONGLONG)lastSysUser.dwHighDateTime) << 32));
    ULONGLONG procKernelDiff =
        procKernelTime - ((ULONGLONG)lastProcKernel.dwLowDateTime | (((ULONGLONG)lastProcKernel.dwHighDateTime) << 32));
    ULONGLONG procUserDiff =
        procUserTime - ((ULONGLONG)lastProcUser.dwLowDateTime | (((ULONGLONG)lastProcUser.dwHighDateTime) << 32));

    ULONGLONG totalSysTimeDiff = (sysKernelDiff + sysUserDiff);
    ULONGLONG totalProcTimeDiff = (procKernelDiff + procUserDiff);

    if (totalSysTimeDiff > 0)
    {
        cpuUsage = (totalProcTimeDiff * 100.0) / totalSysTimeDiff;
    }
    else
    {
        cpuUsage = 0.0;
    }

    // Update last times
    lastSysIdle = sysIdle;
    lastSysKernel = sysKernel;
    lastSysUser = sysUser;
    lastProcKernel = procKernel;
    lastProcUser = procUser;
}

void GetRamUsage(DWORDLONG &totalMemory, DWORDLONG &freeMemory, DWORDLONG &availableMemory)
{
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStatus);
    totalMemory = memStatus.ullTotalPhys;
    freeMemory = memStatus.ullAvailPhys;
    availableMemory = memStatus.ullAvailPhys;
}

void GetCpuTemperature(float &cpuTemperature)
{
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        std::cerr << "Failed to initialize COM library. Error code = 0x" << std::hex << hres << std::endl;
        return;
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                EOAC_NONE, NULL);

    if (FAILED(hres))
    {
        std::cerr << "Failed to initialize security. Error code = 0x" << std::hex << hres << std::endl;
        CoUninitialize();
        return;
    }

    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void **)&pLoc);

    if (FAILED(hres))
    {
        std::cerr << "Failed to create IWbemLocator object. Error code = 0x" << std::hex << hres << std::endl;
        CoUninitialize();
        return;
    }

    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

    if (FAILED(hres))
    {
        std::cerr << "Could not connect. Error code = 0x" << std::hex << hres << std::endl;
        pLoc->Release();
        CoUninitialize();
        return;
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL,
                             RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

    if (FAILED(hres))
    {
        std::cerr << "Could not set proxy blanket. Error code = 0x" << std::hex << hres << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_TemperatureProbe"),
                           WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    if (FAILED(hres))
    {
        std::cerr << "Query for temperature failed. Error code = 0x" << std::hex << hres << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;
        VariantInit(&vtProp);

        hr = pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr))
        {
            cpuTemperature = (float)(vtProp.uintVal) / 10.0 - 273.15; // Convert from tenths of Kelvin to Celsius
            VariantClear(&vtProp);
        }

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}

int main()
{
    float cpuUsage;
    FILETIME lastSysIdle = {0}, lastSysKernel = {0}, lastSysUser = {0};
    FILETIME lastProcKernel = {0}, lastProcUser = {0};
    DWORDLONG totalMemory;
    DWORDLONG freeMemory;
    DWORDLONG availableMemory;
    // float cpuTemperature;

    // Initial call to setup last times
    GetCpuUsage(cpuUsage, lastSysIdle, lastSysKernel, lastSysUser, lastProcKernel, lastProcUser);

    while (true)
    {
        GetCpuUsage(cpuUsage, lastSysIdle, lastSysKernel, lastSysUser, lastProcKernel, lastProcUser);
        GetRamUsage(totalMemory, freeMemory, availableMemory);
        // GetCpuTemperature(cpuTemperature);

        // Calculate used memory
        DWORDLONG usedMemory = totalMemory - freeMemory;
        double usedMemoryMB = usedMemory / (1024.0 * 1024.0);
        double totalMemoryMB = totalMemory / (1024.0 * 1024.0);
        double freeMemoryMB = freeMemory / (1024.0 * 1024.0);
        double availableMemoryMB = availableMemory / (1024.0 * 1024.0);

        // Print CPU and RAM usage with precision and overwrite previous output
        std::cout << "\rCPU Usage: " << std::fixed << std::setprecision(2) << cpuUsage
                  << "% | RAM Usage: " << std::fixed << std::setprecision(2) << usedMemoryMB
                  << " MB (Used) | Free: " << std::fixed << std::setprecision(2) << freeMemoryMB
                  << " MB | Total: " << std::fixed << std::setprecision(2) << totalMemoryMB << " MB" << std::flush;

        Sleep(1000); // Sleep for 1 second
    }

    return 0;
}
