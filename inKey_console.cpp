//////////////////////////////////////////////////////////////////////
//////////////////////////// inKey 6.00.P ////////////////////////////
//////////////////////////////////////////////////////////////////////
// �����: �������� ���� (DrAF) // draf@mail.ru                     //
//////////////////////////////////////////////////////////////////////
// ����������� ���������: ������� �������� (ProFit) // 0176@mail.ru //
//////////////////////////////////////////////////////////////////////
// ��������: 2 ����� 2003 �, 02 ������ 2013 �.                      //
//////////////////////////  �. ��������� /////////////////////////////

#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

#include <iostream>
using namespace std;

//////////////////////////////////////////////////////////////////////
// ���������� �� "ASC 1.3.0.3"
#include "inKey.cpp"  // 212CFBF8 (CRC32, WinRAR)
//         "inKey.h"  // 116A0EA1 (CRC32, WinRAR)
//////////////////////////////////////////////////////////////////////

#define TO_READ  0  // �������: ��� ������
#define TO_WRITE 1  // �������: ��� ������
#define TARGET_EXT_LENGTH   3          // ����� ���������� �����
#define MAX_NAME_LENGTH     215        // �����������-���������� ����� ����� �����
#define MIN_ENC_BLOCK_SIZE  100 * 1024 // ����������� ������ ����� ��� ����������
#define INT32_SIZE          4          // ������ int32
#define ARGC_NUM            5          // ��������� ���������� ����������

//////////////////////////////////////////////////////////////////////

typedef __int32           int32;
typedef __int64           int64;
typedef unsigned __int32  uint32;
typedef unsigned __int64  uint64;

//////////////////////////////////////////////////////////////////////
// ���� union ��������� ��� �������������� Int32 � �������� �������������
typedef union
{
	int32 intVariable;
	char rgbVariable[4];
} TCharToInt32;

//////////////////////////////////////////////////////////////////////

class Random // by Eugene Roshal (public domain)
{
  private:
    uint32 x,y,z,w;
  public:
    Random(uint32 Seed) { x = Seed; y = 362436069; z = 521288629; w = 88675123; }
    uint32 Get32()    { uint32 t = x ^ (x << 11); x = y; y = z; z = w; return w = w ^ (w >> 19) ^ (t ^ (t >> 8)); }
    uint64 Get64()    { return (uint64(Get32())<<32) | Get32(); }
};

//////////////////////////////////////////////////////////////////////
// ��������� ��������� �� ������ �������� �����
void file_error_message(char *szFilename, int errKind)
{
	cout << "ERROR: Can't open " << szFilename;

	// ���������� ������������, � ����� ������ �������� �����
	// ��������� ���� (��� ������ ��� ��� ������)
	switch (errKind)
	{
		case TO_READ:
		{
			cout << " to read!";
			break;
		}

		case TO_WRITE:
		{
			cout << " to write!";
			break;
		}
	}

	cout << endl;
}

//////////////////////////////////////////////////////////////////////
// ���������� ����� �����
long int file_length(char *szFileName)
{
	int fHandle;
	long int lFileSize;
	
	fHandle   = open(szFileName, O_RDONLY);
	lFileSize = filelength(fHandle);
	close(fHandle);

	return lFileSize;
}

//////////////////////////////////////////////////////////////////////
// ������������ �������������:
// 1 - ��������� ���������
// 2 - ����������, �������� ����� �����
// � �����: ����� �� ���� ������� ����������� �������� ����� ��� ������
FILE* open_file_to_read(char *szFilename, long int &lFileSize)
{
	// �������� ���������
	FILE *fHandle;

	// ��������� ���� �� ����������� ����������� ��������...
	if ((fHandle = fopen(szFilename, "rb")) == NULL)
	{
		// ���� ������� ������ - ������� ��������� �� ������...
	  	file_error_message(szFilename, TO_READ);        
		// ... � ������� �� �������
		return NULL;
	}
	
	// ... ���� ���� �������� - �� OK, �� ��� ���������� ����������
	// ����������� ��� �������, ��� ����� ����� ����������� ��� 
	// � ������ ������, ������� ��� �������� ���������...
	fclose(fHandle);

	// ... � �������� ������ �����.
	lFileSize = file_length(szFilename);

	// � ��� � �������� ����� ��� ������ � ���...
	fHandle = fopen(szFilename, "rb");

	//...���������� �������� ���������...
	return fHandle;
}

