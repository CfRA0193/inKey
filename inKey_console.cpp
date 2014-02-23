//////////////////////////////////////////////////////////////////////
//////////////////////////// inKey 6.00.Q ////////////////////////////
//////////////////////////////////////////////////////////////////////
// Автор:  Дробанов Артём (DrAF)                                    //
// e-mail: artem.drobanov@gmail.com, draf@mail.ru                   //
//////////////////////////////////////////////////////////////////////
// Редакции: [ 2 марта 2003 г, 23 февраля 2014 г. ]                 //
//////////////////////////// г. Череповец ////////////////////////////

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
// Шифрование из "ASC 1.3.0.3" ("ASC 1.4.0.0")
#include "inKey.cpp"
//////////////////////////////////////////////////////////////////////

#define TO_READ            0    // Атрибут: ДЛЯ ЧТЕНИЯ
#define TO_WRITE           1    // Атрибут: ДЛЯ ЗАПИСИ
#define TARGET_EXT_LENGTH  3    // Длина расширения файла
#define MAX_NAME_LENGTH    215  // Макс. допустимая длина имени файла
#define INT32_SIZE         4    // Размер int32
#define ARGC_NUM           4    // Требуемое количество аргументов
#define PROGRESS_STEP      64   // Шаг аргумента прогресса для вывода

//////////////////////////////////////////////////////////////////////

typedef __int32           int32;
typedef __int64           int64;
typedef unsigned __int32  uint32;
typedef unsigned __int64  uint64;

//////////////////////////////////////////////////////////////////////
// Этот union необходим для преобразования Int32 в байт-представление
typedef union
{
    int32 intVariable;
    char rgbVariable[4];

} TCharToInt32;

