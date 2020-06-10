//service functions

#include "service/get_native_context_pointer_size.h"
#include "service/open_sdc_via_descriptor.h"
#include "service/open_sdc_via_java_reader.h"
#include "functions/service/add_morpho_via_descriptor.h"
#include "functions/service/add_morpho_via_java_reader.h"
#include "service/close_sdc.h"
#include "service/switch_direction.h"

//info functions

#include "info/GetEngineVersion.h"
#include "info/GetDictionaryVersion.h"
#include "info/GetListCount.h"
#include "info/GetDictionaryLocalizedStrings.h"
#include "info/GetListType.h"
#include "info/GetListLanguages.h"
#include "info/GetListLocalizedStrings.h"
#include "info/GetListVariants.h"

//fill functions

#include "fill/GetWordCount.h"
#include "fill/GetWordByLocalIndex.h"
#include "fill/GetRealWordLocation.h"
#include "fill/GetPathFromGlobalIndex.h"
#include "fill/GetPreloadedFavorites.h"
#include "fill/GetCurrentWordStylizedVariant.h"
#include "fill/GetExternalImage.h"

//search

#include "search/ResetList.h"
#include "search/GetWordByText.h"
#include "search/FullTextSearch.h"
#include "search/DeleteAllSearchLists.h"
#include "functions/search/GetBaseForms.h"
#include "functions/search/SpellingSearch.h"
#include "search/WildCardSearch.h"
#include "search/AnagramSearch.h"
#include "search/FuzzySearch.h"
#include "search/GetWordReferenceInList.h"
#include "search/GetArticleLinks.h"
#include "search/GetQueryReferenceInPhrase.h"
#include "search/GetWordReference.h"
#include "search/GetHighlightWordReferenceIncludingWhitespace.h"
#include "search/GetQueryHighlightData.h"

//article

#include "article/Translate.h"
#include "article/GetHistoryElementByGlobalIndex.h"
#include "article/GetArticleIndexesByHistoryElement.h"
#include "article/Swipe.h"
#include "article/StartsWith.h"

//sound

#include "sound/GetSoundIndexByWordGlobalIndex.h"
#include "sound/GetSoundIndexByText.h"
#include "sound/PlaySoundByIndex.h"
#include "sound/ConvertSpx.h"
