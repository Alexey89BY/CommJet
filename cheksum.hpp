#ifndef  CHEKSUM_H
#define  CHEKSUM_H

/*======================================================================================================*/
// подсчет контрольной суммы методом исключтельного ИЛИ
#define XorSum_Step(sum, ch) ((sum)^(ch))

unsigned char XorSum
(
  unsigned char previousValue,  /* (вх) предыдущее значение контрольной суммы   */
  unsigned char *buffer,        /* (вх) указатель на буфер с данными            */
  unsigned int  length          /* (вх) количество байт для подсчета            */
);

/*======================================================================================================*/
// подсчет контрольной суммы методом суммирования без учёта переполнения
#define CheckSum8_Step(crc, ch) ((crc)+(ch))

unsigned char CheckSum8_Eval(unsigned char crc, unsigned char *Block, unsigned int Length);

#endif   /* CHEKSUM_H */
