cmake_minimum_required(VERSION 3.4.1)

set(MAIN_SOURCE_DIR src/main/cpp/)

set(SOURCE_FILES ${MAIN_SOURCE_DIR}/native_engine.cpp
                 ${MAIN_SOURCE_DIR}/functions.cpp

                 ${MAIN_SOURCE_DIR}/java/java_callback.cpp
                 ${MAIN_SOURCE_DIR}/java/java_objects.cpp

                 ${MAIN_SOURCE_DIR}/context/env_aware.cpp
                 ${MAIN_SOURCE_DIR}/context/dictionary_context.cpp
                 ${MAIN_SOURCE_DIR}/context/morpho_context.cpp

                 ${MAIN_SOURCE_DIR}/context/reader/descriptor_reader.cpp
                 ${MAIN_SOURCE_DIR}/context/reader/java_reader.cpp

                 ${MAIN_SOURCE_DIR}/context/builder/DataAccess.cpp
                 ${MAIN_SOURCE_DIR}/context/builder/HtmlBuilder.cpp
                 ${MAIN_SOURCE_DIR}/context/builder/SoundBuilder.cpp
                 ${MAIN_SOURCE_DIR}/context/builder/LayerAccess.cpp

                 ${MAIN_SOURCE_DIR}/functions/native_function.cpp
                 ${MAIN_SOURCE_DIR}/functions/i_native_function.cpp
                 ${MAIN_SOURCE_DIR}/functions/dictionary_context_function.cpp
                 ${MAIN_SOURCE_DIR}/functions/byte_array_function.cpp
                 ${MAIN_SOURCE_DIR}/functions/dictionary_function.cpp
                 ${MAIN_SOURCE_DIR}/functions/all_functions.cpp

                 # service
                 ${MAIN_SOURCE_DIR}/functions/service/get_native_context_pointer_size.cpp
                 ${MAIN_SOURCE_DIR}/functions/service/open_sdc_via_descriptor.cpp
                 ${MAIN_SOURCE_DIR}/functions/service/open_sdc_via_java_reader.cpp
                 ${MAIN_SOURCE_DIR}/functions/service/add_morpho_via_descriptor.cpp
                 ${MAIN_SOURCE_DIR}/functions/service/add_morpho_via_java_reader.cpp
                 ${MAIN_SOURCE_DIR}/functions/service/close_sdc.cpp
                 ${MAIN_SOURCE_DIR}/functions/service/switch_direction.cpp

                 # info
                 ${MAIN_SOURCE_DIR}/functions/info/GetEngineVersion.cpp
                 ${MAIN_SOURCE_DIR}/functions/info/GetDictionaryVersion.cpp
                 ${MAIN_SOURCE_DIR}/functions/info/GetListCount.cpp
                 ${MAIN_SOURCE_DIR}/functions/info/GetDictionaryLocalizedStrings.cpp
                 ${MAIN_SOURCE_DIR}/functions/info/GetListType.cpp
                 ${MAIN_SOURCE_DIR}/functions/info/GetListLanguages.cpp
                 ${MAIN_SOURCE_DIR}/functions/info/GetListLocalizedStrings.cpp
                 ${MAIN_SOURCE_DIR}/functions/info/GetListVariants.cpp

                 # fill
                 ${MAIN_SOURCE_DIR}/functions/fill/GetWordCount.cpp
                 ${MAIN_SOURCE_DIR}/functions/fill/GetWordByLocalIndex.cpp
                 ${MAIN_SOURCE_DIR}/functions/fill/GetRealWordLocation.cpp
                 ${MAIN_SOURCE_DIR}/functions/fill/GetPathFromGlobalIndex.cpp
                 ${MAIN_SOURCE_DIR}/functions/fill/GetPreloadedFavorites.cpp
                 ${MAIN_SOURCE_DIR}/functions/fill/GetCurrentWordStylizedVariant.cpp
                 ${MAIN_SOURCE_DIR}/functions/fill/GetExternalImage.cpp

                 # search
                 ${MAIN_SOURCE_DIR}/functions/search/ResetList.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/GetWordByText.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/FullTextSearch.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/GetArticleLinks.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/SpellingSearch.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/WildCardSearch.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/AnagramSearch.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/FuzzySearch.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/DeleteAllSearchLists.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/GetBaseForms.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/GetWordReferenceInList.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/GetQueryReferenceInPhrase.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/GetWordReference.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/GetHighlightWordReferenceIncludingWhitespace.cpp
                 ${MAIN_SOURCE_DIR}/functions/search/GetQueryHighlightData.cpp

                 # article
                 ${MAIN_SOURCE_DIR}/functions/article/Translate.cpp
                 ${MAIN_SOURCE_DIR}/functions/article/GetHistoryElementByGlobalIndex.cpp
                 ${MAIN_SOURCE_DIR}/functions/article/GetArticleIndexesByHistoryElement.cpp
                 ${MAIN_SOURCE_DIR}/functions/article/Swipe.cpp
                 ${MAIN_SOURCE_DIR}/functions/article/StartsWith.cpp

                 # sound
                 ${MAIN_SOURCE_DIR}/functions/sound/GetSoundIndexByWordGlobalIndex.cpp
                 ${MAIN_SOURCE_DIR}/functions/sound/GetSoundIndexByText.cpp
                 ${MAIN_SOURCE_DIR}/functions/sound/PlaySoundByIndex.cpp
                 ${MAIN_SOURCE_DIR}/functions/sound/ConvertSpx.cpp
)

