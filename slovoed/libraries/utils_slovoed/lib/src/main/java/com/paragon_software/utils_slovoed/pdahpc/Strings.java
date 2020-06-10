package com.paragon_software.utils_slovoed.pdahpc;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.ElementList;

import java.util.List;

public final class Strings {
    @Attribute(name = "locale")
    public String locale;

    @ElementList(name = "lang_translations", inline = true, entry = "lang_translations")
    public List<LangTranslations> langTranslations;

    @Element(name = "dicts_pattern")
    public DictsPattern dictsPattern;
}
