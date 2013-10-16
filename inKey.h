//******************************************************
//* inKey.h  - Класс шифрования данных
//******************************************************
//* Дата:    12.01.2006
//* Версия:  6.00
//* Автор:   Дробанов Артём Федорович (DrAF)
//******************************************************

#pragma once

#define MVS_NUM 40320  // Количество перестановок в векторе из 8-и
					   // символов
#define NBITS 8        // MVS_NUM = NBITS! (факториал)

#define ENCRYPT 1      // Режим: ШИФРОВАТЬ
#define DECRYPT 2      // Режим: РАСШИФРОВАТЬ

////////////////////////////////////////////////////////////////////////////////
// Этот union необходим для формирования индекса в MVS
// Индекс складывается из двух байтов парольного блока
// При следующей итерации происходит смещение "рамки считывания"
// двух байт по буферу парольного файла
typedef union
{
	int intVariable;
	char rgbVariable[2];
} TCharToInt;

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
	int cMVS; 	
	int bitDataPosition;
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
                       char *prgbBitData, long int L,int mode);
	void MixBitData(long int lSourceSize, char *prgbPasswordBuffer,
                    char *prgbBitData, long int L, long int R, int mode);
	void (*UpdateProgress)(int progress);
};