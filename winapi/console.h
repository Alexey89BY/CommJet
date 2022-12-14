#pragma once
#include <windows.h>

class CConsole
{
public:
  CConsole()
    {
    Output = NULL;
    }

  ~CConsole();

  void Initialize(HWND hOutput);
  void Clear(void);
  void Log(unsigned long Color, const TCHAR *Message);
  void Bin(unsigned long Length, const void *Message);
  bool Error(TCHAR* Stage, unsigned long Code, bool Success = true);

private:
  HWND Output;
};
