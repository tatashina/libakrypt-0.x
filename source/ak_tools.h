/* ----------------------------------------------------------------------------------------------- */
/*   Copyright (c) 2014 - 2017 by Axel Kenzo, axelkenzo@mail.ru                                    */
/*   All rights reserved.                                                                          */
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
/*   ak_tools.h                                                                                    */
/* ----------------------------------------------------------------------------------------------- */
/*   Файл содержит описания служебных функций и переменных, не экспортруемых за пределы библиотеки */
/* ----------------------------------------------------------------------------------------------- */
 #ifndef    __AK_TOOLS_H__
 #define    __AK_TOOLS_H__

/* ----------------------------------------------------------------------------------------------- */
 #include <libakrypt.h>

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Неэкспортируемая функция установления уровня аудита */
 int ak_log_set_level( int );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Функция считывает настройки библиотеки из конфигурационного файла */
 ak_bool ak_libakrypt_load_options( void );
/*! \brief Функция возвращает ресурс секретного ключа для алгоритмов ГОСТ 28147-89 и Магма */
 ak_uint32 ak_libakrypt_get_magma_resource( void );
/*! \brief Функция возвращает ресурс секретного ключа для алгоритма ГОСТ 34.12-2015 (Кузнечик) */
 ak_uint32 ak_libakrypt_get_kuznechik_resource( void );
/*! \brief Функция возвращает длину номера формируемого библиотекой ключа в байтах */
 ak_uint32 ak_libakrypt_get_key_number_length( void );
/*! \brief Функция возвращает количество итерация для алгоритма pbkdf2 */
 ak_uint32 ak_libakrypt_get_pbkdf2_iteration_count( void );

/* ----------------------------------------------------------------------------------------------- */
/*! \brief Обобщенная реализация функции snprintf для различных компиляторов */
 int ak_snprintf( char *str, size_t size, const char *format, ... );
/*! \brief Вывод сообщений о возникшей в процессе выполнения ошибке */
 int ak_error_message_fmt( const int , const char *, const char *, ... );

 #endif
/* ----------------------------------------------------------------------------------------------- */
/*                                                                                 ak_libakrypt.h  */
/* ----------------------------------------------------------------------------------------------- */
