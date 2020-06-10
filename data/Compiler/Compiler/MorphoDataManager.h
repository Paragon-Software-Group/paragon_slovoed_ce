#pragma once

#include <map>
#include <vector>

#include "sld_Platform.h"
#include "util.h"

using namespace std;


/// ���� ���������� ��� ����������� �����
struct TMorphoData
{
	UInt32 LangCode;
	UInt32 DictId;
	MemoryBuffer Data;

	TMorphoData(UInt32 lang, UInt32 id, MemoryBuffer&& data)
		: LangCode(lang), DictId(id), Data(std::move(data)) {}
};

/// ����� ��� ������ � ������ ����������
class CMorphoDataManager
{
public:

	/// ��������� ���� ����������
	int AddMorphoBase(UInt32 aLangCode, const wchar_t* aFilename);

	/// ���������� ���������� ����������� ��� ����������
	UInt32 GetMorphoBasesCount(void) const;

private:

	/// ������ ��� ����������
	vector<TMorphoData> m_MorphoBases;

public:
	auto bases() const -> RangeImpl<decltype(m_MorphoBases.begin())> {
		return{ m_MorphoBases.begin(), m_MorphoBases.end() };
	}
};
