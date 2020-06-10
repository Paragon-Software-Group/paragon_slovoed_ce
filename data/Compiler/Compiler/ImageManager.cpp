#include "ImageManager.h"

#include "SDC_CRC.h"
#include "Tools.h"

using namespace std;

/***********************************************************************
* ���������� ����� ���������� �������� � ���������� �������
*
* @return ����� ���������� �������� � ���������� �������
************************************************************************/
UInt32 CImageManager::GetImageCount(void) const
{
	return (UInt32)m_UniqueImages.size();
}

/***********************************************************************
* ��������� ��������� � ����������
*
* @param aImageContainer - ������ �� ����������� ������
*
* @return ������� ������������ ���������� ������� ������� �� ���������� ����������� �������
************************************************************************/
AddedFileResourceMap CImageManager::AddImageContainer(const CImageContainer &aImageContainer)
{
	const UInt32 imageCount = aImageContainer.GetImageCount();
	if (imageCount == 0)
		return AddedFileResourceMap(SLD_INDEX_PICTURE_NO);

	// ��������� ���������� ������ �������� �������� ������� ������������
	std::vector<UInt32> map(imageCount + 1, SLD_INDEX_PICTURE_NO);
	for (UInt32 i = 1; i <= imageCount; i++)
		map[i] = AddImage(aImageContainer.GetFullImageName(i));

	return AddedFileResourceMap(SLD_INDEX_PICTURE_NO, std::move(map));
}

/***********************************************************************
* ���������� ������ ��� ����� �������� �� ����������� �������
*
* @param aIndex - ���������� ������ ��������
*
* @return ������ ��� ����� �������� ��� ������ ������, ���� ����� �������� �� �������
************************************************************************/
wstring CImageManager::GetFullImageNameByImageIndex(UInt32 aIndex) const
{
	// ��������� �������� ���������� � 1
	if (aIndex == 0 || (aIndex > GetImageCount()))
		return wstring();

	const auto it = m_UniqueImages.find(aIndex);
	return it != m_UniqueImages.end() ? it->second : wstring();
}

/***********************************************************************
* ��������� ��������, ��������� ������ ��� ����� ��������
*
* @param aFullImageName - ������ ��� ����� ��������
*
* @return ���������� ������ ��������
************************************************************************/
UInt32 CImageManager::AddImage(wstring aFullImageName)
{
	if (aFullImageName.empty())
		return SLD_INDEX_PICTURE_NO;

	// ���������, ����� ����� �������� ��� ����
	map<wstring, UInt32>::iterator it = m_Images.find(aFullImageName);
	if (it!=m_Images.end())
		return it->second;

	UInt32 imageCRC = -1;

	const MemoryBuffer buf = sld::read_file(aFullImageName);
	if (buf.size())
	{
		imageCRC = CRC32(buf.data(), static_cast<UInt32>(buf.size()), SDC_CRC32_START_VALUE);

		auto itCRC = m_ImageCRC32.equal_range(imageCRC);

		for (auto it = itCRC.first; it != itCRC.second; ++it)
		{
			const MemoryBuffer buf2 = sld::read_file(it->second);
			if (buf.size() == buf2.size() && memcmp(buf.data(), buf2.data(), buf.size()) == 0)
			{
				const UInt32 index = m_Images[it->second];
				m_Images.emplace(aFullImageName, index);
				return index;
			}
		}

	}


	// ������ ����������� ��������
	// ��������� �������� ���������� � 1, 0 ������������ ��� ������� ���������� ��������
	UInt32 Index = (UInt32)m_UniqueImages.size() + 1;

	// ���������	
	m_Images.emplace(aFullImageName, Index);
	m_UniqueImages.emplace(Index, aFullImageName);
	m_ImageCRC32.emplace(imageCRC, aFullImageName);

	return Index;
}
