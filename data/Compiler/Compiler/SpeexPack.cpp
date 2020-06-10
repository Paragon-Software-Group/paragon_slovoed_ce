#include "SpeexPack.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "sld_Platform.h"
#include "StringFunctions.h"
#include "Tools.h"


CSpeexPackWrite::CSpeexPackWrite(void)
{
	Clear();
}

CSpeexPackWrite::~CSpeexPackWrite(void)
{
	Close();
}

void CSpeexPackWrite::Clear(void)
{
	m_Data.clear();
	m_TotalFilesCount = 0;
	m_TotalDataSize = 0;
}

void CSpeexPackWrite::Close(void)
{
	map<wstring, TSpeexPackFile>::iterator it = m_Data.begin();
	map<wstring, TSpeexPackFile>::iterator eit = m_Data.end();
	while (it != eit)
	{
		if (it->second.Data)
			delete [] it->second.Data;
		if (it->second.Header)
			delete it->second.Header;
		it++;
	}
	Clear();
}

UInt32 CSpeexPackWrite::Load(const wchar_t* aFilename)
{
	Close();
	
	if (!aFilename)
		return spMemNullPointer;
	
	FILE* f = sld::fopen(aFilename, L"rb");
	if (!f)
	{
		// Такого файла еще нет, это не считается ошибкой
		if (errno == ENOENT)
			return spOK;
		// А это ошибка
		return spErrorOpenFile;
	}
	
	if (fseek(f, 0, SEEK_END))
	{
		fclose(f);
		return spErrorReadFile;
	}
	UInt32 FileSize = (UInt32)_ftelli64(f);
	if (fseek(f, 0, SEEK_SET))
	{
		fclose(f);
		return spErrorReadFile;
	}
	
	TSpeexPackContainerHeader Header;
	memset(&Header, 0, sizeof(Header));
	
	if (FileSize < sizeof(Header))
	{
		fclose(f);
		return spItIsNotContainer;
	}
	
	if (fread(&Header, sizeof(Header), 1, f) != 1)
	{
		fclose(f);
		return spErrorReadFile;
	}
	
	if (Header.Signature != SPEEX_PACK_SIGNATURE)
	{
		fclose(f);
		return spItIsNotContainer;
	}
	
	if (Header.FileSize != FileSize)
	{
		fclose(f);
		return spCorruptedContainer;
	}
	
	if (fseek(f, Header.ShiftToFilesTable, SEEK_SET))
	{
		fclose(f);
		return spErrorReadFile;
	}
	
	if (Header.FilesTableSize != Header.FilesCount*sizeof(TSpeexPackFileHeader))
	{
		fclose(f);
		return spCorruptedContainer;
	}
	
	TSpeexPackFileHeader* pFilesTable = NULL;
	pFilesTable = new TSpeexPackFileHeader[Header.FilesCount];
	if (!pFilesTable)
	{
		fclose(f);
		return spMemBadAlloc;
	}
	
	if (fread(pFilesTable, Header.FilesTableSize, 1, f) != 1)
	{
		delete [] pFilesTable;
		fclose(f);
		return spErrorReadFile;
	}
	
	if (fseek(f, Header.ShiftToData, SEEK_SET))
	{
		delete [] pFilesTable;
		fclose(f);
		return spErrorReadFile;
	}
	
	UInt8* pData = NULL;
	pData = new UInt8[Header.DataSize];
	if (!pData)
	{
		delete [] pFilesTable;
		fclose(f);
		return spMemBadAlloc;
	}
	
	if (fread(pData, Header.DataSize, 1, f) != 1)
	{
		delete [] pData;
		delete [] pFilesTable;
		fclose(f);
		return spErrorReadFile;
	}
	
	fclose(f);
	
	for (UInt32 i=0;i<Header.FilesCount;i++)
	{
		TSpeexPackFile File;
		memset(&File, 0, sizeof(File));
		
		File.Header = new TSpeexPackFileHeader;
		if (!File.Header)
		{
			delete [] pData;
			delete [] pFilesTable;
			Close();
			return spMemBadAlloc;
		}
		memmove(File.Header, &pFilesTable[i], sizeof(pFilesTable[0]));
		
		File.Data = new UInt8[File.Header->Size];
		if (!File.Data)
		{
			delete File.Header;
			delete [] pData;
			delete [] pFilesTable;
			Close();
			return spMemBadAlloc;
		}
		memmove(File.Data, pData + File.Header->Shift, File.Header->Size);
		
		wstring key((const wchar_t*)(File.Header->Key));
		m_Data[key] = File;
	}
	
	m_TotalFilesCount = Header.FilesCount;
	m_TotalDataSize = Header.DataSize;
	
	delete [] pData;
	delete [] pFilesTable;
	
	return spOK;
}

