/********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * http://www.rt-labs.com
 * Copyright 2022 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#include "cyhal_wdt.h"
#include "shell.h"
#include "utils.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

/* add utility function to print 64 bit values as 
   %lld format not supported */
const char * u64_to_str (uint64_t value)
{
   static char buffers[4][32];
   static int index = 0;
   char * buf;

   index = (index + 1) % 4;
   buf = buffers[index];

   if (value == 0)
   {
      buf[0] = '0';
      buf[1] = '\0';
      return buf;
   }

   char tmp[32];
   int pos = 0;

   while (value && pos < (int)(sizeof (tmp) - 1))
   {
      tmp[pos++] = '0' + (value % 10);
      value /= 10;
   }

   for (int i = 0; i < pos; ++i)
   {
      buf[i] = tmp[pos - i - 1];
   }

   buf[pos] = '\0';
   return buf;
}


/**
 * Override the OSAL system reset function.
 * os_system_reset() is defined with a weak attribute in the OSAL
 * implementation.
 */
void os_system_reset (void)
{
   NVIC_SystemReset();
}

int _cmd_reboot (int argc, char * argv[])
{
   os_system_reset();
   return 0;
}

const shell_cmd_t cmd_reboot = {
   .cmd = _cmd_reboot,
   .name = "reboot",
   .help_short = "reboot the device",
   .help_long = "Trigger a system reset using the hw watchdog."};

SHELL_CMD (cmd_reboot);
