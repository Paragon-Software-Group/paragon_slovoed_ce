package com.paragon_software.sound_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import com.paragon_software.article_manager.ArticleItem;
import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;
import com.paragon_software.native_engine.EngineSoundAPI;
import com.paragon_software.native_engine.data.SoundData;
import java.util.ArrayList;
import java.util.List;

public class BaseSoundManager implements SoundManagerAPI, EngineSoundAPI.OnSoundReadyCallback
{
  private @NonNull
  final EngineSoundAPI       mEngineSound;
  private @NonNull
  final DictionaryManagerAPI mDictionaryManager;
  private @NonNull
  final SoundPlayer mPlayer;

  private @Nullable
  final OnlineSoundApi mOnlineSound;

  private @NonNull
  final List< OnStateChangedListener > stateChangedListeners = new ArrayList<>();

  /**
   * Channel for play audio
   */
  private @Nullable String mPlayChannel;

    /**
     * Sound player state change listener to notify {@link OnStateChangedListener} listeners about state changes.
     */
  private final SoundPlayer.OnPlayingStateChangedListener mPlayingStateChangedListener = () ->
  {
    for ( OnStateChangedListener listener : stateChangedListeners )
    {
      listener.onSoundStateChanged();
    }
  };

  public BaseSoundManager(@NonNull EngineSoundAPI engineSound, @NonNull DictionaryManagerAPI dictionaryManager, @Nullable OnlineSoundApi onlineSound)
  {
    this(engineSound, dictionaryManager, new BaseSoundPlayer(), onlineSound);
  }

  BaseSoundManager(@NonNull EngineSoundAPI engineSound, @NonNull DictionaryManagerAPI dictionaryManager,
                           @NonNull SoundPlayer player, @Nullable OnlineSoundApi onlineSound)
  {
    mEngineSound = engineSound;
    mDictionaryManager = dictionaryManager;
    mPlayer = player;
    mOnlineSound = onlineSound;
  }

  @Override
  public boolean playSound( @NonNull ArticleItem currentItem )
  {
    boolean itemHasSound = itemHasSound(currentItem);
    if ( itemHasSound )
    {
      mEngineSound.getSoundData(currentItem, this);
    }
    return itemHasSound;
  }

  @Override
  public boolean playSound( @NonNull ArticleItem currentItem, int soundIdx )
  {
    mEngineSound.getSoundData(currentItem, soundIdx, this);
    return false;
  }

  @Override
  public boolean playSound( @NonNull ArticleItem currentItem, String language, String dictId, int listId, String key )
  {
    // TODO add implementation
    return false;
  }

  @Override
  public boolean playSound( @NonNull ArticleItem currentItem, String soundBaseIdx, String soundKey )
  {
      mEngineSound.getSoundData(currentItem, soundBaseIdx, soundKey, this);
      return true;
  }

  @Override
  public boolean playSound(@NonNull Dictionary.DictionaryId dictionaryId,
                           @NonNull Dictionary.Direction direction,
                           String soundBaseIdx,
                           String soundKey)
  {
    //In quiz we can't get article id and user default methods for play sound.
    //Therefore, we create a special Article Item for playing sound
    ArticleItem item = new ArticleItem.Builder(dictionaryId, -1, -1)
            .setDirection(direction)
            .build();
    return playSound(item, soundBaseIdx, soundKey);
  }

  @Override
  public boolean playSoundOnline(Dictionary.DictionaryId dictionaryId, String soundBaseIdx, String soundKey) {
    if (mOnlineSound != null) {
      mOnlineSound.play(dictionaryId, soundBaseIdx, soundKey, this);
      //mOnlineSound.play(dictionaryId, "549A", "/_/_b/_ba/back#_gbs_1.spx", this);
    }
    return true;
  }

  @Override
  public void registerOnStateChangedListener( @NonNull OnStateChangedListener listener )
  {
    if ( !stateChangedListeners.contains(listener) )
    {
      stateChangedListeners.add(listener);
    }
  }

  @Override
  public void unregisterOnStateChangedListener( @NonNull OnStateChangedListener listener )
  {
    stateChangedListeners.remove(listener);
  }

  @Override
  public boolean isPlaying()
  {
    return mPlayer.isPlaying();
  }

  /**
   * Sets the current play channel.
   * If the channel has changed (switched to another screen), then the previous audio stops
   * @param playChannel Playback channel
   */
  @Override
  public void setPlayChannel(@NonNull String playChannel)
  {
      if(mPlayChannel == null || !mPlayChannel.equals(playChannel))
      {
          if(mPlayer.isPlaying())
          {
            mPlayer.stop();
          }

          mPlayChannel = playChannel;
      }
  }

  @Override
  public boolean itemHasSound( @NonNull ArticleItem item )
  {
    return !isFromDemoDict(item) && mEngineSound.itemHasSound(item);
  }

  private boolean isFromDemoDict( @NonNull ArticleItem item )
  {
    return true;
  }

  @Override
  public void onSoundReady( @Nullable SoundData soundData )
  {
    mPlayer.play(soundData, mPlayingStateChangedListener);
  }
 }
