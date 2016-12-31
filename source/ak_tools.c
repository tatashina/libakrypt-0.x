/* ----------------------------------------------------------------------------------------------- */
/*   Copyright (c) 2014 - 2017 by Axel Kenzo, axelkenzo@mail.ru                                    */
/*   All rights reserved.                                                                          */
/*                                                                                                 */
/*   Redistribution and use in source and binary forms, with or without modification, are          */
/*   permitted provided that the following conditions are met:                                     */
/*                                                                                                 */
/*   1. Redistributions of source code must retain the above copyright notice, this list of        */
/*      conditions and the following disclaimer.                                                   */
/*   2. Redistributions in binary form must reproduce the above copyright notice, this list of     */
/*      conditions and the following disclaimer in the documentation and/or other materials        */
/*      provided with the distribution.                                                            */
/*   3. Neither the name of the copyright holder nor the names of its contributors may be used     */
/*      to endorse or promote products derived from this software without specific prior written   */
/*      permission.                                                                                */
/*                                                                                                 */
/*   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   */
/*   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               */
/*   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL        */
/*   THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, */
/*   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE */
/*   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    */
/*   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     */
/*   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  */
/*   OF THE POSSIBILITY OF SUCH DAMAGE.                                                            */
/*                                                                                                 */
/*   ak_tools.c                                                                                    */
/* ----------------------------------------------------------------------------------------------- */
 #include <libakrypt.h>
 #include <ak_tools.h>
 #include <ak_oid.h>
 #include <ak_hash.h>
 #include <ak_skey.h>
 #include <ak_keylist.h>
 #include <ak_curves.h>

 #include <errno.h>
 #include <fcntl.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdarg.h>

 #ifndef _MSC_VER
  #ifndef __MINGW32__
   #include <syslog.h>
  #endif
  #include <unistd.h>
 #endif

 #ifndef _WIN32
  #include <unistd.h>
 #else
  #include <windows.h>
 #endif

 #include <sys/stat.h>
 #ifdef _MSC_VER
  #include <io.h>
  #include <conio.h>
 #endif

/* ----------------------------------------------------------------------------------------------- */
/*!  Переменная, содержащая в себе код последней ошибки                                            */
 static int ak_errno = ak_error_ok;

/* ----------------------------------------------------------------------------------------------- */
/*! Внутренний указатель на функцию аудита                                                         */
 static ak_function_log *ak_function_log_default = NULL;
 static pthread_mutex_t ak_function_log_default_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ----------------------------------------------------------------------------------------------- */
 int ak_error_set_value( const int value )
{
  return ( ak_errno = value );
}

/* ----------------------------------------------------------------------------------------------- */
 int ak_error_get_value( void )
{
  return ak_errno;
}

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Явное определение функции аудита

   Функция устанавливает в качестве основного обработчика
   вывода сообщений функцию, задаваемую указателем function. Если аргумент function равен NULL,
   то используется функция по-умолчанию.
   Выбор функции по умолчанию зависит от операционной системы, под ОС Linux это вывод с
   использованием демона syslogd.

   @param function указатель на функцию вывода сообщений
   @return функция всегда возвращает ak_error_ok (ноль).                                           */
