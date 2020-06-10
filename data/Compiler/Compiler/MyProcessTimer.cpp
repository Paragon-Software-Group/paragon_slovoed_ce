#include "MyProcessTimer.h"

#ifdef _MSC_VER
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <Windows.h>
#  include <Psapi.h>
#  pragma comment(lib, "psapi")
#endif

#include "Tools.h"

#ifdef _MSC_VER
struct CMyProcessTimer::Data
{
	/// Текущий процесс
	HANDLE m_hCurrProcess;

	/// Замеры времени
	FILETIME m_kernel_start;
	FILETIME m_kernel_end;
	FILETIME m_user_start;
	FILETIME m_user_end;

	Data()
	{
		memset(this, 0, sizeof(*this));
		m_hCurrProcess = GetCurrentProcess();
	}
};
#else
struct CMyProcessTimer::Data {};
#endif

// Надо по хорошему переделать обработку времени через 11 стандарт (chrono), тогда будет кросс-платформенно
// Пока специализированно только под Windows т.к. под Linux такой задачи не стояло.
// Под никсами/posix практически полный аналог можно реализовать через getrusage()
// Всю платформо зависимую часть можно вынести в Data (наростив методы и т.п.)

using namespace std;

CMyProcessTimer::CMyProcessTimer() : m_logf(nullptr)
{
#ifndef _LINUX
	m_data.reset(new Data);
#endif
}

CMyProcessTimer::CMyProcessTimer(const char* aLogFileName)
	: CMyProcessTimer()
{
#ifndef _LINUX
	m_logf = fopen(aLogFileName, "wt");
#endif
}

CMyProcessTimer::~CMyProcessTimer(void)
{
#ifndef _LINUX
	if (m_logf)
		fclose(m_logf);
#endif
}

void CMyProcessTimer::SetFilename(string aFilename)
{
#ifndef _LINUX
	if (m_logf)
		fclose(m_logf);

	m_logf = fopen(aFilename.c_str(), "wt");
#endif
}

void CMyProcessTimer::Start(void)
{
#ifndef _LINUX
	FILETIME tmp1, tmp2;
	if (!GetProcessTimes(m_data->m_hCurrProcess, &tmp1, &tmp2, &m_data->m_kernel_start, &m_data->m_user_start))
		fprintf(stderr, "Error %d\n", GetLastError());
#endif
}

void CMyProcessTimer::Stop(void)
{
#ifndef _LINUX
	FILETIME tmp1, tmp2;
	if (!GetProcessTimes(m_data->m_hCurrProcess, &tmp1, &tmp2, &m_data->m_kernel_end, &m_data->m_user_end))
		fprintf(stderr, "Error %d\n", GetLastError());
#endif
}

string CMyProcessTimer::GetTimeString(void)
{
#ifndef _LINUX
	ULARGE_INTEGER kernel_start;
	ULARGE_INTEGER kernel_end;
	ULARGE_INTEGER user_start;
	ULARGE_INTEGER user_end;

	ULARGE_INTEGER kernel;
	ULARGE_INTEGER user;
	FILETIME kernel_filetime;
	FILETIME user_filetime;
	SYSTEMTIME kernel_systime;
	SYSTEMTIME user_systime;

	
	kernel_start.LowPart = m_data->m_kernel_start.dwLowDateTime;
	kernel_start.HighPart = m_data->m_kernel_start.dwHighDateTime;

	kernel_end.LowPart = m_data->m_kernel_end.dwLowDateTime;
	kernel_end.HighPart = m_data->m_kernel_end.dwHighDateTime;

	user_start.LowPart = m_data->m_user_start.dwLowDateTime;
	user_start.HighPart = m_data->m_user_start.dwHighDateTime;

	user_end.LowPart = m_data->m_user_end.dwLowDateTime;
	user_end.HighPart = m_data->m_user_end.dwHighDateTime;

	
	kernel.QuadPart = kernel_end.QuadPart - kernel_start.QuadPart;
	user.QuadPart = user_end.QuadPart - user_start.QuadPart;

	kernel_filetime.dwLowDateTime = kernel.LowPart;
	kernel_filetime.dwHighDateTime = kernel.HighPart;

	user_filetime.dwLowDateTime = user.LowPart;
	user_filetime.dwHighDateTime = user.HighPart;

	FileTimeToSystemTime(&kernel_filetime, &kernel_systime);
	FileTimeToSystemTime(&user_filetime, &user_systime);

	string str("");
	char buf[256] = {0};

	sprintf(buf, "Kernel Time: %02d:%02d:%02d:%03d\n\0", kernel_systime.wHour, kernel_systime.wMinute, kernel_systime.wSecond, kernel_systime.wMilliseconds);
	str += buf;

	sprintf(buf, "User Time: %02d:%02d:%02d:%03d\n\0", user_systime.wHour, user_systime.wMinute, user_systime.wSecond, user_systime.wMilliseconds);
	str += buf;

	sprintf(buf, "Total Time: %02d:%02d:%02d:%03d\n\0", kernel_systime.wHour + user_systime.wHour, kernel_systime.wMinute + user_systime.wMinute, kernel_systime.wSecond + user_systime.wSecond, kernel_systime.wMilliseconds + user_systime.wMilliseconds);
	str += buf;

	return str;
#endif
	return "";
}

void CMyProcessTimer::PrintTimeString(string aMessage)
{
#ifndef _LINUX
	printString(GetTimeString(), aMessage);
#endif
}

void CMyProcessTimer::PrintTimeStringW(wstring aMessage)
{
#ifndef _LINUX
	printString(GetTimeString(), sld::as_utf8(aMessage));
#endif
}

string CMyProcessTimer::GetMemoryString(void)
{
#ifndef _LINUX
	PROCESS_MEMORY_COUNTERS pmc;

	if (!GetProcessMemoryInfo(m_data->m_hCurrProcess, &pmc, sizeof(pmc)))
	{
		fprintf(stderr, "Error %d\n", GetLastError());
		return string("");
	}

	string str("");
	char buf[1024] = {0};

	sprintf(buf, "WorkingSetSize: %u\n\0", pmc.WorkingSetSize);
	str += buf;

	sprintf(buf, "PeakWorkingSetSize: %u\n\0", pmc.PeakWorkingSetSize);
	str += buf;

	sprintf(buf, "PagefileUsage: %u\n\0", pmc.PagefileUsage);
	str += buf;

	sprintf(buf, "PeakPagefileUsage: %u\n\0", pmc.PeakPagefileUsage);
	str += buf;

	return str;
#endif
	return "";
}

void CMyProcessTimer::PrintMemoryString(string aMessage)
{
#ifndef _LINUX
	printString(GetMemoryString(), aMessage);
#endif
}

void CMyProcessTimer::PrintMemoryStringW(wstring aMessage)
{
#ifndef _LINUX
	printString(GetMemoryString(), sld::as_utf8(aMessage));
#endif
}

void CMyProcessTimer::printString(const std::string &aString, const std::string &aMessage)
{
#ifdef _MSC_VER
	if (m_logf)
	{
		if (!aMessage.empty())
			fprintf(m_logf, "%s\n%s\n", aMessage.c_str(), aString.c_str());
		else
			fprintf(m_logf, "%s\n", aString.c_str());
	}
	else
	{
		if (!aMessage.empty())
			printf("%s\n%s\n", aMessage.c_str(), aString.c_str());
		else
			printf("%s\n", aString.c_str());
	}
#endif
}
