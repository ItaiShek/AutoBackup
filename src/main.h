// main.h
#ifndef MAIN_H
#define MAIN_H

// include libraries
#include <windows.h>
#include <Dbt.h>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>

// function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam);
wchar_t FirstDriveFromMask(DWORD unitmask);
int backup(wchar_t wcFirstLetter, int i, int j);
int readFiles();
int countArgs(std::wstring str);
bool isHexNotation(std::wstring const& str);

// global variables
std::vector<std::wstring> g_wsBackupList{};
std::vector<DWORD> g_wsSerialNumbersList{};
std::vector<std::wstring> g_wsVolumePathsList{};
int g_saveOriginal{ 1 };

#endif 