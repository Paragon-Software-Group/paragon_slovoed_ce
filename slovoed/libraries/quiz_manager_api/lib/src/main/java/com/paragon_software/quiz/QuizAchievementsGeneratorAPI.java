package com.paragon_software.quiz;

import android.content.Context;

import java.util.List;

public interface QuizAchievementsGeneratorAPI
{
    interface Factory
    {
        Factory registerContext( Context context );
        QuizAchievementsGeneratorAPI create();
    }

    List< QuizAchievement > getList();
}
