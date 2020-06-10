/*
 * license_manager_api
 *
 *  Created on: 09.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api.test;

import com.paragon_software.license_manager_api.Purchase;
import org.junit.Test;

import static org.junit.Assert.*;

public class PurchaseTest
{
  @Test
  public void createPurchase() throws Exception
  {
    final String orderId = "E1F3CF75-749C-4E6A-88FD-3289CF9365EE";
    final String skuId = "gp_slovoedEnDictionaryFull";
    final long purchaseTime = 125564;
    final String purchaseToken = "C6AE477A-5E34-47F9-8C6A-7E9D7492001E";
    //noinspection SpellCheckingInspection
    final String extra = "Qm80fCV8MTIzT3wlMXwlfFVzZXIxfCV8VXNlckBjb21wYW55LmNvbQo=";

    final Purchase purchase = new Purchase(orderId, skuId, purchaseTime, purchaseToken, extra, "");
    assertEquals("Invalid Order Id", orderId, purchase.getOrderId());
    assertEquals("Invalid SKU Id", skuId, purchase.getSkuId());
    assertEquals("Invalid purchase token", purchaseToken, purchase.getPurchaseToken());
    assertEquals("Invalid purchase time", purchaseTime, purchase.getPurchaseTime());
    assertEquals("Invalid extra data", extra, purchase.getExtra());
  }

  @Test
  public void equalPurchase() throws Exception
  {
    final String orderId1 = "E1F3CF75-749C-4E6A-88FD-3289CF9365EE";
    final String skuId = "gp_slovoedEnDictionaryFull";
    final long purchaseTime = 125564;
    final String purchaseToken = "C6AE477A-5E34-47F9-8C6A-7E9D7492001E";
    //noinspection SpellCheckingInspection
    final String extra = "Qm80fCV8MTIzT3wlMXwlfFVzZXIxfCV8VXNlckBjb21wYW55LmNvbQo=";

    final Purchase purchase1 = new Purchase(orderId1, skuId, purchaseTime, purchaseToken, extra, "");
    final Purchase purchase2 = new Purchase(orderId1, skuId, purchaseTime, purchaseToken, extra, "");
    assertEquals("Purchase not equal", purchase1, purchase2);
    assertEquals("Purchase not equal", purchase2, purchase1);
  }

  @Test
  public void notEqualPurchase() throws Exception
  {
    final String orderId1 = "E1F3CF75-749C-4E6A-88FD-3289CF9365EE";
    //noinspection SpellCheckingInspection
    final String orderId2 = "F47D5752-7056-478E-9D0B-DAE087BABAD6";
    final String skuId = "gp_slovoedEnDictionaryFull";
    final long purchaseTime = 125564;
    final String purchaseToken = "C6AE477A-5E34-47F9-8C6A-7E9D7492001E";
    //noinspection SpellCheckingInspection
    final String extra = "Qm80fCV8MTIzT3wlMXwlfFVzZXIxfCV8VXNlckBjb21wYW55LmNvbQo=";

    final Purchase purchase1 = new Purchase(orderId1, skuId, purchaseTime, purchaseToken, extra, "");
    final Purchase purchase2 = new Purchase(orderId2, skuId, purchaseTime, purchaseToken, extra, "");
    assertNotEquals("Purchase is equal", purchase1, purchase2);
    assertNotEquals("Purchase is equal", purchase2, purchase1);
  }

  @Test
  public void equalExtraPurchase() throws Exception
  {
    final String orderId1 = "E1F3CF75-749C-4E6A-88FD-3289CF9365EE";
    final String skuId = "gp_slovoedEnDictionaryFull";
    final long purchaseTime = 125564;
    final String purchaseToken = "C6AE477A-5E34-47F9-8C6A-7E9D7492001E";
    //noinspection SpellCheckingInspection
    final String extra = "Qm80fCV8MTIzT3wlMXwlfFVzZXIxfCV8VXNlckBjb21wYW55LmNvbQo=";

    final Purchase purchase1 = new Purchase(orderId1, skuId, purchaseTime, purchaseToken, extra, "");
    assertEquals("Purchase not equal", purchase1, purchase1);
    assertNotEquals("Purchase is equal", purchase1, null);
    assertNotEquals("Purchase is equal", purchase1, "123");
  }

  @Test
  public void equalHashPurchase() throws Exception
  {
    final String orderId1 = "E1F3CF75-749C-4E6A-88FD-3289CF9365EE";
    final String skuId = "gp_slovoedEnDictionaryFull";
    final long purchaseTime = 125564;
    final String purchaseToken = "C6AE477A-5E34-47F9-8C6A-7E9D7492001E";
    //noinspection SpellCheckingInspection
    final String extra = "Qm80fCV8MTIzT3wlMXwlfFVzZXIxfCV8VXNlckBjb21wYW55LmNvbQo=";

    final Purchase purchase1 = new Purchase(orderId1, skuId, purchaseTime, purchaseToken, extra, "");
    final Purchase purchase2 = new Purchase(orderId1, skuId, purchaseTime, purchaseToken, extra, "");
    assertEquals("Purchase hash not equal", purchase1.hashCode(), purchase2.hashCode());
  }

  @Test
  public void notEqualHashPurchase() throws Exception
  {
    final String orderId1 = "E1F3CF75-749C-4E6A-88FD-3289CF9365EE";
    //noinspection SpellCheckingInspection
    final String orderId2 = "F47D5752-7056-478E-9D0B-DAE087BABAD6";
    final String skuId = "gp_slovoedEnDictionaryFull";
    final long purchaseTime = 125564;
    final String purchaseToken = "C6AE477A-5E34-47F9-8C6A-7E9D7492001E";
    //noinspection SpellCheckingInspection
    final String extra = "Qm80fCV8MTIzT3wlMXwlfFVzZXIxfCV8VXNlckBjb21wYW55LmNvbQo=";

    final Purchase purchase1 = new Purchase(orderId1, skuId, purchaseTime, purchaseToken, extra, "");
    final Purchase purchase2 = new Purchase(orderId2, skuId, purchaseTime, purchaseToken, extra, "");
    assertNotEquals("Purchase hash is equal", purchase1.hashCode(), purchase2.hashCode());
  }

  @Test
  public void clonePurchase() throws Exception
  {
    final String orderId1 = "E1F3CF75-749C-4E6A-88FD-3289CF9365EE";
    final String skuId = "gp_slovoedEnDictionaryFull";
    final long purchaseTime = 125564;
    final String purchaseToken = "C6AE477A-5E34-47F9-8C6A-7E9D7492001E";
    //noinspection SpellCheckingInspection
    final String extra = "Qm80fCV8MTIzT3wlMXwlfFVzZXIxfCV8VXNlckBjb21wYW55LmNvbQo=";

    final Purchase purchase1 = new Purchase(orderId1, skuId, purchaseTime, purchaseToken, extra, "");
    final Purchase result = (Purchase) purchase1.clone();

    assertNotSame("Is same object", purchase1, result);
    assertEquals("Is not same Purchase", purchase1, result);
    assertEquals("Is not same Purchase.OrderId", purchase1.getOrderId(), result.getOrderId());
    assertEquals("Is not same Purchase.SkuId", purchase1.getSkuId(), result.getSkuId());
    assertEquals("Is not same Purchase.time", purchase1.getPurchaseTime(), result.getPurchaseTime());
    assertEquals("Is not same Purchase.token", purchase1.getPurchaseToken(), result.getPurchaseToken());
    assertEquals("Is not same Purchase.extra", purchase1.getExtra(), result.getExtra());
  }

  @Test
  public void stringPurchase() throws Exception
  {
    final Purchase purchase1 = new Purchase("O1", "S1", 123, "PT1", "E1", "");
    assertEquals("Purchase info not valid", "{orderId='O1', skuId='S1', purchaseTime=123}", purchase1.toString());
  }
}
