package com.paragon_software.utils_slovoed.pdahpc;

import com.paragon_software.utils_slovoed.text.LocalizedString;

import static com.paragon_software.utils_slovoed.pdahpc.LocalizeStringHelper.getLocalizedString;

public class AboutProvider {

    private final About about;
    private final String defaultLanguage;

    public AboutProvider(About about, String defaultLanguage) {
        this.about = about;
        this.defaultLanguage = defaultLanguage;
    }

    public LocalizedString getAppAbout() {
        return getLocalizedString(about.getAppAboutMap(), defaultLanguage);
    }

    public LocalizedString getProductAbout(String selectedDictionaryId) {
        if(about.getProductAbout().containsKey(selectedDictionaryId)) {
            return getLocalizedString(about.getProductAbout().get(selectedDictionaryId), defaultLanguage);
        }
        return LocalizedString.empty();
    }
}
