//
// Created by popov on 17.09.2019.
//

#include "GetExternalImage.h"
#include "java/java_objects.h"

jobject GetExternalImage::native(
        JNIEnv *env,
        CSldDictionary* dictionary,
        jobject _listIndex,
        jobject _imgKey

) const noexcept {
    jint listIndex = JavaObjects::GetInteger( env, _listIndex );
    JavaObjects::string text = JavaObjects::GetString(env, _imgKey);

    if (dictionary->SetCurrentWordlist(listIndex) == eOK) {
        UInt32 aResultFlag = 0;
        ESldError error = dictionary->GetMostSimilarWordByText(text.c_str(), &aResultFlag);

        TImageElement imageElement;
        CSldVector<Int32> indexes;
        if (error == eOK && aResultFlag == 1 && dictionary->GetCurrentWordPictureIndex(indexes) == eOK &&
            indexes.size()) {
            if (dictionary->GetWordPicture(indexes.front(), 0xffff, &imageElement) != eOK)
                return nullptr;
        }


        jbyteArray imageBytes = env->NewByteArray(imageElement.Size);
        if (imageBytes && imageElement.Size >= 0) {
            env->SetByteArrayRegion(imageBytes, 0, imageElement.Size, (jbyte *) imageElement.Data);
            return imageBytes;
        }
    }
    return nullptr;
}
