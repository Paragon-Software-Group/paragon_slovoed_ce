//
// Created by popov on 14.10.2019.
//

#include "java/java_objects.h"
#include "ConvertSpx.h"

jobject ConvertSpx::native(
    JNIEnv* env,
    CDictionaryContext* context,
    jobject _byteArray
) const noexcept
{
    LayerAccess *layerAccess = context->GetLayerAccess();

    jbyteArray byteArray = static_cast<jbyteArray>(_byteArray);
    UInt32 size = env->GetArrayLength(byteArray);
    jbyteArray soundBytes = nullptr;

    auto *buf = new UInt8[size];
    env->GetByteArrayRegion(byteArray, 0, size, (jbyte *) buf);

    FSoundBuilderMethodPtr soundBuilderPtr = &ISldLayerAccess::BuildSoundRight;

    ESldError error = SpeexDecode(layerAccess, soundBuilderPtr, buf, size, 1, nullptr, 0);

    if (error == eOK && layerAccess->GetSoundBuilder()) {
        const SoundBuilder *soundBuilder = layerAccess->GetSoundBuilder();
        int len = soundBuilder->GetSoundDataSize();
        soundBytes = env->NewByteArray(len);
        if (soundBytes != nullptr && len != 0) {
            env->SetByteArrayRegion(soundBytes, 0, len, (jbyte *) soundBuilder->GetSoundData());
        }
        return JavaSoundData::GetSoundData(env, soundBytes, soundBuilder->GetSoundFrequency(),
                                           soundBuilder->GetSoundDecodedFormat());
    }

    return JavaSoundData::GetSoundData(env, soundBytes, 0, 0);
}
