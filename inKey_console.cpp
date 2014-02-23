//////////////////////////////////////////////////////////////////////
//////////////////////////// inKey 6.00.Q ////////////////////////////
//////////////////////////////////////////////////////////////////////
// �����:  �������� ���� (DrAF)                                    //
// e-mail: artem.drobanov@gmail.com, draf@mail.ru                   //
//////////////////////////////////////////////////////////////////////
// ��������: [ 2 ����� 2003 �, 23 ������� 2014 �. ]                 //
//////////////////////////// �. ��������� ////////////////////////////

#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

#include <iostream>
using namespace std;

#pragma warning (disable: 4996)

//////////////////////////////////////////////////////////////////////
// ���������� �� "ASC 1.3.0.3" ("ASC 1.4.0.0")
#include "inKey.cpp"
//////////////////////////////////////////////////////////////////////

#define TO_READ            0    // �������: ��� ������
#define TO_WRITE           1    // �������: ��� ������
#define TARGET_EXT_LENGTH  3    // ����� ���������� �����
#define MAX_NAME_LENGTH    215  // ����. ���������� ����� ����� �����
#define INT32_SIZE         4    // ������ int32
#define ARGC_NUM           4    // ��������� ���������� ����������
#define PROGRESS_STEP      64   // ��� ��������� ��������� ��� ������

//////////////////////////////////////////////////////////////////////

typedef __int32           int32;
typedef __int64           int64;
typedef unsigned __int32  uint32;
typedef unsigned __int64  uint64;

//////////////////////////////////////////////////////////////////////
// ���� union ��������� ��� �������������� Int32 � ����-�������������
typedef union
{
    int32 intVariable;
    char rgbVariable[4];

} TCharToInt32;

