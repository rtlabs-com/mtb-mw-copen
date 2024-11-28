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

#include "coal_can.h"
#include "osal.h"
#include "options.h"
#include "osal_log.h"
#include "co_log.h"
#include "co_main.h"
#include "co_rtk.h"
#include "shell.h"

#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include <stdio.h>
#include "cy_pdl.h"
#include "cycfg.h"
#include "cybsp.h"
#include "cy_canfd.h"
#include "cy_device.h"

#include "can_rb.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/

#ifdef XMC7200D_E272K8384
#define KIT_XMC72

#elif defined XMC7100D_F176K4160
#define KIT_XMC71_V1

#elif defined XMC7100D_F100K4160
#define KIT_XMC71_V2
#endif

/* CAN node definition */
#define CAN_NODE_1 1
#define CAN_NODE_2 2


// set this dynamically ??

// fixme -- force standard mode

/* Set the example CAN node */
#define USE_CAN_NODE CAN_NODE_2

/* CAN channel number */
#if defined(KIT_XMC71_V1) || defined(KIT_XMC71_V2)
#define CAN_HW_CHANNEL 0
#else
#define CAN_HW_CHANNEL 1
#endif

#define CAN_BUFFER_INDEX 0

/* canfd interrupt handler */
void isr_canfd (void);

/* canfd frame receive callback */
void canfd_rx_callback (
   bool rxFIFOMsg,
   uint8_t msgBufOrRxFIFONum,
   cy_stc_canfd_rx_buffer_t * basemsg);

/* This structure initializes the CANFD interrupt for the NVIC */
cy_stc_sysint_t canfd_irq_cfg = {
   .intrSrc = (NvicMux2_IRQn << 16) | CANFD_IRQ_0, /* Source of interrupt signal
                                                    */
   .intrPriority = 1U,                             /* Interrupt priority */
};

static cy_stc_canfd_global_filter_config_t globalFilterConfig =
{
    .nonMatchingFramesStandard = CY_CANFD_REJECT_NON_MATCHING,
    .nonMatchingFramesExtended = CY_CANFD_REJECT_NON_MATCHING,
    .rejectRemoteFramesStandard = false,
    .rejectRemoteFramesExtended = false,
};

/* Array to store the data bytes of the CANFD frame */
uint32_t canfd_dataBuffer[] = {
   [CANFD_DATA_0] = 0x04030201U,
   [CANFD_DATA_1] = 0x08070605U,
};

static canrb_t can_rx_q;

/* only one active interface supported */
static os_channel_t * active_chan = NULL;

/* This is a shared context structure, unique for each canfd channel */
static cy_stc_canfd_context_t canfd_context;

static void configureCanBitrate(cy_stc_canfd_bitrate_t* bitrateConfig, uint32_t bitrate_bps)
{
    uint32_t systemClock = 12500000;  // 12.5 MHz
    bitrateConfig->prescaler = (systemClock / bitrate_bps) - 1U;

    /* values chosen for good timing characteristics for most situations
     * adjust as needed */
    bitrateConfig->timeSegment1 = 5U - 1U;  // Adjust as needed (before sample point)
    bitrateConfig->timeSegment2 = 2U - 1U;  // Adjust as needed (after sample point)
    bitrateConfig->syncJumpWidth = 2U - 1U;  // Set the synchronization jump width (typically set to 1 or 2)
}

void isr_canfd (void)
{
   /* Just call the IRQ handler with the current channel number and context */
   Cy_CANFD_IrqHandler (CANFD_HW, CAN_HW_CHANNEL, &canfd_context);
}

/*******************************************************************************
 * Function Name: canfd_rx_callback
 ********************************************************************************
 * Summary:
 * This is the callback function for canfd reception
 *
 * Parameters:
 *    rxFIFOMsg                      Message was received in Rx FIFO (0/1)
 *    msgBufOrRxFIFONum              RxFIFO number of the received message
 *    basemsg                        Message buffer
 *
 *******************************************************************************/

void canfd_rx_callback (
   bool rxFIFOMsg,
   uint8_t msgBufOrRxFIFONum,
   cy_stc_canfd_rx_buffer_t * basemsg)
{
   /* Message was received in Rx FIFO */
   if (rxFIFOMsg == true)
   {
      /* Checking whether the frame received is a data frame */
      if (CY_CANFD_RTR_DATA_FRAME == basemsg->r0_f->rtr)
      {
         /* Toggle the user LED */
         // Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);

    	 /* only standard can for now */
         canrb_enqueue_data_from_isr(&can_rx_q, basemsg->r0_f->id, (const uint8_t *)basemsg->data_area_f, (uint8_t)basemsg->r1_f->dlc);

         /* signal reader task */
         active_chan->callback(active_chan->arg);
      }
   }

   /* These parameters are not used in this snippet */
   (void)msgBufOrRxFIFONum;
}