UInt32 CSpeexPackWrite::Save(const wchar_t* aFilename)
{
	if (!aFilename)
		return spMemNullPointer;
	
	if (m_Data.empty() || !m_TotalDataSize)
		return spEmptyContainer;
	
	if (m_Data.size() != m_TotalFilesCount)
		return spCorruptedContainer;
	
	TSpeexPackContainerHeader Header;
	memset(&Header, 0, sizeof(Header));
	
	Header.Signature = SPEEX_PACK_SIGNATURE;
	Header.Version = SPEEX_PACK_CONTAINER_VERSION;
	Header.StructSize = sizeof(Header);
	
	Header.FilesCount = m_TotalFilesCount;
	Header.DataSize = m_TotalDataSize;
	
	Header.FilesTableSize = Header.FilesCount*sizeof(TSpeexPackFileHeader);
	Header.FileSize = Header.StructSize + Header.FilesTableSize + Header.DataSize;
	
	Header.ShiftToFilesTable = Header.StructSize;
	Header.ShiftToData = Header.StructSize + Header.FilesTableSize;
	
	
	FILE* f = sld::fopen(aFilename, L"wb");
	if (!f)
		return spErrorOpenFile;
	
	if (fseek(f, Header.ShiftToData, SEEK_SET))
	{
		fclose(f);
		return spErrorWriteFile;
	}
		
	map<wstring, TSpeexPackFile>::iterator it = m_Data.begin();
	map<wstring, TSpeexPackFile>::iterator eit = m_Data.end();
	UInt32 CurrDataShift = 0;
	while (it != eit)
	{
		it->second.Header->Shift = CurrDataShift;
		if (fwrite(it->second.Data, it->second.Header->Size, 1, f) != 1)
		{
			fclose(f);
			return spErrorWriteFile;
		}
		CurrDataShift += it->second.Header->Size;
		it++;
	}
	
	if (fseek(f, 0, SEEK_SET))
	{
		fclose(f);
		return spErrorWriteFile;
	}
	
	if (fwrite(&Header, Header.StructSize, 1, f) != 1)
	{
		fclose(f);
		return spErrorWriteFile;
	}
	
	it = m_Data.begin();
	eit = m_Data.end();
	while (it != eit)
	{
		if (fwrite(it->second.Header, it->second.Header->StructSize, 1, f) != 1)
		{
			fclose(f);
			return spErrorWriteFile;
		}
		it++;
	}
	
	fclose(f);
		
	return spOK;
}

UInt32 CSpeexPackWrite::AddFile(const wchar_t* aFilename, const wchar_t* aFileKey)
{
	if (!aFilename)
		return spMemNullPointer;
	
	FILE* f = sld::fopen(aFilename, L"rb");
	if (!f)
		return spErrorOpenFile;
	
	if (fseek(f, 0, SEEK_END))
	{
		fclose(f);
		return spErrorReadFile;
	}
	UInt32 Size = (UInt32)_ftelli64(f);
	if (fseek(f, 0, SEEK_SET))
	{
		fclose(f);
		return spErrorReadFile;
	}
	
	if (!Size)
	{
		fclose(f);
		return spAddingAnEmptyFile;
	}
	
	UInt8* ptr = 0;
	ptr = new UInt8[Size];
	if (!ptr)
	{
		fclose(f);
		return spMemBadAlloc;
	}
	
	if (fread(ptr, Size, 1, f) != 1)
	{
		delete [] ptr;
		fclose(f);
		return spErrorReadFile;
	}
	fclose(f);
	
	UInt32 error = 0;
	wchar_t wbuf[1024] = {0};
	if (aFileKey)
		wcscpy(wbuf, aFileKey);
	else
		wcscpy(wbuf, aFilename);

	error = AddFile(_wcslwr(wbuf), ptr, Size);
		
	delete [] ptr; 
	return error;
}