///////////////////////////////////////////////////////////////////////
// ������������ ����� ��������� ���������
void update_progress(int progress)
{
    if ((progress % 64) == 0)
    {
        cout << "\r" << "position : " << progress;
    }
}

///////////////////////////////////////////////////////////////////////
// ������������ �������� ������ ��� ����������/�����������
int process_file(char* szSourceFile, char* szPasswordFile, int mode, int password_selector)
{
	FILE *fSource, *fPassword, *fTarget;
	TCharToInt32 charToInt;	
	LARGE_INTEGER seed1, seed2, seed3, seed4;
	Random pass_rnd(password_selector); // by Eugene Roshal (public domain)
	int seed;
	long int i = 0, j, dataLen, toRead, toWrite, readed, written;

	// ������ ��������� �����, �����-������, ������ ����������,
	// �������� ������ �������� ������ � ����������...
	long int lSourceSize, lPasswordSize, lContainerSize, lRealSourceSize;
	char rgTargetExt[] = ".iK";
	char *szTargetFile = new char[MAX_NAME_LENGTH];
	char *prgbSourceBuffer, *prgbPasswordProBuffer, *prgbPasswordBuffer, *prgbBitData;

	///////////////////////////////////////////
	// ���������� �� "ASC 1.3.0.3"
	// "inKey.h"   // 116A0EA1 (CRC32, WinRAR)
	// "inKey.cpp" // 212CFBF8 (CRC32, WinRAR)
	CInKey inKey(update_progress);
	QueryPerformanceCounter(&seed1);
	///////////////////////////////////////////

	// � ��������������� ����� ���� "switch" ����������
	// ������������������ ��������� ����� ��������� �����
	// �� ����� �������� ����� � ����������� �� ������ ������
	// ���������
	switch (mode)
	{
	    // ���� ������� ����������...
	    case ENCRYPT:
		{
			cout << "Encryption mode selected..." << endl << endl;

			// �������� ��� ���������� ����� � ��� �������� �����...
			while (szSourceFile[i] != '\0')
			{
				szTargetFile[i] = szSourceFile[i];
				i++;
			}
		
			// "�����������" � ����� ��������� ����� ���������� ".iK"...
			for (j = 0; j < TARGET_EXT_LENGTH; j++)
			{
				szTargetFile[i + j] = rgTargetExt[j];
			}

			// ����� ��� � ������ ����� ������...
			szTargetFile[i + j] = '\0';

			break;
		}

	    // ���� ������� �����������...
	    case DECRYPT:
		{	
			int LastPointPosition = 0; // ������� ��������� �����
                                       // � ����� �����...

			cout << "Decryption mode selected..." << endl << endl;

			// �������� ��� ��������� ����� � ��� �������� �����,
			// ��������� �������, �� ������� ����������� ���������
			// ����� � �����...
			while (szSourceFile[i] != '\0')
			{
				szTargetFile[i] = szSourceFile[i];
				
				if (szSourceFile[i] == '.')
				{
					LastPointPosition = i;
				}

				i++;
			}

			// �������� ��� �������� ���������� ����� ���������
			// ������� ����� ������ ������ ��������� �����...
			szTargetFile[LastPointPosition] = '\0';
		}
	}	

	// ��������� �������� ���� � � ������ �������
	// ��������� ������ ���������...
	if ((fSource = open_file_to_read(szSourceFile, lSourceSize)) == NULL)
	{
		cout << endl;
		cout << "       Press any key to exit...";
		getch();

		delete [] szTargetFile;

		return 1;
	}

	// ��������� ��������� ���� � �������������� ���������...
	if ((fPassword = open_file_to_read(szPasswordFile, lPasswordSize)) == NULL)
	{
		cout << endl;
		cout << "       Press any key to exit...";
		getch();

		delete [] szTargetFile;
		fclose(fSource);

		return 1;
	}

	QueryPerformanceCounter(&seed2);

	// ���������� ������ ����������, � ������� ����� ��������� ������...
	lContainerSize = (lSourceSize < MIN_ENC_BLOCK_SIZE) ? MIN_ENC_BLOCK_SIZE : lSourceSize;
	switch (mode)
	{
	    // ���� ������� ����������...
	    case ENCRYPT:
		{
			lContainerSize += INT32_SIZE; // ��������� ��������� ����...
			break;
		}
	    
	    // ���� ������� �����������...
	    case DECRYPT:
		{
			// ��������� ���� ��� ������.
		}
	}

	// ��������� ���� ������ ���� �� ������ ���������!
	if (lPasswordSize < lContainerSize)
	{
		cout << "ERROR: The file-password is too small!" << endl;
		cout << "       It's size must be >= " << lContainerSize << " bytes!" << endl;
		cout << endl;
		cout << "       Press any key to exit...";
		getch();

		delete [] szTargetFile;
		fclose(fSource);
		fclose(fPassword);

		return 1;
	}

	// ���� �� ����� ������� �������� ����...
	if ((fTarget = fopen(szTargetFile, "wb")) == NULL)
	{        
		cout << "ERROR: Can't open output file (*.iK) to write!" << endl;
		cout << endl;
		cout << "       Press any key to exit...";
		getch();

		file_error_message(szTargetFile, TO_WRITE);
		delete [] szTargetFile;
		fclose(fSource);
		fclose(fPassword);

		return 1;
	}

	// �������� ������ ��� ���������� ������...
	prgbSourceBuffer      = new char [lContainerSize];
	prgbPasswordBuffer    = new char [lContainerSize];
	prgbPasswordProBuffer = new char [lPasswordSize];

	// �������� ������ ��� "�������� ������"...
	prgbBitData = new char[2 * NBITS * lContainerSize];

	// ��������� ������ �� ���������� ����� (�������, ������� ����� ��� ��������� ����������)...
	toRead = dataLen = lPasswordSize;
	readed = 0;
	while((toRead = dataLen - (readed += fread((prgbPasswordProBuffer + readed), 1, toRead, fPassword))) != 0) ;
	fclose(fPassword);

	// ��������� ����������� ��������� ����� �� ��������� ������� ��������������� �����...
	for (i = 0; i < lContainerSize; i++)
	{
		prgbPasswordBuffer[i] = prgbPasswordProBuffer[pass_rnd.Get32() % lContainerSize];
	}

	// ������� ������ ��������� �����...
	memset(prgbPasswordProBuffer, 0x00, lPasswordSize);
	delete [] prgbPasswordProBuffer;

	QueryPerformanceCounter(&seed3);
	    
	// ��������� ��������� ����� ������ ���������...
	switch (mode)
	{
		//  ���� ������� ����������...
		case ENCRYPT:
		{
			// ��������� ������ �� �������� �����...
			toRead = dataLen = lSourceSize;
			readed = 0;
			while((toRead = dataLen - (readed += fread((prgbSourceBuffer + readed), 1, toRead, fSource))) != 0) ;
			fclose(fSource);

			QueryPerformanceCounter(&seed4);

			// ������������� "��������" ���������� ��������� �����...
			seed = GetTickCount();
			srand((unsigned int)seed);

			// ������������� ���������� ��������� ����� �� ������� ������...
			charToInt.rgbVariable[0] = seed1.LowPart & 0xFF;
			charToInt.rgbVariable[1] = seed2.LowPart & 0xFF;
			charToInt.rgbVariable[2] = seed3.LowPart & 0xFF;
			charToInt.rgbVariable[3] = seed4.LowPart & 0xFF;
			Random padding_rnd(charToInt.intVariable); // by Eugene Roshal (public domain)

			// ��������� ������ ����� "�������"...
			for (i = 0, j = lSourceSize; i < (lContainerSize - lSourceSize) - INT32_SIZE; i++, j++)
			{
				// �������� ��������� ���������� ��������� ����� ��������� ��� ������� ���������������� ����������
				// (�� �������� int32 �� ��������� ���������� ������ �� 0..24 ���)...
				prgbSourceBuffer[j] = (padding_rnd.Get32() >> (((uint32)rand()) % 25)) & 0xFF;
			}
			
			//...����������� � ����� ��������� ���� ������� �������� ������...
			charToInt.intVariable = lSourceSize;
			for (i = 0; i < INT32_SIZE; i++, j++)
			{
				prgbSourceBuffer[j] = charToInt.rgbVariable[i];
			}

			////////////////////////////////////////////////////////////////////
			// ������� ������...
			////////////////////////////////////////////////////////////////////
			cout << "seed     : " << (unsigned int)seed << "\n";
			cout << "selector : " << password_selector  << "\n";
			cout << "count    : " << (lContainerSize - INT32_SIZE) << "\n";
			inKey.Encrypt(prgbSourceBuffer, lContainerSize, prgbPasswordBuffer);
			////////////////////////////////////////////////////////////////////

			// ������ ������������ ������ �� ����...
			toWrite = dataLen = lContainerSize;
			written = 0;
			while((toWrite = dataLen - (written += fwrite((prgbSourceBuffer + written), 1, toWrite, fTarget))) != 0) ;
			break;
		}

		// ���� ������� �����������...
		case DECRYPT:
		{
			// ��������� ������ �� �������� �����-����������...
			toRead = dataLen = lContainerSize;
			readed = 0;
			while((toRead = dataLen - (readed += fread((prgbSourceBuffer + readed), 1, toRead, fSource))) != 0) ;
			fclose(fSource);

			////////////////////////////////////////////////////////////////////
			// �������������� ������...
			////////////////////////////////////////////////////////////////////
			cout << "selector : " << password_selector  << "\n";
			cout << "count    : " << (lContainerSize - INT32_SIZE) << "\n";
			inKey.Decrypt(prgbSourceBuffer, lContainerSize, prgbPasswordBuffer);
			////////////////////////////////////////////////////////////////////

			// ������ "���������" ������ ������������� ������...
			for (i = 0, j = (lContainerSize - INT32_SIZE); i < INT32_SIZE; i++, j++)
			{
				charToInt.rgbVariable[i] = prgbSourceBuffer[j];
			}
			lRealSourceSize = charToInt.intVariable;
            
			if (lRealSourceSize > (lContainerSize - INT32_SIZE))
			{
				lRealSourceSize = (lContainerSize - INT32_SIZE); // ���������� ���� ���������...
			}

			// ������ ������������ ������ �� ����...
			toWrite = dataLen = lRealSourceSize;
			written = 0;
			while((toWrite = dataLen - (written += fwrite((prgbSourceBuffer + written), 1, toWrite, fTarget))) != 0) ;
		}
	}

	// ���������� �������� �����...
	fflush(fTarget);
	// ...� ������ ����� ����� ��������� ����
	fclose(fTarget);
   
	delete [] szTargetFile;
  	delete [] prgbSourceBuffer;
	delete [] prgbPasswordBuffer;
	delete [] prgbBitData;

	// ���� �� ������ ������� - ���������� ��� ������:0
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// ������������ ��������� � ������ ������� ��������� �����
int main(int argc, char* argv[])
{	
	// szTargetFile ���������������� � ������� ��������� ������� proceed_file
	char* szMode, *szSourceFile, *szPasswordFile;
	int mode, password_selector;

	cout << endl;
	cout << "inKey 6.00.P   Copyright (C) DrAF, Cherepovets, 2003 - 2013." << endl;
	cout << endl;
	
	// ���� ���������� ���������� � ��������� ������ ������������...
	if (argc != ARGC_NUM)
	{
		// ... � ���� ��������� ����������� � ��������� ������ ����������...
		if (argc != 1)
		{
			// ... ���� ���������� ������� ����...
			if (argc < ARGC_NUM)
			{
				cout << endl;
				cout << "ERROR: There is not enough arguments in a command line!" << endl;
				cout << endl;
			}

			// ... ���� ���������� ������� �����...
			if (argc > ARGC_NUM)
			{
				cout << endl;
				cout << "ERROR: It is too many arguments in a command line!" << endl;
				cout << endl;
			}
		}

		cout << "Usage:         inKey <command> <source file> <file-password> <password selector>"  << endl;
		cout << endl;
		cout << "               <commands>" << endl;
		cout << "               e - to encrypt source file (*.*)"    << endl;
		cout << "               d - to decrypt source file (*.*.iK)" << endl;
		cout << endl;
		cout << "               <password selector>"  << endl;
		cout << "               - integer value (seed)" << endl;
		cout << endl;

		cout << "Press any key to exit...";
		cout << endl;
		getch();
		
		return 0;
	} 

	// ��������� ������ �� ��������� ������...
	szMode         = argv[1];
	szSourceFile   = argv[2];
	szPasswordFile = argv[3];

	try
	{
		password_selector = atoi(argv[4]);
	}
	catch (...)
	{
		cout << endl;
		cout << "ERROR: Wrong password selector! Were used the default value (0)." << endl;
		cout << endl;
		password_selector = 0;
	}

	// ����������� ����� ������...
	if ((szMode[0] == 'e') || (szMode[0] == 'E'))
	{
		mode = ENCRYPT;

	} else
	if ((szMode[0] == 'd') || (szMode[0] == 'D'))
	{
		mode = DECRYPT;

	} else
	{
		cout << "ERROR: Wrong command!" << endl;
		cout << endl;
		cout << "       Press any key to exit...";
		getch();

		return 1;
	}
	
	// ������� ��� ����������� ������, ��������� ��������� �����...
	return process_file(szSourceFile, szPasswordFile, mode, password_selector);
}