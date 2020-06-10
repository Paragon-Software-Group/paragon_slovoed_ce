#include "SldMorphology.h"

namespace {

class CSDCShiftedFile : public ISDCFile
{
public:
	CSDCShiftedFile() : m_file(NULL), m_shift(0), m_size(0) {}

	void setFile(ISDCFile *aFile, UInt32 aShift, UInt32 aSize) {
		m_file = aFile;
		m_shift = aShift;
		m_size = aSize;
	}

	Int8 IsOpened() const override {
		return m_file ? m_file->IsOpened() : 0;
	}

	UInt32 Read(void *aDestPtr, UInt32 aSize, UInt32 aOffset) override {
		return m_file ? m_file->Read(aDestPtr, aSize, m_shift + aOffset) : 0;
	}

	UInt32 GetSize() const override { return m_size; }

private:
	ISDCFile *m_file;
	UInt32   m_shift;
	UInt32   m_size;
};

}

/// Инициализация
ESldError CSldMorphology::Init(CSDCReadMy &aData)
{
	auto res = aData.GetResource(RESOURCE_TYPE_MORPHOLOGY_DATA_HEADER, 0);
	if (res != eOK)
		return res.error();

	UInt32 count = *(const UInt32*)res.ptr();
	m_Morphology = decltype(m_Morphology)(count);
	if (count && m_Morphology.empty())
		return eMemoryNotEnoughMemory;

	const UInt8 *ptr = res.ptr() + sizeof(UInt32);
	for (Morpho &morpho : m_Morphology)
	{
		sldMemCopy(&morpho.info, ptr, sizeof(TMorphoBaseElement));
		ptr += sizeof(TMorphoBaseElement);
	}

	return eOK;
}

/// Получает индекс (порядковый номер) ресурса морфологии в словаре, которая соответствует заданным коду языка и id базы
Int32 CSldMorphology::GetResourceIndexByLangCode(UInt32 aLanguageCode, UInt32 aDictId) const
{
	if (m_Morphology.empty())
		return -1;

	for (UInt32 index = 0; index < m_Morphology.size(); index++)
	{
		const Morpho &morpho = m_Morphology[index];
		if (morpho.info.LanguageCode == aLanguageCode && ((aDictId && morpho.info.DictId == aDictId) || !aDictId))
			return (Int32)index;
	}
	return -1;
}

/// Инициализация базы морфологии
ESldError CSldMorphology::InitMorphology(ISDCFile *aFileData, ISldLayerAccess * aLayerAcces, UInt32 aIndex, UInt32 aShift, UInt32 aSize)
{
	if (!aFileData)
		return eMemoryNullPointer;
	if (!aFileData->IsOpened())
		return eResourceCantOpenContainer;

	Morpho &morpho = m_Morphology[aIndex];

	// проверяем не инициализирована ли уже база
	if (morpho.data && morpho.data->IsInit())
		return eOK;

	if (morpho.data == NULL)
	{
		auto data = sld2::make_unique<MorphoData>();
		if (!data)
			return eMemoryNotEnoughMemory;

		auto file = sld2::make_unique<CSDCShiftedFile>();
		if (!file)
			return eMemoryNotEnoughMemory;

		morpho.data = sld2::move(data);
		morpho.file = sld2::move(file);
	}

	((CSDCShiftedFile*)morpho.file.get())->setFile(aFileData, aShift, aSize);

	// Инициализируем морфологию
	return morpho.data->Init(morpho.file.get(), aLayerAcces) ? eOK : eResourceCantInitMorphology;
}

/// Получает указатель на базу морфологии
MorphoData* CSldMorphology::GetMorphologyByIndex(UInt32 aIndex)
{
	Morpho &morpho = m_Morphology[aIndex];
	return (morpho.data && morpho.data->IsInit()) ? morpho.data.get() : nullptr;
}

/// Проверяет инициализирована ли база морфологии
bool CSldMorphology::IsMorphologyInit(UInt32 aIndex) const
{
	const Morpho &morpho = m_Morphology[aIndex];
	return morpho.data && morpho.data->IsInit();
}
