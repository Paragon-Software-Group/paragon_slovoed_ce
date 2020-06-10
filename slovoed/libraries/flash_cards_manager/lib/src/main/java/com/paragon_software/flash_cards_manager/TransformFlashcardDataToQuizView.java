package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.flash_cards_manager.QuizItemView.AnswerType;
import com.paragon_software.sound_manager.SoundManagerAPI;
import com.paragon_software.utils_slovoed.collections.MutableCollectionView;
import com.paragon_software.utils_slovoed.collections.TransformCollectionView;
import com.paragon_software.utils_slovoed.ui_states.ButtonState;
import com.paragon_software.utils_slovoed.ui_states.CheckState;
import com.paragon_software.utils_slovoed.ui_states.VisibilityState;

import java.util.Arrays;
import java.util.EnumMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

class TransformFlashcardDataToQuizView extends
                                       TransformCollectionView<QuizItemView,
                                                               FlashcardBox,
                                                               TransformFlashcardDataToQuizView.TransformDataToQuizView,
                                                               BaseQuizAndFlashcardMetadata,
                                                               MutableCollectionView<FlashcardBox,
                                                                                     BaseQuizAndFlashcardMetadata>>
                                       implements AbstractQuizCollectionView {
    private final static ButtonState BUTTON_VISIBLE = new ButtonState(VisibilityState.enabled, CheckState.uncheckable);
    private final static ButtonState BUTTON_GONE    = new ButtonState(VisibilityState.gone, CheckState.uncheckable);

    private boolean front = true;

    @Nullable
    private SoundManagerAPI soundManager;

    TransformFlashcardDataToQuizView(@Nullable SoundManagerAPI soundManager, @NonNull MutableCollectionView<FlashcardBox, BaseQuizAndFlashcardMetadata> randomCards) {
        super(randomCards, new TransformDataToQuizView());
        this.soundManager = soundManager;
    }

    @Override
    public BaseQuizAndFlashcardMetadata getMetadata()  {
        int current = getTransformFunction().current, count = getCount();
        QuizMetadata res;
        if((current >= 0) && (current < count))
            if(front)
                res = new QuizMetadata(getItem(current).getArticleItem(), VisibilityState.enabled, VisibilityState.gone,
                                       visibleButtons(QuizMetadata.ButtonType.ShowMeaning, QuizMetadata.ButtonType.Delete,
                                                      (hasSound() ? QuizMetadata.ButtonType.Pronunciation : null)));
            else
                res = new QuizMetadata(VisibilityState.gone, VisibilityState.disabled,
                                        visibleButtons(QuizMetadata.ButtonType.Right,
                                                       QuizMetadata.ButtonType.Wrong,
                                                       ( hasSound() ? QuizMetadata.ButtonType.Pronunciation
                                                               : null ), QuizMetadata.ButtonType.Delete));
        else {
            int correctAnswersCount = getTransformFunction().correctAnswers.size();
            res = new QuizMetadata(correctAnswersCount, count - correctAnswersCount, VisibilityState.enabled,
                                   VisibilityState.gone, (count > 0) ? visibleButtons(QuizMetadata.ButtonType.StartAgain) : visibleButtons());
        }
        return res;
    }

    @Override
    public void showMeaning() {
        front = false;
        int current = getTransformFunction().current;
        if((current >= 0) && (current < getCount()))
            FlashcardManagerHolder.get().showQuizArticle(getItem(current).getArticleItem());
        callOnMetadataChanged();
    }

    @Override
    public void answer(boolean right) {
        int current = getTransformFunction().current, count = getCount();
        front = true;
        if((current >= 0) && (current < count)) {
            FlashcardBox currentBox = getCore().getItem(current);
            if (null != currentBox) {
                if(right)
                    getTransformFunction().correctAnswers.add(currentBox);
                currentBox = FlashcardBox.increaseShowCount(currentBox);
                if(right)
                    currentBox = FlashcardBox.increaseRightAnswerCount(currentBox);
                getCore().update(current, currentBox);
            }
            current = ++getTransformFunction().current;
            if(current < count)
                onItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, current, 1);
        }
        callOnMetadataChanged();
    }

    @Override
    public void restart() {
        front = true;
        getTransformFunction().current = 0;
        getTransformFunction().correctAnswers.clear();
        onItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, 0, getCount());
        callOnMetadataChanged();
    }

    @Override
    public void playSound() {
        int current = getTransformFunction().current;
        if((null != soundManager) && (current >= 0) && (current < getCount())) {
            FlashcardBox item = getCore().getItem(current);
            if(null != item)
                soundManager.playSound(item.getParent());
        }
    }

    private boolean hasSound() {
        int current = getTransformFunction().current;
        if((null != soundManager) && (current >= 0) && (current < getCount())) {
            FlashcardBox item = getCore().getItem(current);
            if(null != item)
                return soundManager.itemHasSound(item.getParent());
        }
        return false;
    }

    @Override
    public void remove() {
        int current = getTransformFunction().current;
        if((current >= 0) && (current < getCount())) {
            getCore().remove(current);
            if(current < getCount())
                callOnItemRangeChanged(OPERATION_TYPE.ITEM_RANGE_CHANGED, current, 1);
        }
        front = true;
        callOnMetadataChanged();
    }

    @Override
    public void setInQuizUI(boolean value)  { }

    private static Map<QuizMetadata.ButtonType, ButtonState> visibleButtons(QuizMetadata.ButtonType... visibleButtons) {
        List<QuizMetadata.ButtonType> visible = Arrays.asList(visibleButtons);
        Map<QuizMetadata.ButtonType, ButtonState> res = new EnumMap<>(QuizMetadata.ButtonType.class);
        for(QuizMetadata.ButtonType buttonType : QuizMetadata.ButtonType.values())
            res.put(buttonType, visible.contains(buttonType) ? BUTTON_VISIBLE : BUTTON_GONE);
        return res;
    }

    static class TransformDataToQuizView implements TransformCollectionView.TransformFunction<QuizItemView, FlashcardBox> {
        private Set<FlashcardBox> correctAnswers = new HashSet<>();
        private int current = 0;

        @Override
        public QuizItemView transform(int position, FlashcardBox box) {
            AnswerType answerType;
            if(position < current)
                answerType = correctAnswers.contains(box) ? AnswerType.Right : AnswerType.Wrong;
            else
                answerType = (position == current) ? AnswerType.Current : AnswerType.Future;
            return new QuizItemView(answerType, box.getParent());
        }

        @Override
        public void onItemRangeChanged(OPERATION_TYPE type, int startPosition, int itemCount) { }
    }
}
