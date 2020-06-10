package com.paragon_software.sound_manager;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Handler;
import android.os.Looper;
import androidx.annotation.Nullable;

import com.paragon_software.native_engine.data.SoundData;

/**
 * Base class for playing sound data from word and sound bases.
 */
class BaseSoundPlayer implements SoundPlayer
{

  /**
   * Коэффициент для подсчета времени проигрывания.
   * Кол-во байт на одну выборку : SignificantBitsPerSample / 8 * NumChannels .
   * SignificantBitsPerSample - в нашем случаем 16 бит ({@link AudioFormat#ENCODING_PCM_16BIT});
   * NumChannels - моно 1({@link AudioFormat#CHANNEL_OUT_MONO});
   * Итого: 16/8 * 1 = 2 .
   * Далее будем рассчитывать время проигрывания на основании среднего кол-ва байтов в
   * секунду AvgBytesPerSec = freq * (Кол-во байт на одну выборку) = freq * 2 .
   * Тогда среднее время проигрывания файла timePlay = size / AvgBytesPerSec *
   * 1000 (чтобы считать в милисекунах).
   */
  private static final float WAV_TIME_FACTOR = 2;

  private OnPlayingStateChangedListener mPlayingStateChangedListener;

  private final Handler    mHandlerUI = new Handler(Looper.getMainLooper());
  private       AudioTrack mAudioTrack;

  private boolean isPlaying;

  private final Runnable mAudioTrackRelease = new Runnable()
  {
    @Override
    public void run()
    {
      mAudioTrack.stop();
      updateState(false);
      mAudioTrack.release();
      mHandlerUI.removeCallbacksAndMessages(null);
    }
  };

  @Override
  public void play( @Nullable SoundData soundData, @Nullable OnPlayingStateChangedListener listener )
  {
    mPlayingStateChangedListener = listener;
    if ( soundData != null )
    {
      switch ( soundData.getFormat() )
      {
        case WAV:
          playWav(soundData.getData(), soundData.getFrequency());
        default:
          return;
      }
    }
  }



  @Override
  public void stop()
  {
    if ( isPlaying )
    {
      mAudioTrackRelease.run();
    }
  }

  @Override
  public boolean isPlaying()
  {
    return isPlaying;
  }

  /**
   * Play wav sound. Canceling a previous sound play.
   *
   * @param data array with a sound data bytes
   * @param freq sample rate of a sound data
   */
  synchronized private void playWav( byte[] data, int freq )
  {
    if ( data != null && data.length > 0 )
    {
      if ( isPlaying )
      {
        mAudioTrackRelease.run();
      }
      long timePlaying = calculateWavDuration(data, freq);
      mAudioTrack =
          new AudioTrack(AudioManager.STREAM_MUSIC, freq, AudioFormat.CHANNEL_OUT_MONO, AudioFormat.ENCODING_PCM_16BIT,
                         data.length, AudioTrack.MODE_STATIC);
      mAudioTrack.setVolume(1.0f);
      mAudioTrack.write(data, 0, data.length);
      updateState(true);
      mAudioTrack.play();
      mHandlerUI.postDelayed(mAudioTrackRelease, timePlaying);
    }
  }

  /**
   * Update sound player state and notify listeners about changes if needed.
   *
   * @param _isPlaying new state
   */
  private synchronized void updateState( boolean _isPlaying )
  {
    if ( isPlaying != _isPlaying )
    {
      isPlaying = _isPlaying;
      if ( mPlayingStateChangedListener != null )
      {
        mPlayingStateChangedListener.onPlayingStateChanged();
      }
    }
  }

  /**
   * Calculate duration of sound in wav format with default parameters
   * {@link AudioFormat#ENCODING_PCM_16BIT}, {@link AudioFormat#CHANNEL_OUT_MONO}.
   *
   * @param data array with a sound data bytes
   * @param freq sample rate of a sound data
   *
   * @return duration of wav sound in milliseconds
   */
  private long calculateWavDuration( byte[] data, int freq )
  {
    return (long) ( ( data.length / ( freq * WAV_TIME_FACTOR ) ) * 1000f );
  }
}
