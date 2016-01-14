#pragma once
#include <string>


template<typename FunT>
FunT* GetSharedLibFun(const char* libraray, const char* fun);


#if defined(_BL_WIN32_PLATFROM_)
#include <windows.h>
typedef HMODULE  MODULE_HANDLE;
#endif

#if defined(_BL_LINUX_PLATFROM_)
#include <dlfcn.h>
typedef void *  MODULE_HANDLE;
#endif


MODULE_HANDLE gdl_Open(const char *plname);
void gdl_Close(MODULE_HANDLE h);
void *gdl_GetProc(MODULE_HANDLE h, const char *pfname);
void gdl_GetLastErrorMsg(char *p, int size);

class CDynamicLibrary
{
public:
    CDynamicLibrary();
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
FunT* GetSharedLibFun(const char* libraray, const char* fun)
{
    // let it leak.
    CDynamicLibrary* dllloader =  new CDynamicLibrary;
    dllloader->Open(libraray);
    return reinterpret_cast<FunT*>(dllloader->GetProc(fun));
}