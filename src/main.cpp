/*  
*   (c) Itai Shek 2022
*   https://GitHub.com/ItaiShek/AutoBackup
*   Scheduled task to backup files on an external hard drive.
*   Requires C++ 17 or grater.
*/

#include "main.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Parse arguments
    int wArgc{};
    LPWSTR* wArgv = CommandLineToArgvW(GetCommandLine(), &wArgc);

    if (wArgv == NULL)
    {
        return 0;
    }

    for (int i{ 1 }; i < wArgc; i++)
    {
        size_t len{ wcslen(wArgv[i]) };

        if (_wcsnicmp(wArgv[i], L"nocopy", len) == 0)
        {
            g_saveOriginal = 0;
			break;
        }
    }
    
    // The default is to create a copy of the file, and update existing if it's older then
    // the current file on the system.
    const wchar_t CLASS_NAME[] = L"Backup Window Class";

    WNDCLASS wc{};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    RegisterClass(&wc);
    HWND hwnd = CreateWindow(
        CLASS_NAME,                     // Window class
        L"Backup KPXC",                 // Window text
        NULL,                           // Window style
        0, 0,                           // position
        0, 0,                           // size
        NULL,                           // Parent window    
        NULL,                           // Menu
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }
    
    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
}

// window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        if (readFiles())
        {
            PostQuitMessage(0);
        }
        break;
    case WM_DEVICECHANGE:
        OnDeviceChange(wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }

    return 0;
}

// Check if the device is one of the storage devices we want to use
// return value: 0 - success, 1 - failure
LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
    // Check whether the message is a device arrival message
    if (wParam != DBT_DEVICEARRIVAL)
    {
        return 1;
    }

    // Get a pointer to the structure header
    PDEV_BROADCAST_HDR devHdr = (PDEV_BROADCAST_HDR)lParam;

    // Check whether the message is about a device volume
    if (devHdr->dbch_devicetype != DBT_DEVTYP_VOLUME)
    {
        return 1;
    }

    // Get a pointer to the device volume structure
    PDEV_BROADCAST_VOLUME devVolume = (PDEV_BROADCAST_VOLUME)devHdr;

    std::wstring wsRootPathName{ L"X:\\" };     // device path, e.g.    "F:\"
    wchar_t wcFirstLetter = FirstDriveFromMask(devVolume->dbcv_unitmask);
    wsRootPathName[0] = wcFirstLetter;
    
    // get the serial number of the device
    DWORD dwSerialNumber{};

    if (GetVolumeInformation(wsRootPathName.data(), NULL, NULL, &dwSerialNumber, NULL, NULL, NULL, NULL) == 0)    
    {
        return 1;
    }

    // loop through the serial numbers and compare to the device's SN
    int i{};
    int result{};
    for (const auto& serialNumber : g_wsSerialNumbersList)
    {
        // device's serial number found - create a backup on the device
        if (serialNumber == dwSerialNumber)
        {
            for (int j{}; j < g_wsBackupList.size(); j++)
            {
                result |= backup(wcFirstLetter, i, j);
            }
        }
        i++;
    }

    return result;
}


// Find drive letter from a mask of drive letters
// The mask must be in the format bit 0 = A, bit 1 = B, bit 2 = C, etc...
wchar_t FirstDriveFromMask(DWORD unitmask)
{
    wchar_t i{};
    
    while (unitmask > 1)
    {
        i++;
        unitmask >>= 1;
    }

    return(i + L'A');
}

// backup the file from the system on the external drive
// return value: 0 - success, 1 - failure
int backup(wchar_t wcFirstLetter, int i, int j)
{
    // check if the file we want to backup exist
    std::filesystem::path p{ g_wsBackupList[j]};
    if (std::filesystem::exists(p) == false)
    {
        return 1;
    }

    // get the file's name from path
    std::wstring fileName{ p.filename() };

    // check if the directory exists on the external drive
    g_wsVolumePathsList[i][0] = { wcFirstLetter };
    p = { g_wsVolumePathsList[i] };
    if (std::filesystem::exists(p) == false || std::filesystem::is_directory(p) == false)
    {
        return 1;
    }
    
    if (g_wsVolumePathsList[i].back() != L'\\')
    {
        g_wsVolumePathsList[i] += L'\\';
    }

    // backup
    const auto copyOptions{ std::filesystem::copy_options::update_existing | std::filesystem::copy_options::recursive };
    
    std::wstring wsBackupFile{ g_wsVolumePathsList[i] + fileName };
        
    // check if there's already a backup file
    p = { wsBackupFile };
    if (g_saveOriginal && std::filesystem::exists(p))
    {
        // if the file already exists then create another backup
        std::wstring wsOriginal{ g_wsVolumePathsList[i] + L"original_" + fileName };        
        std::filesystem::copy(wsBackupFile, wsOriginal, copyOptions);
    }

    std::filesystem::copy(g_wsBackupList[j], wsBackupFile, copyOptions);    

    return 0;
}

// read paths and serial numbers to global variables
// return value: 0 - success, 1 - failure
int readFiles()
{
    // read files to backup
    std::wifstream is("BackupList.txt");
    if (is.is_open() == false)
    {
        return 1;
    }

    std::wstring curLine{};
    while (std::getline(is, curLine))
    {
        if (curLine.empty())
        {
            continue;
        }
        g_wsBackupList.push_back(curLine);
    }
    is.close();
    
    if (g_wsBackupList.empty())
    {
        return 1;
    }

    // read volumes serial numbers and backup directories
    is.open("VolumeList.txt");
    if (is.is_open() == false)
    {
        return 1;
    }
    
    while (std::getline(is, curLine))
    {
        if (curLine.empty() || (countArgs(curLine) != 2))
        {
            continue;
        }
        
        std::wstringstream wss(curLine);
        std::wstring wsTemp{};
        // SN
        if (std::getline(wss, wsTemp, L';'))
        {
            // check if it's a number
            wchar_t* p;
            int base = 10;
            if (isHexNotation(wsTemp))
            {
                base = 16;
            }
            DWORD curSN = std::wcstoul(wsTemp.c_str(), &p, base);
            if (*p != 0)
            {
                continue;
            }
            g_wsSerialNumbersList.push_back(curSN);
        }

        // Path
        if (std::getline(wss, wsTemp))
        {
            g_wsVolumePathsList.push_back(wsTemp);
        }
    }
    is.close();
    

    if (g_wsSerialNumbersList.empty() || g_wsVolumePathsList.empty())
    {
        return 1;
    }

    if (g_wsSerialNumbersList.size() != g_wsVolumePathsList.size())
    {
        return 1;
    }

    return 0;
}

// return the number of arguments separated by semicolon from a string
int countArgs(std::wstring str)
{
    str.erase(std::remove(str.begin(), str.end(), L' '), str.end());
    std::replace(str.begin(), str.end(), L';', L' ');
    std::wstringstream wss(str);

    return std::distance(std::istream_iterator<std::wstring, wchar_t>(wss), std::istream_iterator<std::wstring, wchar_t>());    
}

// check if wstring is a hex number
bool isHexNotation(std::wstring const& str)
{
    return (str.compare(0, 2, L"0x") == 0
        && str.size() > 2
        && str.find_first_not_of(L"0123456789abcdefABCDEF", 2) == std::wstring::npos);
}