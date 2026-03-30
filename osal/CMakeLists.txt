#********************************************************************
#        _       _         _
#  _ __ | |_  _ | |  __ _ | |__   ___
# | '__|| __|(_)| | / _` || '_ \ / __|
# | |   | |_  _ | || (_| || |_) |\__ \
# |_|    \__|(_)|_| \__,_||_.__/ |___/
#
# www.rt-labs.com
# Copyright 2025 rt-labs AB, Sweden.
#
# This software is licensed under the terms of the BSD 3-clause
# license. See the file LICENSE distributed with this software for
# full license information.
#*******************************************************************/

cmake_minimum_required(VERSION 3.14)
project(OSAL VERSION 0.1.0)

add_library(osal osal.c osal_log.c)

target_include_directories(osal
  PUBLIC
  ${OSAL_SOURCE_DIR}
  ${OSAL_SOURCE_DIR}/sys
  )