//////////////////////////////////////////////////////////////////////
// ��������� ��������������� ����� � ����������� ��������������
class Random // by Eugene Roshal (public domain)
{
  private:
    uint32 x,y,z,w;
  public:
    Random(uint32 Seed) { x = Seed; y = 362436069; z = 521288629;\
                          w = 88675123; }
    uint32 Get32()      { uint32 t = x ^ (x << 11); x = y; y = z;\
                          z = w; return w = w ^ (w >> 19) ^ (t ^ \
                          (t >> 8)); }
    uint64 Get64()      { return (uint64(Get32()) << 32) | Get32(); }
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
// � �����: ����� �� ���� ������� ����������� ����. ����� ��� ������
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

//////////////////////////////////////////////////////////////////////
// ������������ ����� ��������� ���������
void update_progress(int progress)
{
    if ((progress % PROGRESS_STEP) == 0)
    {
        cout << "\r" << "position   : " << progress;
    }
}

//////////////////////////////////////////////////////////////////////
// ������������ �������� ������ ��� ����������/�����������
int process_file(char* szSourceFile, char* szKeyFile, int mode,
                 int key_selector, int minimal_key_size)
{
    FILE *fSource, *fKey, *fTarget;
    TCharToInt32 charToInt;    
    LARGE_INTEGER seed1, seed2, seed3, seed4;
    Random pass_rnd(key_selector); // by Eugene Roshal (public domain)
    int seed;
    long int i = 0, j, dataLen, toRead, toWrite, readed, written;

    // ������ ��������� �����, �����-�����, ������ ����������,
    // �������� ������ �������� ������ � ����������...
    long int lSourceSize, lKeySize, lContainerSize, lRealSourceSize;
    char rgTargetExt[] = ".iK";
    char *szTargetFile = new char[MAX_NAME_LENGTH];
    char *prgbSourceBuffer, *prgbKeyProBuffer, *prgbKeyBuffer, *prgbBitData;

    ///////////////////////////////////////////////
    // ���������� �� "ASC 1.3.0.3" ("ASC 1.4.0.0")
    CInKey inKey(update_progress);
    QueryPerformanceCounter(&seed1);
    ///////////////////////////////////////////////

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

    // ��������� �������� ���� � �������������� ���������...
    if ((fKey = open_file_to_read(szKeyFile, lKeySize)) == NULL)
    {
        cout << endl;
        cout << "       Press any key to exit...";
        getch();

        delete [] szTargetFile;
        fclose(fSource);

        return 1;
    }

    QueryPerformanceCounter(&seed2);

    switch (mode)
    {
        // ���� ������� ����������...
        case ENCRYPT:
        {
            // ���������� ������ ����������, � ������� �����
            // ��������� ������ - �� ����� minimal_key_size
            // � �� ����� ������� �������� ������...
            lContainerSize = (lSourceSize < minimal_key_size)\
                ? minimal_key_size : lSourceSize;
            
            // ��������� ��������� ����...
            lContainerSize += INT32_SIZE;

            break;
        }
        
        // ���� ������� �����������...
        case DECRYPT:
        {
            // ��������� ���� ��� ������.
            lContainerSize = lSourceSize;            
        }
    }

    // �������� ���� ������ ���� �� ������ ���������!
    if (lKeySize < lContainerSize)
    {
        cout << "ERROR: The key file is too small!" << endl;
        cout << "       It's size must be >= " << lContainerSize\
             << " bytes!" << endl;
        cout << endl;
        cout << "       Press any key to exit...";
        getch();

        delete [] szTargetFile;
        fclose(fSource);
        fclose(fKey);

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
        fclose(fKey);

        return 1;
    }

    // �������� ������ ��� ���������� ������...
    prgbSourceBuffer = new char [lContainerSize];
    prgbKeyBuffer    = new char [lContainerSize];
    prgbKeyProBuffer = new char [lKeySize];

    // �������� ������ ��� "�������� ������"...
    prgbBitData = new char[2 * NBITS * lContainerSize];

    // ��������� ������ �� ��������� ����� (�������, ������� �����
    // ��� ��������� ����������)...
    toRead = dataLen = lKeySize;
    readed = 0;
    while((toRead = dataLen - (readed += fread((prgbKeyProBuffer\
        + readed), 1, toRead, fKey))) != 0) ;
    fclose(fKey);

    // ��������� ����������� �������� ����� �� ��������� ������� ��������������� �����...
    for (i = 0; i < lContainerSize; i++)
    {
        prgbKeyBuffer[i] = prgbKeyProBuffer[pass_rnd.Get32()\
            % lContainerSize];
    }

    // ������� ������ ��������� �����...
    memset(prgbKeyProBuffer, 0x00, lKeySize);
    delete [] prgbKeyProBuffer;

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
            while((toRead = dataLen - (readed += fread((prgbSourceBuffer\
                + readed), 1, toRead, fSource))) != 0) ;
            fclose(fSource);

            QueryPerformanceCounter(&seed4);

            // ������������� "��������" ���������� ��������� �����...
            seed = GetTickCount();
            srand((unsigned int)seed);

            // ������������� ���������� ��������� �����
            // �� ������� ������...
            charToInt.rgbVariable[0] = seed1.LowPart & 0xFF;
            charToInt.rgbVariable[1] = seed2.LowPart & 0xFF;
            charToInt.rgbVariable[2] = seed3.LowPart & 0xFF;
            charToInt.rgbVariable[3] = seed4.LowPart & 0xFF;
            Random padding_rnd(charToInt.intVariable);

            // ��������� ������ ����� "�������"...
            for (i = 0, j = lSourceSize; i < (lContainerSize\
                - lSourceSize) - INT32_SIZE; i++, j++)
            {
                // �������� ��������� ���������� ��������� �����
                // ��������� ��� ������� ���������������� ����������
                // (�� �������� int32 �� ��������� ���������� ������
                // �� 0..24 ���)...
                prgbSourceBuffer[j] = (padding_rnd.Get32()\
                    >> (((uint32)rand()) % 25)) & 0xFF;
            }
            
            //...����������� � ����� ��������� ���� �������
            // �������� ������...
            charToInt.intVariable = lSourceSize;
            for (i = 0; i < INT32_SIZE; i++, j++)
            {
                prgbSourceBuffer[j] = charToInt.rgbVariable[i];
            }

            /////////////////////////////////////////////////////////////////
            // ������� ������...
            /////////////////////////////////////////////////////////////////
            cout << "seed       : " << (unsigned int)seed            << "\n";
            cout << "min.k.size : " << minimal_key_size              << "\n";
            cout << "selector   : " << key_selector                  << "\n";
            cout << "count      : " << (lContainerSize - INT32_SIZE) << "\n";
            inKey.Encrypt(prgbSourceBuffer, lContainerSize, prgbKeyBuffer);
            /////////////////////////////////////////////////////////////////

            // ������ ������������ ������ �� ����...
            toWrite = dataLen = lContainerSize;
            written = 0;
            while((toWrite = dataLen - (written += fwrite((prgbSourceBuffer\
                + written), 1, toWrite, fTarget))) != 0) ;
            break;
        }

