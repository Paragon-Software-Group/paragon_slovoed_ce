#ifndef _Colour_H_
#define _Colour_H_

#include "sld_Types.h"

class CColour
{
public:

	CColour() : m_colour(0)																			{}

	CColour (UInt32 a, UInt32 r, UInt32 g, UInt32 b)
		: m_colour(((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff))		{}

	CColour(UInt32 clr) : m_colour(clr)																{}

	UInt32 getAlpha() const																			{ return m_colour >> 24; }

	UInt32 getRed() const																			{ return (m_colour >> 16) & 0xff; }

	UInt32 getGreen() const																			{ return (m_colour >> 8) & 0xff; }

	UInt32 getBlue() const																			{ return m_colour & 0xff; }

	Float32 getLuminance() const
	{
		return 0.3f * getRed() + 0.59f * getGreen() + 0.11f * getBlue();
	}

	UInt32 getAverage() const
	{
		return ( getRed() + getGreen() + getBlue() ) / 3;
	}

	void setAlpha(UInt32 a)																			{ m_colour = ((a & 0xff) << 24) | (m_colour & 0x00ffffff); }

	void setRed(UInt32 r)																			{ m_colour = ((r & 0xff) << 16) | (m_colour & 0xff00ffff); }

	void setGreen(UInt32 g)																			{ m_colour = ((g & 0xff) << 8) | (m_colour & 0xffff00ff); }

	void setBlue(UInt32 b)																			{ m_colour = (b & 0xff) | (m_colour & 0xffffff00); }

	void set(UInt32 a, UInt32 r, UInt32 g, UInt32 b)
	{
		m_colour = (((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff));
	}
	void set(const UInt32 col)																		{ m_colour = col; }
	
	UInt32 get() const																				{ return m_colour; }

	bool operator==(const CColour& other) const														{ return other.m_colour == m_colour; }

	bool operator!=(const CColour& other) const														{ return other.m_colour != m_colour; }

	bool operator<(const CColour& other) const														{ return (m_colour < other.m_colour); }

	CColour operator+(const CColour& other) const
	{
		return CColour( min_(getAlpha() + other.getAlpha(), 255), 
						min_(getRed() + other.getRed(), 255),
						min_(getGreen() + other.getGreen(), 255),
						min_(getBlue() + other.getBlue(), 255));
	}

protected:
	static const UInt32& min_(const UInt32& a, const UInt32& b)
	{
		return a < b ? a : b;
	}
	static const UInt32& min_(const UInt32& a, const UInt32& b, const UInt32& c)
	{
		return a < b ? min_(a, c) : min_(b, c);
	}

	// Colour in A8R8G8B8 Format
	UInt32 m_colour;
};

#endif // _Colour_H_