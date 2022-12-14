#include "HexParser.hpp"
//-------------------------------------------------------------------------------------------------
inline uint8_t FromHex(char ch)
{
ch |= 0x20;
return (ch - ((ch > L'9')?(L'a'-10):L'0')) & 0xFF;
}
//-------------------------------------------------------------------------------------------------
inline char ToHex(uint8_t ch)
{
return ch + ((ch > 9)?(L'a'-10):L'0');
}
//-------------------------------------------------------------------------------------------------
bool HexExParser::Encode(QByteArray const &srcData, QString &dstText, bool ForceHex)
{
if (ForceHex)
  {
  foreach (auto const &qByte, srcData)
    {
    uint8_t ch = qByte;
    dstText.append('\\');
    dstText.append(ch >> 4);
    dstText.append(ch & 0x0F);
    }
  }
else
  {
  foreach (auto const &qByte, srcData)
    {
    uint8_t ch = qByte;
    if ((ch >= '!') && (ch <= '~'))
      {
      if (ch == '\\')
        {
        dstText.append(ch);
        }
      dstText.append(ch);
      }
    else
      {
      dstText.append('\\');
      dstText.append(ToHex(ch >> 4));
      dstText.append(ToHex(ch & 0x0F));
      }
    }
  }

return true;
}
//-------------------------------------------------------------------------------------------------
bool HexExParser::Decode(QString const &srcText, QByteArray &dstData)
{
uchar Char = 0;

enum _STATE
  {
  state_Normal = 0,
  state_HexH,
  state_HexL
  } State = state_Normal;

foreach (auto const &qChar, srcText)
  {
  uchar ch = qChar.cell();
  switch (State)
    {
    case state_Normal:
      if (ch == '\\')
        {
        State = state_HexH;
        continue;
        }

      Char = ch;
      break;

    case state_HexH:
      if (ch != '\\')
        {
        Char = FromHex(ch);
        State = state_HexL;
        continue;
        }

      Char = '\\';
      State = state_Normal;
      break;

    case state_HexL:
      Char = (Char<<4) | FromHex(ch);
      State = state_Normal;
      break;
    }

  dstData.append(Char);
  }

return true;
}
//-------------------------------------------------------------------------------------------------