/* last error status */
static uint32_t canLastErrorMask;

typedef struct can_err_stats {
	uint32_t cy_canfd_rx_fifo_0_watermark_reached;
    uint32_t cy_canfd_rx_fifo_0_full;
    uint32_t cy_canfd_rx_fifo_0_msg_lost;
    uint32_t cy_canfd_rx_fifo_1_watermark_reached;
    uint32_t cy_canfd_rx_fifo_1_full;
    uint32_t cy_canfd_rx_fifo_1_msg_lost;
    uint32_t cy_canfd_tx_fifo_1_watermark_reached;
    uint32_t cy_canfd_tx_fifo_1_full;
    uint32_t cy_canfd_tx_fifo_1_msg_lost;
    uint32_t cy_canfd_timestamp_wraparound;
    uint32_t cy_canfd_mram_access_failure;
    uint32_t cy_canfd_timeout_occurred;
    uint32_t cy_canfd_bit_error_corrected;
    uint32_t cy_canfd_bit_error_uncorrected;
    uint32_t cy_canfd_error_log_overflow;
    uint32_t cy_canfd_error_passive;
    uint32_t cy_canfd_warning_status;
    uint32_t cy_canfd_bus_off_status;
    uint32_t cy_canfd_watchdog_interrupt;
    uint32_t cy_canfd_protocol_error_arb_phase;
    uint32_t cy_canfd_protocol_error_data_phase;
    uint32_t cy_canfd_access_reserved_addr;
} can_err_stats_t;

static can_err_stats_t err_stats;

void canfd_error_func(uint32_t errorMask)
{
	canLastErrorMask = errorMask;

	if (errorMask & CY_CANFD_RX_FIFO_0_WATERMARK_REACHED)
		err_stats.cy_canfd_rx_fifo_0_watermark_reached++;

	if (errorMask & CY_CANFD_RX_FIFO_0_FULL)
		err_stats.cy_canfd_rx_fifo_0_full++;

	if (errorMask & CY_CANFD_RX_FIFO_0_MSG_LOST)
		err_stats.cy_canfd_rx_fifo_0_msg_lost++;

	if (errorMask & CY_CANFD_RX_FIFO_1_WATERMARK_REACHED)
		err_stats.cy_canfd_rx_fifo_1_watermark_reached++;

	if (errorMask & CY_CANFD_RX_FIFO_1_FULL)
		err_stats.cy_canfd_rx_fifo_1_full++;

	if (errorMask & CY_CANFD_RX_FIFO_1_MSG_LOST)
		err_stats.cy_canfd_rx_fifo_1_msg_lost++;

	if (errorMask & CY_CANFD_TX_FIFO_1_WATERMARK_REACHED)
		err_stats.cy_canfd_tx_fifo_1_watermark_reached++;

	if (errorMask & CY_CANFD_TX_FIFO_1_FULL)
		err_stats.cy_canfd_tx_fifo_1_full++;

	if (errorMask & CY_CANFD_TX_FIFO_1_MSG_LOST)
		err_stats.cy_canfd_tx_fifo_1_msg_lost++;

	if (errorMask & CY_CANFD_TIMESTAMP_WRAPAROUND)
		err_stats.cy_canfd_timestamp_wraparound++;

	if (errorMask & CY_CANFD_MRAM_ACCESS_FAILURE)
		err_stats.cy_canfd_mram_access_failure++;

	if (errorMask & CY_CANFD_TIMEOUT_OCCURRED)
		err_stats.cy_canfd_timeout_occurred++;

	if (errorMask & CY_CANFD_BIT_ERROR_CORRECTED)
		err_stats.cy_canfd_bit_error_corrected++;

	if (errorMask & CY_CANFD_BIT_ERROR_UNCORRECTED)
		err_stats.cy_canfd_bit_error_uncorrected++;

	if (errorMask & CY_CANFD_ERROR_LOG_OVERFLOW)
		err_stats.cy_canfd_error_log_overflow++;

	if (errorMask & CY_CANFD_ERROR_PASSIVE)
		err_stats.cy_canfd_error_passive++;

	if (errorMask & CY_CANFD_WARNING_STATUS)
		err_stats.cy_canfd_warning_status++;

	if (errorMask & CY_CANFD_BUS_OFF_STATUS)
		err_stats.cy_canfd_bus_off_status++;

	if (errorMask & CY_CANFD_WATCHDOG_INTERRUPT)
		err_stats.cy_canfd_watchdog_interrupt++;

	if (errorMask & CY_CANFD_PROTOCOL_ERROR_ARB_PHASE)
		err_stats.cy_canfd_protocol_error_arb_phase++;

	if (errorMask & CY_CANFD_PROTOCOL_ERROR_DATA_PHASE)
		err_stats.cy_canfd_protocol_error_data_phase++;

	if (errorMask & CY_CANFD_ACCESS_RESERVED_ADDR)
		err_stats.cy_canfd_access_reserved_addr++;
}

