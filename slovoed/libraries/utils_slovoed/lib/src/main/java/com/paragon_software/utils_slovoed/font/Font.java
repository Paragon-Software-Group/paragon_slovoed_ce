package com.paragon_software.utils_slovoed.font;

import android.graphics.Typeface;
import androidx.annotation.NonNull;

import com.google.gson.annotations.SerializedName;

public class Font
{
  @SerializedName("filePath")
  private String filePath;

  @SerializedName("family")
  private String family;

  @SerializedName("semibold")
  private boolean semibold;

  @SerializedName("bold")
  private boolean bold;

  @SerializedName("italic")
  private boolean italic;

  @NonNull
  private Typeface typeface = Typeface.DEFAULT;

  public String getFilePath() {
    return filePath;
  }

  public String getFamily() {
    return family;
  }

  public boolean isSemiBold() {
    return semibold;
  }

  public boolean isBold() {
    return bold;
  }

  public boolean isItalic() {
    return italic;
  }

  public @NonNull Typeface getTypeface()
  {
    return typeface;
  }

  void setTypeface( @NonNull Typeface _typeface )
  {
    typeface = _typeface;
  }
}
