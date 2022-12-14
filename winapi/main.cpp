#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_WINDOWS 0x0410
#define _WIN32_IE 0x0600
//-------------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
//-------------------------------------------------------------------------------------------------
// #include <stdio.h>
//-------------------------------------------------------------------------------------------------
#include <windows.h>
#include <Commdlg.h>
#include <commctrl.h>
#include <stdlib.h> // C RunTime Header Files
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <shellapi.h>
//-------------------------------------------------------------------------------------------------
#include "afxres.h"
#include "resource.h"
//-------------------------------------------------------------------------------------------------
#include "crc.c" // Υπενό!!!!!!!!!!!!!!!!!
#include "console.h"
#include "NTCB.h"
#include "SAX.h"
#include "HexParser.h"
#include "CommPort.h"
//-------------------------------------------------------------------------------------------------
#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
//-------------------------------------------------------------------------------------------------
#define MIN(a, b)  (((a) > (b))? (b): (a))
#define MAX(a, b)  (((a) > (b))? (a): (b))
//-------------------------------------------------------------------------------------------------
HINSTANCE hAppInstance;
HACCEL hAccelerators;
HWND hAppWindow;
HWND hDlgConHexEx;
HWND hDlgConText;
HMENU hMainMenu;
unsigned int SendCounter;
CConsole Console;
CConsole Console2;
TCHAR CurrDir[MAX_PATH+1];
CNTCBCommand NTCBCommand;
//-------------------------------------------------------------------------------------------------
bool Connected;
CCommPort CommPort;
CCommPort::PROPERTIES ConnProps;
//-------------------------------------------------------------------------------------------------
const TCHAR ConnPropsFileName[] = TEXT("settings.bin");
//-------------------------------------------------------------------------------------------------
INT_PTR CALLBACK MainDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CommPropsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MiniConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConsoleDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//-------------------------------------------------------------------------------------------------
void FixConnProps(CCommPort::PROPERTIES *ConProps);
#define FreestyleTextLimit (4096)
///////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////
//int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
//INITCOMMONCONTROLSEX icc;
//icc.dwSize = sizeof(icc);
//icc.dwICC = 0x7FF;
//InitCommonControlsEx(&icc);

LoadLibrary(TEXT("Riched20.dll"));

GetCurrentDirectory(MAX_PATH,CurrDir);

hAppInstance = GetModuleHandle(NULL);// hInstance;
hAccelerators = LoadAccelerators(hAppInstance,MAKEINTRESOURCE(IDR_ACCELERATORS));

  {
  HICON hIcon = LoadIcon(hAppInstance,MAKEINTRESOURCE(IDI_ICON));

  hAppWindow = CreateDialogParam(hAppInstance,MAKEINTRESOURCE(IDD_MAINDIALOG),NULL,MainDialogProc,(LPARAM)hIcon);

  hDlgConHexEx = CreateDialogParam(hAppInstance,MAKEINTRESOURCE(IDD_CON_HEXEX),hAppWindow,ConsoleDialogProc,PtrToLong(&Console));
  hDlgConText = CreateDialogParam(hAppInstance,MAKEINTRESOURCE(IDD_CON_TEXT),hAppWindow,ConsoleDialogProc,0);

  hMainMenu = GetMenu(hAppWindow);

  RECT r;
  RECT dr;

  GetWindowRect(hAppWindow, &r);
  GetClientRect(GetDesktopWindow(), &dr);
  LONG width = (r.right-r.left)>>1;

  LONG x = (dr.right-dr.left - (r.right-r.left)) >> 1;
  LONG y = (dr.bottom-dr.top - (r.bottom+width-r.top)) >> 1;

  SetWindowPos(hAppWindow, HWND_BOTTOM, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE|SWP_SHOWWINDOW);

  GetWindowRect(hAppWindow, &r);

  SetWindowPos(hDlgConHexEx, HWND_BOTTOM, r.left, r.bottom, width, width, SWP_NOZORDER|SWP_SHOWWINDOW);
  SetWindowPos(hDlgConText, HWND_BOTTOM, r.left+width, r.bottom, width, width, SWP_NOZORDER|SWP_SHOWWINDOW);

  SetFocus(hAppWindow);
  }

