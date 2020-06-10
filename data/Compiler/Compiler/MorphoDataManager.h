#pragma once

#include <map>
#include <vector>

#include "sld_Platform.h"
#include "util.h"

using namespace std;


/// База морфологии для конкретного языка
struct TMorphoData
{
	UInt32 LangCode;
	UInt32 DictId;
	MemoryBuffer Data;

	TMorphoData(UInt32 lang, UInt32 id, MemoryBuffer&& data)
		: LangCode(lang), DictId(id), Data(std::move(data)) {}
};

/// Класс для работы с базами морфологии
class CMorphoDataManager
{
public:

	/// Добавляет базу морфологии
	int AddMorphoBase(UInt32 aLangCode, const wchar_t* aFilename);

	/// Возвращает количество загруженных баз морфологии
	UInt32 GetMorphoBasesCount(void) const;

private:

	/// Массив баз морфологии
	vector<TMorphoData> m_MorphoBases;

public:
	auto bases() const -> RangeImpl<decltype(m_MorphoBases.begin())> {
		return{ m_MorphoBases.begin(), m_MorphoBases.end() };
	}
};
