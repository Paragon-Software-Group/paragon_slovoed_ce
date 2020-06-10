/*
 * dictionary_manager_factory
 *
 *  Created on: 23.03.18
 *      Author: Ivan Zhdanov
 * Last Modify: $Id$
 *
 * Copyright © 1994-2018 Paragon Technologie GmbH.
 */

package com.paragon_software.dictionary_manager_factory;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.drawable.Drawable;
import android.os.Build;
import androidx.annotation.IdRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.graphics.drawable.DrawableCompat;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.appcompat.content.res.AppCompatResources;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryPack;
import com.paragon_software.dictionary_manager.FeatureName;
import com.paragon_software.dictionary_manager.IDictionaryIcon;
import com.paragon_software.dictionary_manager.IDictionaryManagerFactory;
import com.paragon_software.dictionary_manager.Language;
import com.paragon_software.dictionary_manager.MarketingData;
import com.paragon_software.dictionary_manager.components.DictionaryComponent;
import com.paragon_software.dictionary_manager.exceptions.NotALocalResourceException;
import com.paragon_software.trial_manager.TrialManagerAPI;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

@SuppressWarnings( "unused" )
public final class ParagonDictionaryManagerFactory implements IDictionaryManagerFactory
{

  private static final Map< Integer, Integer > DIRECTION_ICONS_MAP = new HashMap<>();

  /**
   * Array of bases ids from catalog (from raw catalog.json) used for testing.
   */
  @NonNull
  private static final HashMap< Dictionary.DictionaryId, Dictionary > dictionaryMap      = new HashMap<>();
  @NonNull
  private static       FeatureName[]                                  demoFeatures       = new FeatureName[]{};
  @NonNull
  private static       FeatureName[]                                  fullFeatures       = new FeatureName[]{};
  @NonNull
  private static       Map< FeatureName, FeatureName >                demoToFullFeatures = new HashMap<>();
  @NonNull
  private static       FeatureName[]                                  allFeatures        = new FeatureName[]{};
  @NonNull
  private static       Dictionary[]                                   paidDictionaries   = new Dictionary[]{};
  @NonNull
  private static       Dictionary[]                                   freeDictionaries   = new Dictionary[]{};
  @NonNull
  private static       DictionaryPack[]                               dictionaryPacks    = new DictionaryPack[]{};

  private static void initDictionaryMap( Dictionary[] dictionaryArray )
  {
    final List< FeatureName > demoList = new ArrayList<>();
    final List< FeatureName > fullList = new ArrayList<>();
    final Map< FeatureName, FeatureName > demoToFullMap = new HashMap<>();
    for ( Dictionary d : dictionaryArray )
    {
      for ( DictionaryComponent component : d.getDictionaryComponents() )
      {
        if ( component.isDemo() )
        {
          demoList.add(component.getFeatureName());
        }
        else
        {
          fullList.add(component.getFeatureName());
        }
      }

      for ( DictionaryComponent component : d.getDictionaryComponents() )
      {
        if ( component.isDemo() )
        {
          DictionaryComponent fullComponent = findFullComponentByDemo(d.getDictionaryComponents(), component);
          if ( null != fullComponent && !FeatureName.isEmpty(fullComponent.getFeatureName()) )
          {
            if (null != demoToFullMap.put(component.getFeatureName(), fullComponent.getFeatureName()))
            {
              throw new IllegalStateException("Duplicate 'feature name' for demo components of dictionary");
            }
          }
        }
      }

      dictionaryMap.put(d.getId(), d);
    }

    demoFeatures = demoList.toArray(new FeatureName[0]);
    fullFeatures = fullList.toArray(new FeatureName[0]);
    demoToFullFeatures.putAll(demoToFullMap);
  }


  private static DictionaryComponent findFullComponentByDemo( List< DictionaryComponent > dictionaryComponents,
                                                              DictionaryComponent demoComponent )
  {
    for ( DictionaryComponent component : dictionaryComponents )
    {
      if ( !component.isDemo() && component.getType() == demoComponent.getType() )
      {
        return component;
      }
    }
    return null;
  }

  public static void initDictionaries( Context context, @NonNull CatalogDictionariesCreator catalogDictionariesCreator )
  {
    initDirectionIconsMap(context);
    Dictionary[] dictionaries = catalogDictionariesCreator.createDictionaries();
    if ( dictionaries.length > 0 )
    {
      List<Dictionary> paid = new ArrayList<>(dictionaries.length);
      List<Dictionary> free = new ArrayList<>(dictionaries.length);
      for (Dictionary dictionary : dictionaries) {
          MarketingData marketingData = dictionary.getMarketingData();
          boolean isFree = false;
          if (marketingData != null)
              isFree = marketingData.isFree();
          if (!isFree)
              paid.add(dictionary);
          else
              free.add(dictionary);
      }
      paidDictionaries = paid.toArray(new Dictionary[0]);
      freeDictionaries = free.toArray(new Dictionary[0]);
      initDictionaryMap(dictionaries);
    }
    dictionaryPacks = catalogDictionariesCreator.createDictionaryPacks();
    
    allFeatures = new FeatureName[dictionaryPacks.length + demoFeatures.length + fullFeatures.length];
    for (int i = 0; i < dictionaryPacks.length; i++)
      allFeatures[i] = dictionaryPacks[i].getPurchaseFeatureName();
    System.arraycopy(demoFeatures, 0, allFeatures, dictionaryPacks.length, demoFeatures.length);
    System.arraycopy(fullFeatures, 0, allFeatures, dictionaryPacks.length + demoFeatures.length, fullFeatures.length);
  }

