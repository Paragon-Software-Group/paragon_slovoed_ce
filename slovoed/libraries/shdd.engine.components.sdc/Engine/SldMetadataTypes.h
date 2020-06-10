#pragma once
#ifndef _SLD_METADATA_TYPES_H_
#define _SLD_METADATA_TYPES_H_

#include "SldDefines.h"
#include "SldPlatform.h"

// "opaque" string "reference"
struct TMetadataString
{
	enum { DataBits = 29, MaxDataValue = 1 << DataBits };
	union {
		struct {
			UInt32 type : 32 - DataBits;
			UInt32 data : DataBits;
		};
		UInt32 _u32;
	};
	TMetadataString() : _u32(0) {}
};
static_assert(sizeof(TMetadataString) == sizeof(UInt32),
			  "TMetadataString must have a size of a single uint32_t! Check your compiler/typedefs!");

// the type of a TMetadataString "reference" <* internal *>
enum EMetadataStringSource {
	// "empty" string
	eMetadataStringSource_Empty = 0,
	// string for binary metadata contained inside a list
	eMetadataStringSource_List,
	// string for "textual" metadata "owned" by the metadata parser
	eMetadataStringSource_Memory,
	// string for binary metadata contained inside a string store
	eMetadataStringSource_StringStore,
	// string for binary metadata stored right inside the block string
	eMetadataStringSource_Embedded,
};

// ссылка на внешнюю sdc базу
struct TMetadataExtKey
{
	// id внешней sdc базы, на которую ссылаемся
	TMetadataString DictId;
	// номер списка слов во внешней sdc базе
	Int32 ListIdx;
	// фраза (строка), которую нужно искать во внешней sdc базе
	TMetadataString Key;

	TMetadataExtKey() : ListIdx(SLD_DEFAULT_LIST_INDEX) {}
};

// Empty metadata struct to be used as a fallback for "structless" metadata in traits
struct TMetadataEmptyStruct {};

// Metadata traits used to map ESldStyleMetaTypeEnum values to the corresponding metadata structs
// This is a default unspecialized version that maps to TMetadataEmptyStruct
template <ESldStyleMetaTypeEnum Enum>
struct SldMetadataTraits { typedef TMetadataEmptyStruct struct_type; };

// Macro used for defining metadata traits. T must be a metadata struct type with a metaType
#define DEFINE_TRAITS(T) template <> struct SldMetadataTraits<T::metaType> { typedef T struct_type; };

struct TMetadataAbstractResource
{
	// тип (см. #ESldAbstractResourceType)
	UInt16 Type;
	// explicitly pad to the next uint32
	UInt16 _pad0;
	// ссыдка на внешнюю базу
	TMetadataExtKey ExtKey;
	// номер ресурса, если ресурс находится внутри базы
	UInt32 ItemIdx;

	TMetadataAbstractResource() :
		Type(~0),
		_pad0(0),
		ItemIdx(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaAbstractResource;
};
DEFINE_TRAITS(TMetadataAbstractResource)

// типы возможных обработчиков событий
enum ESldMetadataDOMEventType {
	eSldMetadataDOMEventType_Invalid = 0,

	eSldMetadataDOMEvent_OnLoad,
	eSldMetadataDOMEvent_OnResize,

	eSldMetadataDOMEventType_Count,
	eSldMetadataDOMEventType_Last = eSldMetadataDOMEventType_Count - 1
};

// обработчик событий, не имеет "закрывающих" блоков
struct TMetadataArticleEventHandler
{
	// тип события (см. #ESldMetadataDOMEventType)
	UInt16 Type;
	// padding
	UInt16 _pad0;
	// строка - тело обработчика
	TMetadataString JSString;

	TMetadataArticleEventHandler() :
		Type(eSldMetadataDOMEventType_Invalid),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaArticleEventHandler;
};
DEFINE_TRAITS(TMetadataArticleEventHandler)

struct TMetadataAtomicObject
{
	// индекс записи в списке типа eWordListType_Atomic
	UInt32 Index;
	// тип контента (см. #ESldAtomicObjectContentType)
	UInt16 ContentType;
	// тип смыслового значения (см. #ESldLogicalType)
	UInt16 LogicalType;
	// тип деятельности (см. #ESldActivityType)
	UInt16 ActivityType;
	// explicitly pad to the next uint32
	UInt16 _pad0;

	TMetadataAtomicObject() :
		Index(0),
		ContentType(eContentType_Unknown),
		LogicalType(eLogicalType_Unknown),
		ActivityType(eActivityType_Unknown),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaAtomicObject;
};
DEFINE_TRAITS(TMetadataAtomicObject)

struct TMetadataBackgroundImage
{
	// индекс картинки-превью
	Int32 PictureIndex;
	// ширина картинки-превью
	TSizeValue ShowWidth;
	// высота картинки-превью
	TSizeValue ShowHeight;
	// положение картинки относительно текста (см. #ESldStyleLevelEnum)
	UInt16 Level;
	// explicitly pad to the next uint32
	UInt16 _pad0;

