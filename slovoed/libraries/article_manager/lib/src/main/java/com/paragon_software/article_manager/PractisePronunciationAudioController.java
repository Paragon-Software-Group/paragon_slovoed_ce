package com.paragon_software.article_manager;

import android.content.Context;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaRecorder;
import androidx.annotation.NonNull;
import android.util.SparseArray;

import java.io.File;
import java.io.IOException;

public class PractisePronunciationAudioController implements PractisePronunciationAudioControllerAPI
{
    private static final String AUDIO_RECORDING_CACHED_NAME = "practise_pronunciation_audio";
    private static final String AUDIO_RECORDING_CACHED_EXTENSION = "m4a";
    private static final String AUDIO_RECORDING_CACHED_FILE_FORMAT = "%s.%s";

    private MediaRecorder mMediaRecorder;
    private MediaPlayer mMediaPlayer;
    private SparseArray<File> mAudioFiles;
    private File mCachedDir;

    PractisePronunciationAudioController( @NonNull Context context )
    {
        mCachedDir = context.getCacheDir();
        mAudioFiles = new SparseArray<>();
    }

    @Override
    public boolean startRecord(int id)
    {
        try
        {
            File file = mAudioFiles.get(id);
            if( file == null )
            {
                mAudioFiles.put(id, file = createOutAudioFile(id));
            }

            initMediaRecorder(file);
            mMediaRecorder.prepare();
            mMediaRecorder.start();
        }
        catch (IOException ex)
        {
            return false;
        }

        return true;
    }

    @Override
    public boolean stopRecord()
    {
        if( mMediaRecorder == null )
            return false;

        try
        {
            mMediaRecorder.stop();
            mMediaRecorder.release();
            return true;
        }
        catch (IllegalStateException ex)
        {
            return false;
        }
        finally
        {
            mMediaRecorder = null;
        }
    }

    @Override
    public void startPlaying( int id, @NonNull Runnable onAudioPlayingComplete )
    {
        File file = mAudioFiles.get(id);
        if( file == null || !file.exists() )
            return;

        initMediaPlayer(file, onAudioPlayingComplete);
        mMediaPlayer.start();
    }

    @Override
    public void stopPlaying()
    {
        if( mMediaPlayer == null )
            return;

        mMediaPlayer.stop();
        mMediaPlayer.reset();
        mMediaPlayer = null;
    }

    @Override
    public void release()
    {
        for( int i = 0; i < mAudioFiles.size(); i++)
        {
            File file = mAudioFiles.valueAt(i);
            if( file.exists() )
            {
                file.delete();
            }
        }

        mAudioFiles.clear();
        mMediaRecorder = null;
        mMediaPlayer = null;
    }

    private void initMediaRecorder(File file)
    {
        if(mMediaRecorder == null)
        {
            mMediaRecorder = new MediaRecorder();
            mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
            mMediaRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);
            mMediaRecorder.setOutputFile(file.getPath());
        }
    }

    private boolean initMediaPlayer(File file, @NonNull final Runnable onAudioPlayComplete)
    {
        if( mMediaPlayer == null )
        {
            try
            {
                mMediaPlayer = new MediaPlayer();
                mMediaPlayer.setDataSource(file.getAbsolutePath());
                mMediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
                mMediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener()
                {
                    @Override
                    public void onCompletion(MediaPlayer mp)
                    {
                        mMediaPlayer = null;
                        onAudioPlayComplete.run();
                    }
                });
                mMediaPlayer.prepare();
            }
            catch (IOException ex)
            {
                return false;
            }
        }
        return true;
    }

    private String getOutAudioFilePath(int id)
    {
        if(id == -1)
        {
            return String.format(
                    AUDIO_RECORDING_CACHED_FILE_FORMAT,
                    AUDIO_RECORDING_CACHED_NAME,
                    AUDIO_RECORDING_CACHED_EXTENSION
            );
        }
        else
        {
            return String.format(
                    AUDIO_RECORDING_CACHED_FILE_FORMAT,
                    AUDIO_RECORDING_CACHED_NAME + id,
                    AUDIO_RECORDING_CACHED_EXTENSION
            );
        }
    }

    private File createOutAudioFile(int id)
    {
       return new File(mCachedDir, getOutAudioFilePath(id));
    }
}
