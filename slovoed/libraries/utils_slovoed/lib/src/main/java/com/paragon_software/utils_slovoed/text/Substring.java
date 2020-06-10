package com.paragon_software.utils_slovoed.text;

public class Substring implements SubstringInfo
{

  private int offset;
  private int length;

  public Substring( int offset, int length )
  {
    this.offset = offset;
    this.length = length;
  }

  @Override
  public int getOffset()
  {
    return offset;
  }

  @Override
  public int getLength()
  {
    return length;
  }

  @Override
  public int hashCode()
  {
    int result = offset;
    result = 31 * result + length;
    return result;
  }

  @Override
  public boolean equals( Object o )
  {
    if ( this == o )
    {
      return true;
    }
    if ( o == null || getClass() != o.getClass() )
    {
      return false;
    }

    Substring substring = (Substring) o;

    //noinspection SimplifiableIfStatement
    if ( offset != substring.offset )
    {
      return false;
    }
    return length == substring.length;
  }
}
