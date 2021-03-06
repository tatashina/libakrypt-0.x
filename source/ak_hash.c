/* ----------------------------------------------------------------------------------------------- */
/*  Copyright (c) 2014 - 2017 by Axel Kenzo, axelkenzo@mail.ru                                     */
/*                                                                                                 */
/*  Разрешается повторное распространение и использование как в виде исходного кода, так и         */
/*  в двоичной форме, с изменениями или без, при соблюдении следующих условий:                     */
/*                                                                                                 */
/*   1. При повторном распространении исходного кода должно оставаться указанное выше уведомление  */
/*      об авторском праве, этот список условий и последующий отказ от гарантий.                   */
/*   2. При повторном распространении двоичного кода должна сохраняться указанная выше информация  */
/*      об авторском праве, этот список условий и последующий отказ от гарантий в документации     */
/*      и/или в других материалах, поставляемых при распространении.                               */
/*   3. Ни имя владельца авторских прав, ни имена его соратников не могут быть использованы в      */
/*      качестве рекламы или средства продвижения продуктов, основанных на этом ПО без             */
/*      предварительного письменного разрешения.                                                   */
/*                                                                                                 */
/*  ЭТА ПРОГРАММА ПРЕДОСТАВЛЕНА ВЛАДЕЛЬЦАМИ АВТОРСКИХ ПРАВ И/ИЛИ ДРУГИМИ СТОРОНАМИ "КАК ОНА ЕСТЬ"  */
/*  БЕЗ КАКОГО-ЛИБО ВИДА ГАРАНТИЙ, ВЫРАЖЕННЫХ ЯВНО ИЛИ ПОДРАЗУМЕВАЕМЫХ, ВКЛЮЧАЯ, НО НЕ             */
/*  ОГРАНИЧИВАЯСЬ ИМИ, ПОДРАЗУМЕВАЕМЫЕ ГАРАНТИИ КОММЕРЧЕСКОЙ ЦЕННОСТИ И ПРИГОДНОСТИ ДЛЯ КОНКРЕТНОЙ */
/*  ЦЕЛИ. НИ В КОЕМ СЛУЧАЕ НИ ОДИН ВЛАДЕЛЕЦ АВТОРСКИХ ПРАВ И НИ ОДНО ДРУГОЕ ЛИЦО, КОТОРОЕ МОЖЕТ    */
/*  ИЗМЕНЯТЬ И/ИЛИ ПОВТОРНО РАСПРОСТРАНЯТЬ ПРОГРАММУ, КАК БЫЛО СКАЗАНО ВЫШЕ, НЕ НЕСЁТ              */
/*  ОТВЕТСТВЕННОСТИ, ВКЛЮЧАЯ ЛЮБЫЕ ОБЩИЕ, СЛУЧАЙНЫЕ, СПЕЦИАЛЬНЫЕ ИЛИ ПОСЛЕДОВАВШИЕ УБЫТКИ,         */
/*  ВСЛЕДСТВИЕ ИСПОЛЬЗОВАНИЯ ИЛИ НЕВОЗМОЖНОСТИ ИСПОЛЬЗОВАНИЯ ПРОГРАММЫ (ВКЛЮЧАЯ, НО НЕ             */
/*  ОГРАНИЧИВАЯСЬ ПОТЕРЕЙ ДАННЫХ, ИЛИ ДАННЫМИ, СТАВШИМИ НЕПРАВИЛЬНЫМИ, ИЛИ ПОТЕРЯМИ ПРИНЕСЕННЫМИ   */
/*  ИЗ-ЗА ВАС ИЛИ ТРЕТЬИХ ЛИЦ, ИЛИ ОТКАЗОМ ПРОГРАММЫ РАБОТАТЬ СОВМЕСТНО С ДРУГИМИ ПРОГРАММАМИ),    */
/*  ДАЖЕ ЕСЛИ ТАКОЙ ВЛАДЕЛЕЦ ИЛИ ДРУГОЕ ЛИЦО БЫЛИ ИЗВЕЩЕНЫ О ВОЗМОЖНОСТИ ТАКИХ УБЫТКОВ.            */
/*                                                                                                 */
/*  ak_hash.c                                                                                      */
/* ----------------------------------------------------------------------------------------------- */
 #include <ak_hash.h>
 #include <ak_oid.h>

 #include <errno.h>
 #include <fcntl.h>
 #ifndef _WIN32
  #include <unistd.h>
 #endif
 #include <sys/stat.h>

