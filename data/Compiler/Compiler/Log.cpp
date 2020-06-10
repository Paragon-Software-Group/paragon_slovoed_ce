#include "Log.h"

#include "stdio.h"

#ifdef _MSC_VER
#  define NOMINMAX
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#include "sld_Platform.h"
#include "Tools.h"

using namespace std;

struct CLogW::Private
{
	/// »м€ файла дл€ вывода сообщений об ошибках
	std::wstring logFileName;

	/// ‘лаг, определ€ющий способ вывода сообщений об ошибках
	ELogType logType;

	/// ‘айл куда вывод€тс€ сообщени€ об ошибках
	FILE *logFile;

	Private() : logType(ELogType::Default), logFile(nullptr) {}

	~Private() { closeFile(); }

	void closeFile()
	{
		if (logFile)
			fclose(logFile);
		logFile = nullptr;
	}
};

CLogW& CLogW::InstanceXmlLog()
{
	static CLogW item;
	return item;
}

CLogW& CLogW::InstanceLog()
{
	static CLogW item;
	return item;
}

CLogW::CLogW() : priv(new Private)
{
	assert(priv);
}

CLogW::~CLogW()
{
	if (priv)
		delete priv;
}

/**
	ѕечатает что-то, означающее начало ведени€ лога
*/
void CLogW::PrintStartLine(void)
{
	Log("----------------------------------------------------------------\n");
}

#ifdef _MSC_VER

static HANDLE getConsoleInfo(CONSOLE_SCREEN_BUFFER_INFO *aInfo)
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!hOut || hOut == INVALID_HANDLE_VALUE)
		return INVALID_HANDLE_VALUE;

	if (!GetConsoleScreenBufferInfo(hOut, aInfo))
		return INVALID_HANDLE_VALUE;
	return hOut;
}

static UInt16 getConsoleAttribs(ELogStatusLevel aLevel)
{
	static const UInt16 attribs[] = {
		FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED,
		FOREGROUND_INTENSITY | FOREGROUND_RED,
	};
	return aLevel < sld::array_size(attribs) ? attribs[aLevel] : FOREGROUND_INTENSITY;
}

#endif

/// ѕечатает строку лога на экран, определенным цветом, который зависит от aLevel
void CLogW::PrintToConsole(const char *aText, ELogStatusLevel aLevel)
{
#ifdef _MSC_VER
	CONSOLE_SCREEN_BUFFER_INFO coInfo;
	HANDLE hOut = getConsoleInfo(&coInfo);
	if (hOut != INVALID_HANDLE_VALUE)
	{
		Int16 width = coInfo.dwSize.X - coInfo.dwCursorPosition.X - 1;
		sld::printf(aLevel, "%*s", width, aText);
	}
	else
	{
		sld::printf(aLevel, "%s", aText);
	}
#else
	// TODO: Add alignment
	sld::printf(aLevel, "%s", aText);
#endif
}

/**
	”станавливает им€ файла дл€ вывода сообщений об ошибках

	@param LogFileName	- им€ файла
*/
void CLogW::SetLogFileName(wstring LogFileName)
{
	if (LogFileName == priv->logFileName)
		return;

	priv->logFileName = LogFileName;
	priv->closeFile();
}

/**
	”станавливает способ вывода сообщений об ошибках

	@param vLogType	-	флаг, определ€ющий способ вывода
	@see TLogType
*/
void CLogW::SetLogType(ELogType vLogType)
{
	if (vLogType == priv->logType)
		return;

	if (priv->logType == ELogType::File)
		priv->closeFile();

	priv->logType = vLogType;
}

void CLogW::Log(fmt::CStringRef aFormat, fmt::ArgList aArgs)
{
	if (priv->logType == ELogType::File)
	{
		if (!priv->logFile)
			priv->logFile = sld::fopen(priv->logFileName.c_str(), L"a");

		if (priv->logFile)
		{
			fmt::fprintf(priv->logFile, aFormat, std::move(aArgs));
			::fflush(priv->logFile);
		}
	}
	else if (priv->logType == ELogType::Screen)
	{
		sld::printf(aFormat, std::move(aArgs));
	}
}

void sld::printf(detail::cstring_ref aFormat, fmt::ArgList aArgs)
{
	thread_local fmt::MemoryWriter w;

	w.clear();
	fmt::printf(w, ::fmt::CStringRef(aFormat.c_str()), std::move(aArgs));

#ifdef _MSC_VER
	thread_local std::wstring wstr;
	sld::as_wide(sld::string_ref(w.c_str(), w.size()), wstr);
	std::fputws(wstr.c_str(), stdout);
#else
	std::fputs(w.c_str(), stdout);
#endif
}

void sld::printf(ELogStatusLevel aLevel, detail::cstring_ref aFormat, fmt::ArgList aArgs)
{
#ifdef _MSC_VER
	CONSOLE_SCREEN_BUFFER_INFO coInfo;
	HANDLE hOut = getConsoleInfo(&coInfo);

	if (hOut == INVALID_HANDLE_VALUE)
	{
		sld::printf(aFormat, std::move(aArgs));
		return;
	}

	// ÷вет текста в консоли
	const UInt16 newAttribs = getConsoleAttribs(aLevel);

	const UInt16 oldAttribs = coInfo.wAttributes;
	SetConsoleTextAttribute(hOut, newAttribs);
	sld::printf(aFormat, std::move(aArgs));
	SetConsoleTextAttribute(hOut, oldAttribs);
#else
	static const char *const colors[] = { "\x1B[32m", "\x1B[33m", "\x1B[31m" };
	const char *const color = aLevel < sld::array_size(colors) ? colors[aLevel] : "";
	std::fputs(color, stdout);
	sld::printf(aFormat, std::move(aArgs));
	std::fputs("\033[0m", stdout);
#endif
}
