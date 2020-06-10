package com.paragon_software.favorites_manager;

import androidx.annotation.ColorInt;

public class ExportParams
{
    private String header;
    private String chooserTitle;
    private String footer;
    private String subject;
    private String exportFileDescription;
    @ColorInt
    private int partOfSpeechColor;

    public ExportParams(String header,
                        String chooserTitle,
                        String footer,
                        String subject,
                        @ColorInt int partOfSpeechColor) {
        this.header = header;
        this.chooserTitle = chooserTitle;
        this.footer = footer;
        this.subject = subject;
        this.partOfSpeechColor = partOfSpeechColor;
    }

    public ExportParams(String header,
                        String chooserTitle,
                        String footer,
                        String subject,
                        String exportFileDescription,
                        @ColorInt int partOfSpeechColor) {
        this.header = header;
        this.chooserTitle = chooserTitle;
        this.footer = footer;
        this.subject = subject;
        this.exportFileDescription = exportFileDescription;
        this.partOfSpeechColor = partOfSpeechColor;
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

    public String getSubject()
    {
        return subject;
    }

    public int getPartOfSpeechColor()
    {
        return partOfSpeechColor;
    }

    public String getExportFileDescription() {
        return exportFileDescription;
    }
}
