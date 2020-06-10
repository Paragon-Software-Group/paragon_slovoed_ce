#pragma once
#include "sld_Types.h"

#include <utility>
#include <vector>

/// ���������� �������� �� ���� �����������, ������ �������� "����" - ��������� ������
std::pair<bool, int> IsDirectory(std::wstring aPath);

/// �������������� ����. ��� ������ ����� ���� �������� � ������������� ���� "Media/1.mesh", ����� ������� ��� ������ ���� ��� ./ ���� ����� ���� ����� ���������� ����� � ������ �������
void CorrectFilePathA(std::string& aPath);
	
/// �������������� ����. ��� ������ ����� ���� �������� � ������������� ���� "Media/1.mesh", ����� ������� ��� ������ ���� ��� ./ ���� ����� ���� ����� ���������� ����� � ������ �������
void CorrectFilePathW(std::wstring& aPath);

/// �������� ������ ������ � ����������
void GetFilesInDirectory(std::vector<std::string> &aOut, std::string aDirectory);

namespace sld {

// ��������� ����
FILE* fopen(std::wstring aPath, const wchar_t *aMode);

} // namespace sld