  public static void initTrials( @NonNull CatalogDictionariesCreator catalogDictionariesCreator,
                                 @NonNull TrialManagerAPI.Builder trialBuilder)
  {
    catalogDictionariesCreator.addFeaturesToTrialBuilder(trialBuilder, dictionaryMap);
  }

  private static void initDirectionIconsMap( @NonNull final Context context )
  {
    final TypedArray directionIcons = context.getResources().obtainTypedArray(R.array.direction_icons);
    for ( int i = 0; i < directionIcons.length(); i++)
    {
      @IdRes int iconId = directionIcons.getResourceId(i, -1);
      if ( iconId != -1 )
      {
        String name = context.getResources().getResourceEntryName(iconId);
        DIRECTION_ICONS_MAP.put(Language.getLangCodeFromShortForm(name), iconId);
      }
    }
    directionIcons.recycle();
  }

  private static @IdRes int getLanguageIconResId( int languageCode )
  {
    return DIRECTION_ICONS_MAP.containsKey(languageCode) ? DIRECTION_ICONS_MAP.get(languageCode) : -1;
  }

  static IDictionaryIcon createDirectionIcon( int fromCode, int toCode )
  {
    @IdRes int fromResId = getLanguageIconResId(fromCode);
    @IdRes int toResId = getLanguageIconResId(toCode);
    return new ParagonDictionaryManagerFactory.DirectionIcon(fromResId, toResId);
  }

  @NonNull
  public static Dictionary[] getPaidDictionaries()
  {
    return paidDictionaries;
  }

  @NonNull
  public static Dictionary[] getFreeDictionaries()
  {
      return freeDictionaries;
  }

  @NonNull
  public static DictionaryPack[] getDictionaryPacks() { return dictionaryPacks; }

  @NonNull
  public static FeatureName[] getAllFeatures()
  {
    return allFeatures;
  }

  @NonNull
  public static FeatureName[] getDemos()
  {
    return demoFeatures;
  }

  @NonNull
  public static Map< FeatureName, FeatureName > getDemoToFull()
  {
    return demoToFullFeatures;
  }

  @NonNull
  @Override
  public Dictionary[] getDictionaries( @NonNull final Context context,
                                       @NonNull final Dictionary.DictionaryId[] dictionariesId )
  {

    final ArrayList< Dictionary > result = new ArrayList<>(dictionariesId.length);
    List<Dictionary.DictionaryId> catalogIds = new ArrayList<>(Arrays.asList(dictionariesId));
    for ( final Dictionary.DictionaryId dictionaryId : catalogIds )
    {
      Dictionary dictionary = dictionaryMap.get(dictionaryId);
      if ( null != dictionary && dictionary.getIcon() instanceof DictionaryIcon )
      {
        final DictionaryIcon icon = (DictionaryIcon) dictionary.getIcon();
        icon.initialize(context);
        initDirectionIcons(dictionary, context);
        result.add(dictionary);
      }
    }
    return result.toArray(new Dictionary[0]);
  }

  @NonNull
  @Override
  public DictionaryPack[] getDictionaryPacks(@NonNull final Context context,
                                             @NonNull final Dictionary.DictionaryId[] dictionariesId){
    List<DictionaryPack> res = new ArrayList<>(Math.min(dictionaryPacks.length, dictionariesId.length));
    Set<Dictionary.DictionaryId> ids = new HashSet<>(Arrays.asList(dictionariesId));
    for (DictionaryPack pack : dictionaryPacks) {
      IDictionaryIcon icon = pack.getIcon();
      if ((icon instanceof DictionaryIcon) && (ids.contains(pack.getId()))) {
        ((DictionaryIcon) icon).initialize(context);
        res.add(pack);
      }
    }
    return res.toArray(new DictionaryPack[0]);
  }

  private void initDirectionIcons(Dictionary dictionary, Context context)
  {
    Collection <Dictionary.Direction> directions = dictionary.getDirections();
    if ( null != directions )
    {
      for ( Dictionary.Direction direction : directions)
      {
        if ( direction.getIcon() instanceof DirectionIcon )
        {
          final DirectionIcon icon = (DirectionIcon) direction.getIcon();
          icon.initialize(context);
        }
      }
    }
  }

  public static String[] getTrials()
  {
    return new String[0];
  }

  // NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
  @SuppressWarnings( "RedundantThrows" )
  static class DictionaryIcon implements IDictionaryIcon
  {
    private final int id;
    @Nullable
    Context context = null;
    @Nullable
    private Bitmap bitmap = null;

    DictionaryIcon( final int resourceId )
    {
      this.id = resourceId;
    }

    void initialize( @NonNull final Context context )
    {
      this.context = context;
    }

    @Nullable
    @Override
    public Bitmap getBitmap()
    {
      if ( null == bitmap )
      {
        if ( null != context )
        {
          bitmap = BitmapFactory.decodeResource(context.getResources(), id);
        }
      }
      return bitmap;
    }

    @Override
    public int getResourceId() throws NotALocalResourceException
    {
      return id;
    }
  }

  // NOTE This class should be immutable otherwise don't forget to implement cloning method and use it in Dictionary objects cloning
  static class DirectionIcon implements IDictionaryIcon
  {
    // All methods creating icon bitmap are designed with assumption what all icons are square
    private static final float SCENE_BASE_SIZE = 72f;
    private static final float BIG_ICON_BASE_SIZE = 56f;
    private static final float SMALL_ICON_BASE_SIZE = 38f;

    // These ratio values are intended to be used with sizes of big icon
    private static final float SCENE_RATIO = SCENE_BASE_SIZE / BIG_ICON_BASE_SIZE;
    private static final float SMALL_ICON_RATIO = SMALL_ICON_BASE_SIZE / BIG_ICON_BASE_SIZE;
    // All other ratio values are intended to be used with sizes of scene
    private static final float BIG_ICON_MARGIN_RATIO = (SCENE_BASE_SIZE - BIG_ICON_BASE_SIZE) / ( 2 * SCENE_BASE_SIZE );
    private static final float SMALL_ICON_VERTICAL_MARGIN_RATIO = (SCENE_BASE_SIZE - SMALL_ICON_BASE_SIZE) / ( 2 * SCENE_BASE_SIZE );
    private static final float SMALL_ICON_LEFT_MARGIN_RATIO = (SCENE_BASE_SIZE - SMALL_ICON_BASE_SIZE) / SCENE_BASE_SIZE;


    private final int idFrom;
    private final int idTo;
    @Nullable
    Context context = null;
    @Nullable
    private Bitmap bitmap = null;

    DirectionIcon( final int resIdFrom, final int resIdTo )
    {
      idFrom = resIdFrom;
      idTo = resIdTo;
    }

    void initialize( @NonNull final Context context )
    {
      this.context = context;
    }

    @Nullable
    @Override
    public Bitmap getBitmap()
    {
      if ( null == bitmap )
      {
        if ( null != context )
        {
          createDirectionIconBitmap();
        }
      }
      return bitmap;
    }

    private void createDirectionIconBitmap()
    {
      if ( idFrom == -1 || idTo == -1 )
      {
        return;
      }
      Bitmap bitmapFrom = createBitmapFromResource(idFrom, 1);
      int sceneSize = (int) (bitmapFrom.getWidth() * SCENE_RATIO);
      bitmap = Bitmap.createBitmap(sceneSize, sceneSize, Bitmap.Config.ARGB_8888);
      Canvas canvas = new Canvas(bitmap);
      if ( idFrom == idTo )
      {
        canvas.drawBitmap(bitmapFrom, sceneSize * BIG_ICON_MARGIN_RATIO,
                          sceneSize * BIG_ICON_MARGIN_RATIO, null);
      }
      else
      {
        Bitmap bitmapTo = createBitmapFromResource(idTo, SMALL_ICON_RATIO);
        canvas.drawBitmap(bitmapTo, sceneSize * SMALL_ICON_LEFT_MARGIN_RATIO,
                          sceneSize * SMALL_ICON_VERTICAL_MARGIN_RATIO, null);
        canvas.drawBitmap(bitmapFrom, 0, sceneSize * BIG_ICON_MARGIN_RATIO, null);
      }
      // create immutable bitmap
      bitmap = bitmap.copy(bitmap.getConfig(), false);
    }

    private Bitmap createBitmapFromResource( int resId, float sizeRatio )
    {
      if ( !AppCompatDelegate.isCompatVectorFromResourcesEnabled() )
      {
        AppCompatDelegate.setCompatVectorFromResourcesEnabled(true);
      }
      Drawable drawable = AppCompatResources.getDrawable(context, resId);
      if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP)
      {
        drawable = (DrawableCompat.wrap(drawable)).mutate();
      }
      Bitmap bitmap = Bitmap.createBitmap((int) (drawable.getIntrinsicWidth() * sizeRatio),
                                          (int) (drawable.getIntrinsicHeight() * sizeRatio), Bitmap.Config.ARGB_8888);
      Canvas canvas = new Canvas(bitmap);
      drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
      drawable.draw(canvas);
      return bitmap;
    }

    @Override
    public int getResourceId() throws NotALocalResourceException
    {
      return idFrom;
    }
  }
}
