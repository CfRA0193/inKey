//******************************************************
//* inKey.h  - Класс шифрования данных
//******************************************************
//* Дата:    12.01.2006
//* Версия:  6.00
//* Автор:   Дробанов Артём Федорович (DrAF)
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
    // Подписываемся на функтор-обработчик обновления прогресса
    this->UpdateProgress = UpdateProgress;

    Initialize();
}

void CInKey::Initialize()
{
	// Инициализируем массив "битами", которые будут участвовать
    // в операциях выделения соответствующих битов
    rgbBitMask[0] = (char)0x01;
    rgbBitMask[1] = (char)0x02;
    rgbBitMask[2] = (char)0x04;
    rgbBitMask[3] = (char)0x08;
    rgbBitMask[4] = (char)0x10;
    rgbBitMask[5] = (char)0x20;
    rgbBitMask[6] = (char)0x40;
    rgbBitMask[7] = (char)0x80;

    // Инициализируем множество векторов MVS - 56 штук
    prgubMVS = new unsigned char[MVS_NUM * NBITS];
}

CInKey::~CInKey()
{
    // Деинициализируем множество векторов MVS
	if (prgubMVS)
	{
		delete [] prgubMVS;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Генерирует множество векторов, предназначенных для изменения
// порядка следования "столбцов". // РЕКУРСИВНАЯ ЧАСТЬ АЛГОРИТМА
void CInKey::GenerateMVS(char rgbBank[NBITS],int bankLen,
						 char rgbMVVector[NBITS])
{
	int i, j;

	char rgbSavedBank[NBITS];

	// Если мы использовали все символы для генерирования векторов,
	// то продолжать генерирование не представляется возможным
	if (bankLen == 0)
	{
		  // ЗДЕСЬ --> ЗАПИСЬ ГОТОВОГО ВЕКТОРА В БЛОК ВЕКТОРОВ
		  for (j = 0; j < NBITS; j++)
		  {
			    prgubMVS[cMVS * NBITS + j] = rgbMVVector[j];
		  }
		  cMVS++;

		  return;
	}

	// Этот цикл порождает множество рекурсивных вызовов,
	// добавляя один символ в вектор, и передавая его вниз по
	// рекурсии. Так, например, первая ветка рекурсии
	// создаст вектор "01234567".

	// Сохраняем исходное состояние переданного банка
	for (j = 0; j < bankLen; j++)
	{
		  rgbSavedBank[j] = rgbBank[j];
	}

	for (i = 0; i < bankLen; i++)
	{
	   // Каждый из векторов, получаемых в различных рекурсивных
	   // поддеревьях будет отличаться от другого на 1 символ на этой
	   // стадии
	   rgbMVVector[NBITS - bankLen] = rgbBank[i];
	   
	   // Теперь, когда ОДИН ИЗ СИМВОЛОВ ИЗЪЯТ,
	   // банк должен уменьшиться на ЭТОТ СИМВОЛ и
	   // затем поступить в сокращенном виде в
	   // дальнейшую рекурсию.
	   // Переносим символы влево, затирая "дырку"
	   // оставшимися в банке символами
	   for (j = i; j < (bankLen - 1); j++)
	   {
			  rgbBank[j] = rgbBank[j + 1];
	   }
	   
	   // РЕКУРСИЯ
	   GenerateMVS(rgbBank, (bankLen - 1), rgbMVVector);
	   
	   // Теперь восстанавливаем то состояние банка, в котором
	   // он был передан в рекурсивную функцию
	   for (j = 0; j < bankLen; j++)
	   {
			  rgbBank[j] = rgbSavedBank[j];
	   }
	}
}

////////////////////////////////////////////////////////////////////////////////
// Генерирует множество векторов, предназначенных для изменения
// порядка следования "столбцов".
// BankLen - количество символов в массиве Bank для генерирования
// каждого вектора в массиве. Каждый раз, когда очередной символ
// порождает поддерево комбинаций, он исчезает. Таким образом,
// генерируемое поддерево уже не будет содержать свой
// корень ни в одном из поддеревьев.
void CInKey::StartToGenerateMVS()
{	
	// ! ПОТЕНЦИАЛЬНАЯ ТОЧКА МОДИФИЦИРОВАНИЯ АЛГОРИТМА
	char rgbBank[NBITS] = {(char)0x00,(char)0x01,(char)0x02,(char)0x03,
						   (char)0x04,(char)0x05,(char)0x06,(char)0x07};	
	// ! ПОТЕНЦИАЛЬНАЯ ТОЧКА МОДИФИЦИРОВАНИЯ АЛГОРИТМА

	char rgbMVVector[NBITS];

	// Длина начального банка символов
	int bankLen = NBITS;

	// Счетчик количества записанных векторов для перемещения
	// "столбцов"
	cMVS = 0;

	// Рекурсивная функция генерирования множества векторов
	// для изменения порядка следования столбцов - 
	// заполняет каждый вектор значениями 0..7	
	GenerateMVS(rgbBank, bankLen, rgbMVVector);	
}

////////////////////////////////////////////////////////////////////////////////
// Ликвидирует данные переданного массива char из соображений 
// безопасности
void CInKey::WipeData(char *prgbData, long int lDataSize)
{
	long int i;

	for (i = 0; i < lDataSize; i++)
	{
		prgbData[i] = (char)0x00;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Возвращает состояние требуемого бита из переданного байта
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
// Устанавливает состояние требуемого бита в переданный байт,
// которое закодировано байтом. Возвращает модифицированный байт
// !!! Внимание !!! Перед наложением он должен быть установлен в 0!
inline char CInKey::Putbit(char byte, int nbit, char bit)
{
	if (bit != 0)
	{
		byte |= rgbBitMask[nbit];
	}

	return byte;
}

////////////////////////////////////////////////////////////////////////////////
// Перекодирование исходных данных в "строку" 
void CInKey::ConvertSourceToBitData(char *prgbSourceBuffer,
									long int lSourceSize, char *prgbBitData)
{
   long int i, j;

   int nbit;

   // Обрабатываем каждый из символов исходной гаммы...
   for (i = 0, j = 0; i < lSourceSize; i++)
   {
		 // В байте 8 бит, извлекаем каждый...
	   for (nbit = 0; nbit < NBITS; nbit++,j++)
	   {
		    prgbBitData[j] = Getbit(prgbSourceBuffer[i], nbit);		  
	   }	   
   }   
}

////////////////////////////////////////////////////////////////////////////////
// Перекодирование данных "строки" в блок исходных данных
void CInKey::ConvertBitDataToSource(char *prgbSourceBuffer,
								    long int lSourceSize, char *prgbBitData)
{
   long int i, j = 0;

   int nbit;

   // Учитываем довесок
   if (bitDataPosition != 0)
   {
		  j = NBITS * lSourceSize;
   }
   
   // Все исходные байты должны быть возвращены на места...
   for (i = 0; i < lSourceSize; i++)
   {
		// В байте 8 бит, возвращаем на место каждый...
	   for (nbit = 0; nbit < NBITS; nbit++,j++)
	   {		  		  
		    prgbSourceBuffer[i] = Putbit(prgbSourceBuffer[i], nbit, prgbBitData[j]);
	   }	   
   }	   
}
//////////////////////////////////////////////////////////////////////
// Вращает prgBitData (ликвидация циклов перемешивания)
void CInKey::RotateBitData (long int lSourceSize,char *prgbPasswordBuffer,
							char *prgbBitData, long int L, int mode)
{
	long int i, j;
	int sPartIndex;

	// Индекс, с которого начинается "вторая часть" данных
	sPartIndex = (unsigned char)prgbPasswordBuffer[L] + 1;

	// Направление вращения напрямую зависит от режима работы
	// программы - шифрование или расшифровка
	switch (mode)
	{
		case ENCRYPT:
		{		
				// Переносим "вторую часть" данных на первое место
				for (i = sPartIndex, j = 0; i < (NBITS * lSourceSize); i++, j++)
				{
					prgbBitData[(bitDataPosition ^ 0x01) * NBITS * lSourceSize + j] = prgbBitData[bitDataPosition * NBITS * lSourceSize + i];
				}

				// Переносим "первую часть" данных на второе место				
				for (i = (sPartIndex - 1); j < (NBITS * lSourceSize); i--, j++)
				{
					prgbBitData[(bitDataPosition ^ 0x01) * NBITS * lSourceSize + j] = prgbBitData[bitDataPosition * NBITS * lSourceSize + i];
				}

				// Перекидываем "указатель" на данные
				bitDataPosition ^= 0x01;

				break;
		}

		case DECRYPT:
		{		
				// Переносим "первую часть" данных на первое место
				for (i = (NBITS * lSourceSize - sPartIndex), j = (sPartIndex - 1); j >= 0; i++, j--)
				{
					prgbBitData[(bitDataPosition ^ 0x01) * NBITS * lSourceSize + j] = prgbBitData[bitDataPosition * NBITS * lSourceSize + i];
				}

				// Переносим "вторую часть" данных на второе место				
				for (i = 0, j = sPartIndex; j < (NBITS * lSourceSize); i++, j++)
				{
					prgbBitData[(bitDataPosition ^ 0x01) * NBITS * lSourceSize + j] = prgbBitData[bitDataPosition * NBITS * lSourceSize + i];
				}

				// Перекидываем "указатель" на данные
				bitDataPosition ^= 0x01;				
		}				
	}		
}

//////////////////////////////////////////////////////////////////////
// УСТАНАВЛИВАЕТ ИНДЕКС iMVS и РЕАЛИЗУЕТ ЕГО (перемешивание битов)
void CInKey::MixBitData(long int lSourceSize, char *prgbPasswordBuffer,
						char *prgbBitData, long int L, long int R, int mode)
{
	int iMVS; // Индекс в массиве указателей на char-векторы
			  // перестановок

	int nbit, nbit2 = 0;

	long int i, lIndexTarget, lIndexSource;

	// Этот union необходим для формирования индекса в MVS
	// Индекс складывается из двух байтов парольного блока
	// При следующей итерации происходит смещение "рамки считывания"
	// двух байт по буферу парольного файла
	static TCharToInt charToInt;

	// Инициализируем "исходный" блок данных в union-е
	// данными, взятыми из парольного блока данных	
	charToInt.rgbVariable[0] = prgbPasswordBuffer[L];
	charToInt.rgbVariable[1] = prgbPasswordBuffer[R];

	// Осуществляем преобразование Char[2] --> Int
	iMVS = charToInt.intVariable;

	// Нормализуем индекс ВЫБОРА ТЕКУЩЕГО ВЕКТОРА
	if (iMVS > (MVS_NUM - 1))
	{
		iMVS -= MVS_NUM;
	}

	// РЕАЛИЗУЕМ ВЕКТОР ПЕРЕСТАНОВКИ ДАННЫХ:
	// Необходимо ВСЕ    N-е   Б И Т Ы    исходного блока
	// поместить в  N-ый  "С Т О Л Б Е Ц"  блока-близнеца
	for (nbit = 0; nbit < NBITS; nbit++)
	{
		// N-ых битов столько, сколько байтов в блоке исходных данных
		for (i = 0; i < lSourceSize; i++)
		{
			// При переносе данных всегда пишем в массив-близнец
			// НОРМАЛЬНАЯ АДРЕСАЦИЯ - ЧТЕНИЕ ПО СТРОКАМ - nbit*lSourceSize+i
			// ТРАНСПОНИРОВАННАЯ АДРЕСАЦИЯ - ЧТЕНИЕ ПО СТОЛБЦАМ - nbit*i+lSourceSize			
			
			// Режим адресации напрямую зависит от режима работы программы -
			// шифрование или расшифровка
			switch (mode)
			{
				case ENCRYPT:
				{		
					// Берем по вектору - пишем подряд	
					lIndexSource = (long int)((prgubMVS[iMVS * NBITS + nbit]) * lSourceSize + i);	
					lIndexTarget = nbit * lSourceSize + i;

					break;
				}

				case DECRYPT:
				{
					// Берем подряд - пишем по вектору	
					lIndexSource = nbit * lSourceSize + i;
					lIndexTarget = (long int)prgubMVS[iMVS * NBITS + nbit] * lSourceSize + i;
				}				
			}		
				
			// Учитываем возможное смещение на вторую часть массива
			// "битовых" данных
			// Если читаем из второй части массива, то
			// bitDataPosition != 0, следовательно, необходимо
			// добавить "довесок" к "исходному" индексу...
			if (bitDataPosition != 0)
			{
				lIndexSource += (NBITS * lSourceSize);

			} else
			// ... а если bitDataPosition==0, то для 
			// корректной записи в более высокий слой
			// также вносим "довесок"
			{
				lIndexTarget += (NBITS * lSourceSize);
			}
				
			// Перемещаем данные
			prgbBitData[lIndexTarget] = (prgbBitData[lIndexSource] ^ Getbit(prgbPasswordBuffer[i], nbit2));

			nbit2++;

			if (nbit2 >= NBITS)
			{
				nbit2 = 0;
			}
		}
	}
	// "Перекидываем" указатель на один из двух логических подмассивов
	// в массиве prgbBitData
	bitDataPosition ^= (char)0x01;
}

//////////////////////////////////////////////////////////////////////
// Шифрование исходного блока данных парольным блоком данных
void CInKey::Encrypt(char *prgbSourceBuffer, long int lSourceSize,
					 char *prgbPasswordBuffer)
{	
	long int i;

	// Курсоры в блоке парольных данных
	long int L, R;

	char *prgbBitData = new char[2 * NBITS * lSourceSize];


	int mode = ENCRYPT;

    // ГЕНЕРИРУЕМ MVS - Move VectorS
	StartToGenerateMVS();

    // Изначально данные лежат в начале массива (после перекодирования)
    bitDataPosition = 0;

	// Курсор R в парольном буфере встает на свое место...
	R = (lSourceSize - 1);
	// а L - на свое.
	L = 0;

	// Перекодирование исходных данных в "битовое" отображение
	ConvertSourceToBitData(prgbSourceBuffer, lSourceSize, prgbBitData);

	// Очищаем буфер исходных данных, чтобы обеспечить корректное
	// наложение битовых данных из prgbBitData
	WipeData(prgbSourceBuffer, lSourceSize);

	////////////////////////////////////////
	// Обработка данных: ШИФРОВАНИЕ
	////////////////////////////////////////
	// Работаем до тех пор, пока все парольные данные не использованы
	for (i = 0; i < lSourceSize; i++)
	{	    
		// Установка вектора перестановок (циклический вызов приводит
		// к изменению порядка следования "битов")
		MixBitData(lSourceSize, prgbPasswordBuffer,
					prgbBitData, L, R, mode);

		// Вращаем битовые данные первым алгоритмом
		RotateBitData(lSourceSize, prgbPasswordBuffer,
	     				  prgbBitData, L, mode);

		L++;
				
		R--;

		// Обновляем прогресс
		if (UpdateProgress != NULL)
		{
			UpdateProgress(i);
		}
	}
	
	// Перекодирование данных из "битовой" формы в исходную
	ConvertBitDataToSource(prgbSourceBuffer, lSourceSize, prgbBitData);	

	// Уничтожаем данные...
	WipeData(prgbBitData, 2 * NBITS * lSourceSize);

	if (prgbBitData)
	{
		delete [] prgbBitData;
		prgbBitData = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Шифрование исходного блока данных парольным блоком данных
void CInKey::Decrypt(char *prgbSourceBuffer, long int lSourceSize,
					 char *prgbPasswordBuffer)
{	
	long int i;

	// Курсоры в блоке парольных данных
	long int L, R;

	char *prgbBitData = new char[2 * NBITS * lSourceSize];

	int mode = DECRYPT;

    // ГЕНЕРИРУЕМ MVS - Move VectorS
	StartToGenerateMVS();

    // Изначально данные лежат в начале массива (после перекодирования)
    bitDataPosition = 0;

	// Курсор R в парольном буфере встает на свое место...
	L = (lSourceSize - 1);
	// а L - на свое.
	R = 0;
			
	// Перекодирование исходных данных в "битовое" отображение
	ConvertSourceToBitData(prgbSourceBuffer, lSourceSize, prgbBitData);

	// Очищаем буфер исходных данных, чтобы обеспечить корректное
	// наложение битовых данных из prgbBitData
	WipeData(prgbSourceBuffer, lSourceSize);

	////////////////////////////////
	// Обработка данных: РАСШИФРОВКА
	////////////////////////////////
	// Работаем до тех пор, пока все парольные данные не использованы
	for (i = 0; i < lSourceSize; i++)
	{		
		// Вращаем битовые данные первым алгоритмом
		RotateBitData(lSourceSize, prgbPasswordBuffer,
	     				  prgbBitData, L, mode);

		// Установка вектора перестановок (циклический вызов приводит
		// к изменению порядка следования "битов")
		MixBitData(lSourceSize, prgbPasswordBuffer,
					prgbBitData, L, R, mode);

		L--; 
		
		R++;

		// Обновляем прогресс
		if (UpdateProgress != NULL)
		{
			UpdateProgress(i);
		}
	}

	// Перекодирование данных из "битовой" формы в исходную
	ConvertBitDataToSource(prgbSourceBuffer, lSourceSize, prgbBitData);

	// Уничтожаем данные...
	WipeData(prgbBitData, 2 * NBITS * lSourceSize);

	if (prgbBitData)
	{
		delete [] prgbBitData;
		prgbBitData = NULL;
	}
}