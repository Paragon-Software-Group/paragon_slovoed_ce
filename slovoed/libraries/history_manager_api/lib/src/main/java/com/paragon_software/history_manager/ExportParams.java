package com.paragon_software.history_manager;

import androidx.annotation.ColorInt;

public class ExportParams
{
    private String header;
    private String chooserTitle;
    private String footer;
    private String exportFileDescription;
    @ColorInt
    private int partOfSpeechColor;

    public ExportParams(String header,
                        String chooserTitle,
                        String footer,
                        @ColorInt int partOfSpeechColor) {
        this.header = header;
        this.chooserTitle = chooserTitle;
        this.footer = footer;
        this.partOfSpeechColor = partOfSpeechColor;
    }

    public ExportParams(String header,
                        String chooserTitle,
                        String footer,
                        String exportFileDescription,
                        @ColorInt int partOfSpeechColor) {
        this.header = header;
        this.chooserTitle = chooserTitle;
        this.footer = footer;
        this.partOfSpeechColor = partOfSpeechColor;
        this.exportFileDescription = exportFileDescription;
    }

    public String getHeader()
    {
        return header;
    }

    public String getFooter()
    {
        return footer;
    }

    public String getChooserTitle()
    {
        return chooserTitle;
    }

    public int getPartOfSpeechColor()
    {
        return partOfSpeechColor;
    }

    public String getExportFileDescription() {
        return exportFileDescription;
    }
}