for(;;)
  {
  MSG Msg;

  switch (GetMessage(&Msg, NULL, 0, 0))
    {
    case TRUE:
      if (TranslateAccelerator(hAppWindow, hAccelerators, &Msg))
        { continue; }

      if (IsDialogMessage(hAppWindow, &Msg))
        { continue; }

      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
      continue;

    case FALSE:
      return (int)Msg.wParam;

    default:
      return (-1);
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long LoadConnPropsFromFile()
{
TCHAR FileName[MAX_PATH+1];
_tcsncpy(FileName,CurrDir,MAX_PATH);
_tcsncat(FileName,TEXT("\\"),MAX_PATH);
_tcsncat(FileName,ConnPropsFileName,MAX_PATH);

HANDLE hFile = CreateFile(FileName,GENERIC_READ,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
if (hFile == INVALID_HANDLE_VALUE) return GetLastError();

unsigned long ulError = ERROR_SUCCESS, BytesRead;
if (!ReadFile(hFile,&ConnProps,sizeof(CCommPort::PROPERTIES),&BytesRead,NULL)) ulError = GetLastError();

CloseHandle(hFile);
return ulError;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long SaveConnPropsToFile()
{
TCHAR FileName[MAX_PATH+1];
_tcsncpy(FileName,CurrDir,MAX_PATH);
_tcsncat(FileName,TEXT("\\"),MAX_PATH);
_tcsncat(FileName,ConnPropsFileName,MAX_PATH);

HANDLE hFile = CreateFile(FileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,NULL);
if (hFile == INVALID_HANDLE_VALUE) return GetLastError();

unsigned long ulError = ERROR_SUCCESS, BytesWritten;
if (!WriteFile(hFile,&ConnProps,sizeof(CCommPort::PROPERTIES),&BytesWritten,NULL)) ulError = GetLastError();

CloseHandle(hFile);

return ulError;
}
//-------------------------------------------------------------------------------------------------
#define RECEIVE_LIMIT (4096)

unsigned char Data[RECEIVE_LIMIT];
TCHAR Msg[3*RECEIVE_LIMIT+1];

///////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////

void CropRect(RECT *r, int left, int top, int right, int bottom)
{
r->bottom -= bottom;
r->left += left;
r->right -= right;
r->top += top;
}

INT_PTR CALLBACK MainDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
switch (uMsg)
  {
  case WM_INITDIALOG:
    SetClassLong(hwndDlg,GCL_HICON,(LONG)lParam);
    SendDlgItemMessage(hwndDlg,IDC_EDIT1,EM_SETLIMITTEXT,FreestyleTextLimit,0);
    SendDlgItemMessage(hwndDlg,IDC_EDIT2,EM_SETLIMITTEXT,FreestyleTextLimit,0);
    {
    HWND hCombo = GetDlgItem(hwndDlg,IDC_COMBO2);
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"LENGTH");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"XOR");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"SUM");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"SUM-NEG");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"SUM-NOT");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"EGTS CRC8");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"CRC8 POLY x^8+x^7+x^4+x^0");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"CRCMODBUS");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"CRCDALLAS");
    SendMessage(hCombo,CB_SETCURSEL,0,0);

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO3);
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"USER");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"NAVTELECOM NTCB");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"NAVTELECOM NTCB FLEX");
    SendMessage(hCombo,CB_SETCURSEL,0,0);
    }

    {
    HWND hUpDown = GetDlgItem(hwndDlg,IDC_SPIN1);
    SendMessage(hUpDown, UDM_SETBUDDY, PtrToLong(GetDlgItem(hwndDlg, IDC_EDIT3)), 0);
    SendMessage(hUpDown, UDM_SETRANGE32, 1, 60000);
    SendMessage(hUpDown, UDM_SETPOS32, 0, 1000);
    }

    Console.Error(TEXT("Load props"), LoadConnPropsFromFile());
    FixConnProps(&ConnProps);

    {
    RECT r;
    GetClientRect(hwndDlg, &r);

    PostMessage(hwndDlg,WM_SIZE, 0, MAKELPARAM(r.right-r.left, r.bottom-r.top));
    }


    PostMessage(hwndDlg,WM_COMMAND,ID_COMM_REFRESHINFO,0);
    PostMessage(hwndDlg,WM_COMMAND,ID_DISCONNECT,0);
    PostMessage(hwndDlg,WM_COMMAND,IDC_BUTTON1,0);
    break;


 //case WM_NOTIFY:
 //   switch (((LPNMHDR)lParam)->code)
 //     {
 //     case TCN_SELCHANGE:
 //       {
 //       int TabNo = TabCtrl_GetCurSel(hTab);
 //
 //       for (unsigned int i = 0; i<TABS_COUNT; i++)
 //         ShowWindow(hTabs[i], (TabNo == i)? SW_SHOW: SW_HIDE);
 //       }
 //       return (INT_PTR)TRUE;
 //
 //     default:
 //       break;
 //     }
 //   break;
  case WM_COMMMSG:
      {
      switch (wParam)
        {
        case CCommPort::evnt_Receive:
          while (lParam != 0)
            {
            unsigned long len = MIN((unsigned long)lParam, RECEIVE_LIMIT);

            Console.Error(TEXT("Receive"), CommPort.Receive(Data,&len), false);

            if (len == 0)
              { break; }

            lParam -= len;

            //Console2.Bin(len, Data);

            //HexExParser::Encode(Data, len, Msg, &len, false);

            _sntprintf(Msg, ARRAYSIZE(Msg), TEXT("receive %lu"), len);
            Console.Log(0, Msg);
            }
          break;

        case CCommPort::evnt_Signal:
          {
          CheckDlgButton(hAppWindow,IDC_CHECK_CTS,(lParam & MS_CTS_ON)?BST_CHECKED:BST_UNCHECKED);
          CheckDlgButton(hAppWindow,IDC_CHECK_DSR,(lParam & MS_DSR_ON)?BST_CHECKED:BST_UNCHECKED);
          CheckDlgButton(hAppWindow,IDC_CHECK_RI,(lParam & MS_RING_ON)?BST_CHECKED:BST_UNCHECKED);
          }
          break;

        case CCommPort::evnt_Error:
          {

          }
          break;

        case CCommPort::evnt_Critical:
          {
          Console.Error(TEXT("Comm port"), (unsigned long)lParam);
          }
          break;
        }

      /*switch (Event)
  {
  case peDataReceive:
    {
    PostMessage(hAppWindow, WM_COMMPORT, peDataReceive, Param);

    }

  case peKernelError:
    PostMessage(hAppWindow, WM_COMMPORT, peKernelError, Param);


    Console.ErrorDetect(hAppWindow,Param);
    PostMessage(hAppWindow,WM_COMMAND,ID_DISCONNECT,NULL);
    return TRUE;



  default:
    return 0;
  }*/

      break;
      }

  case WM_CONTEXTMENU:
    {
    POINT pt;
    GetCursorPos(&pt);
    if (WindowFromPoint(pt) == GetDlgItem(hwndDlg,IDC_LIST1)) TrackPopupMenu(GetSubMenu(hMainMenu,3),TPM_LEFTALIGN,pt.x,pt.y,0,hwndDlg,NULL);
    }
    break;

  case WM_TIMER:
    PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(ID_SEND, 0), 0);
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
      case ID_COMM_REFRESHINFO:
        {
        TCHAR Buffer[8]=TEXT("COM0000");
        //_itow_s(ConnProps.PortNo+1,&Buffer[3],4,10);
        SetDlgItemText(hwndDlg,IDC_EDIT_PORT,Buffer);
        //_itow_s(ConnProps.BaudRate,Buffer,7,10);
        SetDlgItemText(hwndDlg,IDC_EDIT_BAUDRATE,Buffer);
        Buffer[0]=L'0'+ConnProps.ByteSize;
        Buffer[1]=L'-';
        switch (ConnProps.Parity)
          {
          case NOPARITY: Buffer[2]=L'N'; break;
          case ODDPARITY: Buffer[2]=L'O'; break;
          case EVENPARITY: Buffer[2]=L'E'; break;
          case MARKPARITY: Buffer[2]=L'M'; break;
          case SPACEPARITY: Buffer[2]=L'S'; break;
          }
        Buffer[3]=L'-';
        switch (ConnProps.StopBits)
          {
          case ONESTOPBIT: Buffer[4]=L'1'; Buffer[5]=0; break;
          case ONE5STOPBITS: Buffer[4]=L'1'; Buffer[5]=L'.'; Buffer[6]=L'5'; Buffer[7]=0; break;
          case TWOSTOPBITS: Buffer[4]=L'2'; Buffer[5]=0; break;
          }
        Buffer[3]=L'-';
        SetDlgItemText(hwndDlg,IDC_EDIT_MODE,Buffer);
        }
        break;

      case ID_CONNECT:
        if (!Console.Error(TEXT("Connect"), CommPort.Open(&ConnProps, hwndDlg)))
          {
          EnableMenuItem(GetSubMenu(hMainMenu,1),ID_CONNECT,MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
          EnableMenuItem(GetSubMenu(hMainMenu,1),ID_DISCONNECT,MF_BYCOMMAND|MF_ENABLED);
          EnableMenuItem(GetSubMenu(hMainMenu,1),ID_SEND,MF_BYCOMMAND|MF_ENABLED);
          EnableWindow(GetDlgItem(hwndDlg,ID_SEND),TRUE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DTR),TRUE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_RTS),TRUE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_REFRESH),TRUE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_SPIN1),TRUE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT3),TRUE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK1),TRUE);
          CheckDlgButton(hwndDlg,IDC_CHECK_DTR,BST_UNCHECKED);
          CheckDlgButton(hwndDlg,IDC_CHECK_RTS,BST_UNCHECKED);
          SetDlgItemText(hwndDlg,IDC_BUTTON_CONNECT,TEXT("Disconnect"));
          PostMessage(hwndDlg,WM_COMMAND,IDC_BUTTON_REFRESH,0);
          Connected = true;
          }
        break;

      case ID_DISCONNECT:
        CheckDlgButton(hwndDlg, IDC_CHECK1, BST_UNCHECKED);
        SendMessage(hwndDlg,WM_COMMAND,IDC_CHECK1,0);

        if (!Console.Error(TEXT("Disconnect"), CommPort.Close()))
          {
          Connected = false;
          EnableMenuItem(GetSubMenu(hMainMenu,1),ID_DISCONNECT,MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
          EnableMenuItem(GetSubMenu(hMainMenu,1),ID_CONNECT,MF_BYCOMMAND|MF_ENABLED);
          EnableMenuItem(GetSubMenu(hMainMenu,1),ID_SEND,MF_BYCOMMAND|MF_GRAYED|MF_DISABLED);
          EnableWindow(GetDlgItem(hwndDlg,ID_SEND),FALSE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_DTR),FALSE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK_RTS),FALSE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON_REFRESH),FALSE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_SPIN1),FALSE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT3),FALSE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_CHECK1),FALSE);
          SetDlgItemText(hwndDlg,IDC_BUTTON_CONNECT,TEXT("Connect"));
          }
        break;

      case IDC_BUTTON_CONNECT:
        PostMessage(hwndDlg,WM_COMMAND,(Connected)?ID_DISCONNECT:ID_CONNECT,0);
        break;

      case ID_COMM_PROPERTIES:
        if (DialogBoxParam(hAppInstance,MAKEINTRESOURCE(IDD_PORTPROPS),hwndDlg,CommPropsDialogProc,0)==IDOK)
          {
          SendMessage(hwndDlg,WM_COMMAND,ID_COMM_REFRESHINFO,0);
          if (Connected)
            {
            SendMessage(hwndDlg,WM_COMMAND,ID_DISCONNECT,0);
            SendMessage(hwndDlg,WM_COMMAND,ID_CONNECT,0);
            }
          }
        break;

      case ID_SEND:
        {
        const int len = (FreestyleTextLimit*3)+1;
        TCHAR Buffer[len];
        size_t Length = GetDlgItemText(hwndDlg,IDC_EDIT1,Buffer,len);
        HexExParser::Decode(Buffer, Length, (unsigned char *)Buffer, &Length);

        switch (SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_GETCURSEL,0,0))
          {
          default:
            Console.Error(TEXT("Send"), CommPort.Send((unsigned char *)Buffer, Length));
            break;

          case 1:
            NTCBCommand.BeginWrite();
            NTCBCommand.Write(Buffer, Length);
            NTCBCommand.Complete(1, 0);
            Console.Error(TEXT("Send"), CommPort.Send((unsigned char *)NTCBCommand.Data(), NTCBCommand.Length()));
            break;

          case 2:
            break;
          }

        SendCounter++;
        SetDlgItemInt(hwndDlg, IDC_EDIT4, SendCounter, FALSE);
        }
        break;

      case IDC_BUTTON1:
        SendCounter = 0;
        SetDlgItemInt(hwndDlg, IDC_EDIT4, SendCounter, FALSE);
        break;

      case ID_CALCULATE:
       {
       TCHAR Buffer[FreestyleTextLimit+1];
       size_t Length = GetDlgItemText(hwndDlg,IDC_EDIT2,Buffer,FreestyleTextLimit+1);
       HexExParser::Decode(Buffer, Length, (unsigned char *)Buffer, &Length);

       unsigned int Result = 0;
       int len = 1;
       switch (SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0))
         {
         case 0:
           Result = (Result > 0xFF)? 0xFF: ((unsigned char)Length);
           break;

         case 1:
           for (unsigned long i=0;i<Length;i++)
             Result ^= ((unsigned char *)Buffer)[i];
           break;

         case 2:
           for (unsigned long i=0;i<Length;i++)
             Result += ((unsigned char *)Buffer)[i];
           break;

         case 3:
           for (unsigned long i=0;i<Length;i++)
             Result += ((unsigned char *)Buffer)[i];
           Result = -Result;
           break;

         case 4:
           for (unsigned long i=0;i<Length;i++)
             Result += ((unsigned char *)Buffer)[i];
           Result = ~Result;
           break;

         case 5:
           Result = egts_crc8((unsigned char *)Buffer, Length);
           break;

         case 6:
           Result = 0xFF - crc8_poly91_eval(0xFF, (unsigned char *)Buffer, Length);
           break;

         case 7:
           Result = eval_crc16_modbus((unsigned char *)Buffer, Length);
           len = 2;
           break;

         case 8:
           Result = eval_dallas_crc8(0, (unsigned char *)Buffer, Length);
           break;
         }

       HexExParser::Encode((unsigned char*)&Result, len, Buffer, &Length, true);
       SetDlgItemText(hwndDlg,IDC_EDIT9,Buffer);
       }
       break;

      case ID_CONSOLE_CLEAR:
        Console.Clear();
        Console2.Clear();
        break;

      case ID_ABOUT_VERSION:
        MessageBox(hwndDlg, TEXT("Build: ")TEXT(__DATE__)TEXT(" ")TEXT(__TIME__), TEXT("Version"), MB_ICONINFORMATION|MB_OK);
        break;

      case ID_MINI_CONFIG:
        DialogBoxParam(hAppInstance, MAKEINTRESOURCE(IDD_DLG_MINI_CONFIG), hwndDlg, MiniConfigDialogProc, (LPARAM)GetClassLong(hwndDlg,GCL_HICON));
        break;

      case ID_FILE_SAVE_AS:
        {
        TCHAR FileName[MAX_PATH+1] = {0};
        OPENFILENAME ofn = {0};
        ofn.lStructSize=sizeof(OPENFILENAME);
        ofn.hwndOwner=hwndDlg;
        ofn.hInstance=hAppInstance;
        ofn.lpstrDefExt=TEXT("txt");
        ofn.lpstrFilter = TEXT("All Files\0*.*\0\0");
        ofn.lpstrFile=FileName;
        ofn.nMaxFile=MAX_PATH;
        ofn.Flags=OFN_CREATEPROMPT|OFN_ENABLESIZING|OFN_OVERWRITEPROMPT;

        if (GetSaveFileName(&ofn))
          {
          unsigned long ulError = ERROR_SUCCESS;

          HANDLE hFile = CreateFile(FileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,NULL);
          if (hFile != INVALID_HANDLE_VALUE)
            {
            //ulError = Console.SaveToFile(hFile);
            CloseHandle(hFile);
            } else ulError = GetLastError();

          Console.Error(TEXT("Save"), ulError);
          }
        }
        break;

      case IDC_CHECK_DTR:
       if (!Connected) break;
       switch (SendDlgItemMessage(hwndDlg,IDC_CHECK_DTR,BM_GETCHECK,0,0))
         {
         case BST_CHECKED:
           if (Console.Error(TEXT("Clear DTR"), CommPort.SetDTR(false))) break;
           CheckDlgButton(hwndDlg,IDC_CHECK_DTR,BST_UNCHECKED);
           break;
         case BST_UNCHECKED:
           if (Console.Error(TEXT("Set DTR"), CommPort.SetDTR(true))) break;
           CheckDlgButton(hwndDlg,IDC_CHECK_DTR,BST_CHECKED);
           break;
         }
       break;

      case IDC_CHECK_RTS:
       if (!Connected) break;
       switch (SendDlgItemMessage(hwndDlg,IDC_CHECK_RTS,BM_GETCHECK,0,0))
         {
         case BST_CHECKED:
           if (Console.Error(TEXT("Clear RTS"), CommPort.SetRTS(false))) break;
           CheckDlgButton(hwndDlg,IDC_CHECK_RTS,BST_UNCHECKED);
           break;
         case BST_UNCHECKED:
           if (Console.Error(TEXT("Set RTS"), CommPort.SetRTS(true))) break;
           CheckDlgButton(hwndDlg,IDC_CHECK_RTS,BST_CHECKED);
           break;
         }
       break;

      case IDC_CHECK1:
        if (IsDlgButtonChecked(hwndDlg, IDC_CHECK1) == BST_CHECKED)
          {
          EnableWindow(GetDlgItem(hwndDlg,IDC_SPIN1),FALSE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT3),FALSE);
          SetTimer(hwndDlg, 1, (UINT)SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_GETPOS32, 0, 0), NULL);
          } else
          {
          EnableWindow(GetDlgItem(hwndDlg,IDC_SPIN1),TRUE);
          EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT3),TRUE);
          KillTimer(hwndDlg, 1);
          }
        break;

      case IDCANCEL:
        if (MessageBox(hwndDlg,TEXT("Do you really want to exit program?"),TEXT("Question"),MB_ICONQUESTION|MB_YESNO) == IDNO) break;
        Console.Error(TEXT("Save props"), SaveConnPropsToFile());
        SetWindowLong(hwndDlg,DWL_MSGRESULT,TRUE);
        PostQuitMessage(0);
        break;
      }
    break;

  default:
    return FALSE;
  }
