// Compiler.cpp : Defines the entry point for the console application.
//

#ifdef _MSC_VER
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#  include <tchar.h>
#  include <fcntl.h>
#  include <io.h>
#endif

#include <wchar.h>

#include "sld_Platform.h"
#include "SlovoEdProject.h"
#include "Compress.h"
#include "MyProcessTimer.h"
#include "Tools.h"
#include "ThreadPool.h"


void PrintHead(void)
{
	sld::printf("SlovoEd 2 Compiler %d bit\n", sizeof(void*)*8);
	sld::printf("Version %d. Build %d.\n", COMPILER_VERSION__NOW, COMPILER_BUILD);
	sld::printf("Copyright Paragon Software (SHDD) 2005-2020\n\n");
}

void PrintUsage(void)
{
	sld::printf("Usage: Compiler.exe filename.sproj [Process Priority] [Resource Compression]\n\n");

	sld::printf("[Process Priority]: -p[1-5]\n");
	sld::printf("\t-p1 - LOW priority\n");
	sld::printf("\t-p2 - BELOW_NORMAL priority (default)\n");
	sld::printf("\t-p3 - NORMAL priority\n");
	sld::printf("\t-p4 - ABOVE_NORMAL priority\n");
	sld::printf("\t-p5 - HIGH priority\n\n");

	sld::printf("[Resource Compression]: -norc\n");
	sld::printf("\t-norc[=lists] Disables resource compression\n");
	sld::printf("\t     Parameters (optional):\n");
	sld::printf("\t       lists - disables list resources compression:\n");
}

static const wchar_t* GetErrorDescription(int aCode)
{
#define wstr(__str) L ## __str
#define error_case(__error) case __error: return wstr( #__error );

	switch (aCode)
	{
	error_case(ERROR_NO)
	error_case(ERROR_WRONG_PROJECT_TAG)
	error_case(ERROR_WRONG_LANGUAGE_FROM)
	error_case(ERROR_WRONG_LANGUAGE_TO)
	error_case(ERROR_WRONG_DICT_ID)
	error_case(ERROR_WRONG_STRING_ATTRIBUTE)
	error_case(ERROR_EMPTY_TAG)
	error_case(ERROR_CANT_OPEN_FILE)
	error_case(ERROR_WRONG_TAG)
	error_case(ERROR_UNKNOWN_TAG)
	error_case(ERROR_WRONG_INDEX)
	error_case(ERROR_CANT_SORT_HIERARCHY)
	error_case(ERROR_NULL_POINTER)
	error_case(ERROR_ARTICLE_NOT_FOUND)
	error_case(ERROR_WRONG_SORT_FILE)
	error_case(ERROR_TOO_LARGE_PREFIX)
	error_case(ERROR_DUPLICATED_ARTICLE_ID)
	error_case(ERROR_DUPLICATED_LIST_ENTRY_ID)
	error_case(ERROR_WRONG_BINARY_DATA_SIZE)
	error_case(ERROR_WRONG_ARTICLES_COUNT)
	error_case(ERROR_WRONG_COMPRESSION_METHOD)
	case ERROR_TOO_LARGE_ARTICLES_DATA:
		return L"ERROR_TOO_LARGE_ARTICLES_DATA\nPlease use more aggressive compression method!";
	error_case(ERROR_NO_ARTICLE)
	error_case(ERROR_WRONG_STYLE_COUNT)
	error_case(SDC_MEM_ERRORS)
	error_case(SDC_MEM_NOT_ENOUGH_MEMORY)
	error_case(SDC_MEM_NULL_POINTER)
	error_case(SDC_WRITE_ERRORS)
	error_case(SDC_WRITE_EMPTY_RESOURCE)
	error_case(SDC_WRITE_ALREADY_EXIST)
	error_case(SDC_WRITE_CANT_CREATE_FILE)
	error_case(SDC_WRITE_CANT_WRITE)
	error_case(SDC_READ_ERRORS)
	error_case(SDC_READ_CANT_OPEN_FILE)
	error_case(SDC_READ_CANT_READ)
	error_case(SDC_READ_WRONG_SIGNATURE)
	error_case(SDC_READ_WRONG_INDEX)
	error_case(SDC_READ_CANT_POSITIONING)
	error_case(SDC_READ_RESOURCE_NOT_FOUND)
	error_case(SDC_READ_NOT_OPENED)
	error_case(SDC_READ_WRONG_FILESIZE)
	error_case(SDC_READ_WRONG_CRC)
	error_case(ERROR_WRONG_LIST_TYPE)
	error_case(ERROR_WRONG_TAG_CONTENT)
	error_case(ERROR_WRONG_STYLE_VARIANTS_COUNT)
	error_case(ERROR_WRONG_STYLE_USAGES_COUNT)
	error_case(ERROR_UNKNOWN_MEDIA_SOURCE_TYPE)
	error_case(ERROR_WRONG_SYMBOL_TABLE_FILE_FORMAT)
	error_case(ERROR_WRONG_SORT_TABLE_FILE_FORMAT)
	error_case(ERROR_WRONG_FULL_SEARCH_LIST_LINK_TYPE)
	error_case(ERROR_DUPLICATED_LIST_ID)
	error_case(ERROR_WRONG_FULL_SEARCH_LIST_SOURCE_LIST_ID)
	error_case(ERROR_WRONG_FULL_SEARCH_LIST_SOURCE_LIST_VARIANTS)
	error_case(ERROR_NULL_SORT_TABLE)
	error_case(ERROR_WRONG_NUMBER_OF_STATES_IN_SWITCH_BLOCKS)
	error_case(ERROR_WRONG_THEMATIC_IN_SWITCH_BLOCK)
	error_case(ERROR_WRONG_MANAGE_ATTRIBUTE_VALUE_IN_SWITCH_BLOCK)
	error_case(ERROR_ILLEGAL_ATTRIBUTE_VALUE_IN_SWITCH_BLOCK)
	error_case(ERROR_DUPLICATED_SWITCH_BLOCK_LABEL)
	error_case(ERROR_NON_EXISTING_SWITCH_BLOCK_LABEL)
	error_case(ERROR_NOT_ENOUGH_ATTRIBUTES_IN_SWITCH_BLOCK)
	error_case(ERROR_WRONG_METADATA_PARAMETER)
	error_case(ERROR_EMPTY_WL_NAME)
	error_case(ERROR_NO_KES_ID)
	error_case(ERROR_TO_LARGE_STYLE_NAME)
	error_case(ERROR_EMPTY_ARTICLE_ID)
	error_case(ERROR_WRONG_AUXILIARY_LINK_TYPE)
	error_case(ERROR_EMPTY_SHOW_VARIANT)
	default:
		return L"UNKNOWN_ERROR";
	}

#undef error_case
#undef wstr
}

