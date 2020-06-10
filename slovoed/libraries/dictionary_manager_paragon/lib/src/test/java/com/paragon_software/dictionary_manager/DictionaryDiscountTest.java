package com.paragon_software.dictionary_manager;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.junit.MockitoJUnitRunner;

import static org.junit.Assert.*;

@RunWith( MockitoJUnitRunner.Silent.class )
public class DictionaryDiscountTest
{
  @Test
  public void invalidDiscounts()
  {
    assertNull(DictionaryDiscount.create(0L, 0L, 1));
    assertNull(DictionaryDiscount.create(-5L, 50L, 1));
    assertNull(DictionaryDiscount.create(50L, -5L, 1));

    try
    {
      DictionaryDiscount.create(50L, 25L, 100);
    }
    catch ( Exception e )
    {
      assertTrue(e instanceof IllegalArgumentException);
    }
    try
    {
      DictionaryDiscount.create(50L, 25L, 0);
    }
    catch ( Exception e )
    {
      assertTrue(e instanceof IllegalArgumentException);
    }

    assertNull(DictionaryDiscount.create(5L, 25L));
    assertNull(DictionaryDiscount.create(5L, 5L));
  }

  @Test
  public void checkDiscountGetters()
  {
    {
      IDictionaryDiscount d = DictionaryDiscount.create(100100100L, 5L);
      assertEquals(100100100L, (long) d.getDefaultPrice());
      assertEquals(5L, (long) d.getActualPrice());
    }
    {
      IDictionaryDiscount d = DictionaryDiscount.create((long) Integer.MAX_VALUE, (long) Integer.MAX_VALUE / 2);
      assertEquals((long) Integer.MAX_VALUE, (long) d.getDefaultPrice());
      assertEquals((long) Integer.MAX_VALUE / 2, (long) d.getActualPrice());
    }
  }

  @Test
  public void checkPercentageDiscounts()
  {
    {
      IDictionaryDiscount d = DictionaryDiscount.create(10L, 5L);
      assertEquals(50, d.getPercentage());
    }
    {
      IDictionaryDiscount d = DictionaryDiscount.create(100100100L, 5L);
      assertEquals(99, d.getPercentage());
    }
    {
      IDictionaryDiscount d = DictionaryDiscount.create(100L, 99L, 1);
      assertEquals(1, d.getPercentage());
    }
    {
      IDictionaryDiscount d = DictionaryDiscount.create(100L, 67L);
      assertEquals(33, d.getPercentage());
    }
    {
      IDictionaryDiscount d = DictionaryDiscount.create(100L, 95L);
      assertEquals(5, d.getPercentage());
    }
  }

  @Test
  public void checkPercentageLimitDiscounts()
  {
    assertNull(DictionaryDiscount.create(100L, 99L));
    assertNotNull(DictionaryDiscount.create(100L, 99L, 1));
    assertNotNull(DictionaryDiscount.create(10L, 5L, 50));
    assertNull(DictionaryDiscount.create(10L, 5L, 51));
    assertNull(DictionaryDiscount.create(10L, 5L, 99));
  }

}
