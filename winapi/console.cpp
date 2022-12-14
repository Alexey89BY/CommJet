#include "console.h"
#include <stdio.h>
#include <richedit.h>
#include <tchar.h>

#define ARRAYSIZE(x) (sizeof(x) / sizeof(0[x]))
//-------------------------------------------------------------------------------------------------
static LRESULT APIENTRY ConsoleSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
WNDPROC OrigWndProc = (WNDPROC)LongToPtr(GetWindowLongPtr(hwnd, GWL_USERDATA));

if (uMsg == WM_APP)
  {
  unsigned int msgType = HIBYTE(HIWORD(wParam));

  if (msgType == 0)
    {
    CallWindowProc(OrigWndProc, hwnd, WM_SETTEXT, 0, 0);
    }
  else
    {
    LRESULT lResult = CallWindowProc(OrigWndProc, hwnd, WM_GETTEXTLENGTH, 0, 0);
    CallWindowProc(OrigWndProc, hwnd, EM_SETSEL, lResult, lResult);

    CHARFORMAT2 chf2;
    chf2.cbSize = sizeof(CHARFORMAT2);
    chf2.dwMask = CFM_COLOR;
    chf2.crTextColor = 0;
    chf2.dwEffects = 0;

    TCHAR Buffer[318];

    if (msgType != 0x03)
      {
      SYSTEMTIME st;
      GetLocalTime(&st);

      if (msgType != 1)
        {
        DWORD Code = (DWORD)wParam & 0x00FFFFFF;

        chf2.crTextColor = (Code != ERROR_SUCCESS)? RGB(128,0,0): RGB(0, 128, 0);;

        TCHAR Buffer2[64];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, 0, Code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), Buffer2, ARRAYSIZE(Buffer2), NULL);

        _sntprintf(Buffer, ARRAYSIZE(Buffer), TEXT("[%02hu:%02hu:%02hu.%03hu] %.10s: %.100s\n"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, lParam, Buffer2);
        }
      else
        {
        chf2.crTextColor = (COLORREF)(wParam & 0x00FFFFFF);

        _sntprintf(Buffer, ARRAYSIZE(Buffer), TEXT("[%02hu:%02hu:%02hu.%03hu] %.300s\n"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, lParam);
        }

      lParam = (LPARAM)Buffer;
      }
    else
      {
      unsigned int length = wParam & 0x00FFFFFF;

      //length = std:min(length, ARRAYSIZE(Buffer)-1);

      unsigned int idx = 0;

      for (; idx<length; idx++)
        {
        unsigned char ch = ((unsigned char*)lParam)[idx];

        if (ch >= 0x80)
          {
          Buffer[idx] = '_';
          }
        else if (ch >= 0x7F)
          {
          Buffer[idx] = '_';
          }
        else if (ch >= 0x21)
          {
          Buffer[idx] = ch;
          }
        else
          {
          Buffer[idx] = '_';
          }
        }

      Buffer[idx] = L'\0';
      lParam = (LPARAM)Buffer;
      }

    CallWindowProc(OrigWndProc, hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&chf2);
    CallWindowProc(OrigWndProc, hwnd, EM_REPLACESEL, FALSE, lParam);
    CallWindowProc(OrigWndProc, hwnd, WM_VSCROLL, SB_BOTTOM, 0);
    }

  return 0;
  }

return CallWindowProc(OrigWndProc, hwnd, uMsg, wParam, lParam);
}
//-------------------------------------------------------------------------------------------------
void CConsole::Log(unsigned long Color, const TCHAR *Message)
{
if (Output != NULL)
  {
  SendMessage(Output,WM_APP,(WPARAM)(Color & 0x00FFFFFF)| 0x01000000,(LPARAM)Message);
  }
}
//-------------------------------------------------------------------------------------------------
void CConsole::Bin(unsigned long Length, const void *Message)
{
if (Output != NULL)
  {
  if (Length > 0x00FFFFFF)
    { Length = 0x00FFFFFF; }

  SendMessage(Output,WM_APP,(WPARAM)(Length | 0x03000000),(LPARAM)Message);
  }
}
//-------------------------------------------------------------------------------------------------
void CConsole::Clear(void)
{
if (Output != NULL)
  {
  SendMessage(Output,WM_APP,(WPARAM)0,(LPARAM)NULL);
  }
}
//-------------------------------------------------------------------------------------------------
bool CConsole::Error(TCHAR* Stage, unsigned long Code, bool Success)
{
bool Failed = (Code != ERROR_SUCCESS);

if (Success || Failed)
  {
  if (Failed)
    {
    MessageBeep(MB_ICONERROR);
    }

  if (Output != NULL)
    {
    Code &= 0x00FFFFFF;
    SendMessage(Output,WM_APP,(WPARAM)Code|0x02000000,(LPARAM)Stage);
    }
  }

return Failed;
}
//-------------------------------------------------------------------------------------------------
void CConsole::Initialize(HWND hOutput)
{
if (hOutput != NULL)
  {
  Output = hOutput;

  SendMessage(Output,EM_SETREADONLY,TRUE,0);

  WNDPROC OrigWndProc = (WNDPROC)LongToPtr(SetWindowLongPtr(Output, GWL_WNDPROC, PtrToLong(ConsoleSubclassProc)));
  SetWindowLongPtr(Output, GWL_USERDATA, PtrToLong(OrigWndProc));
  }
}
//-------------------------------------------------------------------------------------------------
CConsole::~CConsole()
{
if (Output != NULL)
  {
  WNDPROC OrigWndProc = (WNDPROC)LongToPtr(GetWindowLongPtr(Output, GWL_USERDATA));
  SetWindowLongPtr(Output, GWL_WNDPROC, PtrToLong(OrigWndProc));

  Output = NULL;
  }
}
//-------------------------------------------------------------------------------------------------
