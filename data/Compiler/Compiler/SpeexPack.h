#ifndef _SPEEX_PACK_H_
#define _SPEEX_PACK_H_

#include "sld_Types.h"
#include <string>
#include <map>
#include <algorithm>

using namespace std;

/// ������ ����������
#define SPEEX_PACK_CONTAINER_VERSION	1

/// ����� ������ �������
#define SPEEX_PACK_APP_BUILD		2

/// ��������� �����-����������
#define SPEEX_PACK_SIGNATURE	0xAAFFFFAA

/// ������������ ����� ������-����� � �������� ��� ����� ���������� (��� �����), ������� ����������� 0
#define MAX_FILE_KEY_SIZE	128


/// ������ ������ � �����������
enum ESpeexPackError
{
	spOK = 0,
	
	spMemNullPointer,
	spMemBadAlloc,
	
	spEmptyFileKey,
	spEmptyFile,
	
	// ���� � ���� ������ �� �����������, ����� ������ ���������� ��������� ����� �����
	spDuplicatedFileKey,
	
	spToLargeFileKey,
	
	spErrorOpenFile,
	spErrorReadFile,
	spErrorWriteFile,
	
	spEmptyContainer,
	spItIsNotContainer,
	spCorruptedContainer,
	
	spFileNotFound,
	spAddingAnEmptyFile
};


/// ����� ��������� ����������
typedef struct TSpeexPackContainerHeader
{
	/// ��������� �����-����������
	UInt32 Signature;
	/// ������ ��������� ���������
	UInt32 StructSize;
	/// ������ ����������
	UInt32 Version;
	/// ������ ����� �����-���������� � ������
	UInt32 FileSize;
	
	/// �������� �� ������� ���������� ������
	UInt32 ShiftToFilesTable;
	/// ������ ������� ���������� ������ � ������
	UInt32 FilesTableSize;
	/// ���������� ������� � ������� ���������� ������ (���������� ������ � ����������)
	UInt32 FilesCount;
	
	/// �������� �� ������ ����� ������ ���� ������ (������ ������������� ����������)
	UInt32 ShiftToData;
	/// ������ ������ ����� ������ ���� ������
	UInt32 DataSize;
	
	/// ���������������
	UInt32 Reserved[16];
	
} TSpeexPackContainerHeader;


/// ���������, ����������� ���� ���� � ����������
/// ������� ������� ���������� ������
typedef struct TSpeexPackFileHeader
{
	/// ������ ��������� ���������
	UInt32 StructSize;
	/// Id �����
	UInt32 Id;
	/// �������� �� ������ ����� ������
	UInt32 Shift;
	/// ������ �����
	UInt32 Size;
	/// �����
	UInt32 Flags;
	/// ���������������
	UInt32 Reserved[3];
	/// ��������� ������-���� �����
	UInt16 Key[MAX_FILE_KEY_SIZE];
	
} TSpeexPackFileHeader;


/// ���������, �������������� ���� ���� � ����������
typedef struct TSpeexPackFile
{
	/// ���������
	TSpeexPackFileHeader*	Header;
	/// ��������� �� ����������� ������
	UInt8*					Data;
	
} TSpeexPackFile;


/// ����� ��� ������ � ����������� ������
/// �������� � ���������� ���������� � ����
class CSpeexPackWrite
{
public:
	
	/// �����������
	CSpeexPackWrite(void);
	/// ����������
	~CSpeexPackWrite(void);

public:

	/// ��������� ������������ ��������� ��� ��������� (����������, ��������, ������ ������)
	UInt32 Load(const wchar_t* aFilename);
	
	/// �������� ���������� ��� ����������
	void Close(void);

	/// ��������� ��������� � ����
	UInt32 Save(const wchar_t* aFilename);

	/// ��������� ���� � ��������� (���� �������� � �����)
	UInt32 AddFile(const wchar_t* aFilename, const wchar_t* aFileKey = NULL);
	/// ��������� ���� � ��������� (���� ��� �������� � ������)
	UInt32 AddFile(const wchar_t* aFilename, const UInt8* aData, UInt32 aDataSize);

private:

	/// �������
	void Clear(void);

private:

	/// ����� ����������
	map<wstring, TSpeexPackFile> m_Data;

	/// ����� ���������� ����������� ������
	UInt32 m_TotalFilesCount;
		
	/// ����� ������ ������ ����������� ������
	UInt32 m_TotalDataSize;
	
};


/// ����� ��� ������ � ����������� ������
/// ������ � ���������� ������ �� ����������
class CSpeexPackRead
{
public:
	
	/// �����������
	CSpeexPackRead(void);
	/// ����������
	~CSpeexPackRead(void);

public:

	/// ��������� � ��������� ���������
	UInt32 Open(const wchar_t* aFilename);
	
	/// ������ �� ���������
	bool IsOpen(void) const;
	
	/// ��������� ���������
	void Close(void);

public:

	/// ���������� ����� ������ ����������
	UInt32 GetContainerVersion(void) const;

	/// ���������� ����� ���������� ������ � ����������
	UInt32 GetTotalFilesCount(void) const;
	
	/// ���������� ����� ������ ���� ������ � ����������
	UInt32 GetTotalFilesSize(void) const;
	
	/// ���������� ������ ����������� ����� � ���������� �� �����
	UInt32 GetFileData(const wchar_t* aKey, const UInt8** aData, UInt32* aSize);
	
	/// ���������� ��������� �� ����� ������� � ����� �� ������ ����� � ����������
	const TSpeexPackFile* GetFile(UInt32 aIndex);

private:

	/// �������
	void Clear(void);

private:

	/// ��������� ����������
	TSpeexPackContainerHeader m_Header;
	
	/// ������� ���������� ������ ����������
	TSpeexPackFileHeader* m_FilesTable;

	/// ���� ������ ���� ������
	UInt8* m_Data;
	
	/// ����� ���������� (��� ������ �� �����)
	map<wstring, TSpeexPackFile> m_Files;
	
};

#endif // _SPEEX_PACK_H_
