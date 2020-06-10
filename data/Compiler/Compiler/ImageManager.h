#pragma once

#include <map>

#include "ImageContainer.h"

/// ����� ��� ���������� ����������
class CImageManager
{
public:
	/// ���������� ����� ���������� �������� � ���������� �������
	UInt32 GetImageCount(void) const;

	/// ��������� ��������� � ����������
	AddedFileResourceMap AddImageContainer(const CImageContainer &aImageContainer);

	/// ���������� ������ ��� ����� �������� �� ����������� �������
	std::wstring GetFullImageNameByImageIndex(UInt32 aIndex) const;

private:

	/// ��������� ��������, ��������� ������ ��� ����� ��������
	UInt32 AddImage(std::wstring aFullImageName);

private:

	/// ���������� ������ ��������
	/// �������� ������ ����� ������ �������� � �� ���������� �������
	std::map<std::wstring, UInt32> m_Images;

	/// ������ ���������� ��������
	std::map<UInt32, std::wstring> m_UniqueImages;

	/// ������ ���� ������ ��������
	std::multimap<UInt32, std::wstring> m_ImageCRC32;
};
