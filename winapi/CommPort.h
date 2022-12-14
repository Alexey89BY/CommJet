#pragma once

#include <stdio.h>
#include <tchar.h>
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(0[x]))
#endif
//--------------------------------------------------------------------------------------------------
#define WM_COMMMSG    (WM_APP)
//--------------------------------------------------------------------------------------------------
class CCommPort
{
  HWND hOwner;
  HANDLE hPort;
  HANDLE hThread;
  OVERLAPPED WaitOverlap, IOOverlap;

  static unsigned long __stdcall PortThreadProc(void *lpParam);

  void ReleasePort(bool Close)
    {
    if (hPort == INVALID_HANDLE_VALUE)
      { return; }

    SetCommMask(hPort,0);

    if (Close)
      {
      CloseHandle(hPort);
      hPort = INVALID_HANDLE_VALUE;
      }
    }

public:
  enum EVENT
    {
    evnt_Receive = 0,
    evnt_Signal = 1,
    evnt_Error = 2,
    evnt_Critical = 3,
    };

  struct PROPERTIES
    {
    unsigned char PortNo;
    unsigned long BaudRate;
    unsigned char ByteSize;
    unsigned char Parity;
    unsigned char StopBits;
    };

  CCommPort(void);
  ~CCommPort(void);

  unsigned long Send(void *Data, unsigned long Length);
  unsigned long Receive(void *Data, unsigned long *Length);

  unsigned long Open(PROPERTIES *ConnProps, HWND hOwnerWnd);
  unsigned long Close(void);

