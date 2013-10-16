//******************************************************
//* inKey.h  - ����� ���������� ������
//******************************************************
//* ����:    12.01.2006
//* ������:  6.00
//* �����:   �������� ���� ��������� (DrAF)
//******************************************************

#pragma once

#include "inKey.h"

CInKey::CInKey()
{
    this->UpdateProgress = NULL;

    Initialize();
}

CInKey::CInKey(void (*UpdateProgress)(int progress))
{
    // ������������� �� �������-���������� ���������� ���������
    this->UpdateProgress = UpdateProgress;

    Initialize();
}

void CInKey::Initialize()
{
	// �������������� ������ "������", ������� ����� �����������
    // � ��������� ��������� ��������������� �����
    rgbBitMask[0] = (char)0x01;
    rgbBitMask[1] = (char)0x02;
    rgbBitMask[2] = (char)0x04;
    rgbBitMask[3] = (char)0x08;
    rgbBitMask[4] = (char)0x10;
    rgbBitMask[5] = (char)0x20;
    rgbBitMask[6] = (char)0x40;
    rgbBitMask[7] = (char)0x80;

    // �������������� ��������� �������� MVS - 56 ����
    prgubMVS = new unsigned char[MVS_NUM * NBITS];
}

CInKey::~CInKey()
{
    // ���������������� ��������� �������� MVS
	if (prgubMVS)
	{
		delete [] prgubMVS;
	}
}

////////////////////////////////////////////////////////////////////////////////
// ���������� ��������� ��������, ��������������� ��� ���������
// ������� ���������� "��������". // ����������� ����� ���������
void CInKey::GenerateMVS(char rgbBank[NBITS],int bankLen,
						 char rgbMVVector[NBITS])
{
	int i, j;

	char rgbSavedBank[NBITS];

	// ���� �� ������������ ��� ������� ��� ������������� ��������,
	// �� ���������� ������������� �� �������������� ���������
	if (bankLen == 0)
	{
		  // ����� --> ������ �������� ������� � ���� ��������
		  for (j = 0; j < NBITS; j++)
		  {
			    prgubMVS[cMVS * NBITS + j] = rgbMVVector[j];
		  }
		  cMVS++;

		  return;
	}

	// ���� ���� ��������� ��������� ����������� �������,
	// �������� ���� ������ � ������, � ��������� ��� ���� ��
	// ��������. ���, ��������, ������ ����� ��������
	// ������� ������ "01234567".

	// ��������� �������� ��������� ����������� �����
	for (j = 0; j < bankLen; j++)
	{
		  rgbSavedBank[j] = rgbBank[j];
	}

	for (i = 0; i < bankLen; i++)
	{
	   // ������ �� ��������, ���������� � ��������� �����������
	   // ����������� ����� ���������� �� ������� �� 1 ������ �� ����
	   // ������
	   rgbMVVector[NBITS - bankLen] = rgbBank[i];
	   
	   // ������, ����� ���� �� �������� �����,
	   // ���� ������ ����������� �� ���� ������ �
	   // ����� ��������� � ����������� ���� �
	   // ���������� ��������.
	   // ��������� ������� �����, ������� "�����"
	   // ����������� � ����� ���������
	   for (j = i; j < (bankLen - 1); j++)
	   {
			  rgbBank[j] = rgbBank[j + 1];
	   }
	   
	   // ��������
	   GenerateMVS(rgbBank, (bankLen - 1), rgbMVVector);
	   
	   // ������ ��������������� �� ��������� �����, � �������
	   // �� ��� ������� � ����������� �������
	   for (j = 0; j < bankLen; j++)
	   {
			  rgbBank[j] = rgbSavedBank[j];
	   }
	}
}

////////////////////////////////////////////////////////////////////////////////
// ���������� ��������� ��������, ��������������� ��� ���������
// ������� ���������� "��������".
// BankLen - ���������� �������� � ������� Bank ��� �������������
// ������� ������� � �������. ������ ���, ����� ��������� ������
// ��������� ��������� ����������, �� ��������. ����� �������,
// ������������ ��������� ��� �� ����� ��������� ����
// ������ �� � ����� �� �����������.
void CInKey::StartToGenerateMVS()
{	
	// ! ������������� ����� ��������������� ���������
	char rgbBank[NBITS] = {(char)0x00,(char)0x01,(char)0x02,(char)0x03,
						   (char)0x04,(char)0x05,(char)0x06,(char)0x07};	
	// ! ������������� ����� ��������������� ���������

	char rgbMVVector[NBITS];

	// ����� ���������� ����� ��������
	int bankLen = NBITS;

	// ������� ���������� ���������� �������� ��� �����������
	// "��������"
	cMVS = 0;

	// ����������� ������� ������������� ��������� ��������
	// ��� ��������� ������� ���������� �������� - 
	// ��������� ������ ������ ���������� 0..7	
	GenerateMVS(rgbBank, bankLen, rgbMVVector);	
}

