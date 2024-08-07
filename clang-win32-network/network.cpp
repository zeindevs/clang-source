#include <Wbemidl.h>
#include <chrono>
#include <comdef.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

void QueryNetworkBandwidth()
{
    // Initialize COM.
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        std::cerr << "Failed to initialize COM library. Error code = 0x" << std::hex << hres << std::endl;
        return;
    }

    // Set general COM security levels.
    hres = CoInitializeSecurity(NULL,
                                -1,                          // COM authentication
                                NULL,                        // Authentication services
                                NULL,                        // Reserved
                                RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
                                RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
                                NULL,                        // Authentication info
                                EOAC_NONE,                   // Additional capabilities
                                NULL                         // Reserved
    );

    if (FAILED(hres))
    {
        std::cerr << "Failed to initialize security. Error code = 0x" << std::hex << hres << std::endl;
        CoUninitialize();
        return;
    }

    // Obtain the initial locator to WMI.
    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);

    if (FAILED(hres))
    {
        std::cerr << "Failed to create IWbemLocator object. "
                  << "Error code = 0x" << std::hex << hres << std::endl;
        CoUninitialize();
        return;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method.
    IWbemServices *pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
                               NULL,                    // User name. NULL = current user
                               NULL,                    // User password. NULL = current
                               0,                       // Locale. NULL indicates current
                               NULL,                    // Security flags.
                               0,                       // Authority (for example, Kerberos)
                               0,                       // Context object
                               &pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
    {
        std::cerr << "Could not connect. Error code = 0x" << std::hex << hres << std::endl;
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Set security levels on the proxy.
    hres = CoSetProxyBlanket(pSvc,                        // Indicates the proxy to set
                             RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
                             RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
                             NULL,                        // Server principal name
                             RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
                             RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
                             NULL,                        // client identity
                             EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hres))
    {
        std::cerr << "Could not set proxy blanket. Error code = 0x" << std::hex << hres << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    while (true)
    {
        // Use the IWbemServices pointer to make requests of WMI.
        // For example, get the name of the operating system.
        IEnumWbemClassObject *pEnumerator = NULL;
        hres = pSvc->ExecQuery(
            bstr_t("WQL"), bstr_t("SELECT Name, CurrentBandwidth FROM Win32_PerfFormattedData_Tcpip_NetworkInterface"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

        if (FAILED(hres))
        {
            std::cerr << "Query for network interfaces failed."
                      << " Error code = 0x" << std::hex << hres << std::endl;
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return;
        }

        // Get the data from the query.
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

            // Get the value of the Name property.
            hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
            std::wcout << "Name : " << vtProp.bstrVal << std::endl;
            VariantClear(&vtProp);

            // Get the value of the CurrentBandwidth property.
            hr = pclsObj->Get(L"CurrentBandwidth", 0, &vtProp, 0, 0);
            std::wcout << "CurrentBandwidth : " << vtProp.uintVal << " bits per second" << std::endl;
            VariantClear(&vtProp);

            pclsObj->Release();
        }

        pEnumerator->Release();

        // Wait for 5 seconds before the next query
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
}

int main()
{
    QueryNetworkBandwidth();
    return 0;
}
