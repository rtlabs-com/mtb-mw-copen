/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2021 rt-labs AB, Sweden.
 *
 * This software is licensed under the terms of the BSD 3-clause
 * license. See the file LICENSE distributed with this software for
 * full license information.
 ********************************************************************/

#include "osal_log.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef OSAL_LOG_USE_CY_LOG
#include "cy_log.h"
#endif

#ifdef OSAL_LOG_USE_CY_LOG
/* Map osal logging to logging in the Modus Toolbox framework.
 * The idea is to build components with osal logging with a lot
 * of debug information and then use cy_log to filter.
 * A problem is that the log messages not always include the
 * origin of the message so when a lot of components are logging
 * it gets messy.
 * Another problem is that cy_log does not support a "v" function
 * which means log messages have to printed to log_buf even if they
 * will not be shown by cy_log_msg which implements the log filtering.
 */
void os_log_impl (uint8_t type, const char * fmt, ...)
{
   int cy_log_level;
   char log_buf[256];
   va_list list;

   switch (LOG_LEVEL_GET (type))
   {
   case LOG_LEVEL_VERBOSE:
      cy_log_level = CY_LOG_DEBUG1;
      break;
   case LOG_LEVEL_DEBUG:
      cy_log_level = CY_LOG_DEBUG;
      break;
   case LOG_LEVEL_INFO:
      cy_log_level = CY_LOG_INFO;
      break;
   case LOG_LEVEL_WARNING:
      cy_log_level = CY_LOG_WARNING;
      break;
   case LOG_LEVEL_ERROR:
   case LOG_LEVEL_FATAL:
   default:
      cy_log_level = CY_LOG_ERR;
      break;
   }

   va_start (list, fmt);
   vsnprintf (log_buf, sizeof (log_buf), fmt, list);
   va_end (list);

   cy_log_msg (CYLF_MIDDLEWARE, cy_log_level, "%s", log_buf);
}
#else
void os_log_impl (uint8_t type, const char * fmt, ...)
{
   va_list list;

   switch (LOG_LEVEL_GET (type))
   {
   case LOG_LEVEL_VERBOSE:
      printf ("%10ld [VERBOSE] ", xTaskGetTickCount());
      break;
   case LOG_LEVEL_DEBUG:
      printf ("%10ld [DEBUG] ", xTaskGetTickCount());
      break;
   case LOG_LEVEL_INFO:
      printf ("%10ld [INFO ] ", xTaskGetTickCount());
      break;
   case LOG_LEVEL_WARNING:
      printf ("%10ld [WARN ] ", xTaskGetTickCount());
      break;
   case LOG_LEVEL_ERROR:
      printf ("%10ld [ERROR] ", xTaskGetTickCount());
      break;
   case LOG_LEVEL_FATAL:
      printf ("%10ld [FATAL] ", xTaskGetTickCount());
      break;
   default:
      break;
   }

   va_start (list, fmt);
   vprintf (fmt, list);
   va_end (list);
}

#endif

os_log_t os_log = os_log_impl;
