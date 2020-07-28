/* Copyright 2019 SiFive, Inc */
/* SPDX-License-Identifier: Apache-2.0 */

#include <metal/cpu.h>
#include <metal/interrupt.h>
#include <metal/rtc.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define RTC_TIMEOUT_SECONDS 1

bool caught_rtc_int = false;

void metal_rtc_handler(void) {
	struct metal_rtc rtc = metal_rtc_get_device(0);

	/* Stop the rtc */
	metal_rtc_run(rtc, METAL_RTC_STOP);

	/* Disable RTC interrupt */
	metal_rtc_disable_interrupt(rtc);

	/* Clear the pending interrupt by setting the compare to a value
	 * larger than the current count */
	metal_rtc_set_compare(rtc, metal_rtc_get_count(rtc) + 1);

	puts("Caught RTC interrupt\n");

	caught_rtc_int = true;	
}

int main() {
	struct metal_rtc rtc = metal_rtc_get_device(0);

	const uint64_t rate = metal_rtc_get_rate(rtc);

	/* Set RTC to 0 */
	metal_rtc_set_count(rtc, 0);

	/* Timeout after RTC_TIMEOUT_SECONDS seconds */
	metal_rtc_set_compare(rtc, RTC_TIMEOUT_SECONDS * rate);

	/* Enable interrupts */
	metal_rtc_enable_interrupt(rtc);
	metal_cpu_enable_interrupts();

	puts("Starting RTC\n");

	/* Start the rtc */
	metal_rtc_run(rtc, METAL_RTC_RUN);

	/* If the rtc doesn't fire after twice the requested timeout, fail */
	time_t timeout = time(NULL) + (2 * RTC_TIMEOUT_SECONDS);
	
	while (!caught_rtc_int) {
		if (time(NULL) > timeout) {
			/* Stop the rtc */
			metal_rtc_run(rtc, METAL_RTC_STOP);

			puts("RTC interrupt never triggered\n");

			exit(7);
		}
	}

	return 0;
}