UInt32 CSpeexPackWrite::AddFile(const wchar_t* aFilename, const UInt8* aData, UInt32 aDataSize)
{
	if (!aFilename || !aData)
		return spMemNullPointer;
	
	if (!aDataSize)
		return spEmptyFile;

	wchar_t wbuf[1024] = {0};
	wcscpy(wbuf, aFilename);

	wstring NewFileKey(_wcslwr(wbuf));
	if (NewFileKey.empty())
		return spEmptyFileKey;
	
	if (NewFileKey.size() >= MAX_FILE_KEY_SIZE)
		return spToLargeFileKey;
	
	map<wstring, TSpeexPackFile>::iterator fit = m_Data.find(NewFileKey);
	if (fit != m_Data.end())
		return spDuplicatedFileKey;
	
	TSpeexPackFile NewFile;
	memset(&NewFile, 0, sizeof(NewFile));
	
	NewFile.Header = new TSpeexPackFileHeader;
	if (!NewFile.Header)
		return spMemBadAlloc;
	
	memset(NewFile.Header, 0, sizeof(NewFile.Header[0]));
	NewFile.Header->StructSize = sizeof(NewFile.Header[0]);
	
	NewFile.Data = new UInt8[aDataSize];
	if (!NewFile.Data)
	{
		delete NewFile.Header;
		return spMemBadAlloc;
	}
	
	memmove(NewFile.Data, aData, aDataSize);
	NewFile.Header->Size = aDataSize;
	sld::copy(NewFile.Header->Key, NewFileKey);
	
	NewFile.Header->Id = m_TotalFilesCount;
	m_TotalFilesCount++;
	m_TotalDataSize += NewFile.Header->Size;
	m_Data[NewFileKey] = NewFile;
		
	return spOK;
}

/*****************************************************************************************/

CSpeexPackRead::CSpeexPackRead(void)
{
	Clear();
}

CSpeexPackRead::~CSpeexPackRead(void)
{
	Close();
}

void CSpeexPackRead::Clear(void)
{
	memset(&m_Header, 0, sizeof(m_Header));
	m_FilesTable = NULL;
	m_Data = NULL;
	m_Files.clear();
}

void CSpeexPackRead::Close(void)
{
	if (m_FilesTable)
		delete [] m_FilesTable;
	
	if (m_Data)
		delete [] m_Data;
		
	Clear();
}

bool CSpeexPackRead::IsOpen(void) const
{
	return (m_Header.Signature == SPEEX_PACK_SIGNATURE && m_FilesTable && m_Data);
}

