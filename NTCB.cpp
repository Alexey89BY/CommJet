#include "NTCB.hpp"
#include <stdlib.h>
#include <string.h>

const unsigned char NTCBPreamble[CNTCBCommand::PreambleLength+1] = "@NTC";

bool CNTCBCommand::Read(void *Buffer, unsigned int Length)
{
unsigned char *Ptr = (unsigned char *)Buffer;

if (Ptr)
 {
 for (unsigned int i = 0; i<Length; i++)
   *(Ptr++) = *(ReadWritePtr++);
 } else
 {
 ReadWritePtr += Length;
 }
return true;
}

void CNTCBCommand::Complete(unsigned long SenderID, unsigned long ReceiverID)
{
memcpy(Buffer, NTCBPreamble, CNTCBCommand::PreambleLength);
((HEAD *)Buffer)->SenderID = SenderID;
((HEAD *)Buffer)->ReceiverID = ReceiverID;
((HEAD *)Buffer)->DataLength = ((unsigned char *)ReadWritePtr)-((unsigned char *)&Buffer)-sizeof(HEAD); 
((HEAD *)Buffer)->DataCRC = CheckSum; 

unsigned char Result = 0;
for (unsigned int i = 0; i<(sizeof(HEAD)-1);i++)
  Result ^= ((unsigned char *)Buffer)[i];

((HEAD *)Buffer)->HeadCRC = Result;
}

bool CNTCBCommand::Write(void *Buffer, unsigned int Length)
{
unsigned char *Ptr = (unsigned char *)Buffer;

if (Ptr)
 {
 for (unsigned int i = 0; i<Length; i++)
   {
   unsigned char ch = *(Ptr++);
   *(ReadWritePtr++) = ch;
   CheckSum ^= ch;
   }
 } else
 {
 for (unsigned int i = 0; i<Length; i++)
   CheckSum ^= *(ReadWritePtr++);
 }
return true;
}

void *CNTCBCommand::Data()
{
return Buffer;
}
   
unsigned int CNTCBCommand::Length()
{
return ((HEAD *)Buffer)->DataLength+sizeof(HEAD);
}