	TMetadataBackgroundImage() :
		PictureIndex(SLD_INDEX_PICTURE_NO),
		Level(eLevelNormal),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaBackgroundImage;
};
DEFINE_TRAITS(TMetadataBackgroundImage)

struct TMetadataCaption
{
	// тип (см. #ESldCaptionType)
	UInt16 Type;
	// флаг отображения подписи
	UInt8 Hide;
	// флаг редактируемости
	UInt8 Editable;

	TMetadataCaption() :
		Type(eCaptionTypeCaption),
		Hide(0),
		Editable(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaCaption;
};
DEFINE_TRAITS(TMetadataCaption)

struct TMetadataCrosswordItem
{
	// X координата начала кроссворда
	UInt32 X;
	// Y координата начала кроссворда
	UInt32 Y;
	// направление слова (см. #ESldCrosswordItemDirection)
	UInt16 Direction;
	// explicitly pad to the next uint32
	UInt16 _pad0;
	// ответ
	TMetadataString Answer;
	// начальный текст
	TMetadataString Init;

	TMetadataCrosswordItem() :
		X(0),
		Y(0),
		Direction(0),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaCrossword;
};
DEFINE_TRAITS(TMetadataCrosswordItem)

struct TMetadataExternArticle
{
	// индекс списка слов
	UInt32 ListIndex;
	// индекс слова в списке слов
	UInt32 EntryIndex;
	// XXX: ??? еще какой-то индекс
	UInt32 ArticleIndex;
	// ссылка на внешнюю sdc базу
	TMetadataExtKey ExtKey;
	// XXX: ???
	UInt32 BlockStart;
	// XXX: ???
	UInt32 BlockEnd;

	TMetadataExternArticle() :
		ListIndex(SLD_DEFAULT_LIST_INDEX),
		EntryIndex(SLD_DEFAULT_WORD_INDEX),
		ArticleIndex(~0U),
		BlockStart(0),
		BlockEnd(~0U)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaExternArticle;
};
DEFINE_TRAITS(TMetadataExternArticle)

struct TMetadataFlashCardsLink
{
	// индекс списка с front карточками
	UInt32 FrontListId;
	// индекс слова в списке слов с front карточками
	UInt32 FrontListEntryId;
	// индекс списка с back карточками
	UInt32 BackListId;
	// индекс слова в списке слов с back карточками
	UInt32 BackListEntryId;

	TMetadataFlashCardsLink() :
		FrontListId(0),
		FrontListEntryId(0),
		BackListId(0),
		BackListEntryId(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaFlashCardsLink;
};
DEFINE_TRAITS(TMetadataFlashCardsLink)

struct TMetadataFormula
{
	// номер абстрактногоо ресурса, который содержит xml файл с формулой
	UInt32 AbstractItemIdx;

	TMetadataFormula() :
		AbstractItemIdx(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaFormula;
};
DEFINE_TRAITS(TMetadataFormula)

struct TMetadataHide
{
	// метка - id блока
	TMetadataString Label;
	// флаг, содержит ли данный блок контрол
	UInt8 HasControl;
	// explicitly pad to the next uint32
	UInt8  _pad0;
	UInt16 _pad1;

	TMetadataHide() :
		HasControl(0),
		_pad0(0), _pad1(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaHide;
};
DEFINE_TRAITS(TMetadataHide)

struct TMetadataImage
{
	// индекс картинки-превью
	Int32 PictureIndex;
	// ссылка на внешнюю sdc базу картинки-превью
	TMetadataExtKey ExtKey;
	// ширина картинки-превью
	TSizeValue ShowWidth;
	// высота картинки-превью
	TSizeValue ShowHeight;

	// индекс полноразмерной картинки
	Int32 FullPictureIndex;
	// ссылка на внешнюю sdc базу полноразмерной картинки
	TMetadataExtKey FullExtKey;
	// ширина полноразмерной картинки
	TSizeValue FullShowWidth;
	// высота полноразмерной картинки
	TSizeValue FullShowHeight;

	// флаг маштабируемости
	UInt8 Scalable;
	// флаг системности
	UInt8 System;
	// explicitly pad to the next uint32
	UInt16 _pad0;

