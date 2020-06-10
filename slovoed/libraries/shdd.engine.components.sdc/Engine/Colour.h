#ifndef _Colour_H_
#define _Colour_H_

#include "SldTypes.h"

/// Класс обеспечивающий представление цвета
class CColour
{
public:
	/// Конструктор
	CColour() : m_colour(0)																			{}
	/// Конструктор
	CColour (UInt32 a, UInt32 r, UInt32 g, UInt32 b)
		: m_colour(((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff))		{}
	/// Конструктор
	CColour(UInt32 clr) : m_colour(clr)																{}
	/// преобразование содержимого, которое находилось в формате RGBA в ARGB
	void ConvertRGBAtoARGB()																		{ m_colour = ((m_colour << 24) | (m_colour >> 8)) ;}
	/// запись цвета в RGBA и преобразование его в ARGB
	void ConvertRGBAtoARGB(UInt32 clr)																{ m_colour = ((clr >> 8) | (clr << 24));}
	/// возвращает значение альфы [0;255]
	UInt32 getAlpha() const																			{ return m_colour >> 24; }
	/// возвращает значение альфы в вещественном формате [0.0;1.0]
	Float32 getAlphaF() const																		{ return ((m_colour >> 24) / 255.0f); }
	/// возвращает значение красного канала [0;255]
	UInt32 getRed() const																			{ return (m_colour >> 16) & 0xff; }
	/// возвращает значение красного канала в вещественном формате [0.0;1.0]
	Float32 getRedF() const																			{ return (((m_colour >> 16) & 0xff) / 255.0f); }
	/// возвращает значение зеленого канала [0;255]
	UInt32 getGreen() const																			{ return (m_colour >> 8) & 0xff; }
	/// возвращает значение зеленого канала в вещественном формате [0.0;1.0]
	Float32 getGreenF() const																		{ return (((m_colour >> 8) & 0xff) / 255.0f); }
	/// возвращает значение синего канала [0;255]
	UInt32 getBlue() const																			{ return m_colour & 0xff; }
	/// возвращает значение синего канала в вещественном формате [0.0;1.0]
	Float32 getBlueF() const																		{ return ((m_colour & 0xff) / 255.0f); }
	/// вычисление яркости
	Float32 getLuminance() const
	{
		return 0.3f * getRed() + 0.59f * getGreen() + 0.11f * getBlue();
	}
	/// вычисление среднего значение по цветовым каналам
	UInt32 getAverage() const
	{
		return ( getRed() + getGreen() + getBlue() ) / 3;
	}
	/// установить значение прозрачности
	void setAlpha(UInt32 a)																			{ m_colour = ((a & 0xff) << 24) | (m_colour & 0x00ffffff); }
	/// установить значение цвета по каналу Red
	void setRed(UInt32 r)																			{ m_colour = ((r & 0xff) << 16) | (m_colour & 0xff00ffff); }
	/// установить значение цвета по каналу Green
	void setGreen(UInt32 g)																			{ m_colour = ((g & 0xff) << 8) | (m_colour & 0xffff00ff); }
	/// установить значение цвета по каналу Blue
	void setBlue(UInt32 b)																			{ m_colour = (b & 0xff) | (m_colour & 0xffffff00); }
	/// устанавливает значение цвета, используя значения по отдельным каналам
	void set(UInt32 a, UInt32 r, UInt32 g, UInt32 b)
	{
		m_colour = (((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff));
	}
	/// устанавливает значение цвета
	void set(const UInt32 col)																		{ m_colour = col; }
	/// возвращает челочисленное представление цвета по всем каналам
	UInt32 get() const																				{ return m_colour; }
	/// оператор равенства
	bool operator==(const CColour& other) const														{ return other.m_colour == m_colour; }
	/// оператор равенства
	bool operator!=(const CColour& other) const														{ return other.m_colour != m_colour; }
	/// оператор сравнения меньше <
	bool operator<(const CColour& other) const														{ return (m_colour < other.m_colour); }
	/// оператор сложения
	CColour operator+(const CColour& other) const
	{
		return CColour( min_(getAlpha() + other.getAlpha(), 255), 
						min_(getRed() + other.getRed(), 255),
						min_(getGreen() + other.getGreen(), 255),
						min_(getBlue() + other.getBlue(), 255));
	}

protected:
	/// определение минимума для 2х чисел
	static const UInt32& min_(const UInt32& a, const UInt32& b)
	{
		return a < b ? a : b;
	}
	/// определение минимума для 3х чисел
	static const UInt32& min_(const UInt32& a, const UInt32& b, const UInt32& c)
	{
		return a < b ? min_(a, c) : min_(b, c);
	}

	/// Colour in A8R8G8B8 Format
	/// на каждый канал отведено по байту
	UInt32 m_colour;
};

#endif // _Colour_H_