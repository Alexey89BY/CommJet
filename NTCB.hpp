#pragma once

class CNTCBCommand
 {
 public:
#pragma pack(push)
#pragma pack(1)
  struct HEAD
  {
  unsigned long Preamble;
  unsigned long ReceiverID;
  unsigned long SenderID;
  unsigned short DataLength;
  unsigned char DataCRC;
  unsigned char HeadCRC;
  };
#pragma pack(pop)

  enum PackageStage
 {
 ps_Preamble = 0,
 ps_Head,
 ps_Data
 };

  const static unsigned int DataMaxLength = 65536;
 const static unsigned int PreambleLength = 4;

 private:
  unsigned int DataCount;
  unsigned char CheckSum;
  PackageStage Stage;
  unsigned int Index;
  unsigned char *ReadWritePtr;
  unsigned char Buffer[DataMaxLength+sizeof(HEAD)];

public:

   //void BeginParse(void);
   //PARSER::RESULT ParseChar(unsigned char Char);

   unsigned char *GetCurrentPtr()
    {
    return ReadWritePtr;
    }

   CNTCBCommand::HEAD *GetHead()
    {
    return ((HEAD *)Buffer);
    }

   void BeginRead()
     {
     ReadWritePtr = &Buffer[sizeof(HEAD)];
     }

   bool Read(void *Buffer, unsigned int Length);

   void BeginWrite()
     {
     ReadWritePtr = &Buffer[sizeof(HEAD)];
     CheckSum = 0;
     }

   bool Write(void *Buffer, unsigned int Length);

   void Complete(unsigned long SenderID, unsigned long ReceiverID);

   void *Data();
   unsigned int Length();

 };