	TMetadataImage() :
		PictureIndex(SLD_INDEX_PICTURE_NO),
		FullPictureIndex(SLD_INDEX_PICTURE_NO),
		Scalable(0),
		System(0),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaImage;
};
DEFINE_TRAITS(TMetadataImage)

struct TMetadataImageArea
{
	/// Тип события, ассоциированного с областью (см. ESldImageAreaType)
	UInt16 Type;
	/// В процентах ли размеры
	UInt16 Percent;
	/// Идентификатор области
	TMetadataString Id;
	/// X-координата области (ее левой границы)
	UInt32 Left;
	/// Y-координата области (ее левой границы)
	UInt32 Top;
	/// Ширина области (в пикселях)
	UInt32 Width;
	/// Высота области (в пикселях)
	UInt32 Height;
	/// Индекс маски
	UInt32 Mask;
	/// Строка-сценарий, необходимая для обработки события, ассоциированного с областью
	TMetadataString ActionScript;
	/// Строка координат, необходимаая для задания области произвольной формы
	TMetadataString Coords;

	TMetadataImageArea() :
		Type(eImageAreaType_Unknown),
		Percent(0),
		Left(0),
		Top(0),
		Width(0),
		Height(0),
		Mask(SLD_INDEX_PICTURE_NO)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaImageArea;
};
DEFINE_TRAITS(TMetadataImageArea)

struct TMetadataInfoBlock
{
	// тип
	TMetadataString Type;

	TMetadataInfoBlock() {}

	static const ESldStyleMetaTypeEnum metaType = eMetaInfoBlock;
};
DEFINE_TRAITS(TMetadataInfoBlock)

struct TMetadataInteractiveObject
{
	// XXX: There are no comments anywhere...
	TMetadataString Id;
	// см. #ESldInteractiveObjectShapeEnum
	UInt16 Shape;
	// [0 .. 359]
	UInt16 StartingAngle;
	TMetadataString StartingPos;
	TMetadataString StartingSize;
	UInt8 Draggable;
	UInt8 Scalable;
	UInt8 Rotatable;
	// explicitly pad to the next uint32
	UInt8 _pad0;

	TMetadataInteractiveObject() :
		Shape(0),
		StartingAngle(0),
		Draggable(0),
		Scalable(0),
		Rotatable(0),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaInteractiveObject;
};
DEFINE_TRAITS(TMetadataInteractiveObject)

struct TMetadataLabel
{
	// тип метки (см. #ESldLabelType)
	UInt16 Type;
	// explicitly pad to the next uint32
	UInt16 _pad0;
	// значение метки
	TMetadataString Data;

	TMetadataLabel() :
		Type(eLabelNormal),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaLabel;
};
DEFINE_TRAITS(TMetadataLabel)

struct TMetadataLegendItem
{
	// тип объекта легенды (см. #ESldLegendItemType)
	UInt16 Type;
	// explicitly pad to next uint32
	UInt16 _pad0;

	TMetadataLegendItem() :
		Type(eLegendItemTypeDefault),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaLegendItem;
};
DEFINE_TRAITS(TMetadataLegendItem)

struct TMetadataLink
{
	// способ перехода по ссылке (см. #ESldNavigationTypeEnum)
	UInt16 Type;
	// флаг, является ли эта ссылка ссылкой на ту же статью, где находится эта ссылка
	UInt8 Self;
	// explicitly pad to the next uint32
	UInt8 _pad0;
	// индекс списка слов
	UInt32 ListIndex;
	// индекс слова в списке слов
	UInt32 EntryIndex;
	// тайтл
	TMetadataString Title;
	// метка
	TMetadataString Label;
	// ссылка на внешнюю sdc базу
	TMetadataExtKey ExtKey;

	TMetadataLink() :
		Type(eGoToWord),
		Self(0),
		_pad0(0),
		ListIndex(SLD_DEFAULT_LIST_INDEX),
		EntryIndex(SLD_DEFAULT_WORD_INDEX)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaLink;
};
DEFINE_TRAITS(TMetadataLink)

struct TMetadataList
{
	// тип списка (см. #ESldListType)
	UInt16 Type;
	// число элементов списка (параметр обязателен для ordered_reversed списков)
	UInt16 Number;
	// стиль списка (номер стиля)
	UInt16 Style;
	// explicitly pad to the next uint32
	UInt16 _pad0;
	// номер типа маркера списка или символы списка или урлы картинок, используемых в качестве маркера списка
	TMetadataString Marker;

	TMetadataList() :
		Type(eListTypeUnordered),
		Number(~0),
		Style(0),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaList;
};
DEFINE_TRAITS(TMetadataList)

struct TMetadataMap
{
	// XXX: There are no comments anywhere...
	// см. #ESldMapType
	UInt16 Type;
	// explicitly pad to next uint32
	UInt16 _pad0;
	UInt32 PictureIndex;
	UInt32 TopPictureIndex;

