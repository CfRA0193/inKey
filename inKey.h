//******************************************************
//* inKey.h  - ����� ���������� ������
//******************************************************
//* ����:    12.01.2006 / 22.02.2014
//* ������:  6.00
//* �����:   �������� ���� ��������� (DrAF)
//******************************************************

#pragma once

#define MVS_NUM 40320  // ���������� ������������ � ������� �� 8-�
                       // ��������

#define NBITS   8      // MVS_NUM = NBITS! (���������)

#define ENCRYPT 1      // �����: ���������
#define DECRYPT 2      // �����: ������������

//////////////////////////////////////////////////////////////////////
// ���� union ��������� ��� ������������ ������� � MVS
// ������ ������������ �� ���� ������ ���������� �����
// ��� ��������� �������� ���������� �������� "����� ����������"
// ���� ���� �� ������ ���������� �����
typedef union
{
    int  intVariable;
    char rgbVariable[2];

} TCharToInt;

///////////////////////////////
// ����� ���������� inKey 6.00
///////////////////////////////
class CInKey  
{
public:
    CInKey();
    CInKey(void (*UpdateProgress)(int progress));
    virtual ~CInKey();
    void Encrypt(char *prgbSourceBuffer, long int lSourceSize,
                 char *prgbPasswordBuffer);
    void Decrypt(char *prgbSourceBuffer, long int lSourceSize,
                 char *prgbPasswordBuffer);
private:
    unsigned char *prgubMVS;
    char rgbBitMask[NBITS];
    int  cMVS;
    int  bitDataPosition;
    void Initialize();
    void GenerateMVS(char rgbBank[NBITS], int bankLen,
                     char rgbMVVector[NBITS]);
    void StartToGenerateMVS();
    void WipeData(char *prgbData, long int lDataSize);
    inline char Getbit(char byte, int nbit);
    inline char Putbit(char byte, int nbit, char bit);
    void ConvertSourceToBitData(char *prgbSourceBuffer,
                                long int lSourceSize, char *prgbBitData);
    void ConvertBitDataToSource(char *prgbSourceBuffer,
                                long int lSourceSize, char *prgbBitData);
    void RotateBitData(long int lSourceSize, char *prgbPasswordBuffer,
                       char *prgbBitData, long int L, int mode);
    void MixBitData(long int lSourceSize, char *prgbPasswordBuffer,
                    char *prgbBitData, long int L, long int R, int mode);
    void (*UpdateProgress)(int progress);
};