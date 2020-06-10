package com.paragon_software.quiz;

import java.io.Serializable;
import java.util.Objects;

import com.google.gson.annotations.SerializedName;

public class QuizAchievement implements Serializable
{
    // WARNING!!! Don't change @SerializedName values. They already
    // was used in persistent storage of real users devices.
    // (it has so strange names because of ProGuard)
    @SerializedName("a")
    private final String id;
    @SerializedName("b")
    private final String title;
    @SerializedName("c")
    private final QuizAchievementKind kind;
    @SerializedName("d")
    private final boolean enable;

    public QuizAchievement(String id, String title, QuizAchievementKind kind, boolean enable)
    {
        this.id = id;
        this.title = title;
        this.kind = kind;
        this.enable = enable;
    }

    @Override
    public boolean equals(Object o)
    {
        if (this == o) return true;
        if (!(o instanceof QuizAchievement)) return false;
        QuizAchievement that = (QuizAchievement) o;
        return enable == that.enable &&
                Objects.equals(id, that.id) &&
                Objects.equals(title, that.title) &&
                kind == that.kind;
    }

    @Override
    public int hashCode()
    {
        return Objects.hash(id, title, kind, enable);
    }

    public String getId()
    {
        return id;
    }

    public String getTitle()
    {
        return title;
    }

    public QuizAchievementKind getKind()
    {
        return kind;
    }

    public boolean isEnable()
    {
        return enable;
    }
}
