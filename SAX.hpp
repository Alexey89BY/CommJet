/*#ifndef SAX_H
#define SAX_H
////////////////////////////////////////////////////////////////////////////////////////////////////
#define SAX_TITLE_LIMIT           (15)
#define SAX_SUBTITLE_LIMIT        (256)
#define SAX_INTERNALBUFFER_LENGTH (256)
//#define SAX_STACK_DEEP     (3)
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum _PreudoXML_RESULT 
  {
  sax_OK = 0,
  sax_ErrorInitialize,
  sax_ErrorSyntactical,
  sax_ErrorInvalidHead,
  sax_ErrorStructure,
  sax_Unknown
  } SAX_RESULT;
//--------------------------------------------------------------------------------------------------
typedef enum _SAX_TOKEN
  {
  saxtoken_None = 0,
  saxtoken_Error,
  saxtoken_TagBegin,
  saxtoken_Argument,
  saxtoken_TextNode,
  saxtoken_TagEnd
  } SAX_TOKEN;
  //--------------------------------------------------------------------------------------------------
typedef enum _SAX_TAG_TYPE
  {
  saxtag_Normal = 0,
  saxtag_Closing,
  saxtag_Head
  } SAX_TAG_TYPE;
//--------------------------------------------------------------------------------------------------
typedef enum _SAX_STAGE
  {
  saxstage_Initialize = 0,
  saxstage_ReadWrite = 1,
  saxstage_Finalize = 2
  } SAX_STAGE;
//--------------------------------------------------------------------------------------------------
typedef bool (*SAX_PARSE_CALLBACK)(SAX_STAGE Stage);
typedef bool (*SAX_TAG_ITEM_CALLBACK)(SAX_TOKEN Token, SAX_TAG_TYPE Type, char *Value);
typedef bool (*SAX_ARG_ITEM_CALLBACK)(char *Value);
//--------------------------------------------------------------------------------------------------
typedef struct _SAX_ARG_ITEM
 {
 char *Name;
 void *CallbackParam;
 SAX_ARG_ITEM_CALLBACK Callback;
 } SAX_ARG_ITEM;
//--------------------------------------------------------------------------------------------------
typedef struct _SAX_TAG_ITEM
 {
 char *Name;
 void *CallbackParam;
 SAX_TAG_ITEM_CALLBACK Callback;
 SAX_ARG_ITEM *Args;
 struct _SAX_TAG_ITEM *SubTags;
 unsigned short ArgsCount;
 unsigned short SubTagsCount;
 } SAX_TAG_ITEM;
////////////////////////////////////////////////////////////////////////////////////////////////////
extern SAX_PARSE_CALLBACK SAX_ParseCallback;
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SAX_IsBusy(void);
void SAX_Begin(const char *FilePath, const char *Version, void *CacheBuffer, unsigned long CacheSize);
void SAX_SetTags(const SAX_TAG_ITEM *Tags, unsigned int Count);
void SAX_SetArgs(const SAX_ARG_ITEM *Args, unsigned int Count);

//SAX_RESULT SAX_Write(SAX_TOKEN Token, SAX_TAG_TYPE TagType, char *Title, char *SubTitle);
//SAX_RESULT SAX_Flush();

SAX_RESULT SAX_End(void);
//--------------------------------------------------------------------------------------------------
bool SAX_ParseString(void *Dst, unsigned short DstSize, const char *Src);
bool SAX_ParseBitSet(void *Dst, unsigned short SetSize, const char *Src);
bool SAX_ParseFloat(void *Dst, bool Precision, const char *Src);
bool SAX_ParseTime(void *Dst, bool LongFormat, const char *Src);
bool SAX_ParseIntArray(void *Dst, unsigned short ArraySize, unsigned short ItemSize, const char *Src);
bool SAX_ParseUIntArray(void *Dst, unsigned short ArraySize, unsigned short ItemSize, const char *Src);
//--------------------------------------------------------------------------------------------------
void SAX_PT_Parse();
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif*/
