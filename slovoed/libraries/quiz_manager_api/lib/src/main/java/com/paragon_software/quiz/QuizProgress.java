package com.paragon_software.quiz;

import java.io.Serializable;
import java.util.Objects;

import com.google.gson.annotations.SerializedName;

public class QuizProgress implements Serializable
{

    // WARNING!!! Don't change @SerializedName values. They already
    // was used in persistent storage of real users devices.
    @SerializedName("id")
    private final String id;
    @SerializedName("title")
    private final String title;
    @SerializedName("total")
    private final int total;
    @SerializedName("correct")
    private final int correct;

    public QuizProgress(String id, String title, int total, int correct)
    {
        this.id = id;
        this.title = title;
        this.total = total;
        this.correct = correct;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof QuizProgress)) return false;
        QuizProgress progress = (QuizProgress) o;
        return id.equals(progress.id)
                && title.equals(progress.title)
                && total == progress.total
                && correct == progress.correct;
    }

    @Override
    public int hashCode() {
        return Objects.hash(id, title, total, correct);
    }

    @Override
    public String toString() {
        return "QuizProgress{" +
                "id='" + id + '\'' +
                ", title='" + title + '\'' +
                ", total=" + total +
                ", correct=" + correct +
                '}';
    }

    public String getId()
    {
        return id;
    }

    public int getTotal()
    {
        return total;
    }

    public int getCorrect()
    {
        return correct;
    }

    public String getTitle()
    {
        return title;
    }
}
