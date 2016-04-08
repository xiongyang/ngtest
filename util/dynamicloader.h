#pragma once
#include <string>
#include <memory>


#if defined(_BL_WIN32_PLATFROM_)
#include <windows.h>
typedef HMODULE  MODULE_HANDLE;
#endif

#if defined(_BL_LINUX_PLATFROM_)
#include <dlfcn.h>
typedef void *  MODULE_HANDLE;
#endif

class CDynamicLibrary
{
public:
    //  explicit CDynamicLibrary(const char *lpname);
    explicit CDynamicLibrary(const std::string& lpname);
    ~CDynamicLibrary();
    bool Open(const char *lpname);
    void *GetProc(const char *pfname);
    const char *GetLastErrorMsg();


private:
    std::string m_strMsg;
    std::string m_strDllName;
    MODULE_HANDLE m_hModule;
};


template<typename FunT>
FunT* GetSharedLibFun(CDynamicLibrary* dllloader, const char* fun);


MODULE_HANDLE gdl_Open(const char *plname);
void gdl_Close(MODULE_HANDLE h);
void *gdl_GetProc(MODULE_HANDLE h, const char *pfname);
void gdl_GetLastErrorMsg(char *p, int size);


template<typename FunT>
FunT* GetSharedLibFun(CDynamicLibrary* dllloader, const char* fun)
{
    return reinterpret_cast<FunT*>(dllloader->GetProc(fun));
}