	TMetadataMap() :
		Type(0),
		_pad0(0),
		PictureIndex(SLD_INDEX_PICTURE_NO),
		TopPictureIndex(SLD_INDEX_PICTURE_NO)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaMap;
};
DEFINE_TRAITS(TMetadataMap)

struct TMetadataMapElement
{
	// XXX: There are no comments anywhere...
	TMetadataString Name;

	TMetadataMapElement() {}

	static const ESldStyleMetaTypeEnum metaType = eMetaMapElement;
};
DEFINE_TRAITS(TMetadataMapElement)

struct TMetadataMediaContainer
{
	/// тип содержимого (см. #ESldMediaType)
	UInt16	Type;
	/// способ тайлинга для фонового изображения (см. #ESldTiling)
	UInt16	Tiling;
	/// способ обтекания контейнера (см. #ESldFlow)
	UInt16	Flow;
	/// тип вертикального выравнивания (см. #ESldAlignVertical)
	UInt16	AlignVertical;
	/// тип горизонтального выравнивания (см. #ESldAlignHorizont)
	UInt16	AlignHorizont;
	/// Тип применяемого градиента, значение енума #ESldGradient
	UInt16	Gradient;
	/// Тип теней, значение енума #ESldShadowType
	UInt16	Shadow;
	/// расположение overlay, значение перечисления #ESldPlace
	UInt16	OverlayPos;
	/// Стиль границы (см. #EBorderStyle)
	UInt16 BorderStyle;
	/// Способ выравнивания объекта (см. #ESldDisplay)
	UInt16 Display;
	/// Способ отображения объекта (см. #ESldFloat)
	UInt16 Float;
	/// Способ отмены обтекания объекта (см. #ESldClearType)
	UInt16 ClearType;
	/// Выравнивание картинок в тексте
	UInt16 Baseline;

	/// флаг возможно ли развернуть содержимое на всю ширину
	UInt8 Expansion;
	/// флаг зуммируемости
	UInt8 ZoomToFull;
	/// флаг интерактивности
	UInt8 Interactivity;
	// explicitly pad to the next uint32
	UInt8  _pad0;
	UInt16 _pad1;

	/// цвет фона RGBA по 8 бит на канал
	UInt32	Colour;
	/// индекс картинки для фона
	UInt32	Background;
	/// ширина контейнера
	TSizeValue Width;
	/// высота контейнера
	TSizeValue Height;
	/// внешний отступ сверху
	TSizeValue	MarginTop;
	/// внешний отступ справа
	TSizeValue	MarginRight;
	/// внешний отступ снизу
	TSizeValue	MarginBottom;
	/// внешний отступ слева
	TSizeValue	MarginLeft;
	/// внутренний отступ сверху
	TSizeValue	PaddingTop;
	/// внутренний отступ справа
	TSizeValue	PaddingRight;
	/// внутренний отступ снизу
	TSizeValue	PaddingBottom;
	/// внутренний отступ слева
	TSizeValue	PaddingLeft;
	/// Начальный цвет при использовании градиента
	UInt32	ColourBegin;
	/// Конечный цвет при использовании градиента
	UInt32	ColourEnd;
	/// Индекс картинки overlay
	UInt32	OverlaySrc;
	/// Размер границы
	TSizeValue BorderSize;
	/// Цвет границы
	UInt32 BorderColor;
	/// Отступ первого параграфа (CSS text-indent)
	TSizeValue TextIndent;

	TMetadataMediaContainer()
	{
		Clear();
	};

	void Clear()
	{
		Type = eMedia_Text;
		// Цвет медиаконтейра по умолчанию белый прозрачный (http://redmine.etb-lab.com/issues/3990)
		Colour = 0xFFFFFF00;
		Background = SLD_INDEX_PICTURE_NO;
		Tiling = eTilingNone;
		// Параметр считается устаревшим
		// Разбито по параметрам 'Display' и 'Float'
		Flow = eFlow_Unknown;
		Float = eFloat_None;
		Display = eDisplay_Inline;
		ClearType = eClearType_None;
		Width = TSizeValue();
		Height = TSizeValue();
		Expansion = 0;
		ZoomToFull = 0;
		Interactivity = 0;
		AlignVertical = 0;
		AlignHorizont = 0;
		MarginTop = TSizeValue(0, eMetadataUnitType_px);
		MarginRight = TSizeValue(0, eMetadataUnitType_px);
		MarginBottom = TSizeValue(0, eMetadataUnitType_px);
		MarginLeft = TSizeValue(0, eMetadataUnitType_px);
		PaddingTop = TSizeValue(0, eMetadataUnitType_px);
		PaddingRight = TSizeValue(0, eMetadataUnitType_px);
		PaddingBottom = TSizeValue(0, eMetadataUnitType_px);
		PaddingLeft = TSizeValue(0, eMetadataUnitType_px);
		ColourBegin = 0;
		ColourEnd = 0;
		Gradient = eGradientNone;
		Shadow = eShadowNone;
		OverlayPos = ePlace_InText;
		OverlaySrc = SLD_INDEX_PICTURE_NO;
		Baseline = 0;
		BorderStyle = 0;
		BorderColor = 0;
		BorderSize = TSizeValue();
		TextIndent = TSizeValue();

		_pad0 = 0;
		_pad1 = 0;
	}