/* ----------------------------------------------------------------------------------------------- */
/*                               некоторые методы класса ak_hash                                   */
/* ----------------------------------------------------------------------------------------------- */
/*! Функция выделяет память под контекст, а также под внутренние данные контекста
    @param data_size Размер внутренних данных контекста в байтах
    @param block_size Размер внутренних данных контекста в байтах
    @return указатель на контекст или NULL, сли создание контекста произошло неудачно.              */
/* ----------------------------------------------------------------------------------------------- */
 ak_hash ak_hash_new( const size_t data_size, const size_t block_size )
{
  ak_hash ctx = ( ak_hash ) malloc( sizeof( struct hash ));
  if( ctx == NULL ) {
    ak_error_message( ak_error_out_of_memory, __func__ , "incorrect context memory allocation" );
    return NULL;
  }
  if(( ctx->data = malloc( data_size )) == NULL ) {
      ak_error_message( ak_error_create_function, __func__ ,
                                                   "incorrect internal data memory allocation" );
      return ctx = ak_hash_delete( ctx );
  }
  ctx->bsize =  block_size;
  ctx->hsize =           0;
  ctx->oid =          NULL;
  ctx->clean =        NULL;
  ctx->update =       NULL;
  ctx->finalize =     NULL;
 return ctx;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param inctx указатель на контекст хеширования
    @return Функция возвращает NULL. В случае возникновения ошибки, функция сохраняет ее код в
    переменной ak_errno.                                                                           */
/* ----------------------------------------------------------------------------------------------- */
 ak_pointer ak_hash_delete( ak_pointer inctx )
{
  ak_hash ctx = ( ak_hash )inctx;
  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__, "use a null pointer to a context" );
  } else {
           if( ctx->data != NULL ) free( ctx->data );
           ctx->bsize =       0;
           ctx->hsize =       0;
           ctx->data =     NULL;
           ctx->oid =      NULL;
           ctx->clean =    NULL;
           ctx->update =   NULL;
           ctx->finalize = NULL;
           free( inctx );
         }
 return NULL;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param ctx контекст функции хеширования
    @return функция возвращает количество байт, которые нужны для хранения хешкода. В случае
    возникновения ошибки возвращается ее код.                                                      */
/* ----------------------------------------------------------------------------------------------- */
 size_t ak_hash_get_code_size( ak_hash ctx )
{
  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "use a null pointer to a context" );
    return 0;
  }
 return ctx->hsize;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param ctx контекст функции хеширования
    @return функция возвращает количество байт, которые обрабатываются за один такт работы
    сжимающего отображения. В случае возникновения ошибки возвращается ее код.                                                      */
/* ----------------------------------------------------------------------------------------------- */
 size_t ak_hash_get_block_size( ak_hash ctx )
{
  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "use a null pointer to a context" );
    return 0;
  }
 return ctx->bsize;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param ctx контекст функции хеширования
    @return функция возвращает указатель на OID. В случае возникновения ошибки, функция сохраняет
    ее код в переменной ak_errno  возвращает NULL.                                                 */
/* ----------------------------------------------------------------------------------------------- */
 ak_oid ak_hash_get_oid( ak_hash ctx )
{
  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "use a null pointer to a context" );
    return NULL;
  }
 return ctx->oid;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Функция вычисляет хеш-код от заданной области памяти на которую указывает in. Размер памяти
    задается в байтах в переменной size. Результат вычислений помещается в область памяти,
    на которую указывает out. Если out равен NULL, то функция создает новый буффер
    (структуру struct buffer), помещает в нее вычисленное значение и возвращает на указатель на
    буффер. Буффер должен позднее быть удален с помощью вызова ak_buffer_delete().

    @param ctx Контекст алгоритма хеширования, должен быть отличен от NULL.
    @param in Указатель на входные данные для которых вычисляется хеш-код.
    @param size Размер входных данных в байтах.
    @param out Область памяти, куда будет помещен рещультат. Память должна быть заранее выделена.
    Размер выделяемой памяти может быть определен с помощью вызова ak_hash_get_code_size().
    Указатель out может принимать значение NULL.

    @return Функция возвращает NULL, если указатель out не есть NULL, в противном случае
    возвращается указатель на буффер, содержащий результат вычислений.                             */
