#include "cheksum.h"

/*
===================================================================================================
Description: подсчет контрольной суммы методом исключтельного ИЛИ
Returns    : 
Notes      : 
===================================================================================================
*/
unsigned char XorSum
(
  unsigned char previousValue,  /* (вх) предыдущее значение контрольной суммы   */
  unsigned char *buffer,        /* (вх) указатель на буфер с данными            */
  unsigned int  length          /* (вх) количество байт для подсчета            */
)
{
  while (length-- > 0)
    previousValue = XorSum_Step(previousValue, *buffer++);

  return (previousValue);
}

/*
===================================================================================================
Description: подсчет контрольной суммы методом суммирования без учёта переполнения
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