	static const ESldStyleMetaTypeEnum metaType = eMetaMediaContainer;
};
// typedef for backwards compat
typedef TMetadataMediaContainer TSldMediaContainer;
DEFINE_TRAITS(TMetadataMediaContainer)

struct TMetadataParagraph
{
	// горизонтальный отступ параграфа
	TSizeValue Depth;
	// выступ первой строки параграфа
	TSizeValue Indent;
	// отступ по вертикали перед параграфом
	TSizeValue MarginTop;
	// отступ по вертикали после параграфа
	TSizeValue MarginBottom;
	// выравнивание по горизонтали текста в параграфе (см. #ESldTextAlignEnum)
	UInt16 Align;
	// направление написания текста по горизонтали (см. #ESldTextDirectionEnum)
	UInt16 TextDirection;

	TMetadataParagraph() :
		Depth(0, eMetadataUnitType_em),
		Indent(0, eMetadataUnitType_em),
		MarginTop(0, eMetadataUnitType_em),
		MarginBottom(0, eMetadataUnitType_em),
		Align(eTextAlign_Left),
		TextDirection(eTextDirection_LTR)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaParagraph;
};
DEFINE_TRAITS(TMetadataParagraph)

struct TMetadataPopupArticle
{
	// индекс списка слов
	UInt32 ListIndex;
	// индекс слова в списке слов
	UInt32 EntryIndex;
	// параметр title
	TMetadataString Title;
	// метка
	TMetadataString Label;
	// ссылка на внешнюю sdc базу
	TMetadataExtKey ExtKey;

	TMetadataPopupArticle() :
		ListIndex(SLD_DEFAULT_LIST_INDEX),
		EntryIndex(SLD_DEFAULT_WORD_INDEX)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaPopupArticle;
};
DEFINE_TRAITS(TMetadataPopupArticle)

struct TMetadataPopupImage
{
	// индекс картинки
	Int32 PictureIndex;
	// ширина картинки
	TSizeValue ShowWidth;
	// высота картинки
	TSizeValue ShowHeight;
	// ссылка на внешнюю sdc базу
	TMetadataExtKey ExtKey;

	TMetadataPopupImage() :
		PictureIndex(SLD_INDEX_PICTURE_NO)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaPopupImage;
};
DEFINE_TRAITS(TMetadataPopupImage)

struct TMetadataScene3D
{
	// индекс 3d сцены
	Int32 Id;
	// ссылка на внешнюю sdc базу
	TMetadataExtKey ExtKey;

	TMetadataScene3D() :
		Id(SLD_INDEX_SCENE_NO)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaScene;
};
DEFINE_TRAITS(TMetadataScene3D)

struct TMetadataManagedSwitch
{
	// метка switch блока контролируемая данным switch-control'ом
	TMetadataString Label;

	TMetadataManagedSwitch() {}

	static const ESldStyleMetaTypeEnum metaType = eMetaManagedSwitch;
};
DEFINE_TRAITS(TMetadataManagedSwitch)

struct TMetadataSlideShow
{
	/// Индекс глобальной озвучки
	UInt32 Sound;
	/// Время показа одного кадра
	UInt32 Time;
	/// Cпособ обтекания слайдшоу текстом (см. #ESldFlow)
	UInt16 Flow;
	/// Cпособ расположения слайдшоу на странице (см. #ESldPlace)
	UInt16 Place;
	/// Эффект переключения между кадрами (см. #ESldMoveEffect)
	UInt16 MoveEffect;
	/// Эффект отображения слайдшоу на странице (см. #ESldShowEffect)
	UInt16 ShowEffect;
	/// Индекс списка слов
	UInt32 ListIndex;
	/// Индекс слова в списке слов, которое отвечает за слайдшоу
	UInt32 WordIndex;
	/// ширина контейнера
	TSizeValue Width;
	/// высота контейнера
	TSizeValue Height;
	// время отображения слайдов(ShowTime для автопоказа)
	TMetadataString TimeStep;

	TMetadataSlideShow() :
		Sound(0),
		Time(0),
		Flow(eFlow_InText),
		Place(ePlace_InText),
		MoveEffect(eMoveEffect_Base),
		ShowEffect(eShowEffect_Base),
		ListIndex(0), // XXX: SLD_DEFAULT_LIST_INDEX
		WordIndex(0) // XXX: SLD_DEFAULT_WORD_INDEX
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaSlideShow;
};
DEFINE_TRAITS(TMetadataSlideShow)

struct TMetadataSound
{
	// индекс звука
	Int32 Index;
	// язык озвучки
	TMetadataString Lang;
	// ссылка на внешнюю sdc базу
	TMetadataExtKey ExtKey;

