/*
 * license_manager_api
 *
 *  Created on: 08.02.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.license_manager_api.test;

import java.util.Currency;

import com.paragon_software.dictionary_manager.FeatureName;
import com.paragon_software.license_manager_api.LicenseFeature;
import com.paragon_software.license_manager_api.Purchase;
import com.paragon_software.license_manager_api.SKU;
import org.junit.Test;

import static org.junit.Assert.*;

public class LicenseFeatureTest
{
  final private static FeatureName   featureName = new FeatureName("testFeature");
  final private static SKU      sku1        = new SKU("SKU1", "TestSource", Currency.getInstance("USD"), 2201);
  final private static SKU      sku2        = new SKU("SKU2", "TestSource", Currency.getInstance("USD"), 2202);
  final private static Purchase purchase    = new Purchase("Oid1", "SKU1", 123, "T1", "E1", "");

  @Test
  public void featureStateStrings() throws Exception
  {
    assertEquals("FeatureState disable invalid", "DISABLED", LicenseFeature.FEATURE_STATE.DISABLED.toString());
    assertEquals("FeatureState available invalid", "AVAILABLE", LicenseFeature.FEATURE_STATE.AVAILABLE.toString());
    assertEquals("FeatureState enabled invalid", "ENABLED", LicenseFeature.FEATURE_STATE.ENABLED.toString());
  }

  @Test
  public void create_feature_with_name() throws Exception
  {
    final LicenseFeature feature = new LicenseFeature(featureName);

    assertEquals("Name not equal", featureName, feature.getName());
    assertNull("Not empty SKU", feature.getSKU());
    assertNull("Not empty Purchase", feature.getPurchase());
  }

  @Test
  public void create_feature_with_purchase() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(feature1, purchase);

    assertEquals("Name not equal", featureName, feature1.getName());
    assertNull("Not empty SKU", feature1.getSKU());
    assertNull("Not empty Purchase", feature1.getPurchase());

    assertEquals("Name not equal", featureName, feature2.getName());
    assertNull("Not empty SKU", feature2.getSKU());
    assertNotNull("Is empty Purchase", feature2.getPurchase());
    assertEquals("Purchase not equal", purchase, feature2.getPurchase());
  }

  @Test
  public void create_feature_with_sku() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(feature1, sku1);
    final LicenseFeature feature3 = new LicenseFeature(feature2, sku2);
    final LicenseFeature feature4 = new LicenseFeature(feature3, sku1);

    assertEquals("Name not equal", featureName, feature1.getName());
    assertNull("Not empty SKU", feature1.getSKU());
    assertNull("Not empty Purchase", feature1.getPurchase());

    assertEquals("Name not equal", featureName, feature2.getName());
    assertNotNull("Is empty SKU", feature2.getSKU());
    assertNull("Not empty Purchase", feature2.getPurchase());
    assertArrayEquals("SKU not valid", new SKU[]{ sku1 }, feature2.getSKU());

    assertEquals("Name not equal", featureName, feature3.getName());
    assertNotNull("Is empty SKU", feature3.getSKU());
    assertNull("Not empty Purchase", feature3.getPurchase());
    assertArrayEquals("SKU not valid", new SKU[]{ sku1, sku2 }, feature3.getSKU());

    assertEquals("Name not equal", featureName, feature4.getName());
    assertNotNull("Is empty SKU", feature4.getSKU());
    assertNull("Not empty Purchase", feature4.getPurchase());
    assertArrayEquals("SKU not valid", new SKU[]{ sku1, sku2 }, feature4.getSKU());
  }

  @Test
  public void attachFeature() throws Exception
  {
    final Purchase purchase2 = new Purchase("Oid1", "SKU1", 123, "T1", "E1", "");
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(feature1, sku1);
    final LicenseFeature feature3 = new LicenseFeature(feature2, sku2);
    final LicenseFeature feature4 = new LicenseFeature(feature1, purchase);
    final LicenseFeature feature5 = new LicenseFeature(feature1, purchase2);
    final LicenseFeature feature6 = new LicenseFeature(new FeatureName("test"));
    final LicenseFeature testFeature1 = feature2.merge(feature3);
    final LicenseFeature testFeature2 = testFeature1.merge(feature4);
    final LicenseFeature testFeature3 = testFeature2.merge(feature5);
    final LicenseFeature testFeature4 = testFeature3.merge(feature6);

    assertEquals("Name not equal", featureName, testFeature1.getName());
    assertNotNull("Is empty SKU", testFeature1.getSKU());
    assertNull("Not empty Purchase", testFeature1.getPurchase());
    assertArrayEquals("SKU not valid", new SKU[]{ sku1, sku2 }, testFeature1.getSKU());

    assertEquals("Name not equal", featureName, testFeature2.getName());
    assertNotNull("Is empty SKU", testFeature2.getSKU());
    assertNotNull("Is empty Purchase", testFeature2.getPurchase());
    assertEquals("Purchase not equal", purchase, testFeature2.getPurchase());
    assertArrayEquals("SKU not valid", new SKU[]{ sku1, sku2 }, testFeature2.getSKU());

    assertEquals("Name not equal", featureName, testFeature3.getName());
    assertNotNull("Is empty SKU", testFeature3.getSKU());
    assertNotNull("Is empty Purchase", testFeature3.getPurchase());
    assertEquals("Purchase not equal", purchase2, testFeature3.getPurchase());
    assertArrayEquals("SKU not valid", new SKU[]{ sku1, sku2 }, testFeature3.getSKU());

    assertEquals("Name not equal", featureName, testFeature4.getName());
    assertNotNull("Is empty SKU", testFeature4.getSKU());
    assertNotNull("Is empty Purchase", testFeature4.getPurchase());
    assertEquals("Purchase not equal", purchase2, testFeature4.getPurchase());
    assertArrayEquals("SKU not valid", new SKU[]{ sku1, sku2 }, testFeature4.getSKU());
  }

  @Test
  public void check_feature_state() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(feature1, sku1);
    final LicenseFeature feature3 = new LicenseFeature(feature1, purchase);
    final LicenseFeature feature4 = new LicenseFeature(feature2, purchase);
    assertEquals("Invalid state", LicenseFeature.FEATURE_STATE.DISABLED, feature1.getState());
    assertEquals("Invalid state", LicenseFeature.FEATURE_STATE.AVAILABLE, feature2.getState());
    assertEquals("Invalid state", LicenseFeature.FEATURE_STATE.ENABLED, feature3.getState());
    assertEquals("Invalid state", LicenseFeature.FEATURE_STATE.ENABLED, feature4.getState());
  }

  @Test
  public void equalFeature() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(featureName);

    final LicenseFeature feature3 = new LicenseFeature(feature1, sku1);
    final LicenseFeature feature4 = new LicenseFeature(feature2, sku2);

    assertEquals("Feature not equal", feature1, feature2);
    assertEquals("Feature not equal", feature2, feature1);
    assertEquals("Feature not equal", feature4, feature3);
    assertEquals("Feature not equal", feature3, feature4);
    assertEquals("Feature not equal", feature1, feature4);
    assertEquals("Feature not equal", feature1, feature3);
    assertEquals("Feature not equal", feature2, feature4);
    assertEquals("Feature not equal", feature2, feature3);
    assertEquals("Feature not equal", feature4, feature1);
    assertEquals("Feature not equal", feature3, feature1);
    assertEquals("Feature not equal", feature4, feature2);
    assertEquals("Feature not equal", feature3, feature2);
  }

  @Test
  public void notEqualFeature() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(new FeatureName("test Name 2"));
    assertNotEquals("Feature is equal", feature1, feature2);
    assertNotEquals("Feature is equal", feature2, feature1);
  }

  @Test
  public void notEqualExtraFeature() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    assertEquals("Feature is equal", feature1, feature1);
    assertNotEquals("Feature is equal", feature1, null);
    assertNotEquals("Feature is equal", feature1, "123");
  }

  @Test
  public void equalHashFeature() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(featureName);
    assertEquals("Feature hash not equal", feature1.hashCode(), feature2.hashCode());
  }

  @Test
  public void notEqualHashFeature() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(new FeatureName("test Name 2"));
    assertNotEquals("Feature hash is equal", feature1.hashCode(), feature2.hashCode());
  }

  @Test
  public void cloneFeature() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(new LicenseFeature(featureName), sku1);
    final LicenseFeature feature2 = new LicenseFeature(feature1, sku2);
    final LicenseFeature feature3 = new LicenseFeature(feature2, purchase);
    final LicenseFeature feature4 = (LicenseFeature) feature3.clone();

    assertNotSame("Is same object", feature4, feature3);
    assertEquals("Is not same feature", feature4, feature3);
    assertEquals("Is not same feature.name", feature4.getName(), feature3.getName());
    assertEquals("Is not same feature.purchase", feature4.getPurchase(), feature3.getPurchase());
    assertNotSame("Is same array", feature4.getSKU(), feature3.getSKU());
    assertArrayEquals("Is not same feature.SKU", feature4.getSKU(), feature3.getSKU());
  }

  @Test
  public void FeatureToString() throws Exception
  {
    final LicenseFeature feature1 = new LicenseFeature(featureName);
    final LicenseFeature feature2 = new LicenseFeature(feature1, sku2);
    final LicenseFeature feature3 = new LicenseFeature(feature2, purchase);
    assertEquals("Invalid string information", "{name='" + featureName + "', state=DISABLED}", feature1.toString());
    assertEquals("Invalid string information", "{name='" + featureName + "', state=AVAILABLE}", feature2.toString());
    assertEquals("Invalid string information", "{name='" + featureName + "', state=ENABLED}", feature3.toString());
  }
}