////////////////////////////////////////////////////////////////////////////////
// ����������� ������ ����������� ������� char �� ����������� 
// ������������
void CInKey::WipeData(char *prgbData, long int lDataSize)
{
	long int i;

	for (i = 0; i < lDataSize; i++)
	{
		prgbData[i] = (char)0x00;
	}
}

////////////////////////////////////////////////////////////////////////////////
// ���������� ��������� ���������� ���� �� ����������� �����
inline char CInKey::Getbit(char byte, int nbit)
{
	byte &= rgbBitMask[nbit];

	if (byte != 0)
	{
		  return 1;
	} else
	{
		  return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// ������������� ��������� ���������� ���� � ���������� ����,
// ������� ������������ ������. ���������� ���������������� ����
// !!! �������� !!! ����� ���������� �� ������ ���� ���������� � 0!
inline char CInKey::Putbit(char byte, int nbit, char bit)
{
	if (bit != 0)
	{
		byte |= rgbBitMask[nbit];
	}

	return byte;
}

////////////////////////////////////////////////////////////////////////////////
// ��������������� �������� ������ � "������" 
void CInKey::ConvertSourceToBitData(char *prgbSourceBuffer,
									long int lSourceSize, char *prgbBitData)
{
   long int i, j;

   int nbit;

   // ������������ ������ �� �������� �������� �����...
   for (i = 0, j = 0; i < lSourceSize; i++)
   {
		 // � ����� 8 ���, ��������� ������...
	   for (nbit = 0; nbit < NBITS; nbit++,j++)
	   {
		    prgbBitData[j] = Getbit(prgbSourceBuffer[i], nbit);		  
	   }	   
   }   
}

////////////////////////////////////////////////////////////////////////////////
// ��������������� ������ "������" � ���� �������� ������
void CInKey::ConvertBitDataToSource(char *prgbSourceBuffer,
								    long int lSourceSize, char *prgbBitData)
{
   long int i, j = 0;

   int nbit;

   // ��������� �������
   if (bitDataPosition != 0)
   {
		  j = NBITS * lSourceSize;
   }
   
   // ��� �������� ����� ������ ���� ���������� �� �����...
   for (i = 0; i < lSourceSize; i++)
   {
		// � ����� 8 ���, ���������� �� ����� ������...
	   for (nbit = 0; nbit < NBITS; nbit++,j++)
	   {		  		  
		    prgbSourceBuffer[i] = Putbit(prgbSourceBuffer[i], nbit, prgbBitData[j]);
	   }	   
   }	   
}
//////////////////////////////////////////////////////////////////////
// ������� prgBitData (���������� ������ �������������)
void CInKey::RotateBitData (long int lSourceSize,char *prgbPasswordBuffer,
							char *prgbBitData, long int L, int mode)
{
	long int i, j;
	int sPartIndex;

	// ������, � �������� ���������� "������ �����" ������
	sPartIndex = (unsigned char)prgbPasswordBuffer[L] + 1;

	// ����������� �������� �������� ������� �� ������ ������
	// ��������� - ���������� ��� �����������
	switch (mode)
	{
		case ENCRYPT:
		{		
				// ��������� "������ �����" ������ �� ������ �����
				for (i = sPartIndex, j = 0; i < (NBITS * lSourceSize); i++, j++)
				{
					prgbBitData[(bitDataPosition ^ 0x01) * NBITS * lSourceSize + j] = prgbBitData[bitDataPosition * NBITS * lSourceSize + i];
				}

				// ��������� "������ �����" ������ �� ������ �����				
				for (i = (sPartIndex - 1); j < (NBITS * lSourceSize); i--, j++)
				{
					prgbBitData[(bitDataPosition ^ 0x01) * NBITS * lSourceSize + j] = prgbBitData[bitDataPosition * NBITS * lSourceSize + i];
				}

				// ������������ "���������" �� ������
				bitDataPosition ^= 0x01;

				break;
		}

		case DECRYPT:
		{		
				// ��������� "������ �����" ������ �� ������ �����
				for (i = (NBITS * lSourceSize - sPartIndex), j = (sPartIndex - 1); j >= 0; i++, j--)
				{
					prgbBitData[(bitDataPosition ^ 0x01) * NBITS * lSourceSize + j] = prgbBitData[bitDataPosition * NBITS * lSourceSize + i];
				}

				// ��������� "������ �����" ������ �� ������ �����				
				for (i = 0, j = sPartIndex; j < (NBITS * lSourceSize); i++, j++)
				{
					prgbBitData[(bitDataPosition ^ 0x01) * NBITS * lSourceSize + j] = prgbBitData[bitDataPosition * NBITS * lSourceSize + i];
				}

				// ������������ "���������" �� ������
				bitDataPosition ^= 0x01;				
		}				
	}		
}

//////////////////////////////////////////////////////////////////////
// ������������� ������ iMVS � ��������� ��� (������������� �����)
void CInKey::MixBitData(long int lSourceSize, char *prgbPasswordBuffer,
						char *prgbBitData, long int L, long int R, int mode)
{
	int iMVS; // ������ � ������� ���������� �� char-�������
			  // ������������

	int nbit, nbit2 = 0;

	long int i, lIndexTarget, lIndexSource;

	// ���� union ��������� ��� ������������ ������� � MVS
	// ������ ������������ �� ���� ������ ���������� �����
	// ��� ��������� �������� ���������� �������� "����� ����������"
	// ���� ���� �� ������ ���������� �����
	static TCharToInt charToInt;

	// �������������� "��������" ���� ������ � union-�
	// �������, ������� �� ���������� ����� ������	
	charToInt.rgbVariable[0] = prgbPasswordBuffer[L];
	charToInt.rgbVariable[1] = prgbPasswordBuffer[R];

	// ������������ �������������� Char[2] --> Int
	iMVS = charToInt.intVariable;

	// ����������� ������ ������ �������� �������
	if (iMVS > (MVS_NUM - 1))
	{
		iMVS -= MVS_NUM;
	}

	// ��������� ������ ������������ ������:
	// ���������� ���    N-�   � � � �    ��������� �����
	// ��������� �  N-��  "� � � � � � �"  �����-��������
	for (nbit = 0; nbit < NBITS; nbit++)
	{
		// N-�� ����� �������, ������� ������ � ����� �������� ������
		for (i = 0; i < lSourceSize; i++)
		{
			// ��� �������� ������ ������ ����� � ������-�������
			// ���������� ��������� - ������ �� ������� - nbit*lSourceSize+i
			// ����������������� ��������� - ������ �� �������� - nbit*i+lSourceSize			
			
			// ����� ��������� �������� ������� �� ������ ������ ��������� -
			// ���������� ��� �����������
			switch (mode)
			{
				case ENCRYPT:
				{		
					// ����� �� ������� - ����� ������	
					lIndexSource = (long int)((prgubMVS[iMVS * NBITS + nbit]) * lSourceSize + i);	
					lIndexTarget = nbit * lSourceSize + i;

					break;
				}

				case DECRYPT:
				{
					// ����� ������ - ����� �� �������	
					lIndexSource = nbit * lSourceSize + i;
					lIndexTarget = (long int)prgubMVS[iMVS * NBITS + nbit] * lSourceSize + i;
				}				
			}		
				
			// ��������� ��������� �������� �� ������ ����� �������
			// "�������" ������
			// ���� ������ �� ������ ����� �������, ��
			// bitDataPosition != 0, �������������, ����������
			// �������� "�������" � "���������" �������...
			if (bitDataPosition != 0)
			{
				lIndexSource += (NBITS * lSourceSize);

			} else
			// ... � ���� bitDataPosition==0, �� ��� 
			// ���������� ������ � ����� ������� ����
			// ����� ������ "�������"
			{
				lIndexTarget += (NBITS * lSourceSize);
			}
				
			// ���������� ������
			prgbBitData[lIndexTarget] = (prgbBitData[lIndexSource] ^ Getbit(prgbPasswordBuffer[i], nbit2));

			nbit2++;

			if (nbit2 >= NBITS)
			{
				nbit2 = 0;
			}
		}
	}
	// "������������" ��������� �� ���� �� ���� ���������� �����������
	// � ������� prgbBitData
	bitDataPosition ^= (char)0x01;
}

//////////////////////////////////////////////////////////////////////
// ���������� ��������� ����� ������ ��������� ������ ������
void CInKey::Encrypt(char *prgbSourceBuffer, long int lSourceSize,
					 char *prgbPasswordBuffer)
{	
	long int i;

	// ������� � ����� ��������� ������
	long int L, R;

	char *prgbBitData = new char[2 * NBITS * lSourceSize];


	int mode = ENCRYPT;

    // ���������� MVS - Move VectorS
	StartToGenerateMVS();

    // ���������� ������ ����� � ������ ������� (����� ���������������)
    bitDataPosition = 0;

	// ������ R � ��������� ������ ������ �� ���� �����...
	R = (lSourceSize - 1);
	// � L - �� ����.
	L = 0;

	// ��������������� �������� ������ � "�������" �����������
	ConvertSourceToBitData(prgbSourceBuffer, lSourceSize, prgbBitData);

	// ������� ����� �������� ������, ����� ���������� ����������
	// ��������� ������� ������ �� prgbBitData
	WipeData(prgbSourceBuffer, lSourceSize);

	////////////////////////////////////////
	// ��������� ������: ����������
	////////////////////////////////////////
	// �������� �� ��� ���, ���� ��� ��������� ������ �� ������������
	for (i = 0; i < lSourceSize; i++)
	{	    
		// ��������� ������� ������������ (����������� ����� ��������
		// � ��������� ������� ���������� "�����")
		MixBitData(lSourceSize, prgbPasswordBuffer,
					prgbBitData, L, R, mode);

		// ������� ������� ������ ������ ����������
		RotateBitData(lSourceSize, prgbPasswordBuffer,
	     				  prgbBitData, L, mode);

		L++;
				
		R--;

		// ��������� ��������
		if (UpdateProgress != NULL)
		{
			UpdateProgress(i);
		}
	}
	
	// ��������������� ������ �� "�������" ����� � ��������
	ConvertBitDataToSource(prgbSourceBuffer, lSourceSize, prgbBitData);	

	// ���������� ������...
	WipeData(prgbBitData, 2 * NBITS * lSourceSize);

	if (prgbBitData)
	{
		delete [] prgbBitData;
		prgbBitData = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// ���������� ��������� ����� ������ ��������� ������ ������
void CInKey::Decrypt(char *prgbSourceBuffer, long int lSourceSize,
					 char *prgbPasswordBuffer)
{	
	long int i;

	// ������� � ����� ��������� ������
	long int L, R;

	char *prgbBitData = new char[2 * NBITS * lSourceSize];

	int mode = DECRYPT;

    // ���������� MVS - Move VectorS
	StartToGenerateMVS();

    // ���������� ������ ����� � ������ ������� (����� ���������������)
    bitDataPosition = 0;

	// ������ R � ��������� ������ ������ �� ���� �����...
	L = (lSourceSize - 1);
	// � L - �� ����.
	R = 0;
			
	// ��������������� �������� ������ � "�������" �����������
	ConvertSourceToBitData(prgbSourceBuffer, lSourceSize, prgbBitData);

	// ������� ����� �������� ������, ����� ���������� ����������
	// ��������� ������� ������ �� prgbBitData
	WipeData(prgbSourceBuffer, lSourceSize);

	////////////////////////////////
	// ��������� ������: �����������
	////////////////////////////////
	// �������� �� ��� ���, ���� ��� ��������� ������ �� ������������
	for (i = 0; i < lSourceSize; i++)
	{		
		// ������� ������� ������ ������ ����������
		RotateBitData(lSourceSize, prgbPasswordBuffer,
	     				  prgbBitData, L, mode);

		// ��������� ������� ������������ (����������� ����� ��������
		// � ��������� ������� ���������� "�����")
		MixBitData(lSourceSize, prgbPasswordBuffer,
					prgbBitData, L, R, mode);

		L--; 
		
		R++;

		// ��������� ��������
		if (UpdateProgress != NULL)
		{
			UpdateProgress(i);
		}
	}

	// ��������������� ������ �� "�������" ����� � ��������
	ConvertBitDataToSource(prgbSourceBuffer, lSourceSize, prgbBitData);

	// ���������� ������...
	WipeData(prgbBitData, 2 * NBITS * lSourceSize);

	if (prgbBitData)
	{
		delete [] prgbBitData;
		prgbBitData = NULL;
	}
}