	TMetadataSound() :
		Index(SLD_INDEX_SOUND_NO)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaSound;
};
DEFINE_TRAITS(TMetadataSound)

struct TMetadataSwitch
{
	// тип управления блоком (см. #ESldMetaSwitchManageTypeEnum)
	UInt16 Manage;
	// флаг, переносится ли текст блока на новую строку или нет
	UInt8 Inline;
	// explicitly pad to the next uint32
	UInt8 _pad0;
	// метка - уникальный в пределах статьи id блока
	TMetadataString Label;
	// тип содержимого блока (см. #ESldMetaSwitchThematicTypeEnum)
	UInt16 Thematic;
	// количество состояний блока
	UInt16 NumStates;

	TMetadataSwitch() :
		Manage(eSwitchManagedBySwitchControl),
		Inline(1),
		_pad0(0),
		Thematic(eSwitchThematicDefault),
		NumStates(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaSwitch;
};
DEFINE_TRAITS(TMetadataSwitch)

struct TMetadataSwitchControl
{
	// тип сожержимого блока (см. #ESldMetaSwitchThematicTypeEnum)
	UInt16 Thematic;
	// // количество состояний блока
	UInt16 NumStates;

	TMetadataSwitchControl() :
		Thematic(eSwitchThematicDefault),
		NumStates(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaSwitchControl;
};
DEFINE_TRAITS(TMetadataSwitchControl)

struct TMetadataTable
{
	// ширина таблицы
	TSizeValue Width;
	// padding между ячейками таблицы
	TSizeValue CellPadding;
	// spacing между ячейками таблицы
	TSizeValue CellSpacing;

	TMetadataTable() {}

	static const ESldStyleMetaTypeEnum metaType = eMetaTable;
};
DEFINE_TRAITS(TMetadataTable)

struct TMetadataTableCell
{
	// rowspan
	UInt16 RowSpan;
	// colspan
	UInt16 ColSpan;
	// цвет бэкграунда ячейки
	UInt32 BgColor;
	// ширина ячейки
	TSizeValue Width;
	// выравнивание по горизонтали текста в ячейке (см. #ESldTextAlignEnum)
	UInt16 TextAlign;
	// выравнивание по вертикали текста в ячейке (см. #ESldVerticalTextAlignEnum)
	UInt16 TextVertAlign;
	// стиль границы ячейки (см. #EBorderStyle)
	UInt16 BorderStyle;
	// explicitly pad to the next uint32
	UInt16 _pad0;
	// цвет границы ячейки
	UInt32 BorderColor;
	// ширина всей границы ячейки
	TSizeValue BorderSize;
	// ширина верхней границы ячейки
	TSizeValue BorderTopWidth;
	// ширина нижней границы ячейки
	TSizeValue BorderBottomWidth;
	// ширина левой границы ячейки
	TSizeValue BorderLeftWidth;
	// ширина правой границы ячейки
	TSizeValue BorderRightWidth;

	TMetadataTableCell() :
		RowSpan(0),
		ColSpan(0),
		BgColor(0xFFFFFF00), // transparent white
		TextAlign(eTextAlign_Left),
		TextVertAlign(eVerticalTextAlign_Center),
		BorderStyle(eBorderStyleSolid),
		_pad0(0),
		BorderColor(0x000000FF), // solid black
		BorderSize(1 * SLD_SIZE_VALUE_SCALE, eMetadataUnitType_px)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTableCol;
};
DEFINE_TRAITS(TMetadataTableCell)

struct TMetadataTaskBlockEntry
{
	// тип (см. #ESldTaskBlockType)
	UInt16 Type;
	// explicitly pad to the next uint32
	UInt16 _pad0;

	TMetadataTaskBlockEntry() :
		Type(eTaskBlockGiven),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTaskBlockEntry;
};
DEFINE_TRAITS(TMetadataTaskBlockEntry)

struct TMetadataTest
{
	// тип теста (см. #ESldTestTypeEnum)
	UInt16 Type;
	// порядок отображения ответов теста (см. #ESldTestOrderEnum)
	UInt16 Order;
	// сложность теста(см. #ESldTestDifficultyEnum)
	UInt16 Difficulty;
	// XXX: ???
	UInt8 HasDraft;
	// explicitly pad to the next uint32
	UInt8 _pad0;
	// максимальное значение баллов за тест
	UInt32 MaxPoints;

