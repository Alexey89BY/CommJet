#include "SAX.h"
/*////////////////////////////////////////////////////////////////////////////////////////////////////
#define IS_SPACE(ch) ((ch) <= ' ')
#define IS_ALPHA(ch) ((((ch) >= '@') && ((ch) <= '~')) || (((ch) >= '0') && ((ch) <= '9')))
//--------------------------------------------------------------------------------------------------
#define WriteCharToBuffer(buf, len, idx, ch) {if ((idx) < (len)) buf[(idx)++] = ch;}
////////////////////////////////////////////////////////////////////////////////////////////////////
enum _SAX_PARSER_STATE
  {
  saxparser_TextNode = 0,
  saxparser_BeginTag,
  saxparser_TagName,
  saxparser_ArgName,
  saxparser_ArgParam,
  saxparser_EndTag
  };
//--------------------------------------------------------------------------------------------------
struct _SAX_PARSER
  {
  enum _SAX_PARSER_STATE State;
  enum _SAX_TAG_TYPE TagStyle;
  enum _SAX_TOKEN Token;
  unsigned long Index;
  unsigned long Cursor;
  unsigned long TitleLength;
  unsigned long SubTitleLength;
  unsigned char Char;
  char Title[SAX_TITLE_LIMIT+1];
  char SubTitle[SAX_SUBTITLE_LIMIT+1];
  };
////////////////////////////////////////////////////////////////////////////////////////////////////*/
static const char const_xml[]         /*@ "FLASH"*/ = "xml";
static const char const_version[]     /*@ "FLASH"*/ = "version";
static const char const_encoding[]    /*@ "FLASH"*/ = "encoding";
static const char const_windows1251[] /*@ "FLASH"*/ = "windows-1251";
/*//--------------------------------------------------------------------------------------------------
SAX_PARSE_CALLBACK SAX_ParseCallback;
static const char *XMLFilePath;
static const char *XMLVersion;
static unsigned char *DataBuffer;
static unsigned int DataIndex;
static unsigned int DataCount;
static unsigned int DataLimit;
static SAX_RESULT Result;
static SAX_TAG_TYPE CurrentTagType;
static const SAX_TAG_ITEM *CurrentTag;
static const SAX_ARG_ITEM *CurrentArg;
static const SAX_TAG_ITEM *TagsList;
static const SAX_ARG_ITEM *ArgsList;
static unsigned int TagsListCount;
static unsigned int ArgsListCount;
static bool HeadFounded;
static struct _SAX_PARSER Parser;
static unsigned char InternalBuffer[SAX_INTERNALBUFFER_LENGTH];
////////////////////////////////////////////////////////////////////////////////////////////////////
static void SAXTranslateChar(void)
{
switch (Parser.State)
  {
  case saxparser_TextNode:
    if (!IS_SPACE(Parser.Char))
      {
      if (Parser.Char != '<')
        {
        WriteCharToBuffer(Parser.SubTitle,SAX_SUBTITLE_LIMIT,Parser.Index,Parser.Char);
        } else
        {
        Parser.State = saxparser_BeginTag;
        if (Parser.Index)
          {
          Parser.SubTitle[Parser.Index] = 0;
          Parser.SubTitleLength = Parser.Index;
          Parser.Index = 0;
          Parser.Token = saxtoken_TextNode;
          }
        }
      }
    break;

  case saxparser_BeginTag:
    Parser.State = saxparser_TagName;
    if (Parser.Char == '/')
      {
      Parser.TagStyle = saxtag_Closing;
      } else if (Parser.Char == '?')
      {
      Parser.TagStyle = saxtag_Head;
      } else
      {
      Parser.TagStyle = saxtag_Normal;
      return;
      }
    break;

  case saxparser_TagName:
    if (IS_ALPHA(Parser.Char))
    {
    WriteCharToBuffer(Parser.Title,SAX_TITLE_LIMIT,Parser.Index,Parser.Char);
    } else
    {
    if (Parser.Index)
      {
      Parser.Title[Parser.Index] = 0;
      Parser.TitleLength = Parser.Index;
      Parser.Index = 0;
      Parser.State = saxparser_ArgName;
      Parser.Token = saxtoken_TagBegin;
      return;
      } else
      {
      if (!IS_SPACE(Parser.Char))
        Parser.Token = saxtoken_Error;
      }
    }
    break;

  case saxparser_ArgName:
    if (IS_ALPHA(Parser.Char))
      {
      WriteCharToBuffer(Parser.Title,SAX_TITLE_LIMIT,Parser.Index,Parser.Char);
      } else
      {
      if (Parser.Index)
        {
        Parser.Title[Parser.Index] = 0;
        Parser.TitleLength = Parser.Index;
        Parser.State = saxparser_ArgParam;
        Parser.Index = -1;
        Parser.SubTitle[0] = '=';
        return;
        } else if (!IS_SPACE(Parser.Char))
        {
        if (Parser.Char == '>')
          {
          Parser.State = saxparser_TextNode;
          Parser.TagStyle = saxtag_Normal;
          Parser.Token = saxtoken_TagEnd;
          } else if (Parser.Char == '/')
          {
          Parser.State = saxparser_EndTag;
          Parser.TagStyle = saxtag_Closing;
          } else if (Parser.Char == '?')
          {
          Parser.State = saxparser_EndTag;
          Parser.TagStyle = saxtag_Head;
          }
        }
      }
    break;

  case saxparser_ArgParam:
    if (Parser.Index < 0)
      {
      if (!IS_SPACE(Parser.Char))
        {
        if (Parser.Char == Parser.SubTitle[0])
          {
          if (Parser.SubTitle[0] == '=')
            Parser.SubTitle[0] = '\"';
            else Parser.Index = 0;
          } else Parser.Token = saxtoken_Error;
        }
      } else
      {
      if (Parser.Char == '\"')
        {
        Parser.SubTitle[Parser.Index] = 0;
        Parser.SubTitleLength = Parser.Index;
        Parser.Index = 0;
        Parser.State = saxparser_ArgName;
        Parser.Token = saxtoken_Argument;
        } else
        {
        WriteCharToBuffer(Parser.SubTitle,SAX_SUBTITLE_LIMIT,Parser.Index,Parser.Char);
        }
      }
    break;

  case saxparser_EndTag:
    if (Parser.Char == '>')
      {
      Parser.State = saxparser_TextNode;
      Parser.Token = saxtoken_TagEnd;
      } else Parser.Token = saxtoken_Error;
    break;
  }

Parser.Char = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool SAX_IsBusy(void)
{
return (DataBuffer != NULL);
}
//--------------------------------------------------------------------------------------------------
void SAX_Begin(const char *FilePath, const char *Version, void *CacheBuffer, unsigned long CacheSize)
{
if (CacheBuffer == NULL)
  {
  DataBuffer = InternalBuffer;
  DataLimit = SAX_INTERNALBUFFER_LENGTH;
  } else
  {
  DataBuffer = CacheBuffer;
  DataLimit = CacheSize;
  }
XMLFilePath = FilePath;
DataCount = (XMLFilePath == NULL)? DataLimit: 0;
XMLVersion = Version;
TagsListCount = 0;
ArgsListCount = 0;
}
//--------------------------------------------------------------------------------------------------
void SAX_SetTags(const SAX_TAG_ITEM *Tags, unsigned int Count)
{
TagsList = Tags;
TagsListCount = Count;
}
//--------------------------------------------------------------------------------------------------
void SAX_SetArgs(const SAX_ARG_ITEM *Args, unsigned int Count)
{
ArgsList = Args;
ArgsListCount = Count;
}
//--------------------------------------------------------------------------------------------------
bool SAX_DefaultParseCallback(SAX_STAGE Stage)
{
if (Stage == saxstage_ReadWrite)
  {
  //FS_TaskFileRead(fsTask, (void *)DataBuffer, DataLimit, &DataCount);
  //PT_WAIT_UNTIL(pt, FS_TaskGetCompletion(fsTask));
  } else if (Stage == saxstage_Initialize)
  {
  //PT_WAIT_UNTIL(pt,FS_TaskBeginPublic(&fsTask));
  //FS_TaskFileOpen(fsTask, XMLFilePath, FA_OPEN_EXISTING | FA_READ); // Пытаемся открыть файл (открываем только существующий только для чтения).
  //PT_WAIT_UNTIL(pt, FS_TaskGetCompletion(fsTask));
  //SAX_StageResult = FS_TaskGetResult(fsTask) == FR_OK;
  } else if (Stage == saxstage_Finalize)
  {
  //if (FS_TaskFileTest(fsTask))
    //{
    //FS_TaskFileClose(fsTask); // и закрываем его.
    //PT_WAIT_UNTIL(pt, FS_TaskGetCompletion(fsTask));
    //}
  //FS_TaskEnd(&fsTask);
  }
}
//--------------------------------------------------------------------------------------------------
void SAX_PT_Parse(void)
{
HeadFounded = false;
Parser.State = saxparser_TextNode;
Parser.Cursor = 0;
Parser.Index = 0;
Parser.Char = 0;

SAX_ParseCallback = SAX_DefaultParseCallback;

if (!SAX_ParseCallback(saxstage_Initialize)) // Если файл не открылся,
  {
  Result = sax_ErrorInitialize;
  } else
  {
  Result = sax_Unknown;
  while (Result == sax_Unknown)
    {
    Parser.Token = saxtoken_None;
    do {
      if (Parser.Char == 0)
        {
        if (DataIndex < DataCount)
          {
          Parser.Char = DataBuffer[DataIndex++];
          Parser.Cursor++;
          } else
          {

          if ((!SAX_ParseCallback(saxstage_ReadWrite)) || (DataCount == 0))
            {
            Result = sax_OK;
            break;
            } else
            {
            Parser.Char = DataBuffer[0];
            DataIndex = 1;
            Parser.Cursor++;
            }
         }
       }

    SAXTranslateChar();

    } while (Parser.Token == saxtoken_None);

    if (Parser.Token == saxtoken_TextNode)
      {
      if (HeadFounded)
        {
        if (CurrentTag)
          {
          if (!(CurrentTag->Callback(saxtoken_TextNode, CurrentTagType, Parser.SubTitle)))
            Result = sax_ErrorStructure;
          }
        } else Result = sax_ErrorInvalidHead;
      } else if (Parser.Token == saxtoken_TagBegin)
      {
      if (HeadFounded)
        {
        CurrentTag = NULL;
        for (unsigned int i=0; i<TagsListCount; i++)
          if (strcmp(Parser.Title, TagsList[i].Name) == 0)
            {
            CurrentTag = &TagsList[i];
            break;
            }

        if (CurrentTag)
          {
          if (!(CurrentTag->Callback(saxtoken_TagBegin, CurrentTagType = Parser.TagStyle, NULL)))
            Result = sax_ErrorStructure;
          }
        } else if ((Parser.TagStyle != saxtag_Head) || (strcmp(Parser.Title, const_xml) != 0))
        {
        Result = sax_ErrorInvalidHead;
        }
      } else if (Parser.Token == saxtoken_Argument)
      {
      if (HeadFounded)
        {
        CurrentArg = NULL;
        for (unsigned int i=0; i<ArgsListCount; i++)
          if (strcmp(Parser.Title, ArgsList[i].Name) == 0)
            {
            CurrentArg = &ArgsList[i];
            break;
            }

        if (CurrentArg)
          {
          if (!(CurrentArg->Callback(Parser.SubTitle)))
            Result = sax_ErrorStructure;
          }
        } else if (((strcmp(Parser.Title, const_version) == 0) && (strcmp(Parser.SubTitle, XMLVersion) != 0))
                   ||((strcmp(Parser.Title, const_encoding) == 0) && (strcmp(Parser.SubTitle, const_windows1251) != 0)))
        {
        Result = sax_ErrorInvalidHead;
        }
      } else if (Parser.Token == saxtoken_TagEnd)
      {
      if (HeadFounded)
        {
        if (CurrentTag)
          {
          if (!(CurrentTag->Callback(saxtoken_TagEnd, CurrentTagType, Parser.Title)))
            Result = sax_ErrorStructure;
          }
        } else
        {
        if (Parser.TagStyle != saxtag_Head)
          Result = sax_ErrorInvalidHead;
          else HeadFounded = true;
        }
      } else if (Parser.Token == saxtoken_Error)
      {
      Result = sax_ErrorSyntactical;
      }


    }
  }

SAX_ParseCallback(saxstage_Finalize);
}
//--------------------------------------------------------------------------------------------------
SAX_RESULT SAX_End(void)
{
DataBuffer = NULL;
return Result;
}
//--------------------------------------------------------------------------------------------------
bool SAX_ParseString(void *Dst, unsigned short DstSize, const char *Src)
{
strncpy(Dst, Src, DstSize);
return true;
}
//--------------------------------------------------------------------------------------------------
bool SAX_ParseBitSet(void *Dst, unsigned short SetSize, const char *Src)
{
unsigned long x = 0;
for (; *Src; Src++)
  {
  x<<=1;
  if ((*Src) != '0')
    x |= 1;
  }

switch (SetSize)
  {
  case 32:
    *((unsigned long *)Dst) = x;
    break;
  case 16:
    *((unsigned short *)Dst) = (unsigned short)x;
    break;
  case 8:
    *((unsigned char *)Dst) = (unsigned char)x;
    break;

  default:
    return false;
  }

return true;
}
//--------------------------------------------------------------------------------------------------
bool SAX_ParseFloat(void *Dst, bool Precision, const char *Src)
{
char *ptr;
if (Precision)
  {
  double x = strtod(Src, &ptr);
  if (Src == ptr)
    return false;
  *((double *)Dst) = x;
  } else
  {
  float x = strtof(Src, &ptr);
  if (Src == ptr)
    return false;
  *((float *)Dst) = x;
  }
return true;
}
//--------------------------------------------------------------------------------------------------
bool SAX_ParseUIntArray(void *Dst, unsigned short ArraySize, unsigned short ItemSize, const char *Src)
{
char *ptr;
switch (ItemSize)
  {
  case sizeof(unsigned long):
    for (unsigned int i=0; i<ArraySize; i++, Src = ptr)
      {
      unsigned long x = strtoul(Src, &ptr, 10);
      if (ptr == Src)
        return false;
      *((unsigned long *)Dst) = x;
      Dst = (unsigned long *)Dst + 1;
      }
    break;

  case sizeof(unsigned short):
    for (unsigned int i=0; i<ArraySize; i++, Src = ptr)
      {
      unsigned long x = strtoul(Src, &ptr, 10);
      if (ptr == Src)
        return false;
      *((unsigned short *)Dst) = SAT_U16(x);
      Dst = (unsigned short *)Dst + 1;
      }
    break;

  case sizeof(unsigned char):
    for (unsigned int i=0; i<ArraySize; i++, Src = ptr)
      {
      unsigned long x = strtoul(Src, &ptr, 10);
      if (ptr == Src)
        return false;
      *((unsigned char *)Dst) = SAT_U8(x);
      Dst = (unsigned char *)Dst + 1;
      }
    break;

  default:
    return false;
  }

return true;
}
//--------------------------------------------------------------------------------------------------
bool SAX_ParseIntArray(void *Dst, unsigned short ArraySize, unsigned short ItemSize, const char *Src)
{
char *ptr;
switch (ItemSize)
  {
  case sizeof(long):
    for (unsigned int i=0; i<ArraySize; i++, Src = ptr)
      {
      long x = strtol(Src, &ptr, 10);
      if (ptr == Src)
        return false;
      *((long *)Dst) = x;
      Dst = (long *)Dst + 1;
      }
    break;

  case sizeof(short):
    for (unsigned int i=0; i<ArraySize; i++, Src = ptr)
      {
      long x = strtol(Src, &ptr, 10);
      if (ptr == Src)
        return false;
      *((short *)Dst) = SAT_S16(x);
      Dst = (short *)Dst + 1;
      }
    break;

  case sizeof(char):
    for (unsigned int i=0; i<ArraySize; i++, Src = ptr)
      {
      long x = strtol(Src, &ptr, 10);
      if (ptr == Src)
        return false;
      *((char *)Dst) =  SAT_S8(x);
      Dst = (char *)Dst + 1;
      }
    break;

  default:
    return false;
  }

return true;
}
//--------------------------------------------------------------------------------------------------
bool SAX_ParseTime(void *Dst, bool LongFormat, const char *Src)
{
char *ptr;
unsigned long h = strtoul(Src, &ptr, 10);
if ((Src == ptr) || (h >= DAY_HOURS_NUMBER) || (*ptr != ':'))
  return false;

Src = ptr+1;
unsigned long m = strtoul(Src, &ptr, 10);
if ((Src == ptr) || (m >= HOUR_MINUTES_NUMBER))
  return false;

unsigned long s;
if (LongFormat)
  {
  if (*ptr != ':')
    return false;

  Src = ptr+1;
  s = strtoul(Src, &ptr, 10);
  if ((Src == ptr) || (s >= MINUTE_SECONDS_NUMBER))
    return false;
  } else
  {
  s = 0;
  }

((struct tm *)Dst)->tm_hour = h;
((struct tm *)Dst)->tm_min = m;
((struct tm *)Dst)->tm_sec = s;

return true;
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////*/


