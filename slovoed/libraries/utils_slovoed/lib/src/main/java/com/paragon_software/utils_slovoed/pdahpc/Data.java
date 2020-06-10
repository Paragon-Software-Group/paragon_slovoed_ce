package com.paragon_software.utils_slovoed.pdahpc;

import org.simpleframework.xml.ElementList;

import java.util.List;

public final class Data {
    @ElementList(name = "strings", inline = true, entry = "strings")
    public List<Strings> strings;

    @ElementList(name = "lanuages_map")
    public List<Language> languages;
}
