package com.paragon_software.odapi_ui;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Messenger;
import androidx.annotation.NonNull;

import com.paragon_software.dictionary_manager.Dictionary;
import com.paragon_software.dictionary_manager.DictionaryManagerAPI;

import java.lang.ref.WeakReference;
import java.util.Collection;
import java.util.LinkedList;
import java.util.Queue;

class IncomingHandler extends Handler {
    private static final long MIN_DURATION = 100L;

    @NonNull
    private final Queue<Message> mMessages = new LinkedList<>();

    private boolean mReady = true;

    @Override
    public void handleMessage(Message msg) {
        checkDictionaryManager();
        Worker.getDictionaryController();
        Message msgCopy = Message.obtain(msg);
        if(msgCopy != null)
            if(mReady)
                runTask(msgCopy);
            else
                mMessages.add(msgCopy);
    }

    private void onReady() {
        mReady = true;
        Message message = mMessages.poll();
        if(message != null)
            runTask(message);
    }

    private void runTask(@NonNull Message message) {
        AsyncTask<Message, Void, Void> task = new Task(this);
        task.execute(message);
    }

    private boolean __needToCheckDictionaryManager = true;
    private void checkDictionaryManager() {
        if(__needToCheckDictionaryManager) {
            __needToCheckDictionaryManager = false;
            DictionaryManagerAPI dictionaryManager = Worker.getDictionaryManager();
            if(dictionaryManager != null) {
                Collection<Dictionary> dictionaries = dictionaryManager.getDictionaries();
                if(dictionaries.isEmpty()) {
                    mReady = false;
                    dictionaryManager.registerDictionaryListObserver(new DictionaryListener(dictionaryManager));
                }
            }
        }
    }

    private static void waitUntilMinTimeElapses(long start) {
        long end = System.nanoTime();
        long duration = (end - start) / 1000000L;
        if(duration < MIN_DURATION)
            try {
                Thread.sleep(MIN_DURATION - duration);
            }
            catch (InterruptedException ignore) { }
    }

    private class DictionaryListener implements DictionaryManagerAPI.IDictionaryListObserver {
        @NonNull
        private final DictionaryManagerAPI dictionaryManager;

        DictionaryListener(@NonNull DictionaryManagerAPI _dictionaryManager) {
            dictionaryManager = _dictionaryManager;
        }

        @Override
        public void onDictionaryListChanged() {
            if(!dictionaryManager.getDictionaries().isEmpty()) {
                dictionaryManager.unRegisterDictionaryListObserver(this);
                onReady();
            }
        }
    }

    private static class Task extends AsyncTask<Message, Void, Void> {
        @NonNull
        private final WeakReference<IncomingHandler> mWeakParent;

        Task(@NonNull IncomingHandler parent) {
            mWeakParent = new WeakReference<>(parent);
        }

        @Override
        protected void onPreExecute() {
            IncomingHandler parent = mWeakParent.get();
            if(parent != null)
                parent.mReady = false;
        }

        @Override
        protected Void doInBackground(Message... messages) {
            long start = System.nanoTime();
            Message responseMsg = Message.obtain();
            if(responseMsg != null) {
                Bundle data = messages[0].getData();
                Bundle responseData = responseMsg.getData();
                Messenger messenger = messages[0].replyTo;
                if((data != null) && (responseData != null) && (messenger != null))
                    try {
                        Worker worker = WorkerFactory.createWorker(data);
                        worker.trackAnalyticsEvent(messages[0]);
                        Result result = worker.getResult();
                        result.setResult(responseData);
                        waitUntilMinTimeElapses(start);
                        messenger.send(responseMsg);
                    }
                    catch (Exception ignore ) { }
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            IncomingHandler parent = mWeakParent.get();
            if(parent != null)
                parent.onReady();
        }
    }
}
