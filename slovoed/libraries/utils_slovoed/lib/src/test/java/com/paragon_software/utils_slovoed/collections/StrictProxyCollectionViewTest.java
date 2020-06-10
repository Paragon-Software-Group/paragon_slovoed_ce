package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.util.AnyData;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.withSettings;

public class StrictProxyCollectionViewTest
{
  @Test
  public void set()
  {
    BasicCollectionView< AnyData, String > initializedCollectionView = getInitializedCollectionView();
    int start = 0;
    int count = initializedCollectionView.getCount();
    MockCountableNotifierAll notifier = Mockito.mock(MockCountableNotifierAll.class, withSettings()
            .useConstructor(CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, start, count)
            .defaultAnswer(CALLS_REAL_METHODS));

    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();
    proxy.registerListener(notifier);
    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(0, proxy.getCount());
    Assert.assertEquals(0, notifier.counterOnItemRangeChanged);
    proxy.set(initializedCollectionView);
    Assert.assertEquals(initializedCollectionView.getCount(), proxy.getCount());
    Assert.assertEquals(1, notifier.counterOnItemRangeChanged);

    BasicCollectionView< AnyData, String > initializedCollectionView2 = getInitializedCollectionView();
    count = initializedCollectionView2.getCount();
    MockCountableNotifierAll notifier2 = Mockito.mock(MockCountableNotifierAll.class, withSettings()
            .useConstructor(CollectionView.OPERATION_TYPE.ITEM_RANGE_CHANGED, start, count)
            .defaultAnswer(CALLS_REAL_METHODS));

    proxy.registerListener(notifier2);
    proxy.set(initializedCollectionView2);
    Assert.assertEquals(initializedCollectionView2.getCount(), proxy.getCount());
    Assert.assertEquals(1, notifier.counterOnItemRangeChanged);
    Assert.assertEquals(1, notifier2.counterOnItemRangeChanged);
    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(0, proxy.getCount());
    Assert.assertEquals(1, notifier.counterOnItemRangeChanged);
    Assert.assertEquals(1, notifier2.counterOnItemRangeChanged);
  }


  @Test
  public void updateSelection()
  {
    int selection = 42;
    MockCountableNotifierAll notifier = Mockito.spy(MockCountableNotifierAll.class);
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();
    proxy.registerListener(notifier);
    proxy.updateSelection(selection);
    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertNotEquals(selection, proxy.getSelection());

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(2, notifier.counterOnSelectionChange);
    Assert.assertEquals(selection, proxy.getSelection());

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(2, notifier.counterOnSelectionChange);
    Assert.assertEquals(selection, proxy.getSelection());

    selection = 7;
    proxy.updateSelection(selection);
    Assert.assertEquals(3, notifier.counterOnSelectionChange);
    Assert.assertEquals(selection, proxy.getSelection());

    proxy.unregisterListener(notifier);
    proxy.updateSelection(selection);
    Assert.assertEquals(3, notifier.counterOnSelectionChange);

    Assert.assertEquals(1, notifier.counterOnMetadataChanged);
    Assert.assertEquals(0, notifier.counterOnScrollToPosition);
    Assert.assertEquals(1, notifier.counterOnProgressChanged);
  }

  @Test
  public void updateSelectionResetIfInnerCollectionChanges()
  {
    int selection = 42;
    MockCountableNotifierAll notifier = Mockito.spy(MockCountableNotifierAll.class);
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = new StrictProxyCollectionView<>(getEmptyCollectionView(), false);
    proxy.registerListener(notifier);
    proxy.updateSelection(selection);
    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertNotEquals(selection, proxy.getSelection());

    BasicCollectionView< AnyData, String > initializedCollectionView = getInitializedCollectionView();
    proxy.set(initializedCollectionView);
    Assert.assertEquals(2, notifier.counterOnSelectionChange);
    Assert.assertEquals(initializedCollectionView.getSelection(), proxy.getSelection());

    initializedCollectionView = getInitializedCollectionView();
    proxy.set(initializedCollectionView);
    Assert.assertEquals(2, notifier.counterOnSelectionChange);
    Assert.assertEquals(initializedCollectionView.getSelection(), proxy.getSelection());

    selection = 7;
    proxy.updateSelection(selection);
    Assert.assertEquals(3, notifier.counterOnSelectionChange);
    Assert.assertEquals(selection, proxy.getSelection());

    proxy.unregisterListener(notifier);
    proxy.updateSelection(selection);
    Assert.assertEquals(3, notifier.counterOnSelectionChange);

    Assert.assertEquals(1, notifier.counterOnMetadataChanged);
    Assert.assertEquals(0, notifier.counterOnScrollToPosition);
    Assert.assertEquals(1, notifier.counterOnProgressChanged);
  }


