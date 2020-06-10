package com.paragon_software.sound_manager;

import androidx.annotation.Nullable;

import com.paragon_software.native_engine.data.SoundData;

/**
 * Interface for classes playing sound data from word and sound bases.
 */
public interface SoundPlayer
{
  /**
   * Play a specified sound data.
   *
   * @param soundData sound data to play
   * @param listener  listener to handle player state changes
   */
  void play( @Nullable SoundData soundData, @Nullable OnPlayingStateChangedListener listener );

  /**
   * Stop current playing sound data
   * */
  void stop();

  /**
   * Is sound player busy now.
   *
   * @return true if sound player is playing a sound data, otherwise false
   */
  boolean isPlaying();

  /**
   * Interface for player state change listeners.
   */
  interface OnPlayingStateChangedListener
  {
    void onPlayingStateChanged();
  }
}