/* ----------------------------------------------------------------------------------------------- */
 int ak_log_set_function( ak_function_log *function )
{
  pthread_mutex_lock( &ak_function_log_default_mutex );
  if( function != NULL ) ak_function_log_default = function;
   else {
    #ifdef __linux__
      ak_function_log_default = ak_function_log_syslog;
    #else
      ak_function_log_default = ak_function_log_stderr;
    #endif
   }
  pthread_mutex_unlock( &ak_function_log_default_mutex );
  return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция использует установленную ранее функцию-обработчик сообщений. Если сообщение,
    или обработчик не определены (равны NULL) возвращается код ошибки.
    @param message выводимое сообщение
    @return в случае успеха, возвращается ak_error_ok (ноль). В случае возникновения ошибки,
    возвращается ее код.                                                                           */
/* ----------------------------------------------------------------------------------------------- */
 int ak_log_set_message( const char *message )
{
  int result = 0;
  if( ak_function_log_default == NULL ) return ak_error_set_value( ak_error_null_pointer );
  if( message == NULL ) {
    ak_error_message( ak_error_null_pointer, "using a NULL string for message", __func__ );
  } else {
       pthread_mutex_lock( &ak_function_log_default_mutex );
       result = ak_function_log_default( message );
       pthread_mutex_unlock( &ak_function_log_default_mutex );
      return result;
    }
 return ak_error_ok;
}

#ifdef __linux__
/* ----------------------------------------------------------------------------------------------- */
/*! @param message выводимое сообщение
    @return в случае успеха, возвращается ak_error_ok (ноль). В случае возникновения ошибки,
    возвращается ее код.                                                                           */
/* ----------------------------------------------------------------------------------------------- */
 int ak_function_log_syslog( const char *message )
{
  if( message != NULL ) syslog(  LOG_AUTHPRIV|LOG_NOTICE, "%s", message );
 return ak_error_ok;
}
#endif

/* ----------------------------------------------------------------------------------------------- */
/*! @param message выводимое сообщение
    @return в случае успеха, возвращается ak_error_ok (ноль). В случае возникновения ошибки,
    возвращается ее код.                                                                           */
/* ----------------------------------------------------------------------------------------------- */
 int ak_function_log_stderr( const char *message )
{
  if( message != NULL ) fprintf( stderr, "%s\n", message );
 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param str Строка, в которую помещается результат (сообщение)
    @param size Максимальный размер помещаемого в строку str сообщения
    @param format Форматная строка, в соответствии с которой формируется сообщение                 */
/* ----------------------------------------------------------------------------------------------- */
 int ak_snprintf( char *str, size_t size, const char *format, ... )
{
  int result = 0;
  va_list args;
  va_start( args, format );

 #ifdef _MSC_VER
  #if _MSC_VER > 1310
    result = _vsnprintf_s( str, size, size, format, args );
  #else
    result = _vsnprintf( str, size, format, args );
  #endif
 #else
  result = vsnprintf( str, size, format, args );
 #endif
 return result;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция формирует строку специального вилда и выводит ее в логгер с помомощью функции
    ak_log_set_message()

    @param code код ошибки
    @param message читаемое (понятное для пользователя) сообщение
    @param function имя функции, вызвавшей ошибку                                                  */
/* ----------------------------------------------------------------------------------------------- */
 void ak_error_message( const int code, const char *function, const char *message )
{
 /* здесь мы выводим в логгер строку вида [pid] function: message (code: n)                        */
  char error_event_string[1024];
  memset( error_event_string, 0, 1024 );
#ifdef _MSC_VER
  ak_snprintf( error_event_string, 1023, "[%d] %s(): %s (code: %d)",
                                                GetCurrentProcessId(), function, message, code );
#else
  ak_snprintf( error_event_string, 1023, "[%d] %s(): %s (code: %d)",
                                                             getpid(), function, message, code );
#endif
  ak_log_set_message( error_event_string );
  ak_error_set_value( code );
}


/* ----------------------------------------------------------------------------------------------- */
/*! @param code код ошибки
    @param function имя функции, вызвавшей ошибку
    @param format Форматная строка, в соответствии с которой формируется сообщение                 */
/* ----------------------------------------------------------------------------------------------- */
 void ak_error_message_fmt( const int code, const char *function, const char *format, ... )
{
  char message[256];
  va_list args;
  va_start( args, format );
  memset( message, 0, 256 );

 #ifdef _MSC_VER
  #if _MSC_VER > 1310
    _vsnprintf_s( message, 256, 256, format, args );
  #else
    _vsnprintf( message, 256, format, args );
  #endif
 #else
   vsnprintf( message, 256, format, args );
 #endif
  ak_error_message( code, function, message );
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция рассматривает область памяти, на которую указывает указатель ptr, как массив
    последовательно записанных байт фиксированной длины, создает в оперативной памяти строку и
    последовательно выводит в нее значения, хранящиеся в заданной области памяти.
    Значения выводятся в шестнадцатеричной системе счисления.

    Пример использования.
  \code
    ak_uint8 data[5] = { 1, 2, 3, 4, 5 };
    ak_uint8 *str = ak_pointer_to_hexstr( data, 5, ak_false );
    printf("%s\n", str );
    free(str);
  \endcode

    @param ptr Указатель на область памяти
    @param size Размер области памяти (в байтах)
    @param reverse Последовательность вывода байт в строку. Если reverse равно \ref ak_false,
    то байты выводятся начиная с младшего к старшему.  Если reverse равно \ref ak_true, то байты
    выводятся начиная от старшего к младшему (такой способ вывода принят при стандартном выводе
    чисел: сначала старшие разряды, потом младшие).

    @return Функция возвращает указатель на созданную строку, которая должна быть позднее удалена
    пользователем с помощью вызова функции free(). В случае ошибки конвертации возвращается NULL.
    Код ошибки может быть получен с помощью вызова функции ak_error_get_code()                     */
/* ----------------------------------------------------------------------------------------------- */
 char *ak_ptr_to_hexstr( const ak_pointer ptr, const size_t ptr_size, const ak_bool reverse )
{
  char *nullstr = NULL;
  size_t len = 1 + (ptr_size << 1);

  if( ptr == NULL ) {
    ak_error_message( ak_error_null_pointer, "use a null pointer to data", __func__ );
    return NULL;
  }
  if( ptr_size <= 0 ) {
    ak_error_message( ak_error_zero_length, "use a data with zero or negative length", __func__ );
    return NULL;
  }

  if(( nullstr = (char *) malloc( len )) == NULL ) {
    ak_error_message( ak_error_out_of_memory, "incorrect memory allocation", __func__ );
  }
    else {
      size_t idx = 0, js = 0, start = 0, offset = 2;
      ak_uint8 *data = ( ak_uint8 * ) ptr;

      memset( nullstr, 0, len );
      if( reverse ) { // движение в обратную сторону - от старшего байта к младшему
        start = len-3; offset = -2;
      }
      for( idx = 0, js = start; idx < ptr_size; idx++, js += offset ) {
        char str[4];
        ak_snprintf( str, 3, "%02X", data[idx] );
        memcpy( nullstr+js, str, 2 );
      }
    }
 return nullstr;
}

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Конвертация символа в целочисленное значение                                            */
/* ----------------------------------------------------------------------------------------------- */
 inline static ak_uint32 ak_xconvert( const char c )
{
    switch( c )
   {
      case 'a' :
      case 'A' : return 10;
      case 'b' :
      case 'B' : return 11;
      case 'c' :
      case 'C' : return 12;
      case 'd' :
      case 'D' : return 13;
      case 'e' :
      case 'E' : return 14;
      case 'f' :
      case 'F' : return 15;
      case '0' : return 0;
      case '1' : return 1;
      case '2' : return 2;
      case '3' : return 3;
      case '4' : return 4;
      case '5' : return 5;
      case '6' : return 6;
      case '7' : return 7;
      case '8' : return 8;
      case '9' : return 9;
      default : ak_error_set_value( ak_error_undefined_value ); return 0;
 }
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция преобразует строку символов, содержащую последовательность шестнадцатеричных цифр
    в массив данных.

    @param hexstr Строка символов.
    @param ptr Указатель на область памяти (массив) в которую будут размещаться данные.
    @param size Максимальный размер памяти (в байтах), которая может быть помещена в массив.
    Если исходная строка требует больший размер, то возбуждается ошибка.
    @param reverse Последовательность считывания байт в память. Если reverse равно \ref ak_false
    то первые байты строки (с младшими индексами) помещаются в младшие адреса, а старшие байты -
    в старшие адреса памяти. Если reverse равно \ref ak_true, то производится разворот,
    то есть обратное преобразование при котором элементы строки со старшиси номерами помещаются
    в младшие разряды памяти (такое представление используется при считывании больших целых чисел).

    @return В случае успеха возвращается ноль. В противном случае, в частности,
                      когда длина строки превышает размер массива, возвращается код ошибки.        */
/* ----------------------------------------------------------------------------------------------- */
 int ak_hexstr_to_ptr( const char *hexstr, ak_pointer ptr, const size_t size, const ak_bool reverse )
{
  int i = 0;
  ak_uint8 *bdata = ptr;
  size_t len = 0;

  if( hexstr == NULL ) {
    ak_error_message( ak_error_null_pointer, "using null pointer to a hex string", __func__ );
    return ak_error_null_pointer;
  }
  if( ptr == NULL ) {
    ak_error_message( ak_error_null_pointer, "using null pointer to a buffer", __func__ );
    return ak_error_null_pointer;
  }
  if( size == 0 ) {
    ak_error_message( ak_error_zero_length, "using zero value for length of buffer", __func__ );
    return ak_error_zero_length;
  }

  len = strlen( hexstr );
  if( len&1 ) len++;
  len >>= 1;
  if( size < len ) {
    ak_error_message( ak_error_wrong_length, "using a buffer with small length", __func__ );
    return ak_error_wrong_length;
  }

  memset( ptr, 0, size ); // перед конвертацией мы обнуляем исходные данные
  ak_error_set_value( ak_error_ok );
  if( reverse ) {
    for( i = strlen( hexstr )-2, len = 0; i >= 0 ; i -= 2, len++ ) {
       bdata[len] = (ak_xconvert( hexstr[i] ) << 4) + ak_xconvert( hexstr[i+1] );
    }
    if( i == -1 ) bdata[len] = ak_xconvert( hexstr[0] );
  }	else {
        for( i = 0, len = 0; i < strlen( hexstr ); i += 2, len++ ) {
           bdata[len] = (ak_xconvert( hexstr[i] ) << 4);
           if( i < strlen( hexstr )-1 ) bdata[len] += ak_xconvert( hexstr[i+1] );
        }
    }
 return ak_error_get_value();
}

/* ----------------------------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------------------------- */
/*! \brief Тип данных для хранения значений опций библиотеки */
static struct libakrypt_options_ctx {
 /*! \brief Уровень вывода сообщений выполнения функций библиотеки */
  int log_level;
 /*! \brief Ресурс использования ключа (в блоках) алгоритма ГОСТ 28147-89 (Магма) */
  ak_uint32 cipher_key_magma_block_resource;
 /*! \brief Ресурс использования ключа (в блоках) алгоритма ГОСТ 34.12-2015 (Кузнечик) */
  ak_uint32 cipher_key_kuznetchik_block_resource;
} libakrypt_options =
{
  ak_log_standard,
  4194304, /* количество блоков для ГОСТ 28147-89, для объема в 250 Mb (по 64 бита) */
  8388608  /* количество блоков для ГОСТ 34.12-2015, для объема в 1Gb (по 128 бит) */
};

/* ----------------------------------------------------------------------------------------------- */
 int ak_log_get_level( void ) { return libakrypt_options.log_level; }

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Установка уровня аудита библиотеки.

  Все сообщения библиотеки могут быть разделены на три уровня.

  \li Первый уровень аудита определяется константой ak_log_none. На этом уровне выводятся
  сообщения об ошибках, а также минимальный набор сообщений, включающий в себя факт
  успешного тестирования работоспособности криптографических механизмов.

  \li Второй уровень аудита определяется константой ak_log_standard. На этом уровене
  выводятся все сообщения из первого уровня, а также сообщения о фактах использования
  ключевой информации.

  \li Третий (максимальный) уровень аудита определяется константой ak_log_maximum.
  На этом уровне выводятся все сообщения, доступные на первых двух уровнях, а также
  сообщения отладосного характера, позхволяющие прослдедить логику работы функций библиотеки.

  @param level Уровень аудита, может принимать значения ak_log_none,
  ak_log_standard и ak_log_maximum.

  @return функция всегда возвращает ak_error_ok (ноль).                                           */
/* ----------------------------------------------------------------------------------------------- */
int ak_log_set_level( int level )
{
 if( level >= ak_log_maximum ) {
   libakrypt_options.log_level = ak_log_maximum;
   return ak_error_ok;
 }
 if( level <= ak_log_none ) {
   libakrypt_options.log_level = ak_log_none;
   return ak_error_ok;
 }
 libakrypt_options.log_level = ak_log_standard;
return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция возвращает ресурс секретного ключа для алгоритма ГОСТ 28147-89 (Магма) */
ak_uint32 ak_libakrypt_get_magma_resource( void )
{
 return libakrypt_options.cipher_key_magma_block_resource;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция возвращает ресурс секретного ключа для алгоритма ГОСТ 34.12-2015 (Кузнечик) */
ak_uint32 ak_libakrypt_get_kuznetchik_resource( void )
{
 return libakrypt_options.cipher_key_kuznetchik_block_resource;
}

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция создает имя файла в котором содержатся настройки библиотеки.

   @param filename Массив, куда помещается имя файла. Память под массив
          должна быть быделена заранее.
   @param size Размер выделенной памяти.
   @return Функция возвращает код ошибки.                                                         */
/* ----------------------------------------------------------------------------------------------- */
int ak_libakrypt_create_filename_for_options( char *filename, size_t size )
{
 char drive[FILENAME_MAX], hpath[FILENAME_MAX], append[FILENAME_MAX];

 memset( (void *)filename, 0, size );
 memset( (void *)drive, 0, FILENAME_MAX );
 memset( (void *)hpath, 0, FILENAME_MAX );
 memset( (void *)append, 0, FILENAME_MAX );

#ifdef LIBAKRYPT_OPTIONS_PATH
{
 /* здесь мы обрабатываем путь, заданный из командной строки при сборке библиотеки */
 size_t len = 0;
 if(( len = strlen( LIBAKRYPT_OPTIONS_PATH )) > FILENAME_MAX-16 ) {
   ak_error_message( ak_error_wrong_length, "wrong length of predefined filepath", __func__ );
   return ak_error_wrong_length;
 }
 memcpy( hpath, LIBAKRYPT_OPTIONS_PATH, len ); /* массивы drive и append остаются пустыми */
}
#else
 /* здесь обработка пути к файлу вручную */
 #ifdef _WIN32
  /* в начале определяем, находимся ли мы в консоли MSys */
  GetEnvironmentVariableA( "HOME", hpath, FILENAME_MAX-1 );
  /* если мы находимся не в консоли, то строка hpath должна быть пустой */
  if( strlen(hpath) == 0 ) {
    //GetEnvironmentVariableA( "HOMEDRIVE", drive, FILENAME_MAX );
    //GetEnvironmentVariableA( "HOMEPATH", hpath, FILENAME_MAX );
    GetEnvironmentVariableA( "APPDATA", drive, FILENAME_MAX-1 );
    strncpy( append, "\\libakrypt", 33 );
  } else strncpy( append, "/.config/libakrypt", 18 );
 #else
  ak_snprintf( hpath, FILENAME_MAX, "%s/.config/libakrypt", getenv( "HOME" ));
 #endif
#endif

/* собираем имя файла из нескольких фрагментов */
 #ifdef _WIN32
  ak_snprintf( filename, size, "%s%s%s\\libakrypt.conf", drive, hpath, append );
 #else
  ak_snprintf( filename, size, "%s%s%s/libakrypt.conf", drive, hpath, append );
 #endif
return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
ak_bool ak_libakrypt_load_options( void )
{
 char ch;
 size_t idx;
 int fd = 0, off = 0;
 struct stat st;
 char *ptr = NULL;
 char localbuffer[1024], filename[FILENAME_MAX];

/* создаем имя файла */
 if( ak_libakrypt_create_filename_for_options( filename, FILENAME_MAX ) != ak_error_ok ) {
   ak_error_message( ak_error_get_value(), "wrong creation options filename", __func__ );
   return ak_false;
 }

/* проверяем наличие файла и прав доступа к нему */
 if(( fd = open( filename, O_RDONLY | O_BINARY )) < 0 ) {
   ak_error_message_fmt( ak_error_open_file,
                                         __func__, "%s for file %s", strerror( errno ), filename );
   return ak_false;
 }
 if( fstat( fd, &st ) ) {
   close( fd );
   ak_error_message( ak_error_access_file, strerror( errno ), __func__ );
   return ak_false;
 }
/* считываем данные и побайтно формуруем строки с  */
 memset( localbuffer, 0, 1024 );
 for( idx = 0; idx < (size_t) st.st_size; idx++ ) {
   if( read( fd, &ch, 1 ) != 1 ) {
     ak_error_message( ak_error_read_data, "unexpected end of libakrypt.conf", __func__ );
     close(fd);
     return ak_false;
   }
   if( off > 1022 ) {
     ak_error_message( ak_error_read_data,
                              "libakrypt.conf has a line with more than 1022 symbols", __func__ );
     close(fd);
     return ak_false;
   }
   if( ch == 10 ) {
    /* мы обрабатываем только ненулевые строки без символа # */
     if(( off != 0 ) && ( strchr( localbuffer, '#' ) == NULL )) {
       /* проверяем уровень аудита */
       if(( ptr = strstr( localbuffer, "log_level = " )) != NULL ) {
         char *endptr = NULL;
         int value = (int) strtol( ptr+12, &endptr, 10 );
         if( *endptr != '\0' ) {
           ak_error_message( ak_error_undefined_value,
                                            "unexpected value for variable log_level", __func__ );
           close(fd);
           return ak_false;
         }
         if( value < ak_log_none || value > ak_log_maximum ) {
           ak_error_message( ak_error_undefined_value,
                                                 "wrong value for variable log_level", __func__ );
           close(fd);
           return ak_false;
         }
         libakrypt_options.log_level = value;
       }
       /* проверяем ресурс для Магмы */
       if(( ptr = strstr( localbuffer, "magma_block_resource = " )) != NULL ) {
         char *endptr = NULL;
         long int value = strtol( ptr+23, &endptr, 10 );
         if( *endptr != '\0' ) {
           ak_error_message( ak_error_undefined_value,
                                 "unexpected value for variable magma_block_resource", __func__ );
           close(fd);
           return ak_false;
         }
         if( value < 1024 || value > 0xFFFFFFFF ) {
           ak_error_message( ak_error_undefined_value,
                                      "wrong value for variable magma_block_resource", __func__ );
           close(fd);
           return ak_false;
         }
         libakrypt_options.cipher_key_magma_block_resource = value;
         if( libakrypt_options.log_level == ak_log_maximum ) ak_error_message( ak_error_ok,
                                  "reading a new value of block_magma_resoure is Ok", __func__ );
       }
       /* проверяем ресурс для Кузнечика */
       if(( ptr = strstr( localbuffer, "kuznetchik_block_resource = " )) != NULL ) {
         char *endptr = NULL;
         long int value = strtol( ptr+28, &endptr, 10 );
         if( *endptr != '\0' ) {
           ak_error_message( ak_error_undefined_value,
                           "unexpected value for variable kuznetchik_block_resource", __func__ );
           close(fd);
           return ak_false;
         }
         if( value < 1024 || value > 0xFFFFFFFF ) {
           ak_error_message( ak_error_undefined_value,
                                "wrong value for variable kuznetchik_block_resource", __func__ );
           close(fd);
           return ak_false;
         }
         libakrypt_options.cipher_key_kuznetchik_block_resource = value;
         if( libakrypt_options.log_level == ak_log_maximum ) ak_error_message( ak_error_ok,
                             "reading a new value of block_kuznetchik_resoure is Ok", __func__ );
       }
     }
     memset( localbuffer, 0, 1024 );
     off = 0;
   } else localbuffer[off++] = ch;
 }
/* закрываем все и выходим */
 close(fd);
return ak_true;
}

/* ----------------------------------------------------------------------------------------------- */
const char *ak_libakrypt_version( void )
{
#ifdef LIBAKRYPT_VERSION
  return LIBAKRYPT_VERSION;
#else
  return "0.4";
#endif
}

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция проверяет корректность определения базовых типов данных
   @return В случе успешного тестирования возвращает ak_true (истина).
   В противном случае возвращается ak_false.                                                      */
/* ----------------------------------------------------------------------------------------------- */
ak_bool ak_libakrypt_test_types( void )
{
 if( sizeof( ak_int8 ) != 1 ) {
  ak_error_message( ak_error_undefined_value, "wrong size of ak_int8 type", __func__ );
  return ak_false;
 }
 if( sizeof( ak_uint8 ) != 1 ) {
  ak_error_message( ak_error_undefined_value, "wrong size of ak_uint8 type", __func__ );
  return ak_false;
 }
 if( sizeof( ak_int32 ) != 4 ) {
  ak_error_message( ak_error_undefined_value, "wrong size of ak_int32 type", __func__ );
  return ak_false;
 }
 if( sizeof( ak_uint32 ) != 4 ) {
  ak_error_message( ak_error_undefined_value, "wrong size of ak_uint32 type", __func__ );
  return ak_false;
 }
 if( sizeof( ak_int64 ) != 8 ) {
  ak_error_message( ak_error_undefined_value, "wrong size of ak_int64 type", __func__ );
  return ak_false;
 }
 if( sizeof( ak_uint64 ) != 8 ) {
  ak_error_message( ak_error_undefined_value, "wrong size of ak_uint64 type", __func__ );
  return ak_false;
 }

#ifdef LIBAKRYPT_HAVE_BUILTIN_XOR_SI128
 if( ak_log_get_level() >= ak_log_maximum )
    ak_error_message( ak_error_ok, "library applies __m128i base type", __func__ );
#endif
return ak_true;
}

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция проверяет корректность реализации алгоритмов хеширования
@return Возвращает ak_true в случае успешного тестирования. В случае возникновения ошибки
функция возвращает ak_false. Код ошибки можеть быть получен с помощью вызова ak_error_get_value() */
/* ----------------------------------------------------------------------------------------------- */
ak_bool ak_libakrypt_test_hash_functions( void )
{
 int audit = ak_log_get_level();
 if( audit >= ak_log_maximum )
   ak_error_message( ak_error_ok, "testing hash functions started", __func__ );

 /* тестируем функцию ГОСТ Р 34.11-94 */
 if( ak_hash_test_gosthash94() != ak_true ) {
   ak_error_message( ak_error_get_value(), "incorrect gosthash94 testing", __func__ );
   return ak_false;
 }

 /* тестируем функцию Стрибог256 */
 if( ak_hash_test_streebog256() != ak_true ) {
   ak_error_message( ak_error_get_value(), "incorrect streebog256 testing", __func__ );
   return ak_false;
 }

 /* тестируем функцию Стрибог512 */
 if( ak_hash_test_streebog512() != ak_true ) {
   ak_error_message( ak_error_get_value(), "incorrect streebog512 testing", __func__ );
   return ak_false;
 }

 /* тестируем функцию SHA-256 */
 if( ak_hash_test_sha256() != ak_true ) {
   ak_error_message( ak_error_get_value(), "incorrect sha256 testing", __func__ );
   return ak_false;
 }

 /* тестируем функцию SHA-512 */
 if( ak_hash_test_sha512() != ak_true ) {
   ak_error_message( ak_error_get_value(), "incorrect sha512 testing", __func__ );
   return ak_false;
 }

 if( audit >= ak_log_maximum )
   ak_error_message( ak_error_ok, "testing hash functions ended successfully", __func__ );
return ak_true;
}

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция проверяет корректность реализации алгоритмов лочного шифрования
@return Возвращает ak_true в случае успешного тестирования. В случае возникновения ошибки
функция возвращает ak_false. Код ошибки можеть быть получен с помощью вызова ak_error_get_value() */
/* ----------------------------------------------------------------------------------------------- */
ak_bool ak_libakrypt_test_block_ciphers( void )
{
 int audit = ak_log_get_level();
 if( audit >= ak_log_maximum )
   ak_error_message( ak_error_ok, "testing block ciphers started", __func__ );

 /* тестируем алгоритм Магма (ГОСТ 28147-89, ГОСТ Р 34.12-2015) */
 if( ak_cipher_key_test_magma() != ak_true ) {
   ak_error_message( ak_error_get_value(), "incorrect block cipher marma testing", __func__ );
   return ak_false;
 }

 /* вырабатываем долговременные параметры алгоритма Кузнечик */
 if( ak_crypt_kuznetchik_init_tables() != ak_error_ok ) {
   ak_error_message( ak_error_get_value(),
                                 "wrong creation of kuznetchik predefined tables", __func__ );
   return ak_false;
 }

 /* тестируем алгоритм Кузнечик (ГОСТ Р 34.12-2015) */
 if( ak_cipher_key_test_kuznetchik() != ak_true ) {
   ak_error_message( ak_error_get_value(), "incorrect block cipher kuznetchik testing", __func__ );
   return ak_false;
 }

 if( audit >= ak_log_maximum )
   ak_error_message( ak_error_ok, "testing block ciphers ended successfully", __func__ );
return ak_true;
}

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция выводит в файл аудита значения параметров эллиптической кривой                  */
/* ----------------------------------------------------------------------------------------------- */
void ak_libakrypt_wcurve_to_log( const ak_wcurve_paramset ecp )
{
char message[160];

 ak_snprintf( message, 158, " a = %s", ecp->ca );
 ak_log_set_message( message );
 ak_snprintf( message, 158, " b = %s", ecp->cb );
 ak_log_set_message( message );
 ak_snprintf( message, 158, " p = %s", ecp->cp );
 ak_log_set_message( message );
 ak_snprintf( message, 158, " q = %s", ecp->cq );
 ak_log_set_message( message );
 ak_snprintf( message, 158, "px = %s", ecp->cpx );
 ak_log_set_message( message );
 ak_snprintf( message, 158, "py = %s", ecp->cpy );
 ak_log_set_message( message );

}

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция проверяет корректность реализации операций на эллиптических кривых в короткой форме Вейерштрасса

 Проверяются все параметры эллиптических кривых, доступные через механизм OID.
 Проверяется отличие дискриминанта кривой от нуля, принадлежность заданной точки заданной
 эллиптической кривой, а также проверяется порядок заданной точки.

 @return Возвращает ak_true в случае успешного тестирования. В случае возникновения
 ошибки функция возвращает ak_false. Код ошибки можеть быть получен с помощью вызова
 ak_error_get_value().                                                                            */
/* ----------------------------------------------------------------------------------------------- */
ak_bool ak_libakrypt_test_wcurves( void )
{
 size_t idx = 0;
 ak_bool result = ak_true;
 int audit = ak_log_get_level();
 if( audit >= ak_log_maximum )
   ak_error_message( ak_error_ok, "testing Weierstrass curves started", __func__ );

/* перебираем все кривые, которые доступны через механизм OIDэов библиотеки */
 for( idx = 0; idx < ak_oids_get_count(); idx++ ) {
    ak_oid oid = ak_oids_get_oid( idx );
    if( ak_oid_get_mode( oid ) == wcurve_params ) {
      const ak_wcurve_paramset ecp = (const ak_wcurve_paramset) oid->data;
      ak_wcurve ec = ak_wcurve_new(ecp);
      ak_wpoint wp = ak_wpoint_new(ecp);

      /* исправить!!!

if( audit >= ak_log_maximum ) {
 ak_snprintf( message, 1022, "curve %s [OID: %s]", ak_oid_get_name(oid), ak_oid_get_id(oid));
 ak_error_message( ak_error_ok, message, __func__ );
}

if(( result = ak_wcurve_is_ok( ec )) == ak_true ) {
 if( ak_wpoint_is_ok( wp, ec )) {
   if( ak_wpoint_check_order( wp, ec )) {
      if( audit >= ak_log_maximum ) {
        ak_error_message( ak_error_ok, "checked properly, curve is Ok", __func__ );
      }
   } else { ak_libakrypt_wcurve_to_log( ecp );
            ak_error_message( result = ak_error_wcurve_point_order,
                           "incorrect order of elliptic curve's base point", __func__ );
           }
 } else { ak_libakrypt_wcurve_to_log( ecp );
          ak_error_message( result = ak_error_wcurve_point,
                                   "incorrect base point of elliptic curve", __func__ );
        }
} else { ak_libakrypt_wcurve_to_log( ecp );
        ak_error_message( result = ak_error_wcurve_discriminant,
                        "incorrect value of elliptic curve's' discriminant", __func__ );
      }
}
}


      */



      wp = ak_wpoint_delete( wp );
      ec = ak_wcurve_delete( ec );
      if( result != ak_true ) break;
    }
 } /* end of for */

 if( audit >= ak_log_maximum )
   ak_error_message( ak_error_get_value(), "testing Weierstrass curves ended successfully", __func__ );
return result;
}


/* ----------------------------------------------------------------------------------------------- */
/*! Функция должна вызываться перед использованием
   криптографических механизмов библиотеки.

   Пример использования функции.

   \code
    int main( void )
   {
     if( ak_libakrypt_create( NULL ) != ak_true ) {
       // инициализация выполнена не успешна => выход из программы
       return ak_libakrypt_destroy();
     }

     // ... здесь код программы ...

    return ak_libakrypt_destroy();
   }
   \endcode

   \param logger казатель на функцию аудита. Может быть равен NULL.
   \return Функция возвращает ak_true (истина) в случае успешной инициализации и тестирования
   библиотеки. В противном случае, возвращается ak_false. Код ошибки может быть получен с помощью
   вызова функции ak_error_get_value()                                                            */
/* ----------------------------------------------------------------------------------------------- */
int ak_libakrypt_create( ak_function_log *logger )
{
 int error = ak_true;
 ak_error_set_value( ak_error_ok );

 /* инициализируем систему аудита (вывод сообщений) */
 if( ak_log_set_function( logger ) != ak_error_ok ) {
   ak_error_message( ak_error_get_value(), "audit mechanism not started", __func__ );
   return ak_false;
 }
 /* считываем настройки криптографических алгоритмов */
 if(( error = ak_libakrypt_load_options()) != ak_true ) {
   ak_error_message( ak_error_get_value(), "wrong reading of libakrypt options", __func__ );
   return ak_false;
 }
 /* проверяем длины фиксированных типов данных */
 if(( error = ak_libakrypt_test_types( )) != ak_true ) {
   ak_error_message( ak_error_get_value(), "sizes of predefined types is wrong", __func__ );
   return ak_false;
 }
 /* инициализируем механизм обработки идентификаторов */
 if(( error = ak_oids_create()) != ak_error_ok ) {
   ak_error_message( ak_error_get_value(), "OID mechanism not started", __func__ );
   return ak_false;
 }
 /* инициализируем механизм обработки секретных ключей пользователей */
//  if(( error = ak_keylist_create()) != ak_error_ok ) {
//    ak_error_message( ak_error_get_value(), "secret key control mechanism not started", __func__ );
//    return ak_false;
//  }
 /* тестируем работу функций хеширования */
 if(( error = ak_libakrypt_test_hash_functions()) != ak_true ) {
   ak_error_message( ak_error_get_value(), "error while testing hash functions", __func__ );
   return ak_false;
 }
 /* тестируем работу алгоритмов блочного шифрования */
 if(( error = ak_libakrypt_test_block_ciphers()) != ak_true ) {
   ak_error_message( ak_error_get_value(), "error while testing block ciphers", __func__ );
   return ak_false;
 }
 /* тестируем корректность реализации операций с эллиптическими кривыми в короткой форме Вейерштрасса */
 if(( error = ak_libakrypt_test_wcurves()) != ak_true ) {
   ak_error_message( ak_error_get_value(), "error while testing Wierstrass curves", __func__ );
   return ak_false;
 }

 ak_error_message( ak_error_ok, "all crypto mechanisms tested successfully", __func__ );
return ak_true;
}

/* ----------------------------------------------------------------------------------------------- */
int ak_libakrypt_destroy( void )
{
 int error = ak_error_get_value();
 if( error != ak_error_ok )
   ak_error_message( error, "before destroing library holds an error", __func__ );

//  ak_error_set_value( ak_error_ok );
//  if( ak_keylist_destroy() != ak_error_ok )
//    ak_error_message( ak_error_get_value(),
//                             "secret key control mechanism not properly destroyed", __func__ );
 ak_error_set_value( ak_error_ok );
 if( ak_oids_destroy() != ak_error_ok )
   ak_error_message( ak_error_get_value(), "OID mechanism not properly destroyed", __func__ );

 ak_error_message( ak_error_ok, "all crypto mechanisms successfully destroyed", __func__ );
 return ak_error_get_value();
}

/* ----------------------------------------------------------------------------------------------- */
/*! \include doc/libakrypt.dox                                                                     */
/*! \example example-log.c                                                                         */
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                     ak_tools.c  */
/* ----------------------------------------------------------------------------------------------- */