return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#define BaudRateCount 14
const wchar_t *BaudRateCaptions[BaudRateCount] = {L"110",L"300",L"600",L"1200",L"2400",L"4800",L"9600",L"14400",L"19200",L"38400",L"57600",L"115200",L"128000",L"256000"};
const unsigned long BaudRateValues[BaudRateCount] = {110,300,600,1200,2400,4800,9600,14400,19200,38400,57600,115200,128000,256000};
//-------------------------------------------------------------------------------------------------
void FixConnProps(CCommPort::PROPERTIES *ConnProps)
{
unsigned long i=BaudRateCount-1;
for (;i>0;i--) {if (ConnProps->BaudRate == BaudRateValues[i]) break;}
ConnProps->BaudRate = BaudRateValues[i];

if (ConnProps->ByteSize < 5) ConnProps->ByteSize = 5;
if (ConnProps->ByteSize > 8) ConnProps->ByteSize = 8;
if (ConnProps->Parity > SPACEPARITY) ConnProps->Parity = NOPARITY;
if (ConnProps->StopBits > TWOSTOPBITS) ConnProps->StopBits = ONESTOPBIT;
}
//-------------------------------------------------------------------------------------------------
INT_PTR CALLBACK CommPropsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
switch (uMsg)
  {
  case WM_INITDIALOG:
    {
    HWND hCombo = GetDlgItem(hwndDlg,IDC_COMBO1);
    TCHAR Buffer[7]=TEXT("COM000");
    for (int i=1;i<=256;i++)
      {
      //_itow_s(i,&Buffer[3],4,10);
      SendMessage(hCombo,CB_ADDSTRING,0,(WPARAM)Buffer);
      }
    SendMessage(hCombo,CB_SETCURSEL,ConnProps.PortNo,0);

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO2);
    for (int i=0;i<BaudRateCount;i++)
      {
      SendMessage(hCombo,CB_ADDSTRING,0,(WPARAM)BaudRateCaptions[i]);
      if (ConnProps.BaudRate == BaudRateValues[i]) SendMessage(hCombo,CB_SETCURSEL,i,0);
      }

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO3);
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"5");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"6");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"7");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"8");
    SendMessage(hCombo,CB_SETCURSEL,ConnProps.ByteSize-5,0);

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO4);
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"No parity");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"Odd");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"Even");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"Mark");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"Space");
    SendMessage(hCombo,CB_SETCURSEL,ConnProps.Parity,0);

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO5);
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"1");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"1.5");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"2");
    SendMessage(hCombo,CB_SETCURSEL,ConnProps.StopBits,0);
    }

    SendDlgItemMessage(hwndDlg,IDC_COMBO5,CB_SETCURSEL,ConnProps.StopBits,0);
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
      case IDOK:
        ConnProps.PortNo = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
        ConnProps.BaudRate = BaudRateValues[SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0)];
        ConnProps.ByteSize = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_GETCURSEL,0,0)+5;
        ConnProps.Parity = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_COMBO4,CB_GETCURSEL,0,0);
        ConnProps.StopBits = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_COMBO5,CB_GETCURSEL,0,0);

      case IDCANCEL:
        EndDialog(hwndDlg,LOWORD(wParam));
        break;
      }
    break;
  default:
    return FALSE;
  }
