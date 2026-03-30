#ifndef CANRB_H
#define CANRB_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"

// Define the can_frame structure
typedef struct can_frame
{
   uint32_t id;
   uint8_t data[8];
   uint8_t dlc;
   uint8_t pad[3];
} __attribute__ ((packed)) can_frame_t;

#define CANRB_SIZE 30 // Define the buffer size

// Define the ring buffer structure
typedef struct
{
   can_frame_t buffer[CANRB_SIZE];
   int head;
   int tail;
   int count;
   SemaphoreHandle_t mutex; // Mutex for task-level thread safety
} canrb_t;

void canrb_init (canrb_t * rb);
bool canrb_is_full (canrb_t * rb);
bool canrb_is_empty (canrb_t * rb);

bool canrb_enqueue (canrb_t * rb, const can_frame_t * frame);
bool canrb_enqueue_data (canrb_t * rb, uint32_t id, const uint8_t * data, uint8_t dlc);
bool canrb_enqueue_data_from_isr (
   canrb_t * rb,
   uint32_t id,
   const uint8_t * data,
   uint8_t dlc);

bool canrb_dequeue (canrb_t * rb, can_frame_t * frame);
bool canrb_dequeue_data (canrb_t * rb, uint32_t * id, size_t * dlc, uint8_t * data);

#endif // CANRB_H