	TMetadataTest() :
		Type(eTestType_FreeAnswer),
		Order(eTestOrderDefault),
		Difficulty(ESldTestDifficultyDefault),
		HasDraft(0),
		_pad0(0),
		MaxPoints(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTest;
};
DEFINE_TRAITS(TMetadataTest)

struct TMetadataTestContainer
{
	// XXX: There are no comments anywhere...
	// см. #ESldTestModeEnum
	UInt16 Mode;
	// см. #ESldTestDifficultyEnum
	UInt16 Difficulty;
	// см. #ESldTestDemoEnum
	UInt16 Demo;
	// explicitly pad to the next uint32
	UInt16 _pad0;
	UInt32 MaxPoints;

	TMetadataTestContainer() :
		Mode(eTestModeExam),
		Difficulty(ESldTestDifficultyDefault),
		Demo(eTestDemoDefault),
		_pad0(0),
		MaxPoints(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTestContainer;
};
DEFINE_TRAITS(TMetadataTestContainer)

struct TMetadataTestControl
{
	// подтип теста (см. #ESldMappingTestTypeEnum)
	UInt16 Type;
	// флаг уникальности (сколько раз можно использовать каждый spear)
	UInt8 Unique;
	// explicitly pad to the next uint32
	UInt8 _pad0;
	// ответ в формате разобранного дерева зависимостей
	TMetadataString Checklist;

	TMetadataTestControl() :
		Type(eMappingTestTypeDefault),
		Unique(0),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTestControl;
};
DEFINE_TRAITS(TMetadataTestControl)

struct TMetadataTestInput
{
	// тип вопроса теста (см. #ESldTestInputTypeEnum)
	UInt16 Type;
	// положение картинки относительно текста (см. #ESldStyleLevelEnum)
	UInt16 Level;
	// название группы, которой принажлежит вопрос
	TMetadataString Group;
	// строка, содержащая один или несколько правильных ответов (разделенных символом-разделителем - `|`)
	TMetadataString Answers;
	// исходное содержание (текст) или статус (выбрано/выделено или нет) поля для ответа
	TMetadataString Initial;
	// высота текстового поля в символах (при типе input_text = text)
	UInt32 Cols;
	// ширина текстового поля в символах (при типе input_text = text)
	UInt32 Rows;
	// id ответа
	TMetadataString Id;

	TMetadataTestInput() :
		Type(eTestInputType_Text),
		Level(eLevelNormal),
		Cols(0),
		Rows(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTestInput;
};
DEFINE_TRAITS(TMetadataTestInput)

struct TMetadataTestResultElement
{
	// цвет
	UInt32 Color;

	TMetadataTestResultElement() :
		Color(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTestResultElement;
};
DEFINE_TRAITS(TMetadataTestResultElement)

struct TMetadataTestSpear
{
	// идентификатор ответного блока
	TMetadataString Id;

	TMetadataTestSpear() {}

	static const ESldStyleMetaTypeEnum metaType = eMetaTestSpear;
};
DEFINE_TRAITS(TMetadataTestSpear)

struct TMetadataTestTarget
{
	// идентификатор ответного блока
	TMetadataString Id;
	// связи ответного блока с подобными блоками
	TMetadataString Links;
	// флаг скрытости (предопределенности) узла
	UInt8 Static;
	// explicitly pad to the next uint32
	UInt8  _pad0;
	UInt16 _pad1;

	TMetadataTestTarget() :
		Static(0),
		_pad0(0), _pad1(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTestTarget;
};
DEFINE_TRAITS(TMetadataTestTarget)

struct TMetadataTestToken
{
	// порядковый номер данного блока среди остальных блоков при правильном расположении блоков
	UInt16 Order;
	// explicitly pad to the next uint32
	UInt16 _pad0;
	// строка с названием группы, которой принажлежит блок
	TMetadataString Group;
	// текст блока
	TMetadataString Text;

	TMetadataTestToken() :
		Order(0),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTestToken;
};
DEFINE_TRAITS(TMetadataTestToken)

struct TMetadataTextControl
{
	// способ выделения текста в тесте (см. #ESldTextControl)
	UInt16 Scope;
	// explicitly pad to the next uint32
	UInt16 _pad0;

	TMetadataTextControl() :
		Scope(eTextControlFree),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTextControl;
};
DEFINE_TRAITS(TMetadataTextControl)

struct TMetadataTimeLine
{
	// ширина окна контента
	TSizeValue Width;
	// высота окна контента
	TSizeValue Height;
	// ширина ленты времени
	TSizeValue LineWidth;
	// высота ленты времени
	TSizeValue LineHeight;
	// положение ленты времени относительно окна контента (см. #ESldTimeLinePosition)
	UInt16 Position;
	// начальное время, в процентах
	UInt16 DefaultTime;

