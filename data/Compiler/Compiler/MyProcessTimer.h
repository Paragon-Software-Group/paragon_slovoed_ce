#pragma once

#include <stdio.h>
#include <memory>
#include <string>

/// Класс для измерения затраченного процессорного времени между двумя событиями
class CMyProcessTimer
{
public:

	/// Конструктор
	CMyProcessTimer(void);
	/// Конструктор
	CMyProcessTimer(const char* aLogFileName);
	/// Деструктор
	~CMyProcessTimer(void);

public:

	/// Начало отсчета времени
	void Start(void);
	/// Конец отсчета времени
	void Stop(void);
	/// Возвращает строку, содержащую описание затраченного процессорного времени
	std::string GetTimeString(void);
	/// Возвращает строку, содержащую информацию об использовании памяти
	std::string GetMemoryString(void);
	/// Печатает строку со временем в файл, если файл не открыт - на экран
	void PrintTimeString(std::string aMessage);
	/// Печатает строку со временем в файл, если файл не открыт - на экран
	void PrintTimeStringW(std::wstring aMessage);
	/// Печатает строку с информацией об используемой памяти
	void PrintMemoryString(std::string aMessage);
	/// Печатает строку с информацией об используемой памяти
	void PrintMemoryStringW(std::wstring aMessage);

	/// Устанавливает имя файла для записи логов
	void SetFilename(std::string aFilename);

private:

	void printString(const std::string &aString, const std::string &aMessage);

private:
	struct Data;
	std::unique_ptr<Data> m_data;

	/// Файл для печати
	FILE* m_logf;
};