enum class ResourceCompression
{
	Default,
	None,
	Forced
};

static const char* to_string(ResourceCompression compression)
{
	switch (compression)
	{
	case ResourceCompression::Default: return "default";
	case ResourceCompression::None:    return "none";
	case ResourceCompression::Forced:  return "forced";
	}
	return "UNKNOWN";
}

static void ErrorOut(int aCode)
{
	CLogW::PrintToConsole("[FAIL]", eLogStatus_Error);
	STString<1024> wbuf(L"SlovoEdProject error (%d) - %s", aCode, GetErrorDescription(aCode));
	sldILog("%s\n", wbuf);
	sld::printf("\n%s\n", wbuf);
	std::exit(aCode);
}

#ifdef _MSC_VER
// Обработчик необрабатываемых исключений
LONG WINAPI MyCustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
	// выход из программы
	sld::printf("\nUnknown system error! Terminating program!\n");
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#ifdef _MSC_VER
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	// Устанавливаем свой обработчик необрабатываемых исключений
#ifdef _MSC_VER
	LPTOP_LEVEL_EXCEPTION_FILTER hOldUnhandledFilter = SetUnhandledExceptionFilter(MyCustomUnhandledExceptionFilter);

	_setmode(_fileno(stdout), _O_U16TEXT);
#endif

	PrintHead();

	if (argc < 2)
	{
		PrintUsage();
		return 0;
	}

	// Переопределенный метод сжатия
	wstring RedefinedCompressionMethod(L"");
	// Настройка сжатия ресурсов, по дефолту включена
	ResourceCompression resourceCompression = ResourceCompression::Default;
	CompressConfig compConfig;