os_channel_t * os_channel_open (const char * name, void * callback, void * arg)
{
   cy_en_canfd_status_t status;

   if (active_chan != NULL)
	   return NULL;

   os_channel_t * channel = malloc (sizeof (*channel));

   channel->callback = callback;
   channel->arg      = arg;

   /* Hook the interrupt service routine and enable the interrupt */
   (void)Cy_SysInt_Init (&canfd_irq_cfg, &isr_canfd);
   NVIC_EnableIRQ (NvicMux2_IRQn);

   /* c-open currently only supports classic can */
   CANFD_config.canFDMode = false;
   CANFD_config.errorCallback = canfd_error_func;
   CANFD_config.globalFilterConfig = &globalFilterConfig;

   status =
      Cy_CANFD_Init (CANFD_HW, CAN_HW_CHANNEL, &CANFD_config, &canfd_context);

   if (status != CY_CANFD_SUCCESS)
   {
	  free (channel);
	  return NULL;
   }

   active_chan = channel;
   channel->handle = (int)&canfd_context;

   /* set filter, allow all */
   os_channel_set_filter(channel, NULL);

   canrb_init(&can_rx_q);

   /* Setting CAN node identifier */
   CANFD_T0RegisterBuffer_0.id = USE_CAN_NODE;

   /* Assign the user defined data buffer to CANFD data area */
   CANFD_txBuffer_0.data_area_f = canfd_dataBuffer;

   return (void *)channel;
}

int os_channel_send (os_channel_t * channel, uint32_t id, const void * data, size_t dlc)
{
   CANFD_txBuffer_0.t0_f->id  = id & CO_ID_MASK;
   CANFD_txBuffer_0.t1_f->dlc = dlc;
   CANFD_txBuffer_0.t0_f->rtr = (id & CO_RTR_MASK) ? CY_CANFD_RTR_REMOTE_FRAME
                                                   : CY_CANFD_RTR_DATA_FRAME;
   CANFD_txBuffer_0.t0_f->xtd = (id & CO_EXT_MASK) ? CY_CANFD_XTD_EXTENDED_ID
                                                   : CY_CANFD_XTD_STANDARD_ID;

   CANFD_txBuffer_0.data_area_f = (uint32_t *)data;

   LOG_DEBUG (CO_CAN_LOG, "<-- can-tx id:0x%x dlc:%d bytes\n", id, dlc);

   Cy_CANFD_UpdateAndTransmitMsgBuffer (
      CANFD_HW,
      CAN_HW_CHANNEL,
      &CANFD_txBuffer_0,
      CAN_BUFFER_INDEX,
      &canfd_context);

   return 0;
}

int os_channel_receive (
   os_channel_t * channel,
   uint32_t * id,
   void * data,
   size_t * dlc)
{
   return (canrb_dequeue_data(&can_rx_q, id, dlc, data) == false);
}

int os_channel_set_bitrate (os_channel_t * channel, int bitrate)
{
   cy_stc_canfd_bitrate_t bitrate_cfg;

   LOG_INFO (CO_CAN_LOG, "setting bitrate to [%d] bps\n", bitrate);

   /* unittests attempts to set bitrate 0, silently ignore */
   if (bitrate == 0)
	   return 0;

   configureCanBitrate(&bitrate_cfg, bitrate);

   Cy_CANFD_ConfigChangesEnable(CANFD_HW, CAN_HW_CHANNEL);
   Cy_CANFD_SetBitrate(CANFD_HW, CAN_HW_CHANNEL, &bitrate_cfg);
   Cy_CANFD_ConfigChangesDisable(CANFD_HW, CAN_HW_CHANNEL);

   return 0;
}

int os_channel_set_filter (os_channel_t * channel, os_filter_t *filter)
{
   cy_stc_canfd_sid_filter_config_t	config;
   cy_stc_id_filter_t std_filter;

   if (filter == NULL)
   {
	  /* disable filter */
	   std_filter.sfid1 = 0x7FFU; /* allow all */
	   std_filter.sfid2 = 0;
	   std_filter.sfec = CY_CANFD_SFEC_STORE_RX_FIFO_0;
	   std_filter.sft = CY_CANFD_SFT_CLASSIC_FILTER;
   }
   else
   {
      /* only support classic filter using filter & mask */
      std_filter.sfid1 = filter->id;
      std_filter.sfid2 = filter->mask;
      std_filter.sfec = CY_CANFD_SFEC_STORE_RX_FIFO_0;
      std_filter.sft = CY_CANFD_SFT_CLASSIC_FILTER;
   }

   LOG_DEBUG (CO_CAN_LOG, "set filter id:0x%x mask:0x%x\n", std_filter.sfid1, std_filter.sfid2);

   config.numberOfSIDFilters = 1;
   config.sidFilter = &std_filter;

   Cy_CANFD_SidFiltersSetup(CANFD_HW, CAN_HW_CHANNEL, &config, &canfd_context);

   return 0;
}

