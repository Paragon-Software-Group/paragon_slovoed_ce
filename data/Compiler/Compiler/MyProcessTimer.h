#pragma once

#include <stdio.h>
#include <memory>
#include <string>

/// ����� ��� ��������� ������������ ������������� ������� ����� ����� ���������
class CMyProcessTimer
{
public:

	/// �����������
	CMyProcessTimer(void);
	/// �����������
	CMyProcessTimer(const char* aLogFileName);
	/// ����������
	~CMyProcessTimer(void);

public:

	/// ������ ������� �������
	void Start(void);
	/// ����� ������� �������
	void Stop(void);
	/// ���������� ������, ���������� �������� ������������ ������������� �������
	std::string GetTimeString(void);
	/// ���������� ������, ���������� ���������� �� ������������� ������
	std::string GetMemoryString(void);
	/// �������� ������ �� �������� � ����, ���� ���� �� ������ - �� �����
	void PrintTimeString(std::string aMessage);
	/// �������� ������ �� �������� � ����, ���� ���� �� ������ - �� �����
	void PrintTimeStringW(std::wstring aMessage);
	/// �������� ������ � ����������� �� ������������ ������
	void PrintMemoryString(std::string aMessage);
	/// �������� ������ � ����������� �� ������������ ������
	void PrintMemoryStringW(std::wstring aMessage);

	/// ������������� ��� ����� ��� ������ �����
	void SetFilename(std::string aFilename);

private:

	void printString(const std::string &aString, const std::string &aMessage);

private:
	struct Data;
	std::unique_ptr<Data> m_data;

	/// ���� ��� ������
	FILE* m_logf;
};