        // ���� ������� �����������...
        case DECRYPT:
        {
            // ��������� ������ �� �������� �����-����������...
            toRead = dataLen = lContainerSize;
            readed = 0;
            while((toRead = dataLen - (readed += fread((prgbSourceBuffer\
                + readed), 1, toRead, fSource))) != 0) ;
            fclose(fSource);

            /////////////////////////////////////////////////////////////////
            // �������������� ������...
            /////////////////////////////////////////////////////////////////
            cout << "selector   : " << key_selector                  << "\n";
            cout << "count      : " << (lContainerSize - INT32_SIZE) << "\n";                    
            inKey.Decrypt(prgbSourceBuffer, lContainerSize, prgbKeyBuffer);
            /////////////////////////////////////////////////////////////////

            // ������ "���������" ������ ������������� ������...
            for (i = 0, j = (lContainerSize - INT32_SIZE);\
                i < INT32_SIZE; i++, j++)
            {
                charToInt.rgbVariable[i] = prgbSourceBuffer[j];
            }
            lRealSourceSize = charToInt.intVariable;
            
            // ���� ��������� ������, ��� �������...
            if (lRealSourceSize > (lContainerSize - INT32_SIZE))
            {
                //...���������� ��� ����...
                lRealSourceSize = (lContainerSize - INT32_SIZE);
            }

            // ������ ������������ ������ �� ����...
            toWrite = dataLen = lRealSourceSize;
            written = 0;
            while((toWrite = dataLen - (written += fwrite((prgbSourceBuffer\
                + written), 1, toWrite, fTarget))) != 0) ;
        }
    }

    // ���������� �������� �����...
    fflush(fTarget);
    // ...� ������ ����� ����� ��������� ����
    fclose(fTarget);
   
    delete [] szTargetFile;
    delete [] prgbSourceBuffer;
    delete [] prgbKeyBuffer;
    delete [] prgbBitData;

    // ���� �� ������ ������� - ���������� ��� ������:0
    return 0;
}

//////////////////////////////////////////////////////////////////////
// ������������ ��������� � ������ ������� ��������� �����
int main(int argc, char* argv[])
{    
    // szTargetFile ���������������� � ������� ��������� �������
    // proceed_file
    char* szMode, *szSourceFile, *szKeyFile;
    int mode, key_selector, minimal_key_size;

    cout << endl;
    cout << "inKey 6.00.Q   Copyright (C) DrAF, Cherepovets, 2003 - 2014." << endl;
    cout << endl;
    
    // ���� ���������� ���������� � ��������� ������ ������������...
    if (argc < ARGC_NUM)
    {
        // ...� ���� ��������� ����������� � ��������� ������ ����������...
        if (argc != 1)
        {
            cout << "ERROR: There is not enough arguments in a command line!" << endl;
            cout << endl;
        }

        cout << "Usage:         inKey <command> <source file> <key file> <key data selector>" << endl;
        cout << "                     [minimal key size]"                                     << endl;
        cout << endl;
        cout << "               <commands>" << endl;
        cout << "               e - to encrypt source file (*.*)"    << endl;
        cout << "               d - to decrypt source file (*.*.iK)" << endl;
        cout << endl;
        cout << "               <key data selector>"    << endl;
        cout << "               - integer value (seed)" << endl;
        cout << endl;
        cout << "               [minimal key size]"                                      << endl;
        cout << "               - integer value (kilobytes), better if value >= \"100\"" << endl;
        cout << endl;
        cout << "Press any key to exit...";
        cout << endl;
        getch();
        
        return 0;
    }

    // ��������� ������ �� ��������� ������...
    szMode       = argv[1];
    szSourceFile = argv[2];
    szKeyFile    = argv[3];

    try
    {
        if (argc < 5) throw "argc < 5";
        key_selector = atoi(argv[4]);
    }
    catch (...)
    {
        cout << "<key data selector>: were used the default value!" << endl;
        cout << endl;
        key_selector = 0;
    }

    try
    {
        if (argc < 6) throw "argc < 6";
        minimal_key_size = atoi(argv[5]) * 1024; // kilobytes
    }
    catch (...)
    {
        cout << "[minimal key size]: were used the default value!" << endl;
        cout << endl;
        minimal_key_size = 100 * 1024;
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
        cout << "ERROR: Unknown command \"" << szMode[0] << "\"" << endl;
        cout << endl;
        cout << "       Press any key to exit...";
        getch();

        return 1;
    }
    
    // ������� ��� ����������� ������, ��������� ��������� �����...
    return process_file(szSourceFile, szKeyFile, mode, key_selector, minimal_key_size);
}