  @Test
  public void getCount()
  {
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();
    Assert.assertEquals(proxy.getCount(), 0);

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(proxy.getCount(), 0);

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.getCount(), MockCollectionView.COUNT);

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(proxy.getCount(), 0);
  }

  @Test
  public void getItem()
  {
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();
    Assert.assertEquals(0, proxy.getCount());

    proxy.set(getEmptyCollectionView());
    Assert.assertNull(proxy.getItem(0));

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.getItem(MockCollectionView.POSITION), MockCollectionView.ITEM);

    proxy.set(getEmptyCollectionView());
    Assert.assertNull(proxy.getItem(0));
  }

  @Test
  public void getMetadata()
  {
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();
    Assert.assertNull(proxy.getMetadata());

    proxy.set(getEmptyCollectionView());
    Assert.assertNull(proxy.getMetadata());

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.getMetadata(), MockCollectionView.METADATA);

    proxy.set(getEmptyCollectionView());
    Assert.assertNull(proxy.getMetadata());
  }

  @Test
  public void getSelection()
  {
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();
    Assert.assertEquals(-1, proxy.getSelection());

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(-1, proxy.getSelection());

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(MockCollectionView.SELECTION, proxy.getSelection());

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(-1, proxy.getSelection());
  }

  @Test
  public void isInProgress()
  {
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();
    Assert.assertFalse(proxy.isInProgress());

    proxy.set(getEmptyCollectionView());
    Assert.assertFalse(proxy.isInProgress());

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.isInProgress(), MockCollectionView.IN_PROGRESS);

    proxy.set(getEmptyCollectionView());
    Assert.assertFalse(proxy.isInProgress());
  }

  @Test
  public void getPosition()
  {
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();
    Assert.assertEquals(Integer.MIN_VALUE, proxy.getPosition());

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(proxy.getPosition(), Integer.MIN_VALUE);

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.getPosition(), MockCollectionView.POSITION);

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(Integer.MIN_VALUE, proxy.getPosition());
  }

  @Test
  public void callOnProgressChanged()
  {
    BasicCollectionView< AnyData, String > icv = getInitializedCollectionView();
    MockCountableNotifierAll notifier = Mockito.spy(MockCountableNotifierAll.class);
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();

    proxy.registerListener(notifier);

    proxy.set(icv);
    Assert.assertEquals(1, notifier.counterOnProgressChanged);

    icv.callOnProgressChanged();
    Assert.assertEquals(2, notifier.counterOnProgressChanged);

    proxy.unregisterListener(notifier);
    icv.callOnProgressChanged();
    Assert.assertEquals(2, notifier.counterOnProgressChanged);

    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertEquals(1, notifier.counterOnMetadataChanged);
    Assert.assertEquals(0, notifier.counterOnScrollToPosition);
  }

  @Test
  public void updatePosition()
  {
    int position = 42;
    BasicCollectionView< AnyData, String > icv = getInitializedCollectionView();
    MockCountableNotifierAll notifier = Mockito.spy(MockCountableNotifierAll.class);
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();

    proxy.registerListener(notifier);
    Assert.assertEquals(0, notifier.counterOnScrollToPosition);
    Assert.assertEquals(Integer.MIN_VALUE, proxy.getPosition());

    proxy.set(icv);
    icv.updatePosition(position);
    Assert.assertEquals(1, notifier.counterOnScrollToPosition);
    Assert.assertEquals(icv.getPosition(), proxy.getPosition());

    position = 7;
    icv.updatePosition(position);
    Assert.assertEquals(2, notifier.counterOnScrollToPosition);
    Assert.assertEquals(position, proxy.getPosition());
    Assert.assertEquals(position, icv.getPosition());

    proxy.unregisterListener(notifier);
    icv.updatePosition(position);
    Assert.assertEquals(2, notifier.counterOnScrollToPosition);

    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertEquals(1, notifier.counterOnMetadataChanged);
    Assert.assertEquals(1, notifier.counterOnProgressChanged);
  }

  @Test
  public void updateMetadata()
  {
    String metadata = "42";
    BasicCollectionView< AnyData, String > icv = getInitializedCollectionView();
    MockCountableNotifierAll notifier = Mockito.spy(MockCountableNotifierAll.class);
    StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> proxy = getProxyDefault();

    proxy.registerListener(notifier);
    proxy.set(icv);
    Assert.assertEquals(1, notifier.counterOnMetadataChanged);

    icv.updateMetadata(metadata);
    Assert.assertEquals(2, notifier.counterOnMetadataChanged);
    Assert.assertEquals(metadata, icv.getMetadata());

    metadata = "7";
    icv.updateMetadata(metadata);
    Assert.assertEquals(3, notifier.counterOnMetadataChanged);
    Assert.assertEquals(metadata, proxy.getMetadata());
    Assert.assertEquals(metadata, icv.getMetadata());

    proxy.unregisterListener(notifier);
    icv.updateMetadata(metadata);
    Assert.assertEquals(3, notifier.counterOnMetadataChanged);

    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertEquals(0, notifier.counterOnScrollToPosition);
    Assert.assertEquals(1, notifier.counterOnProgressChanged);
  }

  private BasicCollectionView<AnyData, String > getInitializedCollectionView()
  {
    return Mockito.spy(MockCollectionView.class);
  }

  @NonNull
  private StrictProxyCollectionView< AnyData, String, CollectionView<AnyData, String>> getProxyDefault()
  {
    return new StrictProxyCollectionView<>((CollectionView<AnyData, String>) new ArrayCollectionView<AnyData, String>());
  }

  private CollectionView< AnyData, String > getEmptyCollectionView()
  {
    return new BasicCollectionView< AnyData, String >()
    {
      @Override
      public int getCount()
      {
        return 0;
      }

      @Override
      public AnyData getItem( int position )
      {
        return null;
      }
    };
  }
}