set(DICTIONARY_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../shdd.engine.components.sdc/Engine)

set(DICTIONARY_SOURCES ${DICTIONARY_SOURCE_DIR}/SDC_CRC.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldArticles.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldAuxiliary.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldBitInput.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldCatalog.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldCompare.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldCSSDataManager.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldCustomList.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldCustomListControl.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldDictionary.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldDictionaryHelper.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldHistory.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldHTMLBuilder.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldImageAreaItem.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldImageItem.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldImageManager.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldIndexes.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldInputBase.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldInputText.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldIntFormatter.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldList.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldListInfo.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldListLocalizedString.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldLocalizedString.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldLogicalExpression.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldLogicalExpressionImplementation.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldMergedDictionary.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldMergedList.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldMetadataManager.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldMetadataParser.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldMorphology.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldMp3Decoder.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldOggDecoder.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldPhraseUtility.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSceneWrapper.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSDCReadMy.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSearchList.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSearchWordResult.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSerialNumber.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSimpleSortedList.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSoundASCII_table.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSpeexDecoder.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSpeexSinTable.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldStringStore.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldStyleInfo.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldSymbolsTable.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldTools.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldTypes.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldVideoElement.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldVideoItem.cpp
                       ${DICTIONARY_SOURCE_DIR}/SldWavDecoder.cpp

                       ${DICTIONARY_SOURCE_DIR}/Morphology/IMorphoData.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/LanguageSpecific.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/LanguageSpecific_v1.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/LanguageSpecific_v2.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/LastCharMap.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/LastCharMap_v1.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/LastCharMap_v2.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/MorphoData.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/MorphoData_v1.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/MorphoData_v2.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/MorphoData_v3.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/WordSet.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/WordSet_v1.cpp
                       ${DICTIONARY_SOURCE_DIR}/Morphology/WordSet_v2.cpp

                       ${DICTIONARY_SOURCE_DIR}/Speex/bits.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/cb_search.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/exc_5_64_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/exc_5_256_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/exc_8_128_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/exc_10_16_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/exc_10_32_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/exc_20_32_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/filters.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/gain_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/gain_table_lbr.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/hexc_10_32_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/hexc_table.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/high_lsp_tables.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/lpc.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/lsp.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/lsp_tables_nb.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/ltp.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/math_approx.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/misc.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/modes.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/nb_celp.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/quant_lsp.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/sb_celp.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/speex_callbacks.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/speex_header.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/stereo.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/vbr.c
                       ${DICTIONARY_SOURCE_DIR}/Speex/vq.c
)

set(DICTIONARY_INCLUDE ${DICTIONARY_SOURCE_DIR})

add_library( native-engine SHARED ${SOURCE_FILES} ${DICTIONARY_SOURCES})

find_library( log-lib log )

target_link_libraries( native-engine ${log-lib} )
target_include_directories(native-engine PRIVATE ${MAIN_SOURCE_DIR} ${DICTIONARY_INCLUDE})
