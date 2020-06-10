package com.paragon_software.flash_cards_manager;

import androidx.annotation.NonNull;

import com.paragon_software.article_manager.ArticleItem;

public class QuizItemView {
    public enum AnswerType {
        Right, Wrong, Current, Future
    }

    @NonNull
    final AnswerType answerType;
    @NonNull
    private final ArticleItem articleItem;

    QuizItemView( @NonNull AnswerType answerType, @NonNull ArticleItem articleItem ) {
        this.answerType = answerType;
        this.articleItem = articleItem;
    }

    @NonNull
    public ArticleItem getArticleItem()
    {
        return articleItem;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        QuizItemView that = (QuizItemView) o;

        return answerType == that.answerType;
    }

    @Override
    public int hashCode() {
        return answerType.hashCode();
    }
}
