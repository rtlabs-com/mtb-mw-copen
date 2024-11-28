/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2017 rt-labs AB, Sweden.
 *
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

#ifndef VERSION_H
#define VERSION_H

#define CANOPEN_GIT_REVISION "a5be805"

#if !defined(CO_VERSION_BUILD) && defined(CANOPEN_GIT_REVISION)
#define CO_VERSION_BUILD CANOPEN_GIT_REVISION
#endif

/* clang-format-off */

#define CO_VERSION_MAJOR 0
#define CO_VERSION_MINOR 1
#define CO_VERSION_PATCH 0

#if defined(CO_VERSION_BUILD)
#define CO_VERSION \
   "0.1.0+"CO_VERSION_BUILD
#else
#define CO_VERSION \
   "0.1.0"
#endif

/* clang-format-on */

#endif /* VERSION_H */
