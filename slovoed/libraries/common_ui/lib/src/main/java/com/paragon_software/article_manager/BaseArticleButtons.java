package com.paragon_software.article_manager;

import android.content.Context;
import androidx.annotation.IdRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.paragon_software.utils_slovoed.ui_states.ButtonState;

/**
 * Created by Ivan Kuzmin on 13.01.2020;
 * 3van@mail.ru;
 * Copyright © 2020 Example. All rights reserved.
 */

public abstract class BaseArticleButtons {

    @Nullable
    abstract ArticleButtonState getBtnState(Context ctx, @IdRes int id, @NonNull ButtonState state );

}
