package com.paragon_software.dictionary_manager;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.arch.core.util.Function;

import com.paragon_software.utils_slovoed.pdahpc.Catalog;
import com.paragon_software.utils_slovoed.pdahpc.Data;
import com.paragon_software.utils_slovoed.pdahpc.LangTranslations;
import com.paragon_software.utils_slovoed.pdahpc.Language;
import com.paragon_software.utils_slovoed.pdahpc.PDAHPCDataParser;
import com.paragon_software.utils_slovoed.pdahpc.Strings;
import com.paragon_software.utils_slovoed.text.LocalizedString;
import com.paragon_software.utils_slovoed.text.LocalizedStringImpl;

import java.util.Arrays;
import java.util.EnumMap;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

final class LanguageStringsImpl implements LanguageStrings {
    private static final String IDENTITY = "%s";

    private static Catalog catalog = null;
    private static Data data = null;
    private static final Map<Integer, LanguageStrings> map = new TreeMap<>();
    private static Map<String, String> languageCodeToName = null, languageXxxxToName = null;
    private static Set<String> supportedLanguages = null;

    private enum StringsKind {Single, Plural, Dictionaries, From, To}
    private final Map<StringsKind, LocalizedString> strings = new EnumMap<>(StringsKind.class);

    static synchronized void init(Context context) {
        if(catalog == null)
            catalog = PDAHPCDataParser.parseCatalog(context);
        if (data == null)
            data = PDAHPCDataParser.parseXmlCatalog(context);
    }

    @NonNull
    static synchronized LanguageStrings get(Context context, int languageId) {
        LanguageStrings res = map.get(languageId);
        if(res == null) {
            init(context);
            if(catalog == null)
                throw new IllegalStateException("no catalog");
            if(languageCodeToName == null) {
                languageCodeToName = new HashMap<>(data.languages.size());
                for(Language language : data.languages)
                    languageCodeToName.put(language.localeCode, language.name);
            }
            if(languageXxxxToName == null) {
                languageXxxxToName = new HashMap<>(data.languages.size());
                for(Language language : data.languages)
                    languageXxxxToName.put(language.xxxxCode, language.name);
            }
            if(supportedLanguages == null)
                supportedLanguages = new HashSet<>(Arrays.asList(catalog.getLocale().getList()));
            map.put(languageId, res = new LanguageStringsImpl(languageId));
        }
        return res;
    }

    private LanguageStringsImpl(int languageId) {
        strings.put(StringsKind.Single, new LanguageString(languageId, (strings) -> IDENTITY, (langTranslations) -> langTranslations.localName));
        strings.put(StringsKind.Plural, new LanguageString(languageId, (strings) -> IDENTITY, (langTranslations) -> langTranslations.dicts));
        strings.put(StringsKind.Dictionaries, new LanguageString(languageId, (strings) -> strings.dictsPattern.text, (langTranslations) -> langTranslations.dicts));
        strings.put(StringsKind.From, new LanguageString(languageId, (strings) -> IDENTITY, (langTranslations) -> langTranslations.dirFrom));
        strings.put(StringsKind.To, new LanguageString(languageId, (strings) -> IDENTITY, (langTranslations) -> langTranslations.dirTo));
    }

    @NonNull
    @Override
    public LocalizedString getSingle() {
        return LocalizedString.nonNull(strings.get(StringsKind.Single));
    }

    @NonNull
    @Override
    public LocalizedString getPlural() {
        return LocalizedString.nonNull(strings.get(StringsKind.Plural));
    }

    @NonNull
    @Override
    public LocalizedString getDictionaries() {
        return LocalizedString.nonNull(strings.get(StringsKind.Dictionaries));
    }

    @NonNull
    @Override
    public LocalizedString getDirectionFrom() {
        return LocalizedString.nonNull(strings.get(StringsKind.From));
    }

    @NonNull
    @Override
    public LocalizedString getDirectionTo() {
        return LocalizedString.nonNull(strings.get(StringsKind.To));
    }

    @NonNull
    private static String decodeXxxxCode(int languageId) {
        StringBuilder stringBuilder = new StringBuilder(4);
        int mask = 0xFF;
        for(int i = 0; i < 4; i++) {
            stringBuilder.append((char) ((mask & languageId) >>> (8 * i)));
            mask <<= 8;
        }
        return stringBuilder.toString();
    }

    private static class LanguageString extends LocalizedStringImpl<String> {
        @NonNull
        final String languageId;

        @NonNull
        final Function<Strings, String> patternLambda;

        @NonNull
        final Function<LangTranslations, String> stringLambda;

        LanguageString(int _languageId, @NonNull Function<Strings, String> _patternLambda, @NonNull Function<LangTranslations, String> _stringLambda) {
            String xxxxCode = decodeXxxxCode(_languageId);
            languageId = xxxxCode.equals("braz") ? "port" : xxxxCode;
            patternLambda = _patternLambda;
            stringLambda = _stringLambda;
        }

        @Nullable
        @Override
        protected String getCurrentLocale() {
            String res = null;
            String languageCode = Locale.getDefault().getLanguage();
            String languageName = languageCodeToName.get(languageCode);
            if(languageName != null)
                 if(supportedLanguages.contains(languageName))
                     res = languageCode;
            return res;
        }

        @Nullable
        @Override
        protected String getDefaultLocale() {
            return catalog.getLocale().getDefault();
        }

        @Nullable
        @Override
        protected String getEnglishLocale() {
            return "en";
        }

        @Nullable
        @Override
        protected String getStringByKey(@NonNull String s) {
            String res = null;
            String locale = languageCodeToName.get(s);
            String languageName = languageXxxxToName.get(languageId);
            if((locale != null) && (languageName != null))
                for(Strings strings : data.strings)
                    if(locale.equals(strings.locale)) {
                        res = getHelper(strings, languageName);
                        break;
                    }
            return res;
        }

        @Nullable
        @Override
        protected String getAnyString() {
            String res = null;
            String languageName = languageXxxxToName.get(languageId);
            if(languageName != null)
                for(Strings strings : data.strings) {
                    res = getHelper(strings, languageName);
                    if(res != null)
                        break;
                }
            return res;
        }

        @Nullable
        private String getHelper(Strings strings, @NonNull String langaugeName) {
            String res = null;
            for(LangTranslations langTranslations : strings.langTranslations)
                if(langaugeName.equals(langTranslations.name)) {
                    res = String.format(patternLambda.apply(strings), stringLambda.apply(langTranslations));
                    break;
                }
            return res;
        }
    }
}