/* ----------------------------------------------------------------------------------------------- */
 ak_buffer ak_hash_data( ak_hash ctx, const ak_pointer in, const size_t size, ak_pointer out )
{
  ak_buffer result = NULL;
  size_t quot = 0, offset = 0;

  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "use a null pointer to a context" );
    return NULL;
  }
  if( in == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "use a null pointer to a data" );
    return NULL;
  }

  /* вычищаем результаты предыдущих вычислений */
  ctx->clean( ctx );
  quot = size/ctx->bsize;
  offset = quot*ctx->bsize;
  /* вызываем, если длина сообщения не менее одного полного блока */
  if( quot > 0 ) ctx->update( ctx, in, offset );
  /* обрабатываем хвост */
  result = ctx->finalize( ctx, (unsigned char *)in + offset, size - offset, out );
  /* очищаем за собой данные, содержащиеся в контексте */
  ctx->clean( ctx );
 return result;
}

/* ----------------------------------------------------------------------------------------------- */
/*!  Функция вычисляет хеш-код от файла, имя которого задается переменной filename.
     Результат вычислений помещается в область памяти,
     на которую указывает out. Если out равен NULL, то функция создает новый буффер
     (структуру struct buffer), помещает в нее вычисленное значение и возвращает на указатель на
     буффер. Буффер должен позднее быть удален с помощью вызова ak_buffer_delete().

     @param ctx Контекст алгоритма хеширования, должен быть отличен от NULL.
     @param filename Указатель на строку, в которой содержится имя файла.
     @param out Область памяти, куда будет помещен рещультат. Память должна быть заранее выделена.
     Размер выделяемой памяти может быть определен с помощью вызова ak_hash_get_code_size().
     Указатель out может принимать значение NULL.

     @return Функция возвращает NULL, если указатель out не есть NULL, в противном случае
     возвращается указатель на буффер, содержащий результат вычислений.                            */
/* ----------------------------------------------------------------------------------------------- */
 ak_buffer ak_hash_file( ak_hash ctx, const char* filename, ak_pointer out )
{
  int fd = 0;
  struct stat st;
  size_t len = 0;
  ak_uint8 *localbuffer; /* место для локального считывания информации */
  size_t block_size = 4096; /* оптимальная длина блока для Windows пока не ясна */
  ak_buffer result = NULL;

  /* выполняем необходимые проверки */
  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "use a null pointer to a context" );
    return NULL;
  }
  if( filename == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "use a null pointer to a file name" );
    return NULL;
  }
  if(( fd = open( filename, O_RDONLY | O_BINARY )) < 0 ) {
    ak_error_message( ak_error_open_file, strerror( errno ), __func__ );
    return NULL;
  }
  if( fstat( fd, &st ) ) {
    close( fd );
    ak_error_message( ak_error_access_file, strerror( errno ), __func__ );
    return NULL;
  }

  /* для файла нулевой длины результатом будет хеш от нулевого вектора */
  if( !st.st_size ) return ak_hash_data( ctx, "", 0, out );
  /* готовим область для хранения данных */
  #ifdef _WIN32
    block_size = ak_max( 4096, ctx->bsize );
  #else
    block_size = ak_max( st.st_blksize, ctx->bsize );
  #endif
  /* здесь мы выделяем локальный буффер для считывания/обработки данных */
  if((localbuffer = ( ak_uint8 * ) malloc( block_size )) == NULL ) {
    close( fd );
    ak_error_message( ak_error_out_of_memory, __func__ , "out of memory" );
    return NULL;
  }
  /* теперь обрабатываем файл с данными */
  ctx->clean( ctx );
  read_label: len = read( fd, localbuffer, block_size );
  if( len == block_size ) {
    ctx->update( ctx, localbuffer, block_size ); /* добавляем считанные данные */
    goto read_label;
  } else {
           size_t qcnt = len / ctx->bsize,
                  tail = len - qcnt*ctx->bsize;
           if( qcnt ) ctx->update( ctx, localbuffer, qcnt*ctx->bsize );
           result = ctx->finalize( ctx, localbuffer + qcnt*ctx->bsize, tail, out );
         }
 /* очищаем за собой данные, содержащиеся в контексте */
  ctx->clean( ctx );
 /* закрываем данные */
  close(fd);
  free( localbuffer );
 return result;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param oid идентификатор алгоритма хеширования
    @return функция возвращает ссылку на контекст алгоритма хеширования. В случае возникновения
    ошибки возвращается NULL, а код ошибки помещается а переменную ak_errno.                      */
