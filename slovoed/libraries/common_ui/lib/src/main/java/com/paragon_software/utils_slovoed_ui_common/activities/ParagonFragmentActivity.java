package com.paragon_software.utils_slovoed_ui_common.activities;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.fragment.app.FragmentManager;
import androidx.appcompat.app.AppCompatActivity;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.Queue;

public class ParagonFragmentActivity extends AppCompatActivity {
    public interface ParagonFragment {
        void onFragmentResult(int requestCode, int resultCode, Intent data);
    }

    private static final String ARG_PENDING_QUEUE_REQ_CODES = "com.paragon_software.dictionary_manager_ui.ARG_PENDING_QUEUE_REQ_CODES";
    private static final String ARG_PENDING_QUEUE_RES_CODES = "com.paragon_software.dictionary_manager_ui.ARG_PENDING_QUEUE_RES_CODES";
    private static final String ARG_PENDING_QUEUE_DATA = "com.paragon_software.dictionary_manager_ui.ARG_PENDING_QUEUE_DATA";
    private static final String FRAGMENT_INTENT_MARKER = "com.paragon_software.dictionary_manager_ui.FRAGMENT_INTENT_MARKER";
    private static final String FRAGMENT_TAG = "com.paragon_software.dictionary_manager_ui.FRAGMENT_TAG";

    private boolean mFragmentSafe = false;

    @NonNull
    private Queue<ActivityResultArgs> mPendingActivityResultArgs = new LinkedList<>();

    public static PendingIntent createFragmentSafePendingIntent( FragmentActivity activity, int requestCode )
    {
        Intent intent = new Intent();
        intent.putExtra(FRAGMENT_INTENT_MARKER, 0);
        return activity.createPendingResult(requestCode, intent, PendingIntent.FLAG_UPDATE_CURRENT);
    }

    @Nullable
    public static PendingIntent createFragmentSafePendingIntent(Fragment fragment, int requestCode) {
        PendingIntent res = null;
        String tag = fragment.getTag();
        if (tag != null) {
            Intent intent = new Intent();
            intent.putExtra(FRAGMENT_INTENT_MARKER, 0);
            intent.putExtra(FRAGMENT_TAG, fragment.getTag());
            Activity activity = fragment.getActivity();
            if (activity != null)
                res = activity.createPendingResult(requestCode, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        }
        return res;
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (savedInstanceState != null) {
            ArrayList<Integer> reqCodes = savedInstanceState.getIntegerArrayList(ARG_PENDING_QUEUE_REQ_CODES);
            ArrayList<Integer> resCodes = savedInstanceState.getIntegerArrayList(ARG_PENDING_QUEUE_RES_CODES);
            ArrayList<Intent> data = savedInstanceState.getParcelableArrayList(ARG_PENDING_QUEUE_DATA);
            if ((reqCodes != null) && (resCodes != null) && (data != null)) {
                int n = Math.min(reqCodes.size(), Math.min(resCodes.size(), data.size()));
                if (n > 0) {
                    mPendingActivityResultArgs.clear();
                    for (int i = 0; i < n; i++) {
                        Integer reqCode = reqCodes.get(i);
                        Integer resCode = resCodes.get(i);
                        Intent intent = data.get(i);
                        if ((reqCode != null) && (resCode != null) && (intent != null))
                            mPendingActivityResultArgs.add(new ActivityResultArgs(reqCode, resCode, intent));
                    }
                }
            }
        }
        allowWorkWithFragments();
    }

    @Override
    protected void onResumeFragments() {
        super.onResumeFragments();
        allowWorkWithFragments();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        mFragmentSafe = false;
        int n = mPendingActivityResultArgs.size();
        if (n > 0) {
            ArrayList<Integer> reqCodes = new ArrayList<>(n);
            ArrayList<Integer> resCodes = new ArrayList<>(n);
            ArrayList<Intent> data = new ArrayList<>(n);
            for (ActivityResultArgs args : mPendingActivityResultArgs)
                if (args != null) {
                    reqCodes.add(args.requestCode);
                    resCodes.add(args.resultCode);
                    data.add(args.data);
                }
            if ((!reqCodes.isEmpty()) && (!resCodes.isEmpty()) && (!data.isEmpty())) {
                outState.putIntegerArrayList(ARG_PENDING_QUEUE_REQ_CODES, reqCodes);
                outState.putIntegerArrayList(ARG_PENDING_QUEUE_RES_CODES, resCodes);
                outState.putParcelableArrayList(ARG_PENDING_QUEUE_DATA, data);
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (data != null)
            if (data.hasExtra(FRAGMENT_INTENT_MARKER))
                if (mFragmentSafe)
                    onFragmentResultInner(requestCode, resultCode, data);
                else
                    mPendingActivityResultArgs.add(new ActivityResultArgs(requestCode, resultCode, data));
    }

    private void onFragmentResultInner( int requestCode, int resultCode, Intent data )
    {
        if ( data != null )
        {
            String tag = data.getStringExtra(FRAGMENT_TAG);
            if ( null != tag )
            {
                FragmentManager fragmentManager = getSupportFragmentManager();
                Fragment fragment = null;
                if ( fragmentManager != null )
                {
                    fragment = findFragment(fragmentManager, tag);
                }
                if ( fragment instanceof ParagonFragment )
                {
                    ( (ParagonFragment) fragment ).onFragmentResult(requestCode, resultCode, data);
                }
            }
            else
            {
                onFragmentResult(requestCode, resultCode, data);
            }
        }
    }

    protected void onFragmentResult( int requestCode, int resultCode, Intent data )
    {
    }

    private void allowWorkWithFragments() {
        mFragmentSafe = true;
        while (!mPendingActivityResultArgs.isEmpty()) {
            ActivityResultArgs args = mPendingActivityResultArgs.poll();
            if (args != null)
                onFragmentResultInner(args.requestCode, args.resultCode, args.data);
        }
    }

    @Nullable
    private static Fragment findFragment(@NonNull FragmentManager fragmentManager, @NonNull String tag) {
        fragmentManager.executePendingTransactions();
        Fragment res = fragmentManager.findFragmentByTag(tag);
        if(res == null) {
            Collection<Fragment> fragments = fragmentManager.getFragments();
            if(fragments != null)
                for (Fragment fragment : fragments) {
                    fragmentManager = fragment.getChildFragmentManager();
                    res = findFragment(fragmentManager, tag);
                    if(res != null)
                        break;
                }
        }
        return res;
    }

    private static class ActivityResultArgs {
        final int requestCode;
        final int resultCode;
        final Intent data;

        ActivityResultArgs(int _requestCode, int _resultCode, Intent _data) {
            requestCode = _requestCode;
            resultCode = _resultCode;
            data = _data;
        }
    }
}
