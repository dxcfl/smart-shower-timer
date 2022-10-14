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

#include <stdint.h>
#include <cstddef>
#include "ei_result_aggregation.h"
#include "edge-impulse-sdk/classifier/ei_classifier_types.h"
#include "cJSON.h"
#include <logging/log.h>
#include <zephyr.h>

static ei_result_aggregation_t result_aggregation;

void ei_result_aggregation_init(const float value_threshold)
{
    result_aggregation = {0};
    result_aggregation.value_threshold = value_threshold;
}

void ei_result_aggregation_update(ei_impulse_result_t *result)
{
    uint32_t update = k_uptime_get_32();

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
        float value = result->classification[ix].value;
        if (value >= result_aggregation.value_threshold)
        {
            if (result_aggregation.update == result_aggregation.aggregation[ix].update)
            {
                result_aggregation.aggregation[ix].runlength_last++;
            }
            else
            {
                result_aggregation.aggregation[ix].runlength_last = 1;
            }
            if (result_aggregation.aggregation[ix].runlength_last > result_aggregation.aggregation[ix].runlength_max)
            {
                result_aggregation.aggregation[ix].runlength_max = result_aggregation.aggregation[ix].runlength_last;
            }
            result_aggregation.aggregation[ix].count++;
            result_aggregation.aggregation[ix].time_sum += (update - result_aggregation.update);
            result_aggregation.aggregation[ix].update = update;
            ;

            if (value > result_aggregation.aggregation[ix].value_max)
            {
                result_aggregation.aggregation[ix].value_max = value;
            }
            if (value < result_aggregation.aggregation[ix].value_min)
            {
                result_aggregation.aggregation[ix].value_min = value;
            }
            if (result_aggregation.aggregation[ix].value_avg > 0)
            {
                result_aggregation.aggregation[ix].value_avg += value;
                result_aggregation.aggregation[ix].value_avg /= 2;
            }
        }
    }
    result_aggregation.update = update;
    result_aggregation.count++;
}

void ei_result_aggregation_add_to_json_object(cJSON *object, const char *name, size_t ix)
{
    cJSON *agg = cJSON_AddObjectToObject(object, name);
    cJSON_AddNumberToObject(agg, "count", result_aggregation.aggregation[ix].count);
    cJSON_AddNumberToObject(agg, "time", result_aggregation.aggregation[ix].time_sum);
    cJSON_AddNumberToObject(agg, "value.min", result_aggregation.aggregation[ix].value_min);
    cJSON_AddNumberToObject(agg, "value.avg", result_aggregation.aggregation[ix].value_avg);
    cJSON_AddNumberToObject(agg, "value.max", result_aggregation.aggregation[ix].value_max);
    cJSON_AddNumberToObject(agg, "run.last", result_aggregation.aggregation[ix].runlength_last);
    cJSON_AddNumberToObject(agg, "run.max", result_aggregation.aggregation[ix].runlength_max);
}

uint32_t ei_result_aggregation_get_time(size_t ix)
{
    return result_aggregation.aggregation[ix].time_sum;
}
