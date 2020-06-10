package com.paragon_software.engine;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.dictionary_manager.extbase.SoundInfo;
import com.paragon_software.dictionary_manager.location.DictionaryLocation;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.sound.SoundArticleTaskRunner;
import com.paragon_software.native_engine.EngineSoundAPI;
import com.paragon_software.native_engine.data.SoundData;

public class SlovoedEngineSound implements EngineSoundAPI
{
  @NonNull
  private final Context mApplicationContext;

  @Nullable
  private DictionaryManagerAPI mDictionaryManager = null;

  @NonNull
  private final SoundArticleTaskRunner mSoundArticle = new SoundArticleTaskRunner();

  public SlovoedEngineSound( final @NonNull Context context )
  {
    mApplicationContext = context.getApplicationContext();
  }

  @Override
  public void registerDictionaryManager( @NonNull DictionaryManagerAPI manager )
  {
    mSoundArticle.init(mApplicationContext, manager);
    mDictionaryManager = manager;
  }

  @Override
  public boolean itemHasSound( @NonNull ArticleItem item )
  {
    boolean hasSound = item.isHasInternalSound();
    if ( !hasSound )
    {
      // looking for sound data in cached sound bases
      if ( item.getShowVariantText() != null && item.getDirection() != null )
      {
        DictionaryLocation soundBaseLocation = findSoundBaseLocation(item, null);
        if ( soundBaseLocation != null )
        {
          NativeDictionary soundNativeDictionary = ExternalBasesHolder.get(soundBaseLocation);
          if ( soundNativeDictionary == null )
          {
            // if cached sound base not found then throw exception
             throw new IllegalStateException("No cached sound base found in itemHasSound() method.");
          }
          else
          {
            int soundId = soundNativeDictionary.getSoundIndexByText(item.getShowVariantText());
            hasSound = ( soundId != NativeDictionary.SLD_INDEX_SOUND_NO );
          }
        }
      }
    }
    return hasSound;
  }

  @Override
  public void getSoundData( @NonNull ArticleItem item, @NonNull EngineSoundAPI.OnSoundReadyCallback callback )
  {
    NativeDictionary cachedSoundNativeDictionary = ExternalBasesHolder.get(findSoundBaseLocation(item, null));
    mSoundArticle.getSoundData(item, callback, cachedSoundNativeDictionary, null, null);
  }

  @Override
  public void getSoundData(@NonNull ArticleItem item, @NonNull String soundBaseIdx, @NonNull String soundKey, @NonNull EngineSoundAPI.OnSoundReadyCallback callback)
  {
    NativeDictionary cachedSoundNativeDictionary = ExternalBasesHolder.get(findSoundBaseLocation(item, soundBaseIdx));
    mSoundArticle.getSoundData(item, callback, cachedSoundNativeDictionary, soundKey, soundBaseIdx);
  }

  @Override
  public void getSoundData(@NonNull ArticleItem item, int soundIdx, @NonNull OnSoundReadyCallback callback)
  {
    mSoundArticle.getSoundData(item, callback, soundIdx);
  }

  @Override
  public SoundData getSoundDataFromSpxSynch(Dictionary.DictionaryId dictionaryId, byte[] bytes) {
    NativeDictionary nativeDictionary = null;
    SoundData soundData = null;
    if (mDictionaryManager != null) {
      for (Dictionary dict : mDictionaryManager.getDictionaries()) {
        if (dict.getId().equals(dictionaryId)) {
          nativeDictionary = NativeDictionary.open(mApplicationContext, dict, false, true);
          break;
        }
      }
    }
    if (nativeDictionary != null) {
      soundData = nativeDictionary.convertSpx(bytes);
      nativeDictionary.close();
    }
    return soundData;
  }

  private @Nullable
  DictionaryLocation findSoundBaseLocation( @NonNull ArticleItem item, @Nullable String soundBaseIdx )
  {
    if ( item.getDirection() != null && mDictionaryManager != null )
    {
      for ( Dictionary dictionary : mDictionaryManager.getDictionaries() )
      {
        if ( item.getDictId().equals(dictionary.getId()) )
        {
          for (SoundInfo soundInfo : dictionary.getSoundInfoList()) {
            if (soundInfo.getSdcId().equalsIgnoreCase(soundBaseIdx)) {
              return soundInfo.getLocation();
            } else if (soundBaseIdx == null && soundInfo.getLangFrom() == item.getDirection().getLanguageFrom()) {
              return soundInfo.getLocation();
            }
          }
        }
      }
    }
    return null;
  }
}
