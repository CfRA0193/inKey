//////////////////////////////////////////////////////////////////////
//////////////////////////// inKey 6.00.L ////////////////////////////
//////////////////////////////////////////////////////////////////////
// �����: �������� ���� (DrAF) // draf@mail.ru                     //
//////////////////////////////////////////////////////////////////////
// ����������� ���������: ������� �������� (ProFit) // 0176@mail.ru //
//////////////////////////////////////////////////////////////////////
// ��������: 2 ����� 2003 �, 21 ������� 2013 �.                     //
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
#define ARGC_NUM            4          // ��������� ���������� ����������
#define INT32_SIZE          4          // ������ int32

//////////////////////////////////////////////////////////////////////
// ���� union ��������� ��� �������������� Int32 � �������� �������������
typedef union
{
	__int32 intVariable;
	char rgbVariable[4];
} TCharToInt32;

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
int process_file(char* szSourceFile, char* szPasswordFile, int Mode)
{
	FILE *fSource, *fPassword, *fTarget;
	TCharToInt32 charToInt;
	LARGE_INTEGER seed1, seed2, seed3, seed4;
	long int i = 0, j;

	// ������ ��������� �����, �����-������, ������ ����������,
	// �������� ������ �������� ������ � ����������...
	long int lSourceSize, lPasswordSize, lContainerSize, lRealSourceSize;
	char rgTargetExt[] = ".iK";
	char *szTargetFile = new char[MAX_NAME_LENGTH];
	char *prgbSourceBuffer, *prgbPasswordBuffer, *prgbBitData;

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
	switch (Mode)
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
		
			// "�����������" � ����� ��������� ����� ���������� ".iK"
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
	switch (Mode)
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
	prgbSourceBuffer   = new char [lContainerSize];
	prgbPasswordBuffer = new char [lContainerSize];
	
	// �������� ������ ��� "�������� ������"...
	prgbBitData = new char[2 * NBITS * lContainerSize];

	// ��������� ������ �� ���������� ����� (�������, ������� ����� ��� ��������� ����������)...
	fread(prgbPasswordBuffer, lContainerSize, 1, fPassword);
	fclose(fPassword);
	QueryPerformanceCounter(&seed3);
	    
	// ��������� ��������� ����� ������ ���������...
	switch (Mode)
	{
		//  ���� ������� ����������...
		case ENCRYPT:
		{
			// ��������� ������ �� �������� �����...
			fread(prgbSourceBuffer, lSourceSize, 1, fSource);
			fclose(fSource);
			QueryPerformanceCounter(&seed4);
			charToInt.rgbVariable[0] = seed1.LowPart & 0xFF;
			charToInt.rgbVariable[1] = seed2.LowPart & 0xFF;
			charToInt.rgbVariable[2] = seed3.LowPart & 0xFF;
			charToInt.rgbVariable[3] = seed4.LowPart & 0xFF;
			cout << "seed     : " << (unsigned int)charToInt.intVariable << "\n";
			srand((unsigned int)charToInt.intVariable);
			
			// ��������� ������ ����� "�������"...
			for (i = 0, j = lSourceSize; i < (lContainerSize - lSourceSize) - INT32_SIZE; i++, j++)
			{
				prgbSourceBuffer[j] = rand();
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
			cout << "count    : " << lContainerSize << "\n";
			inKey.Encrypt(prgbSourceBuffer, lContainerSize, prgbPasswordBuffer);
			////////////////////////////////////////////////////////////////////

			// ������ ������������ ������ �� ����...
			fwrite(prgbSourceBuffer, lContainerSize, 1, fTarget);
			break;
		}

		// ���� ������� �����������...
		case DECRYPT:
		{
			// ��������� ������ �� �������� �����-����������...
			fread(prgbSourceBuffer, lContainerSize, 1, fSource);
			fclose(fSource);

			////////////////////////////////////////////////////////////////////
			// �������������� ������...
			////////////////////////////////////////////////////////////////////
			cout << "count    : " << lContainerSize << "\n";
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
				lRealSourceSize = lContainerSize - INT32_SIZE;
			}

			// ������ ������������ ������ �� ����...
			fwrite(prgbSourceBuffer, lRealSourceSize, 1, fTarget);
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
	int Mode;

	cout << endl;
	cout << "inKey 6.00.L   Copyright (C) DrAF, Cherepovets, 2003 - 2013." << endl;
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

		cout << "Usage:         inKey <command> <source file> <file-password>"  << endl;
		cout << endl;
		cout << "               <commands>" << endl;
		cout << "               e - to encrypt source file (*.*)"    << endl;
		cout << "               d - to decrypt source file (*.*.iK)" << endl;
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

	// ����������� ����� ������...
	if ((szMode[0] == 'e') || (szMode[0] == 'E'))
	{
		Mode = ENCRYPT;

	} else
	if ((szMode[0] == 'd') || (szMode[0] == 'D'))
	{
		Mode = DECRYPT;

	} else
	{
	   cout << "ERROR: Wrong command!" << endl;
       cout << endl;
       cout << "       Press any key to exit...";
       getch();

	   return 1;
	}
	
	// ������� ��� ����������� ������, ��������� ��������� �����...
	return process_file(szSourceFile, szPasswordFile, Mode);
}