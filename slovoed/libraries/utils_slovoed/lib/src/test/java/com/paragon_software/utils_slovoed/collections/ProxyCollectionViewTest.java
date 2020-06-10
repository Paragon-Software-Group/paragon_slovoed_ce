package com.paragon_software.utils_slovoed.collections;

import androidx.annotation.NonNull;

import com.paragon_software.utils_slovoed.collections.util.AnyData;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import static org.mockito.Mockito.CALLS_REAL_METHODS;
import static org.mockito.Mockito.withSettings;

public class ProxyCollectionViewTest
{
  @Test
  public void set()
  {
    BasicCollectionView<AnyData, String > MockInitializedCollectionView = getInitializedCollectionView();
    int start = 0;
    int count = MockInitializedCollectionView.getCount();
    MockCountableNotifierAll notifier = Mockito.mock(MockCountableNotifierAll.class, withSettings()
            .useConstructor(CollectionView.OPERATION_TYPE.ITEM_RANGE_INSERTED, start, count)
            .defaultAnswer(CALLS_REAL_METHODS));

    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();
    proxy.registerListener(notifier);
    proxy.set(null);
    Assert.assertEquals(0, proxy.getCount());
    Assert.assertEquals(0, notifier.counterOnItemRangeChanged);
    proxy.set(MockInitializedCollectionView);
    Assert.assertEquals(MockInitializedCollectionView.getCount(), proxy.getCount());
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
    proxy.set(null);
    Assert.assertEquals(0, proxy.getCount());
    Assert.assertEquals(1, notifier.counterOnItemRangeChanged);
    Assert.assertEquals(1, notifier2.counterOnItemRangeChanged);
  }


  @Test
  public void updateSelection()
  {
    int selection = 42;
    MockCountableNotifierAll notifier = Mockito.spy(MockCountableNotifierAll.class);
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();
    proxy.registerListener(notifier);
    proxy.updateSelection(selection);
    Assert.assertEquals(0, notifier.counterOnSelectionChange);
    Assert.assertNotEquals(selection, proxy.getSelection());

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertEquals(selection, proxy.getSelection());

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertEquals(selection, proxy.getSelection());

    selection = 7;
    proxy.updateSelection(selection);
    Assert.assertEquals(2, notifier.counterOnSelectionChange);
    Assert.assertEquals(selection, proxy.getSelection());

    proxy.unregisterListener(notifier);
    proxy.updateSelection(selection);
    Assert.assertEquals(2, notifier.counterOnSelectionChange);

    Assert.assertEquals(1, notifier.counterOnMetadataChanged);
    Assert.assertEquals(0, notifier.counterOnScrollToPosition);
    Assert.assertEquals(1, notifier.counterOnProgressChanged);
  }

  @Test
  public void updateSelectionResetIfInnerCollectionChanges()
  {
    int selection = 42;
    MockCountableNotifierAll notifier = Mockito.spy(MockCountableNotifierAll.class);
    ProxyCollectionView< AnyData, String > proxy = new ProxyCollectionView<>(null, false, null, false);
    proxy.registerListener(notifier);
    proxy.updateSelection(selection);
    Assert.assertEquals(0, notifier.counterOnSelectionChange);
    Assert.assertNotEquals(selection, proxy.getSelection());

    BasicCollectionView< AnyData, String > MockInitializedCollectionView = getInitializedCollectionView();
    proxy.set(MockInitializedCollectionView);
    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertEquals(MockInitializedCollectionView.getSelection(), proxy.getSelection());

    MockInitializedCollectionView = getInitializedCollectionView();
    proxy.set(MockInitializedCollectionView);
    Assert.assertEquals(1, notifier.counterOnSelectionChange);
    Assert.assertEquals(MockInitializedCollectionView.getSelection(), proxy.getSelection());

    selection = 7;
    proxy.updateSelection(selection);
    Assert.assertEquals(2, notifier.counterOnSelectionChange);
    Assert.assertEquals(selection, proxy.getSelection());

    proxy.unregisterListener(notifier);
    proxy.updateSelection(selection);
    Assert.assertEquals(2, notifier.counterOnSelectionChange);

    Assert.assertEquals(1, notifier.counterOnMetadataChanged);
    Assert.assertEquals(0, notifier.counterOnScrollToPosition);
    Assert.assertEquals(1, notifier.counterOnProgressChanged);
  }


  @Test
  public void getCount()
  {
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();
    Assert.assertEquals(proxy.getCount(), 0);

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(proxy.getCount(), 0);

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.getCount(), MockCollectionView.COUNT);

    proxy.set(null);
    Assert.assertEquals(proxy.getCount(), 0);
  }

  @Test
  public void getItem()
  {
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();
    Assert.assertEquals(proxy.getItem(0), null);

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(proxy.getItem(0), null);

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.getItem(MockCollectionView.POSITION), MockCollectionView.ITEM);

    proxy.set(null);
    Assert.assertEquals(proxy.getItem(0), null);
  }

  @Test
  public void getMetadata()
  {
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();
    Assert.assertEquals(proxy.getMetadata(), null);

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(proxy.getMetadata(), null);

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.getMetadata(), MockCollectionView.METADATA);

    proxy.set(null);
    Assert.assertEquals(proxy.getMetadata(), null);
  }

  @Test
  public void getSelection()
  {
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();
    Assert.assertEquals(-1, proxy.getSelection());

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(-1, proxy.getSelection());

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(MockCollectionView.SELECTION, proxy.getSelection());

    proxy.set(null);
    Assert.assertEquals(-1, proxy.getSelection());
  }

  @Test
  public void isInProgress()
  {
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();
    Assert.assertEquals(proxy.isInProgress(), false);

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(proxy.isInProgress(), false);

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.isInProgress(), MockCollectionView.IN_PROGRESS);

    proxy.set(null);
    Assert.assertEquals(proxy.isInProgress(), false);
  }

  @Test
  public void getPosition()
  {
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();
    Assert.assertEquals(Integer.MIN_VALUE, proxy.getPosition());

    proxy.set(getEmptyCollectionView());
    Assert.assertEquals(proxy.getPosition(), Integer.MIN_VALUE);

    proxy.set(getInitializedCollectionView());
    Assert.assertEquals(proxy.getPosition(), MockCollectionView.POSITION);

    proxy.set(null);
    Assert.assertEquals(Integer.MIN_VALUE, proxy.getPosition());
  }

  @Test
  public void callOnProgressChanged()
  {
    BasicCollectionView< AnyData, String > icv = getInitializedCollectionView();
    MockCountableNotifierAll notifier = Mockito.spy(MockCountableNotifierAll.class);
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();

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
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();

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
    ProxyCollectionView< AnyData, String > proxy = getProxyDefault();

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

  private BasicCollectionView< AnyData, String > getInitializedCollectionView()
  {
    return Mockito.spy(MockCollectionView.class);
  }

  @NonNull
  private ProxyCollectionView< AnyData, String > getProxyDefault()
  {
    return new ProxyCollectionView<>();
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
