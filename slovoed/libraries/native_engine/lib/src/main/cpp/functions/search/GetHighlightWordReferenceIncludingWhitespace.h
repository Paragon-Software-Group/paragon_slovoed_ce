//
// Created by popov on 18.10.19.
//

#ifndef SLOVOED_GET_HIGHLIGHT_WORD_REFERENCE_INCLUDING_WHITESPACE_H
#define SLOVOED_GET_HIGHLIGHT_WORD_REFERENCE_INCLUDING_WHITESPACE_H

#include <functions/dictionary_context_function.h>

class GetHighlightWordReferenceIncludingWhitespace : public CDictionaryContextFunction {
protected:
    jobject
    native(
            JNIEnv* env,
            CDictionaryContext* context,
            jobject _listIndex,
            jobject _phrase,
            jobject _searchQuery

    ) const noexcept override;
};


#endif //SLOVOED_GET_HIGHLIGHT_WORD_REFERENCE_INCLUDING_WHITESPACE_H