UInt32 CSpeexPackRead::Open(const wchar_t* aFilename)
{
	Close();
	
	if (!aFilename)
		return spMemNullPointer;
	
	FILE* f = sld::fopen(aFilename, L"rb");
	if (!f)
		return spErrorOpenFile;
	
	if (fseek(f, 0, SEEK_END))
	{
		fclose(f);
		return spErrorReadFile;
	}
	UInt32 FileSize = (UInt32)_ftelli64(f);
	if (fseek(f, 0, SEEK_SET))
	{
		fclose(f);
		return spErrorReadFile;
	}
	
	if (FileSize < sizeof(m_Header))
	{
		fclose(f);
		return spItIsNotContainer;
	}
	
	if (fread(&m_Header, sizeof(m_Header), 1, f) != 1)
	{
		Close();
		fclose(f);
		return spErrorReadFile;
	}
	
	if (m_Header.Signature != SPEEX_PACK_SIGNATURE)
	{
		Close();
		fclose(f);
		return spItIsNotContainer;
	}
	
	if (m_Header.FileSize != FileSize)
	{
		Close();
		fclose(f);
		return spCorruptedContainer;
	}
	
	if (fseek(f, m_Header.ShiftToFilesTable, SEEK_SET))
	{
		Close();
		fclose(f);
		return spErrorReadFile;
	}
	
	if (m_Header.FilesTableSize != m_Header.FilesCount*sizeof(TSpeexPackFileHeader))
	{
		Close();
		fclose(f);
		return spCorruptedContainer;
	}
	
	m_FilesTable = new TSpeexPackFileHeader[m_Header.FilesCount];
	if (!m_FilesTable)
	{
		Close();
		fclose(f);
		return spMemBadAlloc;
	}
	
	if (fread(m_FilesTable, m_Header.FilesTableSize, 1, f) != 1)
	{
		Close();
		fclose(f);
		return spErrorReadFile;
	}
	
	if (fseek(f, m_Header.ShiftToData, SEEK_SET))
	{
		Close();
		fclose(f);
		return spErrorReadFile;
	}
	
	m_Data = new UInt8[m_Header.DataSize];
	if (!m_Data)
	{
		Close();
		fclose(f);
		return spMemBadAlloc;
	}
	
	if (fread(m_Data, m_Header.DataSize, 1, f) != 1)
	{
		Close();
		fclose(f);
		return spErrorReadFile;
	}
	
	fclose(f);
	
	for (UInt32 i=0;i<m_Header.FilesCount;i++)
	{
		TSpeexPackFile File;
		memset(&File, 0, sizeof(File));
		
		File.Header = &m_FilesTable[i];
		File.Data = m_Data + File.Header->Shift;

		wstring key = sld::as_wide((const char16_t*)File.Header->Key);
		_wcslwr(&key[0]);
		sld::copy(File.Header->Key, key);
		m_Files[key] = File;
	}
	
	return spOK;
}

UInt32 CSpeexPackRead::GetContainerVersion(void) const
{
	if (!IsOpen())
		return 0;
	
	return m_Header.Version;
}

UInt32 CSpeexPackRead::GetTotalFilesCount(void) const
{
	if (!IsOpen())
		return 0;
	
	return m_Header.FilesCount;
}

UInt32 CSpeexPackRead::GetTotalFilesSize(void) const
{
	if (!IsOpen())
		return 0;
	
	return m_Header.DataSize;
}

UInt32 CSpeexPackRead::GetFileData(const wchar_t* aKey, const UInt8** aData, UInt32* aSize)
{
	if (!aKey || !aData || !aSize)
		return spMemNullPointer;
	
	*aData = NULL;
	*aSize = 0;
	
	if (!IsOpen())
		return spOK;
	
	wchar_t wbuf[1024] = {0};
	wcscpy(wbuf, aKey);
	wstring key(_wcslwr(wbuf));
	
	map<wstring, TSpeexPackFile>::iterator fit = m_Files.find(key);
	if (fit == m_Files.end())
	{
		wstring key1(key);
		str_all_string_replace(key1, L"/", L"\\");
		fit = m_Files.find(key1);
		if (fit == m_Files.end() && key1.front() == L'\\')
		{
			key1.erase(key1.begin());
			fit = m_Files.find(key1);
		}

		if (fit == m_Files.end())
		{
			wstring key2(key);
			str_all_string_replace(key2, L"\\", L"/");
			fit = m_Files.find(key2);
			if (fit == m_Files.end() && key2.front() == L'/')
			{
				key2.erase(key2.begin());
				fit = m_Files.find(key2);
			}
		}
	}
	
	if (fit == m_Files.end())
		return spOK;
	
	*aData = fit->second.Data;
	*aSize = fit->second.Header->Size;
	
	return spOK;
}

const TSpeexPackFile* CSpeexPackRead::GetFile(UInt32 aIndex)
{
	if (!IsOpen())
		return NULL;
	
	if (aIndex >= GetTotalFilesCount())
		return NULL;
	
	wstring key((const wchar_t*)m_FilesTable[aIndex].Key);
	map<wstring, TSpeexPackFile>::const_iterator fit = m_Files.find(key);
	if (fit == m_Files.end())
		return NULL;
	
	return &(fit->second);
}