return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK MiniConfigDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
switch (uMsg)
  {
  case WM_INITDIALOG:
    {
    HWND hCombo = GetDlgItem(hwndDlg,IDC_COMBO1);
    wchar_t Buffer[7]=L"COM000";
    for (int i=1;i<=256;i++)
      {
      //_itow_s(i,&Buffer[3],4,10);
      SendMessage(hCombo,CB_ADDSTRING,0,(WPARAM)Buffer);
      }
    SendMessage(hCombo,CB_SETCURSEL,ConnProps.PortNo,0);

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO2);
    for (int i=0;i<BaudRateCount;i++)
      {
      SendMessage(hCombo,CB_ADDSTRING,0,(WPARAM)BaudRateCaptions[i]);
      if (ConnProps.BaudRate == BaudRateValues[i]) SendMessage(hCombo,CB_SETCURSEL,i,0);
      }

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO3);
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"5");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"6");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"7");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"8");
    SendMessage(hCombo,CB_SETCURSEL,ConnProps.ByteSize-5,0);

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO4);
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"No parity");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"Odd");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"Even");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"Mark");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"Space");
    SendMessage(hCombo,CB_SETCURSEL,ConnProps.Parity,0);

    hCombo = GetDlgItem(hwndDlg,IDC_COMBO5);
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"1");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"1.5");
    SendMessage(hCombo,CB_ADDSTRING,0,(LPARAM)L"2");
    SendMessage(hCombo,CB_SETCURSEL,ConnProps.StopBits,0);
    }

    SendDlgItemMessage(hwndDlg,IDC_COMBO5,CB_SETCURSEL,ConnProps.StopBits,0);
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
      case IDOK:
        ConnProps.PortNo = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
        ConnProps.BaudRate = BaudRateValues[SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0)];
        ConnProps.ByteSize = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_GETCURSEL,0,0)+5;
        ConnProps.Parity = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_COMBO4,CB_GETCURSEL,0,0);
        ConnProps.StopBits = (unsigned char)SendDlgItemMessage(hwndDlg,IDC_COMBO5,CB_GETCURSEL,0,0);

      case IDCANCEL:
        EndDialog(hwndDlg,LOWORD(wParam));
        break;
      }
    break;
  default:
    return FALSE;
  }
return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK ConsoleDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
switch (uMsg)
  {
  case WM_INITDIALOG:
    {
    if (lParam != 0)
      {
      Console.Initialize(GetDlgItem(hwndDlg,IDC_RICHEDIT21));
      } else
      {
      Console2.Initialize(GetDlgItem(hwndDlg,IDC_RICHEDIT21));
      }
    }
    break;

  case WM_SIZE:
    MoveWindow(GetDlgItem(hwndDlg, IDC_RICHEDIT21), 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
    break;

  case WM_CLOSE:
    break;

  case WM_ERASEBKGND:
    break;

  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
      case IDCANCEL:
        EndDialog(hwndDlg,LOWORD(wParam));
        break;
      }
    break;
  default:
    return FALSE;
  }
return TRUE;
}
