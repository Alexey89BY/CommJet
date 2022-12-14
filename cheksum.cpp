#include "cheksum.h"

/*
===================================================================================================
Description: ������� ����������� ����� ������� �������������� ���
Returns    : 
Notes      : 
===================================================================================================
*/
unsigned char XorSum
(
  unsigned char previousValue,  /* (��) ���������� �������� ����������� �����   */
  unsigned char *buffer,        /* (��) ��������� �� ����� � �������            */
  unsigned int  length          /* (��) ���������� ���� ��� ��������            */
)
{
  while (length-- > 0)
    previousValue = XorSum_Step(previousValue, *buffer++);

  return (previousValue);
}

/*
===================================================================================================
Description: ������� ����������� ����� ������� ������������ ��� ����� ������������
Returns    :
Notes      :
===================================================================================================
*/
unsigned char CheckSum8_Eval(unsigned char Crc, unsigned char *Block, unsigned int Length)
{
while (Length--)
  Crc = CheckSum8_Step(Crc, *Block++);

return (Crc);
}
