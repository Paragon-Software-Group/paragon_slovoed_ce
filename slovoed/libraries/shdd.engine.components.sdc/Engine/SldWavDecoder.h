#ifndef _SLD_WAV_DECODER_H_
#define _SLD_WAV_DECODER_H_

#include "SldPlatform.h"
#include "SldTypes.h"
#include "ISldLayerAccess.h"
#include "SldError.h"


#define MAX_FRAME_SIZE			(640)

/// Производит декодирование озвучки в формате wav
ESldError WavDecode(ISldLayerAccess* aLayerAccess, FSoundBuilderMethodPtr aBuilderPtr, const UInt8* aDataPtr, UInt32 aDataSize, UInt8 aIsLast, UInt32* aStartPos);

#endif //_SLD_WAV_DECODER_H_
