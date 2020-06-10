#pragma once

#include <string>

#include "format.h"

/// ������������, ����������� ������ ������ ��������� �� �������
/**
	NoLog		- �� ������� ��������� �� ������� ������
	File		- ������� ��������� �� ������� � ����
	Screen		- ������� ��������� �� ������� �� �����
*/
enum class ELogType { NoLog, File, Screen, Default = File };

/// ������������ ��������� �������� ��������� � ���
enum ELogStatusLevel
{
	/// ��������� �� �������� ��������
	eLogStatus_OK = 0,
	/// ��������������
	eLogStatus_Warning,
	/// ������
	eLogStatus_Error
};


// ********************************************************************************************************** //

/// ����� ��� ������ ��������� �� ������� (������)
class CLogW
{
public:
	CLogW(const CLogW&) = delete;
	CLogW& operator=(const CLogW&) = delete;

	static CLogW& InstanceLog();
	static CLogW& InstanceXmlLog();

	/// ������������� ��� ����� ��� ������ ��������� �� �������
	void SetLogFileName(std::wstring LogFileName);

	/// ������������� ������ ������ ��������� �� �������
	void SetLogType(ELogType vLogType);

	/// ������� ��������� �� ������
	void Log(fmt::CStringRef aFormat, fmt::ArgList aArgs);
	FMT_VARIADIC(void, Log, fmt::CStringRef)

	/// �������� ���-��, ���������� ������ ������� ����
	void PrintStartLine(void);

	/// �������� ������ ���� �� �����, ����������� �� �������/������ ���� �������, ������������ ������, ������� ������� �� aLevel
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
