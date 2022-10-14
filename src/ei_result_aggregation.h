/* EdgeImpulse firmware-nordic-thingy53 extension for simple result aggregation
 * Copyright (c) 2022 dxcfl.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef EI_RESULT_AGGREGATION_H
#define EI_RESULT_AGGREGATION_H

#include <stdint.h>
#include <cstddef>
#include "edge-impulse-sdk/classifier/ei_classifier_types.h"
#include "cJSON.h"

typedef struct
{
    uint32_t update;
    uint32_t count;
    uint32_t time_sum;
    uint32_t runlength_last;
    uint32_t runlength_max;
    float value_min;
    float value_max;
    float value_avg;
} ei_result_aggregation_item_t;

typedef struct
{
    float value_threshold;
    uint32_t update;
    uint32_t count;
    ei_result_aggregation_item_t aggregation[EI_CLASSIFIER_MAX_LABELS_COUNT];
} ei_result_aggregation_t;

void ei_result_aggregation_init(const float value_threshold);

void ei_result_aggregation_update(ei_impulse_result_t *result);

void ei_result_aggregation_add_to_json_object(cJSON *object, const char *name, size_t ix);

uint32_t ei_result_aggregation_get_time(size_t ix);

#endif // EI_RESULT_AGGREGATION_H
