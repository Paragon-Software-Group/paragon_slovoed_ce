package com.paragon_software.engine.rx.sound;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.Collection;
import java.util.Collections;

import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.extbase.SoundInfo;
import com.paragon_software.engine.nativewrapper.NativeDictionary;
import com.paragon_software.engine.rx.EngineTaskRunner;
import com.paragon_software.engine.rx.RunOn;
import com.paragon_software.engine.rx.Task;
import com.paragon_software.native_engine.EngineSoundAPI;
import com.paragon_software.native_engine.data.SoundData;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Function;
import io.reactivex.subjects.PublishSubject;
import io.reactivex.subjects.Subject;

public class SoundArticleTaskRunner extends EngineTaskRunner< Params >
{
  @NonNull
  private final Subject< Task< Params > > source = PublishSubject.create();

  @Nullable
  private Task< Params > currentTask;

  private SoundObserver observer;

  @Override
  protected void emitTask( Task< Params > task )
  {
    source.onNext(task);
  }

  @Override
  protected Collection< Task< Params > > getCurrentTasks()
  {
    Collection< Task< Params > > res = Collections.emptySet();
    if ( currentTask != null )
    {
      res = Collections.singleton(currentTask);
    }
    return res;
  }

  /**
   * Asynchronously read sound data for article item.
   *
   * @param item                        item for which need to get sound data
   * @param callback                    callback that will receive the sound data
   * @param cachedSoundNativeDictionary opened sound native dictionary from cache
   */
  public void getSoundData( @NonNull ArticleItem item, @NonNull EngineSoundAPI.OnSoundReadyCallback callback,
                            @Nullable NativeDictionary cachedSoundNativeDictionary, @Nullable String soundKey,
                            @Nullable String soundBaseIdx )
  {
    assertInitCalled();
    if ( observer == null )
    {
      subscribe();
    }
    emitNewTask(new Params(callback, item, cachedSoundNativeDictionary, soundKey, soundBaseIdx, -1));
  }

  /**
   * Asynchronously read sound data for article item.
   *
   * @param item                        item for which need to get sound data
   * @param callback                    callback that will receive the sound data
   * @param soundIdx                   sound index
   */
  public void getSoundData( @NonNull ArticleItem item, @NonNull EngineSoundAPI.OnSoundReadyCallback callback, int soundIdx)
  {
    assertInitCalled();
    if ( observer == null )
    {
      subscribe();
    }
    emitNewTask(new Params(callback, item, null, null, null, soundIdx));
  }

  private @Nullable
  Dictionary findDictionaryForItem( @NonNull ArticleItem item )
  {
    if ( item != null )
    {
      for ( Dictionary dictionary : dictionaryManager.getDictionaries() )
      {
        if ( item.getDictId().equals(dictionary.getId()) )
        {
          return dictionary;
        }
      }
    }
    return null;
  }

  private @Nullable
  SoundData readSoundDataFromWordBase( @NonNull ArticleItem item, @NonNull Dictionary dictionary )
  {
    SoundData soundData = null;
    NativeDictionary nativeDictionary = NativeDictionary
        .open(applicationContext, dictionary.getDictionaryLocation(), dictionary.getMorphoInfoList(), true);
    if ( nativeDictionary != null )
    {
      int soundId = nativeDictionary.getSoundIndexByWordGlobalIndex(item.getListId(), item.getGlobalIndex());
      soundData = nativeDictionary.getSoundDataByIndex(soundId);
      nativeDictionary.close();
    }
    return soundData;
  }

