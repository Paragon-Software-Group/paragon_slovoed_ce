//
// Created by mamedov on 30.01.2019.
//

#include "PlaySoundByIndex.h"
#include "java/java_objects.h"

jobject PlaySoundByIndex::native(
    JNIEnv* env,
    CDictionaryContext* context,
    jobject _soundIndex
) const noexcept
{
  CSldDictionary* dictionary = context->GetDictionary();
  LayerAccess* layerAccess = context->GetLayerAccess();

  jint soundIndex = JavaObjects::GetInteger( env, _soundIndex );

  jbyteArray soundBytes = 0;

  if (soundIndex != SLD_INDEX_SOUND_NO)
  {
    ESldError error = dictionary->PlaySoundByIndex( soundIndex, 1, NULL, 0 );

    if ( error == eOK && layerAccess->GetSoundBuilder() )
    {
      const SoundBuilder* soundBuilder = layerAccess->GetSoundBuilder();
      int len = soundBuilder->GetSoundDataSize();
      soundBytes = env->NewByteArray( len );
      if ( soundBytes != NULL && len != 0 )
      {
        env->SetByteArrayRegion( soundBytes, 0, len, (jbyte*) soundBuilder->GetSoundData() );
      }
      return JavaSoundData::GetSoundData( env, soundBytes, soundBuilder->GetSoundFrequency(),
          soundBuilder->GetSoundDecodedFormat() );
    }
  }

  return JavaSoundData::GetSoundData( env, soundBytes, 0, 0 );
}
