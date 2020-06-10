package com.paragon_software.license_manager_api.test;

import com.paragon_software.license_manager_api.PeriodShdd;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class PeriodShddTest
{
  @Test
  public void checInitWithConstructor()
  {
    int years = 3;
    int months = 12;
    int days = 31;
    PeriodShdd p = new PeriodShdd(years, months, days);
    assertEquals(years, p.getYears());
    assertEquals(months, p.getMonths());
    assertEquals(days, p.getDays());
    p = new PeriodShdd(0, 0, 0);
    assertEquals(0, p.getYears());
    assertEquals(0, p.getMonths());
    assertEquals(0, p.getDays());
  }

  @Test
  public void checkParseComplex()
  {
    int years = 2;
    int months = 12;
    int weaks = 2;
    int days = 5;
    PeriodShdd p = PeriodShdd.parse("P" + years + "Y" + months + "M" + weaks + "W" + days + "D");

    assertEquals(years, p.getYears());
    assertEquals(months, p.getMonths());
    assertEquals(days + weaks * PeriodShdd.DAYS_IN_WEAK, p.getDays());
  }

  @Test
  public void checkParseComplexZero()
  {
    int years = 0;
    int months = 0;
    int weaks = 0;
    int days = 0;
    PeriodShdd p = PeriodShdd.parse("P" + years + "Y" + months + "M" + weaks + "W" + days + "D");

    assertEquals(years, p.getYears());
    assertEquals(months, p.getMonths());
    assertEquals(days + weaks * PeriodShdd.DAYS_IN_WEAK, p.getDays());
  }

  @Test
  public void checkParsePrimitive()
  {
    int years = 1;
    int months3 = 3;
    int months6 = 6;
    int weaks = 1;
    int days = 8;

    PeriodShdd p = PeriodShdd.parse("P" + years + "y");
    assertEquals(years, p.getYears());
    assertEquals(0, p.getMonths());
    assertEquals(0, p.getDays());

    p = PeriodShdd.parse("P" + months3 + "m");
    assertEquals(months3, p.getMonths());
    assertEquals(0, p.getYears());
    assertEquals(0, p.getDays());

    p = PeriodShdd.parse("P" + months6 + "m");
    assertEquals(months6, p.getMonths());
    assertEquals(0, p.getYears());
    assertEquals(0, p.getDays());

    p = PeriodShdd.parse("P" + weaks + "w");
    assertEquals(weaks * PeriodShdd.DAYS_IN_WEAK, p.getDays());
    assertEquals(0, p.getYears());
    assertEquals(0, p.getMonths());

    p = PeriodShdd.parse("P" + days + "d");
    assertEquals(days, p.getDays());
    assertEquals(0, p.getYears());
    assertEquals(0, p.getMonths());
  }

  @Test
  public void checkHashCode()
  {
    int years = 1;
    int months = 7;
    int days = 31;
    assertEquals(new PeriodShdd(years, months, days).hashCode(), new PeriodShdd(years, months, days).hashCode());
  }

  @Test
  public void checkEquals()
  {
    int years = 1;
    int months = 7;
    int days = 31;
    PeriodShdd p1 = new PeriodShdd(years, months, days);
    PeriodShdd sameP1 = new PeriodShdd(years, months, days);
    assertTrue(p1.equals(sameP1));
    assertTrue(p1.equals(sameP1) && p1.hashCode() == sameP1.hashCode());
    assertFalse(p1.equals(new PeriodShdd(years + 1, months, days)));
  }
}
