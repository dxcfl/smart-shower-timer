/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
/**@file
 *
 * @brief   Buzzer control for the User Interface module. The module uses PWM to
 *	    control the buzzer output frequency.
 */

#ifndef BUZZER_H__
#define BUZZER_H__

#include <zephyr.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**@brief Initialize buzzer. */
    int buzzer_init(void);

    /**
     * @brief Set the buzzer frequency.
     *
     * @param frequency Frequency. If set to 0, the buzzer is disabled.
     *		    The frequency is limited to the range 100 - 10 000 Hz.
     * @param intensity Intensity of the buzzer output. If set to 0, the buzzer is
     *		    disabled.
     *		    The Intensity is limited to the range 0 - 100 %.
     *
     * @return 0 on success or negative error value on failure.
     */
    int buzzer_set_frequency(uint32_t frequency, uint8_t intensity);

#ifdef __cplusplus
}
#endif

#endif /* BUZZER_H__ */