  private @Nullable
  SoundData readSoundDataFromSoundBase( @NonNull ArticleItem item, @NonNull Dictionary dictionary,
                                        @Nullable NativeDictionary cachedSoundNativeDictionary, @Nullable String soundKey,
                                        @Nullable String soundBaseIdx )
  {
    SoundData soundData = null;
    if ( item.getShowVariantText() != null || soundKey != null )
    {
      NativeDictionary soundNativeDictionary = cachedSoundNativeDictionary;
      if ( soundNativeDictionary == null )
      {
        // if there is no cached sound base then try to find and open it
        soundNativeDictionary = openSoundNativeDictionary(item, dictionary, soundBaseIdx);
      }

      if ( soundNativeDictionary != null )
      {
        synchronized ( soundNativeDictionary )
        {
          int soundId = soundNativeDictionary.getSoundIndexByText(soundKey != null ? soundKey : item.getShowVariantText());
          soundData = soundNativeDictionary.getSoundDataByIndex(soundId);
          if ( cachedSoundNativeDictionary == null )
          {
            // if sound base not from cache then force close it
            soundNativeDictionary.close();
          }
        }
      }
    }
    return soundData;
  }

  private @Nullable
  SoundData readSoundDataFromSoundBase(@NonNull ArticleItem item, @NonNull Dictionary dictionary, int soundIdx) {
    SoundData soundData = null;
    if (soundIdx != -1) {
      NativeDictionary soundNativeDictionary = NativeDictionary.open(applicationContext, dictionary.getDictionaryLocation(), null, false);
      if (soundNativeDictionary != null) {
        synchronized (soundNativeDictionary) {
          soundData = soundNativeDictionary.getSoundDataByIndex(soundIdx);
          soundNativeDictionary.close();
        }
      }
    }
    return soundData;
  }

  private @Nullable
  NativeDictionary openSoundNativeDictionary( @NonNull ArticleItem item, @NonNull Dictionary dictionary,
                                              @Nullable String soundBaseIdx )
  {
    if ( item.getDirection() != null )
    {
      for ( SoundInfo soundInfo : dictionary.getSoundInfoList() )
      {
        if (soundInfo.getSdcId().equalsIgnoreCase(soundBaseIdx) ||
            (soundBaseIdx == null && soundInfo.getLangFrom() == item.getDirection().getLanguageFrom()))
        {
          return NativeDictionary.open(applicationContext, soundInfo.getLocation(), null, false);
        }
      }
    }
    return null;
  }

  private void subscribe()
  {
    RunOn< Task< Params > > switch1 = new RunOn<>(RunOn.What.Observe, RunOn.Where.Computation);
    RunOn< Task< Result > > switch2 = new RunOn<>(RunOn.What.Observe, RunOn.Where.Caller);
    observer = source.doOnNext(new SaveCurrentTask()).compose(switch1).map(new GetSoundData()).compose(switch2)
                     .filter(new OnlyUncanceled()).doOnNext(new DiscardCurrentTask())
                     .subscribeWith(new SoundObserver());
  }

  private final class GetSoundData implements Function< Task< Params >, Task< Result > >
  {
    @Override
    public Task< Result > apply( Task< Params > paramsTask )
    {
      Params params = paramsTask.get();
      Result result = new Result(params.callback, null);
      if ( !( paramsTask.isCanceled() || ( params.articleItem == null ) ) )
      {
        Dictionary dictionary = findDictionaryForItem(params.articleItem);
        if ( dictionary != null )
        {
          if ( params.articleItem.isHasInternalSound() )
          {
            result = new Result(params.callback, readSoundDataFromWordBase(params.articleItem, dictionary));
          }
          else if(params.soundIdx!=-1)
          {
            result = new Result(params.callback, readSoundDataFromSoundBase(params.articleItem, dictionary,
                                                                            params.soundIdx));
          }
          else
          {
            result = new Result(params.callback, readSoundDataFromSoundBase(params.articleItem, dictionary,
                                                                            params.cachedSoundNativeDictionary,
                                                                            params.soundKey, params.soundBaseIdx));
          }
        }
      }
      return paramsTask.spawn(result);
    }
  }

  private class SaveCurrentTask implements Consumer< Task< Params > >
  {
    @Override
    public void accept( Task< Params > paramsTask )
    {
      if ( currentTask != null )
      {
        currentTask.cancel();
      }
      currentTask = paramsTask;
    }
  }

  private final class DiscardCurrentTask implements Consumer< Task< Result > >
  {
    @Override
    public void accept( Task< Result > task )
    {
      currentTask = null;
    }
  }
}
