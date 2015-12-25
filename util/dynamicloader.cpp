#include "dynamicloader.h"

#include <cstdio>

MODULE_HANDLE gdl_Open(const char *plname)
{
#if defined(_WIN32_PLATFROM_)
	return LoadLibraryA (plname);
#endif

#if defined(_LINUX_PLATFROM_)
	return dlopen( plname, RTLD_NOW|RTLD_GLOBAL);
#endif
}


void gdl_Close(MODULE_HANDLE h)
{
	if(h)
	{
#if defined(_WIN32_PLATFROM_)
		FreeLibrary(h);
#endif
#if defined(_LINUX_PLATFROM_)
		dlclose (h);
#endif
	}
}

void * gdl_GetProc(MODULE_HANDLE h, const char *pfname)
{
	if(h)
	{
#if defined(_WIN32_PLATFROM_)
		return (void *)GetProcAddress(h, pfname);
#endif

#if defined(_LINUX_PLATFROM_)
		return dlsym(h,pfname);
#endif
	}
	return NULL;
}

void gdl_GetLastErrorMsg(char *p, int size)
{
#if defined(_WIN32_PLATFROM_)
	sprintf(p, "dll error(%u)",::GetLastError());
#endif

#if defined(_LINUX_PLATFROM_)
	sprintf(p, "%s",dlerror());
#endif
}


CDynamicLibrary::CDynamicLibrary()
{

	m_hModule = NULL;
}

CDynamicLibrary::~CDynamicLibrary()
{
	gdl_Close(m_hModule);
}

bool CDynamicLibrary::Open(const char *lpname)
{
	m_strDllName = lpname;
	m_hModule = gdl_Open(lpname);
	return m_hModule!=NULL;
}

void * CDynamicLibrary::GetProc(const char *pfname)
{
	return gdl_GetProc(m_hModule, pfname);
}

const char * CDynamicLibrary::GetLastErrorMsg()
{
	char msg[100];
	gdl_GetLastErrorMsg(msg, 99);
	m_strMsg = msg;
	return m_strMsg.c_str();
}
