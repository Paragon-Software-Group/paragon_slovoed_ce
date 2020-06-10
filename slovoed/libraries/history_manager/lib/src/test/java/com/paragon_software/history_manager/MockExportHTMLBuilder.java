package com.paragon_software.history_manager;

import com.paragon_software.article_manager.ArticleItem;

import java.util.ArrayList;
import java.util.List;

import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;

import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE1;
import static com.paragon_software.history_manager.mock.StaticConsts.ARTICLE2;
import static org.junit.Assert.assertEquals;

public class MockExportHTMLBuilder implements IExportHTMLBuilder {
  final static String HEADER = "header";
  final static String CHOOSER_TITLE = "chooser_title";
  final static String FOOTER = "footer";
  final static String EXPORT_FILE_DESCRIPTION = "export_file_description";

  @ColorInt
  final static int PART_OF_SPEECH_COLOR = 111;

  static final ArrayList ARTICLE_LIST = new ArrayList<ArticleItem>() {{
    add(ARTICLE1);
    add(ARTICLE2);
  }};

  @Override
  public IExportHTMLBuilder addHeader(@NonNull String header) {
    assertEquals(HEADER, header);
    return this;
  }

  @Override
  public IExportHTMLBuilder addFooter(@NonNull String footer) {
    assertEquals(FOOTER, footer);
    return this;
  }

  @Override
  public IExportHTMLBuilder addContent(List<ArticleItem> list) {
    assertEquals(ARTICLE_LIST, list);
    return this;
  }

  @Override
  public IExportHTMLBuilder addPartOfSpeechColor(int color) {
    assertEquals(PART_OF_SPEECH_COLOR, color);
    return this;
  }

  @Override
  public String createHTMLString() {
    return "";
  }

  @NonNull
  @Override
  public String toString() {
    return "";
  }
}

