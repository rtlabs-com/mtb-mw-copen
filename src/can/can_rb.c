#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "can_rb.h"

// Initialize the ring buffer
void canrb_init(canrb_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    rb->mutex = xSemaphoreCreateMutex();
}

// Check if the ring buffer is full (safe for tasks only)
bool canrb_is_full(canrb_t *rb)
{
    bool result;
    xSemaphoreTake(rb->mutex, portMAX_DELAY);
    result = rb->count == CANRB_SIZE;
    xSemaphoreGive(rb->mutex);
    return result;
}

// Check if the ring buffer is empty (safe for tasks only)
bool canrb_is_empty(canrb_t *rb)
{
    bool result;
    xSemaphoreTake(rb->mutex, portMAX_DELAY);
    result = rb->count == 0;
    xSemaphoreGive(rb->mutex);
    return result;
}

// Add an element to the ring buffer (safe for tasks)
bool canrb_enqueue(canrb_t *rb, const can_frame_t *frame)
{
    bool success = false;
    xSemaphoreTake(rb->mutex, portMAX_DELAY);

    if (rb->count < CANRB_SIZE)
    {
        rb->buffer[rb->head] = *frame;
        rb->head = (rb->head + 1) % CANRB_SIZE;
        rb->count++;
        success = true;
    }

    xSemaphoreGive(rb->mutex);
    return success;
}

static bool enqueue_data(canrb_t *rb, uint32_t id, const uint8_t *data, uint8_t dlc)
{
    bool success = false;

    if (rb->count < CANRB_SIZE)
    {
        can_frame_t *frame = &rb->buffer[rb->head];
        frame->id = id;
        memcpy(frame->data, data, dlc);
        frame->dlc = dlc;

        rb->head = (rb->head + 1) % CANRB_SIZE;
        rb->count++;
        success = true;
    }

    return success;
}

bool canrb_enqueue_data(canrb_t *rb, uint32_t id, const uint8_t *data, uint8_t dlc)
{
    bool success = false;
    xSemaphoreTake(rb->mutex, portMAX_DELAY);
    success = enqueue_data(rb, id, data, dlc);
    xSemaphoreGive(rb->mutex);
    return success;
}

bool canrb_dequeue(canrb_t *rb, can_frame_t *frame)
{
    bool success = false;
    xSemaphoreTake(rb->mutex, portMAX_DELAY);

    if (rb->count > 0)
    {
        *frame = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) % CANRB_SIZE;
        rb->count--;
        success = true;
    }

    xSemaphoreGive(rb->mutex);
    return success;
}

bool canrb_dequeue_data(canrb_t *rb, uint32_t *id, size_t *dlc, uint8_t *data)
{
    bool success = false;
    xSemaphoreTake(rb->mutex, portMAX_DELAY);

    if (rb->count > 0)
    {
        can_frame_t *frame = &rb->buffer[rb->tail];
        *id = frame->id;
        memcpy(data, frame->data, frame->dlc);  // Copy only the used bytes (dlc)
        *dlc = (size_t)frame->dlc;

        rb->tail = (rb->tail + 1) % CANRB_SIZE;
        rb->count--;
        success = true;
    }

    xSemaphoreGive(rb->mutex);
    return success;
}


bool canrb_enqueue_data_from_isr(canrb_t *rb, uint32_t id, const uint8_t *data, uint8_t dlc)
{
    bool success = false;
    UBaseType_t saved_interrupt_status = taskENTER_CRITICAL_FROM_ISR();
    success = enqueue_data(rb, id, data, dlc);
    taskEXIT_CRITICAL_FROM_ISR(saved_interrupt_status);
    return success;
}