	TMetadataTimeLine() :
		Position(eTimeLineAbove),
		DefaultTime(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTimeLine;
};
DEFINE_TRAITS(TMetadataTimeLine)

struct TMetadataTimeLineItem
{
	// номер списка, в котором находится отображаемая статья
	UInt32 ListIndex;
	// номер слова в спсиске,  в котором находится отображаемая статья
	UInt32 EntryIndex;
	// ссылка на внешнюю sdc базу
	TMetadataExtKey ExtKey;
	// id элемента ленты времени
	TMetadataString Id;
	// левый отрезок времени отображения контента, в процентах (XXX: что?)
	UInt16 StartTime;
	// правый отрезок времени отображения контента, в процентах (XXX: что?)
	UInt16 EndTime;

	TMetadataTimeLineItem() :
		ListIndex(SLD_DEFAULT_LIST_INDEX),
		EntryIndex(SLD_DEFAULT_WORD_INDEX),
		StartTime(0),
		EndTime(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaTimeLineItem;
};
DEFINE_TRAITS(TMetadataTimeLineItem)

struct TMetadataUiElement
{
	// тип
	TMetadataString Type;

	TMetadataUiElement() {}

	static const ESldStyleMetaTypeEnum metaType = eMetaUiElement;
};
DEFINE_TRAITS(TMetadataUiElement)

struct TMetadataUrl
{
	// адрес ссылки
	TMetadataString Src;

	TMetadataUrl() {}

	static const ESldStyleMetaTypeEnum metaType = eMetaUrl;
};
DEFINE_TRAITS(TMetadataUrl)

struct TMetadataVideoSource
{
	/// Идентификатор видео
	Int32 Id;
	// ссылка на внешнюю sdc базу
	TMetadataExtKey ExtKey;
	/// Размер видеоконтента
	Int32 Resolution;
	/// Тип кодека видеоконтента (см. #ESldVideoType)
	UInt16 Codec;
	// explicitly pad to the next uint32
	UInt16 _pad0;

	TMetadataVideoSource() :
		Id(SLD_INDEX_VIDEO_NO),
		Resolution(0),
		Codec(eVideoType_Unknown),
		_pad0(0)
	{}

	static const ESldStyleMetaTypeEnum metaType = eMetaVideoSource;
};
DEFINE_TRAITS(TMetadataVideoSource)

#undef DEFINE_TRAITS

struct TSldCSSValue
{
	// css value "type" enum
	enum Type {
		// "named" css values
		Initial = 0,
		Inherit,
		Auto,
		None,

		// "generic" css values
		Number,
		Enum,
		Length,
		Percentage,
		Color,
		Url,
		String,

		// sld specific css values
		SldImageUrl, // an url that must be resolved from an internal image index
		SldStringLiteral, // string literal

		Type_Count,
		Type_Last = Type_Count - 1,
	};

	enum { Number_FracStep = 5U, Number_FracMax = 20000U };
	enum { Color_AlphaStep = 5U, Color_AlphaMax = 200U };

	// it packs in 8 bytes, so...
	union {
		struct {
			// type of the value stored
			UInt8 type;
			UInt8 _data[7];
		};

		struct {
			UInt32 _type : 8;
			UInt32 _pad0 : 8;
			UInt32 sign : 1;
			UInt32 fractional : 15;
			UInt32 integer;
		} number;

		struct { // TSizeValue
			UInt32 _type : 8;
			UInt32 _pad0 : 8;
			UInt32 units : 16;
			Int32 value;
		} length, percentage;

		struct {
			UInt32 _type : 8;
			UInt32 _pad0 : 8;
			UInt32 resource : 16;
			UInt32 index;
		} url, string;

		struct {
			UInt32 _type : 8;
			UInt32 _pad0 : 24;
			UInt32 index;
		} imageUrl;

		struct {
			UInt32 _type : 8;
			UInt32 _pad0 : 24;
			UInt32 u32;
		} color, enumeration;
	};
};

// shut up msvc about zero-sized arrays in structs/unions. we know what we are doing
#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable : 4200 )
#endif

struct TSldCSSProperty
{
	// property type #ESldCSSPropertyType
	UInt16 type;
	// count of css values for this property
	UInt8 count;
	// flag if the property is "!important"
	UInt8 important : 1;
	// padding
	UInt8 _pad0 : 7;
	// array of css values for this property
	TSldCSSValue values[];
};

struct TSldCSSPropertyBlock
{
	// count of css properties inside the block
	UInt16 count;
	// padding
	UInt16 _pad0;
	// the indices of the properties
	// potentially stored with less than 32 bits per index
	// the actual index bit size is stored in TSldCSSDataHeader::propsIndexBits
	// and depends on the total amount of property structs
	UInt32 props[];
};

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif // _SLD_METADATA_TYPES_H_
