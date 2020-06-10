#pragma once

#include <string>

#include "format.h"

/// Перечисление, описывающее способ вывода сообщений об ошибках
/**
	NoLog		- не выводим сообщения об ошибках вообще
	File		- выводим сообщения об ошибках в файл
	Screen		- выводим сообщения об ошибках на экран
*/
enum class ELogType { NoLog, File, Screen, Default = File };

/// Перечисление возможных статусов сообщений в лог
enum ELogStatusLevel
{
	/// Сообщение об успешной операции
	eLogStatus_OK = 0,
	/// Предупреждение
	eLogStatus_Warning,
	/// Ошибка
	eLogStatus_Error
};


// ********************************************************************************************************** //

/// Класс для вывода сообщений об ошибках (юникод)
class CLogW
{
public:
	CLogW(const CLogW&) = delete;
	CLogW& operator=(const CLogW&) = delete;

	static CLogW& InstanceLog();
	static CLogW& InstanceXmlLog();

	/// Устанавливает имя файла для вывода сообщений об ошибках
	void SetLogFileName(std::wstring LogFileName);

	/// Устанавливает способ вывода сообщений об ошибках
	void SetLogType(ELogType vLogType);

	/// Выводит сообщение об ошибке
	void Log(fmt::CStringRef aFormat, fmt::ArgList aArgs);
	FMT_VARIADIC(void, Log, fmt::CStringRef)

	/// Печатает что-то, означающее начало ведения лога
	void PrintStartLine(void);

	/// Печатает строку лога на экран, выравненную по правому/левому краю консоли, определенным цветом, который зависит от aLevel
	static void PrintToConsole(const char *aText, ELogStatusLevel aLevel = eLogStatus_OK);

private:
	CLogW();
	~CLogW();

	struct Private;
	Private *priv;
};

template <typename... Args>
static inline void sldILog(fmt::CStringRef aFormat, const Args&... aArgs) {
	CLogW::InstanceLog().Log(aFormat, aArgs...);
}

template <typename... Args>
static inline void sldXLog(fmt::CStringRef aFormat, const Args&... aArgs) {
	CLogW::InstanceXmlLog().Log(aFormat, aArgs...);
}

namespace sld {
namespace detail {
// required to resolve argument dependant lookup between fmt::printf and out own
class cstring_ref {
private:
	const char *data_;
public:
	cstring_ref(const char *s) : data_(s) {}
	cstring_ref(::fmt::CStringRef s) : data_(s.c_str()) {}
	const char *c_str() const { return data_; }
};
} // namespace detail

void printf(detail::cstring_ref aFormat, fmt::ArgList aArgs);
FMT_VARIADIC(void, printf, detail::cstring_ref)

void printf(ELogStatusLevel aLevel, detail::cstring_ref aFormat, fmt::ArgList aArgs);
FMT_VARIADIC(void, printf, ELogStatusLevel, detail::cstring_ref)

} // namespace sld
