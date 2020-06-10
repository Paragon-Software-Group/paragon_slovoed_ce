package com.paragon_software.odapi_ui;

import android.net.Uri;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import android.text.TextUtils;

class Query {
    static ParamParser.ParamFactory<Query> FACTORY = new ParamParser.ParamFactory<Query>() {
        @NonNull
        @Override
        public Query create(@Nullable String paramString) {
            return new Query(paramString);
        }
    };

    private static final String QUERY_TYPE_TRANSLATE = "translate";
    private static final String QUERY_TYPE_MORPHOLOGY = "morphology";

    private static final String MODE_FULL = "full";
    private static final String MODE_SHORT = "short";

    private static final String QUERY_ID_TRANSLATE_AS_TEXT = "translate_as_text";
    private static final String QUERY_ID_TRANSLATE_AS_IMAGE = "translate_as_image";
    private static final String QUERY_ID_MORPHO_BASE_FORMS = "morpho_base_forms";

    enum Type {
        Translate(QUERY_TYPE_TRANSLATE),
        Morphology(QUERY_TYPE_MORPHOLOGY);

        Type(@NonNull String name) {
            mName = name;
        }

        @NonNull
        private final String mName;

        @Nullable
        static Type fromName(@Nullable String name) {
            Type res = null;
            for (Type v : values()) {
                if (v.mName.equals(name)) {
                    res = v;
                    break;
                }
            }
            return res;
        }

        @NonNull
        String getName() {
            return mName;
        }
    }

    enum Mode {
        Full(MODE_FULL),
        Short(MODE_SHORT);

        Mode(@NonNull String name) {
            mName = name;
        }

        @NonNull
        private final String mName;

        @Nullable
        static Mode fromName(@Nullable String name) {
            Mode res = null;
            for (Mode v : values()) {
                if (v.mName.equals(name)) {
                    res = v;
                    break;
                }
            }
            return res;
        }

        @NonNull
        String getName() {
            return mName;
        }
    }

    enum Specific {
        TranslateAsText(QUERY_ID_TRANSLATE_AS_TEXT),
        TranslateAsImage(QUERY_ID_TRANSLATE_AS_IMAGE),
        MorphoBaseForms(QUERY_ID_MORPHO_BASE_FORMS);

        Specific(@NonNull String name) {
            mName = name;
        }

        @NonNull
        private final String mName;

        @Nullable
        static Specific fromName(@Nullable String name) {
            Specific res = null;
            for (Specific v : values()) {
                if (v.mName.equals(name)) {
                    res = v;
                    break;
                }
            }
            return res;
        }

        @NonNull
        String getName() {
            return mName;
        }
    }

    @Nullable
    final Type type;

    @Nullable
    final Specific specific;

    @Nullable
    final Mode mode;

    final int width;
    final int height;

    private Query(@Nullable String _type, @Nullable String _specific, @Nullable String _mode, @Nullable String _width, @Nullable String _height) {
        type = Type.fromName(_type);
        specific = Specific.fromName(_specific);
        mode = Mode.fromName(_mode);
        width = TextUtils.isEmpty(_width) ? -1 : Integer.valueOf(_width);
        height = TextUtils.isEmpty(_height) ? -1 : Integer.valueOf(_height);
    }

    private Query(@NonNull Uri q) {
        this(q.getHost(), q.getQueryParameter("queryId"), q.getQueryParameter("mode"),
             q.getQueryParameter("width"), q.getQueryParameter("height"));
    }

    private Query(@Nullable String query) {
        this(Uri.parse(query));
    }
}
