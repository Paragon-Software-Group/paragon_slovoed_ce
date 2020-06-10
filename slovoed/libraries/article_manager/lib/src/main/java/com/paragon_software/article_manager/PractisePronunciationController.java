package com.paragon_software.article_manager;

import android.content.Context;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.util.SparseArray;

import io.reactivex.Observable;
import io.reactivex.subjects.BehaviorSubject;

public class PractisePronunciationController implements PractisePronunciationControllerAPI
{

    private final PractisePronunciationAudioControllerAPI mPractisePronunciationAudioController;
    private final BehaviorSubject<MediaButtonViewState> mMediaButtonStateSubject =
            BehaviorSubject.createDefault(new MediaButtonViewState(-1, MediaButtonState.PREPARING));
    private final SparseArray<MediaButtonState> mMediaButtonsState = new SparseArray<>();

    public static PractisePronunciationController create( @NonNull Context context )
    {
        return new PractisePronunciationController(context);
    }

    private PractisePronunciationController( @NonNull Context context )
    {
        mPractisePronunciationAudioController = new PractisePronunciationAudioController(context);
    }

    @Override
    public void mediaButtonClick(String buttonId)
    {
        if(buttonId == null)
        {
            return;
        }

        final int buttonIndex = getButtonIdIndex(buttonId);
        final int currentButtonIndex = getCurrentButtonIndex();
        if( buttonIndex != -1 && currentButtonIndex != -1 && buttonIndex != currentButtonIndex )
        {
            MediaButtonState state = mMediaButtonsState.get(currentButtonIndex);
            if(state == MediaButtonState.RECORDING || state == MediaButtonState.PLAYING)
            {
                return;
            }

            mMediaButtonStateSubject.onNext(new MediaButtonViewState(buttonIndex, getMediaButtonStateByIndex(buttonIndex)));
        }

        switch (mMediaButtonStateSubject.getValue().getButtonState())
        {
            case PREPARING:
                if(mPractisePronunciationAudioController.startRecord(buttonIndex))
                {
                    changeState(buttonIndex, MediaButtonState.RECORDING);
                }
                break;

            case RECORDING:
                stopRecord();
                break;

            case RECORDED:
                mPractisePronunciationAudioController.startPlaying(buttonIndex, new Runnable()
                {
                    @Override
                    public void run()
                    {
                        changeState(buttonIndex, MediaButtonState.RECORDED);
                    }
                });
                changeState(buttonIndex, MediaButtonState.PLAYING);
                break;

            case PLAYING:
                stopPlaying();
                break;
        }
    }

    @Override
    public Observable<MediaButtonViewState> getMediaButtonState()
    {
        return mMediaButtonStateSubject;
    }

    private int getButtonIdIndex(@Nullable String id)
    {
        if(id == null)
        {
            return -1;
        }
        String[] params = id.split(":");
        if( params.length > 0 )
        {
            return Integer.valueOf(params[1]);
        }

        return -1;
    }

    @Override
    public void pause()
    {
        MediaButtonViewState state = mMediaButtonStateSubject.getValue();
        if ( state.getButtonState() == MediaButtonState.RECORDING )
        {
            stopRecord();
        }
        else if( state.getButtonState() == MediaButtonState.PLAYING )
        {
           stopPlaying();
        }
    }

    @Override
    public void release()
    {
        MediaButtonViewState state = mMediaButtonStateSubject.getValue();
        if ( state.getButtonState() == MediaButtonState.RECORDING )
        {
            mPractisePronunciationAudioController.stopRecord();
        }
        else if( state.getButtonState() == MediaButtonState.PLAYING )
        {
            mPractisePronunciationAudioController.stopPlaying();
        }

        mPractisePronunciationAudioController.release();
        mMediaButtonsState.clear();
        mMediaButtonStateSubject.onNext(new MediaButtonViewState(-1, MediaButtonState.PREPARING));
    }

    @Override
    public void restoreButtonsState()
    {
        if( mMediaButtonsState.size() > 0 )
        {
            for( int i = 0; i < mMediaButtonsState.size(); i++ )
            {
                mMediaButtonStateSubject.onNext(new MediaButtonViewState(mMediaButtonsState.keyAt(i), mMediaButtonsState.valueAt(i)));
            }
        }
    }

    private void stopRecord()
    {
        if(mPractisePronunciationAudioController.stopRecord())
        {
            changeState(MediaButtonState.RECORDED);
        }
        else
        {
            changeState(MediaButtonState.PREPARING);
        }
    }

    private void stopPlaying()
    {
        mPractisePronunciationAudioController.stopPlaying();
        mMediaButtonsState.put(getCurrentButtonIndex(), MediaButtonState.RECORDED);
        mMediaButtonStateSubject.onNext(new MediaButtonViewState(getCurrentButtonIndex(), MediaButtonState.RECORDED));
    }

    private void changeState(MediaButtonState state)
    {
        changeState(getCurrentButtonIndex(), state);
    }

    private void changeState(int buttonIndex, MediaButtonState state)
    {
        mMediaButtonsState.put(buttonIndex, state);
        mMediaButtonStateSubject.onNext(new MediaButtonViewState(buttonIndex, state));
    }

    private MediaButtonState getMediaButtonStateByIndex(int buttonIndex)
    {
        MediaButtonState state = mMediaButtonsState.get(buttonIndex);
        if( state == null )
        {
            state = MediaButtonState.PREPARING;
        }
        else
        {
            if( state == MediaButtonState.RECORDING || state == MediaButtonState.PLAYING )
            {
                state = MediaButtonState.RECORDED;
            }
            else
            {
                return state;
            }
        }

        return state;
    }

    private int getCurrentButtonIndex()
    {
        return mMediaButtonStateSubject.getValue().getButtonIndex();
    }
}