/* ----------------------------------------------------------------------------------------------- */
 ak_hash ak_hash_new_oid( ak_oid oid )
{
 if( oid == NULL ) {
   ak_error_message( ak_error_null_pointer, __func__ , "using a null pointer to OID" );
   return NULL;
 }
 if( oid->engine != hash_function ) {
     ak_error_message( ak_error_oid_engine, __func__ , "using a not hash function OID" );
     return NULL;
 }

 if( !memcmp( ak_buffer_get_str( oid->name ), "streebog256", 11 )) return ak_hash_new_streebog256();
 if( !memcmp( ak_buffer_get_str( oid->name ), "streebog512", 11 )) return ak_hash_new_streebog512();
 if( !memcmp( ak_buffer_get_str( oid->name ), "gosthash94", 11 )) {
      return ak_hash_new_gosthash94( ak_oids_find_by_name( "id-gosthash94-CryptoPro-A-ParamSet" ));
 }

 ak_error_message( ak_error_undefined_value, __func__ , "using a pointer to undefined OID" );
 return NULL;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param ctx Контекст функции хеширования.
    @return В случае успеха функция возвращает ноль (\ref ak_error_ok). В противном случае
    возвращается код ошибки.                                                                       */
/* ----------------------------------------------------------------------------------------------- */
 int ak_hash_clean( ak_hash ctx )
{
  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "using a null pointer to a context" );
    return ak_error_null_pointer;
  }
  if( ctx->clean == NULL ) {
    ak_error_message( ak_error_undefined_function, __func__ ,
                                                     "using an undefined internal function" );
    return ak_error_undefined_function;
  }
  ctx->clean( ctx ); /* чистим внутренние данные алгоритма */
 return ak_error_ok;
}

/* ----------------------------------------------------------------------------------------------- */
/*! @param ctx Контекст функции хеширования.
    @param in Хешируемые данные.
    @param size Размер хешируемых данных в байтах. Данное значение не может
    быть произвольным и должно быть кратным длине блока обрабатываемых данных.
    Значение длины блока может быть получено с помощью вызова функции
    ak_hash_get_block_size().

    @return В случае успеха функция возвращает ноль (\ref ak_error_ok). В противном случае
    возвращается код ошибки.                                                                       */
/* ----------------------------------------------------------------------------------------------- */
 int ak_hash_update( ak_hash ctx, const ak_pointer in, const size_t size )
{
  int result = ak_error_ok;

  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "using a null pointer to a context" );
    return ak_error_null_pointer;
  }
  if( ctx->update == NULL ) {
    ak_error_message( ak_error_undefined_function, __func__ ,
                                                    "using an undefined internal function" );
    return ak_error_undefined_function;
  }

  ctx->update( ctx, in, size  );
  if(( result = ak_error_get_value()) != ak_error_ok )
    ak_error_message( ak_error_undefined_value, __func__ , "wrong updating a hash function context");
 return result;
}

/* ----------------------------------------------------------------------------------------------- */
/*! Результат вычислений помещается в область памяти,
    на которую указывает out. Если out равен NULL, то функция создает новый буффер
    (структуру struct buffer), помещает в нее вычисленное значение и возвращает на указатель на
    буффер. Буффер должен позднее быть удален с помощью вызова ak_buffer_delete().

    @param ctx Контекст функции хеширования.
    @param in Хешируемые данные.
    @param size Размер хешируемых данных в байтах. Данное значение не может
    быть произвольным и должно ыть строго менее длины блока обрабатываемых данных.
    Значение длины блока может быть получено с помощью вызова функции
    ak_hash_get_block_size().
    @param out Область памяти, куда будет помещен рещультат. Память должна быть заранее выделена.
    Размер выделяемой памяти может быть определен с помощью вызова ak_hash_get_code_size().
    Указатель out может принимать значение NULL.

    @return Функция возвращает NULL, если указатель out не есть NULL, в противном случае
    возвращается указатель на буффер, содержащий результат вычислений.                             */
/* ----------------------------------------------------------------------------------------------- */
 ak_buffer ak_hash_finalize( ak_hash ctx, const ak_pointer in, const size_t size, ak_pointer out )
{
  if( ctx == NULL ) {
    ak_error_message( ak_error_null_pointer, __func__ , "using a null pointer to a context" );
    return NULL;
  }
  if( ctx->finalize == NULL ) {
    ak_error_message( ak_error_undefined_function, __func__ ,
                                                     "using an undefined internal function" );
    return NULL;
  }

 return ctx->finalize( ctx, in, size, out );
}

/* ----------------------------------------------------------------------------------------------- */
/*! \example example-hash.c
    \example example-hash-parts.c
    \example example-hash-oids.c                                                                   */
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                      ak_hash.c  */
/* ----------------------------------------------------------------------------------------------- */