  unsigned long SetDTR(bool Activate);
  unsigned long SetRTS(bool Activate);
};
//--------------------------------------------------------------------------------------------------
CCommPort::CCommPort(void)
{
hPort = INVALID_HANDLE_VALUE;
hThread = NULL;
hOwner = NULL;
WaitOverlap.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL); // ќжидани€ событий на порте
IOOverlap.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL); // ќжидани€ завершени€ передачи данных
}
//--------------------------------------------------------------------------------------------------
unsigned long CCommPort::Open(CCommPort::PROPERTIES *ConnProps, HWND hOwnerWnd)
{
if ((ConnProps == NULL)
 || (hOwnerWnd == NULL))
  { return ERROR_INVALID_PARAMETER; }

if (hPort != INVALID_HANDLE_VALUE)
  return ERROR_BUSY;

TCHAR Name[11];

_sntprintf(Name, ARRAYSIZE(Name), TEXT("\\\\.\\COM%u"), ConnProps->PortNo+1);
hPort=CreateFile(Name, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

if (hPort != INVALID_HANDLE_VALUE)
  {
  hOwner = hOwnerWnd;

  DCB dcb = {0};
  dcb.BaudRate = ConnProps->BaudRate;
  dcb.ByteSize = ConnProps->ByteSize;
  dcb.DCBlength = sizeof(DCB);
  dcb.fBinary = true;
  dcb.fAbortOnError = true;
  dcb.fParity = (ConnProps->Parity) != NOPARITY;
  dcb.Parity = ConnProps->Parity;
  dcb.StopBits = ConnProps->StopBits;

  COMMTIMEOUTS timeouts = {0};
  timeouts.ReadIntervalTimeout = MAXDWORD;

  if (SetCommState(hPort,&dcb)
   && SetCommTimeouts(hPort, &timeouts)
   && PurgeComm(hPort, PURGE_TXCLEAR|PURGE_RXCLEAR|PURGE_TXABORT|PURGE_RXABORT)
   && SetCommMask(hPort, EV_RXCHAR|EV_CTS|EV_DSR|EV_RING))
    {
    DWORD ThreadId;
    hThread = CreateThread(NULL,0,PortThreadProc,this,0,&ThreadId);

    if (hThread != NULL)
      { return ERROR_SUCCESS; }
    }
  }

DWORD ulError = GetLastError();

ReleasePort(true);

return ulError;
}
//--------------------------------------------------------------------------------------------------
unsigned long __stdcall CCommPort::PortThreadProc(void *lpParam)
{
DWORD EventMask = EV_CTS|EV_DSR|EV_RXCHAR;

CCommPort* port = (CCommPort *)lpParam;

for (;;)
  {
  do
    {
    DWORD ErrorMask;
    COMSTAT ComStat;

    if (! ClearCommError(port->hPort,&ErrorMask,&ComStat))
      { break; }

    if (ErrorMask & (CE_RXOVER|CE_OVERRUN|CE_RXPARITY|CE_FRAME|CE_BREAK|CE_TXFULL))
      {
      PostMessage(port->hOwner, WM_COMMMSG, CCommPort::evnt_Error, ErrorMask & (CE_RXOVER|CE_OVERRUN|CE_RXPARITY|CE_FRAME|CE_BREAK|CE_TXFULL));
      }

    if (EventMask & EV_RXCHAR)
      { PostMessage(port->hOwner, WM_COMMMSG, CCommPort::evnt_Receive, ComStat.cbInQue); }

    if (EventMask & (EV_CTS|EV_DSR|EV_RING))
      {
      DWORD StatusMask = 0;

      if (! GetCommModemStatus(port->hPort, &StatusMask))
        { break; }

      PostMessage(port->hOwner, WM_COMMMSG, CCommPort::evnt_Signal, StatusMask & (MS_CTS_ON|MS_DSR_ON|MS_RING_ON));
      }

    }
  while (WaitCommEvent(port->hPort,&EventMask,&(port->WaitOverlap)));

  DWORD Error = GetLastError();

  if (Error == ERROR_IO_PENDING)
    {
    switch (WaitForSingleObjectEx(port->WaitOverlap.hEvent,INFINITE, false))
      {
      case WAIT_OBJECT_0:
        continue;

      default:
        Error = GetLastError();
        break;
      }
    }

  if (Error != ERROR_INVALID_HANDLE)
    {
    PostMessage(port->hOwner, WM_COMMMSG, CCommPort::evnt_Critical, Error);
    port->ReleasePort(false);
    }

  return Error;
  }
}
//--------------------------------------------------------------------------------------------------
unsigned long CCommPort::SetDTR(bool Activate)
{
if (EscapeCommFunction(hPort, (Activate)? SETDTR: CLRDTR))
  { return ERROR_SUCCESS; }

return GetLastError();
}
//--------------------------------------------------------------------------------------------------
unsigned long CCommPort::SetRTS(bool Activate)
{
if (EscapeCommFunction(hPort, (Activate)? SETRTS: CLRRTS))
  { return ERROR_SUCCESS; }

return GetLastError();
}
//--------------------------------------------------------------------------------------------------
unsigned long CCommPort::Send(void *Data, unsigned long Length)
{
DWORD TransferCount;

if (WriteFile(hPort, Data, Length, &TransferCount, &IOOverlap))
  { return ERROR_SUCCESS; }

DWORD Error = GetLastError();

if (Error != ERROR_IO_PENDING)
  { return Error; }

if (GetOverlappedResult(hPort, &IOOverlap, &TransferCount,true))
  { return ERROR_SUCCESS; }

return GetLastError();
}
//--------------------------------------------------------------------------------------------------
unsigned long CCommPort::Receive(void *Data, unsigned long *Length)
{
if (Length == NULL)
  return ERROR_INVALID_PARAMETER;

if (ReadFile(hPort, Data, *Length, Length, &IOOverlap))
  return ERROR_SUCCESS;

DWORD Error = GetLastError();

CancelIo(hPort);

return Error;
}
//--------------------------------------------------------------------------------------------------
unsigned long CCommPort::Close()
{
if (hThread != NULL)
  {
  ReleasePort(true);

  WaitForSingleObjectEx(hThread, INFINITE, false);

  CloseHandle(hThread);
  hThread = NULL;
  }

return ERROR_SUCCESS;
}
//--------------------------------------------------------------------------------------------------
CCommPort::~CCommPort()
{
Close();

CloseHandle(IOOverlap.hEvent);
CloseHandle(WaitOverlap.hEvent);
}
//--------------------------------------------------------------------------------------------------