//////////////////////////////////////////////////////////////////////
// Генератор псевдослучайных чисел с равномерным распределением
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
// Формирует сообщение об ошибке открытия файла
void file_error_message(char *szFilename, int errKind)
{
    cout << "ERROR: Can't open " << szFilename;

    // Показываем пользователю, в каком режиме открытия файла
    // произошел сбой (для чтения или для записи)
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
// Возвращает длину файла
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
// Обеспечивает инициализацию:
// 1 - Файлового указателя
// 2 - Переменной, хранящей длину файла
// В целом: берет на себя функции корректного откр. файла для чтения
FILE* open_file_to_read(char *szFilename, long int &lFileSize)
{
    // Файловый указатель
    FILE *fHandle;

    // Проверяем файл на возможность корректного открытия...
    if ((fHandle = fopen(szFilename, "rb")) == NULL)
    {
        // Если открыть нельзя - выводим сообщение об ошибке...
        file_error_message(szFilename, TO_READ);
        
        // ... и выходим из функции
        return NULL;
    }
    
    // ... если файл открылся - всё OK, но ещё необходимо произвести
    // определение его размера, для этого нужно переоткрыть его 
    // в другом режиме, поэтому его временно закрываем...
    fclose(fHandle);

    // ... и получаем размер файла.
    lFileSize = file_length(szFilename);

    // А вот и открытие файла для работы с ним...
    fHandle = fopen(szFilename, "rb");

    //...возвращаем файловый указатель...
    return fHandle;
}

//////////////////////////////////////////////////////////////////////
// Обеспечивает вывод прогресса обработки
void update_progress(int progress)
{
    if ((progress % PROGRESS_STEP) == 0)
    {
        cout << "\r" << "position   : " << progress;
    }
}

//////////////////////////////////////////////////////////////////////
// Обеспечивает ФАЙЛОВЫЙ СЕРВИС для шифрования/расшифровки
int process_file(char* szSourceFile, char* szKeyFile, int mode,
                 int key_selector, int minimal_key_size)
{
    FILE *fSource, *fKey, *fTarget;
    TCharToInt32 charToInt;    
    LARGE_INTEGER seed1, seed2, seed3, seed4;
    Random pass_rnd(key_selector); // by Eugene Roshal (public domain)
    int seed;
    long int i = 0, j, dataLen, toRead, toWrite, readed, written;

    // Размер исходного файла, файла-ключа, размер контейнера,
    // реальный размер исходных данных в контейнере...
    long int lSourceSize, lKeySize, lContainerSize, lRealSourceSize;
    char rgTargetExt[] = ".iK";
    char *szTargetFile = new char[MAX_NAME_LENGTH];
    char *prgbSourceBuffer, *prgbKeyProBuffer, *prgbKeyBuffer, *prgbBitData;

    ///////////////////////////////////////////////
    // Шифрование из "ASC 1.3.0.3" ("ASC 1.4.0.0")
    CInKey inKey(update_progress);
    QueryPerformanceCounter(&seed1);
    ///////////////////////////////////////////////

    // В нижеприведенном блоке кода "switch" происходит
    // дифференцированное получение имени выходного файла
    // из имени входного файла в зависимости от режима работы
    // программы
    switch (mode)
    {
        // Если выбрано шифрование...
        case ENCRYPT:
        {
            cout << "Encryption mode selected..." << endl << endl;

            // Копируем имя шифруемого файла в имя целевого файла...
            while (szSourceFile[i] != '\0')
            {
                szTargetFile[i] = szSourceFile[i];
                i++;
            }
        
            // "Приклеиваем" к имени выходного файла расширение ".iK"...
            for (j = 0; j < TARGET_EXT_LENGTH; j++)
            {
                szTargetFile[i + j] = rgTargetExt[j];
            }

            // Клеим ещё и символ конца строки...
            szTargetFile[i + j] = '\0';

            break;
        }

        // Если выбрана расшифровка...
        case DECRYPT:
        {    
            int LastPointPosition = 0; // Позиция последней точки
                                       // в имени файла...

            cout << "Decryption mode selected..." << endl << endl;

            // Копируем имя исходного файла в имя целевого файла,
            // определяя позицию, на которой встречается последняя
            // точка в имени...
            while (szSourceFile[i] != '\0')
            {
                szTargetFile[i] = szSourceFile[i];
                
                if (szSourceFile[i] == '.')
                {
                    LastPointPosition = i;
                }

                i++;
            }

            // Отсекаем уже ненужное расширение путем установки
            // символа конца строки вместо последней точки...
            szTargetFile[LastPointPosition] = '\0';
        }
    }    

    // Открываем исходный файл и в случае неудачи
    // завершаем работу программы...
    if ((fSource = open_file_to_read(szSourceFile, lSourceSize)) == NULL)
    {
        cout << endl;
        cout << "       Press any key to exit...";
        getch();

        delete [] szTargetFile;

        return 1;
    }

    // Открываем ключевой файл с вышеуказанными условиями...
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
        // Если выбрано шифрование...
        case ENCRYPT:
        {
            // Определяем размер контейнера, в котором будут
            // храниться данные - не менее minimal_key_size
            // и не более размера исходных данных...
            lContainerSize = (lSourceSize < minimal_key_size)\
                ? minimal_key_size : lSourceSize;
            
            // Добавляем служебное поле...
            lContainerSize += INT32_SIZE;

            break;
        }
        
        // Если выбрана расшифровка...
        case DECRYPT:
        {
            // Служебное поле уже учтено.
            lContainerSize = lSourceSize;            
        }
    }

    // Ключевой файл должен быть не меньше исходного!
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

    // Если не можем открыть выходной файл...
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

    // Выделяем память под содержимое файлов...
    prgbSourceBuffer = new char [lContainerSize];
    prgbKeyBuffer    = new char [lContainerSize];
    prgbKeyProBuffer = new char [lKeySize];

    // Выделяем память для "двоичных данных"...
    prgbBitData = new char[2 * NBITS * lContainerSize];

    // Считываем данные из ключевого файла (столько, сколько нужно
    // для обработки контейнера)...
    toRead = dataLen = lKeySize;
    readed = 0;
    while((toRead = dataLen - (readed += fread((prgbKeyProBuffer\
        + readed), 1, toRead, fKey))) != 0) ;
    fclose(fKey);

    // Формируем оперативный ключевой буфер на основании датчика псевдослучайных чисел...
    for (i = 0; i < lContainerSize; i++)
    {
        prgbKeyBuffer[i] = prgbKeyProBuffer[pass_rnd.Get32()\
            % lContainerSize];
    }

    // Очистка данных прообраза ключа...
    memset(prgbKeyProBuffer, 0x00, lKeySize);
    delete [] prgbKeyProBuffer;

    QueryPerformanceCounter(&seed3);
        
    // Реализуем выбранный режим работы программы...
    switch (mode)
    {
        //  Если выбрано шифрование...
        case ENCRYPT:
        {
            // Считываем данные из входного файла...
            toRead = dataLen = lSourceSize;
            readed = 0;
            while((toRead = dataLen - (readed += fread((prgbSourceBuffer\
                + readed), 1, toRead, fSource))) != 0) ;
            fclose(fSource);

            QueryPerformanceCounter(&seed4);

            // Инициализация "штатного" генератора случайных чисел...
            seed = GetTickCount();
            srand((unsigned int)seed);

            // Инициализация генератора случайных чисел
            // от Евгения Рошаля...
            charToInt.rgbVariable[0] = seed1.LowPart & 0xFF;
            charToInt.rgbVariable[1] = seed2.LowPart & 0xFF;
            charToInt.rgbVariable[2] = seed3.LowPart & 0xFF;
            charToInt.rgbVariable[3] = seed4.LowPart & 0xFF;
            Random padding_rnd(charToInt.intVariable);

            // Заполняем пустое место "мусором"...
            for (i = 0, j = lSourceSize; i < (lContainerSize\
                - lSourceSize) - INT32_SIZE; i++, j++)
            {
                // Значение основного генератора случайных чисел
                // подпадает под влияние вспомогательного генератора
                // (он сдвигает int32 от основного генератора вправо
                // от 0..24 бит)...
                prgbSourceBuffer[j] = (padding_rnd.Get32()\
                    >> (((uint32)rand()) % 25)) & 0xFF;
            }
            
            //...дозаписывая в конце служебное поле размера
            // исходных данных...
            charToInt.intVariable = lSourceSize;
            for (i = 0; i < INT32_SIZE; i++, j++)
            {
                prgbSourceBuffer[j] = charToInt.rgbVariable[i];
            }

            /////////////////////////////////////////////////////////////////
            // Шифруем данные...
            /////////////////////////////////////////////////////////////////
            cout << "seed       : " << (unsigned int)seed            << "\n";
            cout << "min.k.size : " << minimal_key_size              << "\n";
            cout << "selector   : " << key_selector                  << "\n";
            cout << "count      : " << (lContainerSize - INT32_SIZE) << "\n";
            inKey.Encrypt(prgbSourceBuffer, lContainerSize, prgbKeyBuffer);
            /////////////////////////////////////////////////////////////////

            // Запись обработанных данных на диск...
            toWrite = dataLen = lContainerSize;
            written = 0;
            while((toWrite = dataLen - (written += fwrite((prgbSourceBuffer\
                + written), 1, toWrite, fTarget))) != 0) ;
            break;
        }

        // Если выбрана расшифровка...
        case DECRYPT:
        {
            // Считываем данные из входного файла-контейнера...
            toRead = dataLen = lContainerSize;
            readed = 0;
            while((toRead = dataLen - (readed += fread((prgbSourceBuffer\
                + readed), 1, toRead, fSource))) != 0) ;
            fclose(fSource);

            /////////////////////////////////////////////////////////////////
            // Расшифровываем данные...
            /////////////////////////////////////////////////////////////////
            cout << "selector   : " << key_selector                  << "\n";
            cout << "count      : " << (lContainerSize - INT32_SIZE) << "\n";                    
            inKey.Decrypt(prgbSourceBuffer, lContainerSize, prgbKeyBuffer);
            /////////////////////////////////////////////////////////////////

            // Узнаем "настоящий" размер зашифрованных данных...
            for (i = 0, j = (lContainerSize - INT32_SIZE);\
                i < INT32_SIZE; i++, j++)
            {
                charToInt.rgbVariable[i] = prgbSourceBuffer[j];
            }
            lRealSourceSize = charToInt.intVariable;
            
            // Если контейнер больше, чем указано...
            if (lRealSourceSize > (lContainerSize - INT32_SIZE))
            {
                //...раскрываем его весь...
                lRealSourceSize = (lContainerSize - INT32_SIZE);
            }

            // Запись обработанных данных на диск...
            toWrite = dataLen = lRealSourceSize;
            written = 0;
            while((toWrite = dataLen - (written += fwrite((prgbSourceBuffer\
                + written), 1, toWrite, fTarget))) != 0) ;
        }
    }

    // Сбрасываем дисковый буфер...
    fflush(fTarget);
    // ...и только после этого закрываем файл
    fclose(fTarget);
   
    delete [] szTargetFile;
    delete [] prgbSourceBuffer;
    delete [] prgbKeyBuffer;
    delete [] prgbBitData;

    // Если всё прошло успешно - возвращаем код ошибки:0
    return 0;
}

//////////////////////////////////////////////////////////////////////
// Обеспечивает интерфейс и запуск функции обработки файла
int main(int argc, char* argv[])
{    
    // szTargetFile инициализируется в функции файлового сервиса
    // proceed_file
    char* szMode, *szSourceFile, *szKeyFile;
    int mode, key_selector, minimal_key_size;

    cout << endl;
    cout << "inKey 6.00.Q   Copyright (C) DrAF, Cherepovets, 2003 - 2014." << endl;
    cout << endl;
    
    // Если количество аргументов в командной строке неправильное...
    if (argc < ARGC_NUM)
    {
        // ...и если программа запускается с некоторым числом аргументов...
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

    // Извлекаем данные из командной строки...
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

    // Детектируем режим работы...
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
    
    // Получив все необходимые данные, запускаем обработку файла...
    return process_file(szSourceFile, szKeyFile, mode, key_selector, minimal_key_size);
}
