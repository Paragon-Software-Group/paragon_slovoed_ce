package com.paragon_software.license_manager_api;

import androidx.annotation.NonNull;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class PeriodShdd
{
  public static final  int     DAYS_IN_WEAK = 7;
  private static final Pattern PATTERN      = Pattern
      .compile("([-+]?)P(?:([-+]?[0-9]+)Y)?(?:([-+]?[0-9]+)M)?(?:([-+]?[0-9]+)W)?(?:([-+]?[0-9]+)D)?",
               Pattern.CASE_INSENSITIVE);
  private final int years;
  private final int months;
  private final int days;

  public PeriodShdd( int years, int months, int days )
  {
    this.years = years;
    this.months = months;
    this.days = days;
  }

  /**
   * Obtains a {@code PeriodShdd} from a text string such as {@code PnYnMnD}.
   * <p>
   * For example:
   * <pre>
   *   "P15D"
   *   "P3W"
   *   "P6M"
   *   "P1Y"
   *   "P1Y2M3D"
   * </pre>
   */
  public static PeriodShdd parse( @NonNull CharSequence text )
  {
    Matcher matcher = PATTERN.matcher(text);
    if ( matcher.matches() )
    {
      int negate = ( charMatch(text, matcher.start(1), matcher.end(1), '-') ? -1 : 1 );
      int yearStart = matcher.start(2), yearEnd = matcher.end(2);
      int monthStart = matcher.start(3), monthEnd = matcher.end(3);
      int weekStart = matcher.start(4), weekEnd = matcher.end(4);
      int dayStart = matcher.start(5), dayEnd = matcher.end(5);
      if ( yearStart >= 0 || monthStart >= 0 || weekStart >= 0 || dayStart >= 0 )
      {
        try
        {
          int years = parseNumber(text, yearStart, yearEnd, negate);
          int months = parseNumber(text, monthStart, monthEnd, negate);
          int weeks = parseNumber(text, weekStart, weekEnd, negate);
          int days = parseNumber(text, dayStart, dayEnd, negate);
          days = days + weeks * DAYS_IN_WEAK;
          return new PeriodShdd(years, months, days);
        }
        catch ( NumberFormatException ex )
        {
          throw new DateTimeParseException("Text cannot be parsed to a PeriodShdd", text, 0, ex);
        }
      }
    }
    throw new DateTimeParseException("Text cannot be parsed to a PeriodShdd", text, 0);
  }

  private static boolean charMatch( CharSequence text, int start, int end, char c )
  {
    return ( start >= 0 && end == start + 1 && text.charAt(start) == c );
  }

  private static int parseNumber( CharSequence text, int start, int end, int negate )
  {
    if ( start < 0 || end < 0 )
    {
      return 0;
    }
    CharSequence region = text.subSequence(start, end);
    int value = Integer.parseInt(region.toString(), 10);
    try
    {
      return value * negate;
    }
    catch ( Exception e )
    {
      throw new DateTimeParseException("Text cannot be parsed as Number", text, start, e);
    }
  }

  public int getYears()
  {
    return years;
  }

  public int getMonths()
  {
    return months;
  }

  public int getDays()
  {
    return days;
  }

  @Override
  public int hashCode()
  {
    return years + ( months << 8 ) + ( days << 16 );
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
    PeriodShdd that = (PeriodShdd) o;
    return years == that.years && months == that.months && days == that.days;
  }

  public static class DateTimeParseException extends RuntimeException
  {

    private final String parsedString;
    private final int    errorIndex;

    public DateTimeParseException( String message, CharSequence parsedData, int errorIndex )
    {
      super(message);
      this.parsedString = parsedData.toString();
      this.errorIndex = errorIndex;
    }

    public DateTimeParseException( String message, CharSequence parsedData, int errorIndex, Throwable cause )
    {
      super(message, cause);
      this.parsedString = parsedData.toString();
      this.errorIndex = errorIndex;
    }

    public String getParsedString()
    {
      return parsedString;
    }

    public int getErrorIndex()
    {
      return errorIndex;
    }
  }
}