int os_channel_bus_on (os_channel_t * channel)
{
   Cy_CANFD_ConfigChangesDisable(CANFD_HW, CAN_HW_CHANNEL);
   return 0;
}

int os_channel_bus_off (os_channel_t * channel)
{
   Cy_CANFD_ConfigChangesEnable(CANFD_HW, CAN_HW_CHANNEL);
   return 0;
}

int os_channel_get_state (os_channel_t * channel, os_channel_state_t * state)
{
   memset(state, 0, sizeof(os_channel_state_t));

   /* no api available to request current status on demand
    * use last interrupt status */
   if (canLastErrorMask & CY_CANFD_ERROR_PASSIVE)
	  state->error_passive = true;

   if (canLastErrorMask & CY_CANFD_BUS_OFF_STATUS)
	   state->bus_off = true;

   if ((canLastErrorMask & CY_CANFD_RX_FIFO_1_FULL) ||
	   (canLastErrorMask & CY_CANFD_RX_FIFO_1_MSG_LOST))
	   state->overrun = true;

   return 0;
}

#ifdef UNIT_TEST

#include "osal.h"
#include "osal_sys.h"


uint64_t get_custom_time_in_millis(void)
{
    return (uint64_t)os_tick_current();
}
#endif

/* gtest stub */
__attribute__((weak)) int mkdir(const char *pathname, mode_t mode)
{
   return -1;
}

int _cmd_setfilter (int argc, char * argv[])
{
   os_filter_t filter;

   if (argc < 2)
	   return -1;

   filter.id = (uint32_t)strtol(argv[1], NULL, 16);
   filter.mask = (uint32_t)strtol(argv[2], NULL, 16);

   os_channel_bus_off(active_chan);
   os_channel_set_filter(active_chan, &filter);
   os_channel_bus_on(active_chan);

   return 0;
}

const shell_cmd_t cmd_setfilter = {
   .cmd = _cmd_setfilter,
   .name = "setflt",
   .help_short = "Set CAN filter <0xid> <0xmask>",
   .help_long = "Example : setflt 623 0x7ff   -- only allow id 623"};

SHELL_CMD (cmd_setfilter);

int _cmd_silentmode (int argc, char * argv[])
{
   printf("Enter can silent mode (rx only)\n");

   /* no api to restore normal operation */
   Cy_CANFD_ConfigChangesEnable(CANFD_HW, CAN_HW_CHANNEL);
   Cy_CANFD_TestModeConfig(CANFD_HW, CAN_HW_CHANNEL, CY_CANFD_TEST_MODE_BUS_MONITORING);
   Cy_CANFD_ConfigChangesDisable(CANFD_HW, CAN_HW_CHANNEL);

   return 0;
}

const shell_cmd_t cmd_silentmode = {
   .cmd = _cmd_silentmode,
   .name = "silent",
   .help_short = "Enter CAN silent mode",
   .help_long = "Example : silent  -- only monitor bus traffic (reboot to restore)"};

SHELL_CMD (cmd_silentmode);


int _cmd_bus_ctrl (int argc, char * argv[])
{
   bool bus_on;

   if (argc < 2)
	   return -1;

   bus_on = (bool)atoi(argv[1]);

   if (bus_on)
   {
      os_channel_bus_on(active_chan);
   }
   else
   {
      os_channel_bus_off(active_chan);
   }

   printf("bus is now %s\n", bus_on?"on":"off");

   return 0;
}

const shell_cmd_t cmd_busctrl = {
   .cmd = _cmd_bus_ctrl,
   .name = "busctrl",
   .help_short = "Set bus on/off",
   .help_long = "Example : busctrl <0/1>"};

SHELL_CMD (cmd_busctrl);

int _cmd_reboot (int argc, char * argv[])
{
   NVIC_SystemReset();
   return 0;
}

const shell_cmd_t cmd_reboot = {
   .cmd = _cmd_reboot,
   .name = "reboot",
   .help_short = "reboot the device",
   .help_long = "Trigger a system reset using the hw watchdog."};

SHELL_CMD (cmd_reboot);