#ifdef _MSC_VER
	// Приоритет процесса по-умолчанию
	DWORD ProcessPriority = BELOW_NORMAL_PRIORITY_CLASS;
	
	// Обработка параметров командной строки
	for (int i=2;i<argc;i++)
	{
		sld::wstring_ref arg(argv[i]);
		if (_wcsicmp(argv[i], L"-p1") == 0)
			ProcessPriority = IDLE_PRIORITY_CLASS;
		else if (_wcsicmp(argv[i], L"-p2") == 0)
			ProcessPriority = BELOW_NORMAL_PRIORITY_CLASS;
		else if (_wcsicmp(argv[i], L"-p3") == 0)
			ProcessPriority = NORMAL_PRIORITY_CLASS;
		else if (_wcsicmp(argv[i], L"-p4") == 0)
			ProcessPriority = ABOVE_NORMAL_PRIORITY_CLASS;
		else if (_wcsicmp(argv[i], L"-p5") == 0)
			ProcessPriority = HIGH_PRIORITY_CLASS;
		else if (arg == L"-norc")
		{
			resourceCompression = ResourceCompression::None;
			compConfig.setResourceCompression(CSDCWrite::NoCompression);
		}
		else if (arg.starts_with(L"-norc=") && arg.length() > wcslen(L"-norc="))
		{
			arg.remove_prefix(wcslen(L"-norc="));
			if (arg == L"lists")
				compConfig.setCanCompressListResources(false);
		}
	}
	
	// Установка приоритета процесса
	HANDLE hProc = GetCurrentProcess();
	if (!SetPriorityClass(hProc, ProcessPriority))
	{
		sld::printf("Error! Can't change process priority!\n");
		return 1;
	}
#endif

	ThreadPool threadPool;

	// Создаем проект
	SlovoEdProject proj;

	CMyProcessTimer timerCommon("CommonTime.time");
	timerCommon.PrintMemoryString("Memory Usage On Startup");

	// Загружаем проект
	timerCommon.Start();

	//LinuxFix Check
	wstring file1;
#ifdef _MSC_VER
	file1 = argv[1];
#else
	file1 = sld::as_wide(argv[1]);
#endif
	sld::printf("Loading project `%s`...", sld::as_ref(file1));

	int error = proj.Load(file1);
	if (error != ERROR_NO)
		ErrorOut(error);

	// Загружаем информацию о версии словарной базы
	sld::printf("\nLoading version info...");
	error = proj.LoadVersionInfo(proj.GetVersionInfoFilename());
	if (error != ERROR_NO)
		ErrorOut(error);
	CLogW::PrintToConsole("[OK]");

	timerCommon.Stop();
	timerCommon.PrintTimeString("Loading Project");
	timerCommon.PrintMemoryString("Memory Usage After Loading Project");

	// Переопределяем метод сжатия
	if (!RedefinedCompressionMethod.empty())
	{
		sld::printf("\nRedefine compression method to `%s`...", sld::as_ref(RedefinedCompressionMethod));
		error = proj.SetCompressionMethod(RedefinedCompressionMethod);
		if (error != ERROR_NO)
			ErrorOut(error);
		CLogW::PrintToConsole("[OK]");
		sld::printf("\n");
	}

	sld::printf("\nResource compression: %s", to_string(resourceCompression));

	// Сжимаем
	timerCommon.Start();
	sld::printf("\n\nCompressing...");
	error = Compress(threadPool, &proj, compConfig);
	if (error != ERROR_NO)
		ErrorOut(error);
	sld::printf("\nCompressing complete");
	CLogW::PrintToConsole("[OK]");

	timerCommon.Stop();
	timerCommon.PrintTimeString("Compressing");
	timerCommon.PrintMemoryString("Memory Usage After Compressing");

	// Сохраняем информацию о версии словарной базы
	sld::printf("\n\nSaving version info...");
	error = proj.SaveVersionInfo(proj.GetVersionInfoFilename());
	if (error != ERROR_NO)
		ErrorOut(error);
	CLogW::PrintToConsole("[OK]");
	sld::printf("\n");

#ifdef _MSC_VER
	// Восстанавливаем прежний обработчик необрабатываемых исключений
	SetUnhandledExceptionFilter(hOldUnhandledFilter);
#endif

	return 0;
}
