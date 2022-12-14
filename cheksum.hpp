#ifndef  CHEKSUM_H
#define  CHEKSUM_H

/*======================================================================================================*/
// ������� ����������� ����� ������� �������������� ���
#define XorSum_Step(sum, ch) ((sum)^(ch))

unsigned char XorSum
(
  unsigned char previousValue,  /* (��) ���������� �������� ����������� �����   */
  unsigned char *buffer,        /* (��) ��������� �� ����� � �������            */
  unsigned int  length          /* (��) ���������� ���� ��� ��������            */
);

/*======================================================================================================*/
// ������� ����������� ����� ������� ������������ ��� ����� ������������
#define CheckSum8_Step(crc, ch) ((crc)+(ch))

unsigned char CheckSum8_Eval(unsigned char crc, unsigned char *Block, unsigned int Length);

#endif   /* CHEKSUM_H */
