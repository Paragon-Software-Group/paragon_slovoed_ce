/*
 * license_manager_api
 *
 *  Created on: 12.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api.test;

import java.time.Period;
import java.util.Calendar;
import java.util.Currency;
import java.util.Date;

import com.paragon_software.license_manager_api.PeriodShdd;
import com.paragon_software.license_manager_api.SKU;
import org.junit.Test;

import static org.junit.Assert.*;

public class SKUTest
{
  @Test
  public void createSKU() throws Exception
  {
    final String id = "C9D92D9B-024C-4763-A8EF-1D09421CB558";
    final String sourceName = "play.google.com";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2201;

    final SKU result = new SKU(id, sourceName, priceCurrency, priceValue);
    assertEquals("Id not equal", id, result.getId());
    assertEquals("Source not equal", sourceName, result.getSourceName());
    assertEquals("Currency not equal", priceCurrency, result.getPriceCurrency());
    assertEquals("Price not equal", priceValue, result.getPriceValue());
  }

  @Test
  public void equalSKU() throws Exception
  {
    final String id = "C9D92D9B-024C-4763-A8EF-1D09421CB558";
    final String sourceName = "play.google.com";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2202;

    final SKU result1 = new SKU(id, sourceName, priceCurrency, priceValue);
    final SKU result2 = new SKU(id, sourceName, priceCurrency, priceValue);
    assertEquals("SKU not equal", result1, result2);
    assertEquals("SKU not equal", result2, result1);
  }

  @Test
  public void notEqualSKU() throws Exception
  {
    final String id1 = "C9D92D9B-024C-4763-A8EF-1D09421CB558";
    final String id2 = "8913913F-F8BF-4287-B8CB-2786A100CE59";
    final String sourceName = "play.google.com";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2203;

    final SKU result1 = new SKU(id1, sourceName, priceCurrency, priceValue);
    final SKU result2 = new SKU(id2, sourceName, priceCurrency, priceValue);
    assertNotEquals("SKU is equal", result1, result2);
    assertNotEquals("SKU is equal", result2, result1);
  }

  @Test
  public void equalExtraSKU() throws Exception
  {
    final String id = "C9D92D9B-024C-4763-A8EF-1D09421CB558";
    final String sourceName = "play.google.com";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2204;

    final SKU result1 = new SKU(id, sourceName, priceCurrency, priceValue);
    assertEquals("SKU not equal", result1, result1);
    assertNotEquals("SKU not equal", result1, null);
    assertNotEquals("SKU not equal", result1, "123");
  }

  @Test
  public void equalHashSKU() throws Exception
  {
    final String id = "C9D92D9B-024C-4763-A8EF-1D09421CB558";
    final String sourceName = "play.google.com";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2205;

    final SKU result1 = new SKU(id, sourceName, priceCurrency, priceValue);
    final SKU result2 = new SKU(id, sourceName, priceCurrency, priceValue);
    assertEquals("SKU hash not equal", result1.hashCode(), result2.hashCode());
  }

  @Test
  public void notEqualHashSKU() throws Exception
  {
    final String id1 = "C9D92D9B-024C-4763-A8EF-1D09421CB558";
    final String id2 = "8913913F-F8BF-4287-B8CB-2786A100CE59";
    final String sourceName = "play.google.com";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2206;

    final SKU result1 = new SKU(id1, sourceName, priceCurrency, priceValue);
    final SKU result2 = new SKU(id2, sourceName, priceCurrency, priceValue);
    assertNotEquals("SKU hash is equal", result1.hashCode(), result2.hashCode());
  }

  @Test
  public void SKUtoString() throws Exception
  {
    final String id = "ID1";
    final String sourceName = "SRC1";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 22070000;

    final SKU result1 = new SKU(id, sourceName, priceCurrency, priceValue);
    assertEquals("SKU info not valid", "{id='ID1', source='SRC1', price=22.07'$'}", result1.toString());
  }

  @Test
  public void cloneSKU() throws Exception
  {
    final String id = "ID1";
    final String sourceName = "SRC1";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2208;

    final SKU result1 = new SKU(id, sourceName, priceCurrency, priceValue);
    final SKU result2 = (SKU) result1.clone();
    assertNotSame("Is SameObject", result1, result2);
    assertEquals("Is not same SKU", result1, result2);
    assertEquals("Is not same SKU.Id", result1.getId(), result2.getId());
    assertEquals("Is not same SKU.Source", result1.getSourceName(), result2.getSourceName());
    assertEquals("Is not same SKU.Currency", result1.getPriceCurrency(), result2.getPriceCurrency());
    assertEquals("Is not same SKU.Price", result1.getPriceValue(), result2.getPriceValue());
  }

  @Test
  public void createSubscribe() throws Exception
  {
    final String id = "ID1";
    final String sourceName = "SRC1";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2208;
    final Date subscribeEndDate = new Date(123);
    final PeriodShdd period = PeriodShdd.parse("P3M");
    final SKU sku = new SKU(id, sourceName, priceCurrency, priceValue, subscribeEndDate, period);
    assertTrue("Is not subscribe", sku.isSubscription());
  }

  @Test
  public void subscribeExpire() throws Exception
  {
    final String id = "ID1";
    final String sourceName = "SRC1";
    final Currency priceCurrency = Currency.getInstance("USD");
    final long priceValue = 2208;
    final Date nowDate = new Date();
    final Calendar calendar = Calendar.getInstance();
    calendar.setTime(nowDate);
    calendar.add(Calendar.HOUR, 2);
    final Date futureDate = calendar.getTime();
    calendar.add(Calendar.HOUR, -4);
    final Date pastDate = calendar.getTime();
    final PeriodShdd period = PeriodShdd.parse("P1W");
    final SKU skuValid = new SKU(id, sourceName, priceCurrency, priceValue, futureDate, period);
    final SKU skuExpired = new SKU(id, sourceName, priceCurrency, priceValue, pastDate, period);
    assertFalse("Is not valid subscribe", skuValid.isExpire());
    assertTrue("Is valid subscribe", skuExpired.isExpire());
    assertEquals("Invalid end date", futureDate, skuValid.getEndDate());
    assertEquals("Invalid end date", pastDate, skuExpired.getEndDate());
  }
}
