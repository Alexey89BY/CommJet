#ifndef CRC_H_
#define CRC_H_

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC8 Polynomial = 0x31
extern const unsigned char crc8_poly31_table[];
//--------------------------------------------------------------------------------------------------
#define egts_crc8_init()         (0xFF)
#define egts_crc8_step(crc, x)   (crc8_poly31_table[(crc) ^ (x)])
#define egts_crc8(lp_block, len) crc8_poly31_eval(0xFF, lp_block, len)
//--------------------------------------------------------------------------------------------------
#define crc8_poly31_step(crc, x) (crc8_poly31_table[(crc) ^ (x)])
//--------------------------------------------------------------------------------------------------
unsigned char crc8_poly31_eval(unsigned char crc, unsigned char *lp_block, unsigned int len);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC8 Polynomial = 0xD5
extern const unsigned char crc8_polyD5_table[];
//--------------------------------------------------------------------------------------------------
#define crc8_polyD5_step(crc, x) (crc8_polyD5_table[(crc) ^ (x)])
//--------------------------------------------------------------------------------------------------
unsigned char crc8_polyD5_eval(unsigned char crc, unsigned char *lp_block, unsigned int len);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC8 Polynomial = 0x8C
unsigned char eval_revcrc8(unsigned char crc, const unsigned char *msg, unsigned int msg_len);
unsigned char eval_crc8(unsigned char crc, const unsigned char *msg, unsigned int msg_len);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC8 Dallas (polynomial = 0x31)
unsigned char eval_dallas_crc8(unsigned char init_crc, const unsigned char *buf, unsigned size);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC8 Polynomial = 0x91 not at end
extern const unsigned char crc8_poly91_table[];
//--------------------------------------------------------------------------------------------------
#define crc8_poly91_step(crc, x) (crc8_poly91_table[crc ^ x])
//--------------------------------------------------------------------------------------------------
unsigned char crc8_poly91_eval(unsigned char crc, unsigned char *lp_block, unsigned int len);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC16 Polynomial = 0x1021
extern const unsigned short crc16_poly1021_table[];
//--------------------------------------------------------------------------------------------------
#define eval_crc16tbl(crc, msg, msg_len) eval_crc16(crc, msg, msg_len)
//--------------------------------------------------------------------------------------------------
unsigned short crc16_poly1021_step(unsigned short crc, unsigned char ch);
//--------------------------------------------------------------------------------------------------
unsigned short eval_crc16(unsigned short crc, const unsigned char *msg, unsigned int msg_len);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC16 Polynomial = 0xA001
unsigned short eval_modbuscrc(unsigned short init_crc, const unsigned char *msg, unsigned int msg_len);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC16 Polynomial = 0x8005
unsigned short eval_crc16_modbus(unsigned char *p, unsigned short len);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC16 Dallas
unsigned short eval_dallas_crc16(unsigned char indata, unsigned short seed);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC32 Polynomial = 0x77073096
unsigned long eval_crc32tbl(unsigned long init_crc, const unsigned char *buf, unsigned int size, unsigned long xor_out);

////////////////////////////////////////////////////////////////////////////////////////////////////
// CRC24 Polynomial = 0x01864CFB
unsigned long crc24_poly1864CFB_eval(unsigned long crc, unsigned char *data, unsigned int len);

#endif
