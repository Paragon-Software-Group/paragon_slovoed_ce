package com.paragon_software.history_manager;

public class ExportHtmlBuilderBilingual extends ExportHtmlBuilder {


  @Override
  public IExportHTMLBuilder addHeader(String header) {
    this.header = "<h2>" + header + "</h2>";
    return this;
  }

  @Override
  public IExportHTMLBuilder addFooter(String footer) {
    this.footer = "<h3>" + footer + "</h3>";
    return this;
  }
}
