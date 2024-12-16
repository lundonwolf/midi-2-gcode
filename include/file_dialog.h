#pragma once
#include <string>
#include <windows.h>
#include <shobjidl.h> 

class FileDialog {
public:
    static std::string OpenFile(const char* filter = "All Files\0*.*\0MIDI Files\0*.mid\0", HWND owner = NULL) {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr))
            return "";

        IFileOpenDialog *pFileOpen;
        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
                            IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            hr = pFileOpen->Show(owner);
            if (SUCCEEDED(hr)) {
                IShellItem *pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        std::wstring ws(pszFilePath);
                        std::string result(ws.begin(), ws.end());
                        CoTaskMemFree(pszFilePath);
                        pItem->Release();
                        pFileOpen->Release();
                        CoUninitialize();
                        return result;
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        }
        CoUninitialize();
        return "";
    }

    static std::string SaveFile(const char* filter = "All Files\0*.*\0G-code Files\0*.gcode\0", HWND owner = NULL) {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr))
            return "";

        IFileSaveDialog *pFileSave;
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, 
                            IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

        if (SUCCEEDED(hr)) {
            hr = pFileSave->Show(owner);
            if (SUCCEEDED(hr)) {
                IShellItem *pItem;
                hr = pFileSave->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                    if (SUCCEEDED(hr)) {
                        std::wstring ws(pszFilePath);
                        std::string result(ws.begin(), ws.end());
                        CoTaskMemFree(pszFilePath);
                        pItem->Release();
                        pFileSave->Release();
                        CoUninitialize();
                        return result;
                    }
                    pItem->Release();
                }
            }
            pFileSave->Release();
        }
        CoUninitialize();
        return "";
    }
};
