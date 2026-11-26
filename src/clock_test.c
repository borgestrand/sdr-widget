/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */

/* clock_test.c - 20261125: MCU clock switching test, stage A (pin toggle only). Not production code.
 *
 * Build with EXPERIMENT=-DFEATURE_CLOCK_TEST. composite_widget.c includes this file, and main() calls
 * clock_test() right after the early SPRX GPIO setup, still at 12. It never returns: no FreeRTOS, no USB.
 *
 * Names: 12, 22, 24 are the oscillators (OSC0 crystal, 44.1 XO, 48 XO). 66, 45, 49 are the MCU speeds:
 *   66 = PLL from 12: 12 x 11/1 = 132 MHz VCO, /2
 *   45 = PLL from 22 on OSC1: 22.5792 x 8/2 = 90.3168 MHz VCO, /2 = 45.1584 MHz
 *   49 = PLL from 24 on OSC1: 24.576 x 8/2 = 98.304 MHz VCO, /2 = 49.152 MHz
 * CPU = HSB = PBA = PBB throughout. Sequence: 12 at start, then 66, 45, 66, 49, 66, 45, ...
 *
 * Only PLL0 can clock the CPU, and PLL0 can't be changed while the CPU runs on it. PLL1 is free (USB uses
 * the 12 directly), so it scouts ahead:
 *   1. At the current speed: change XO if needed (old off first, never both on), wait CLK_XO_SETTLE_MS (default 0)
 *   2. At the current speed: PLL1 gets the next speed's settings, wait for LOCK1. No lock: error()
 *   3. Gap: CPU to 12, PLL0 gets the same settings, wait for LOCK0, CPU to PLL0. PLL1 off
 * The XO may only change while the CPU doesn't depend on OSC1, so 45 and 49 are always reached from 66.
 * We trust the LOCK0 and LOCK1 bits for now.
 *
 * PA22 toggles 1000 times (500 full periods, always starting at 0) at each speed. The delay count is scaled
 * by the speed the code believes it runs at, so the PA22 period must be THE SAME at 12, 66, 45 and 49.
 * PX33 is high during the gap (step 3, CPU at 12): its pulse width is the cost of a switch. Steps 1 and 2
 * show as the time from the last PA22 toggle to the rising edge of PX33.
 * error() stops the test, Apollo style: CPU at 12 if it can get there, PA22 high, PX33 low.
 */

#ifdef FEATURE_CLOCK_TEST

#include "flashc.h"

#define CLK_HZ_PER_TICK		10000		// Delay count per PA22 half period = Hz / 10000: 6600 at 66, 1200 at 12
#ifndef CLK_XO_SETTLE_MS
	#define CLK_XO_SETTLE_MS	0		// XO start-up after enable. 0: poll LOCK1 at once and let it tell us
#endif
#ifndef CLK_PLLCOUNT
	#define CLK_PLLCOUNT	16			// PLL lock count, 0-63, ASF value. Override to test what LOCKn really waits for
#endif
#define CLK_LOCK_TIMEOUT	1000000		// Polling loops before giving up on LOCKn, 20 ms or more at any speed

enum { CLK_XO_KEEP, CLK_XO_22, CLK_XO_24 };

typedef struct {
	U32 hz;			// Resulting CPU speed
	U8 osc;			// PLL reference: 0 = OSC0 (12), 1 = OSC1 (XO)
	U8 mul;			// PLLMUL field, multiplier - 1
	U8 div;			// PLLDIV field
	U8 xo;			// XO to enable first
} clk_speed_t;

static const clk_speed_t CLK_START = {12000000, 0, 0, 0, CLK_XO_KEEP};	// On OSC0, no PLL

static const clk_speed_t CLK_SEQUENCE[] = {
	{66000000, 0, 10, 1, CLK_XO_KEEP},
	{45158400, 1,  7, 2, CLK_XO_22},
	{66000000, 0, 10, 1, CLK_XO_KEEP},
	{49152000, 1,  7, 2, CLK_XO_24}
};
#define CLK_SEQUENCE_LEN	(sizeof(CLK_SEQUENCE) / sizeof(CLK_SEQUENCE[0]))


static U32 ticks(U32 hz) {
	return hz / CLK_HZ_PER_TICK;
}


static void error(void) {
	pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCSEL_OSC0);
	gpio_set_gpio_pin(AVR32_PIN_PA22);
	gpio_clr_gpio_pin(AVR32_PIN_PX33);
	while (1);
}


// Write the settings with the PLL off, make sure LOCKn has dropped (no stale lock), then enable and wait
// for LOCKn. Returns 0 on timeout
static U8 clk_pll_start(U8 pll, const clk_speed_t *speed) {
	const U32 lock_mask = (pll == 0) ? AVR32_PM_POSCSR_LOCK0_MASK : AVR32_PM_POSCSR_LOCK1_MASK;
	U32 timeout = CLK_LOCK_TIMEOUT;

	pm_pll_setup(&AVR32_PM, pll, speed->mul, speed->div, speed->osc, CLK_PLLCOUNT);	// Writes PLLEN = 0 as well
	pm_pll_set_option(&AVR32_PM, pll,
		1,						// pll_freq: VCO range 80-180 MHz
		1,						// pll_div2
		0);						// pll_wbwdisable

	while (AVR32_PM.poscsr & lock_mask) {
		if (--timeout == 0)
			return 0;
	}

	pm_pll_enable(&AVR32_PM, pll);

	timeout = CLK_LOCK_TIMEOUT;
	while (!(AVR32_PM.poscsr & lock_mask)) {
		if (--timeout == 0)
			return 0;
	}
	return 1;
}


// Same pins as mobo_xo_select(). Old XO off first, then the new one on, never both
static void clk_select_xo(U8 xo, U32 current_hz) {
	if (xo == CLK_XO_22) {
		gpio_clr_gpio_pin(AVR32_PIN_PA21);			// 48 control off
		gpio_set_gpio_pin(AVR32_PIN_PA23);			// 44.1 control on
	}
	else if (xo == CLK_XO_24) {
		gpio_clr_gpio_pin(AVR32_PIN_PA23);			// 44.1 control off
		gpio_set_gpio_pin(AVR32_PIN_PA21);			// 48 control on
	}
	else
		return;
	#if (CLK_XO_SETTLE_MS > 0)
		cpu_delay_ms(CLK_XO_SETTLE_MS, current_hz);
	#endif
}


// Steps 1 and 2, all at the current speed: XO, then PLL1 scouts with the next speed's settings
static U8 pll_is_stable(const clk_speed_t *current, const clk_speed_t *next) {
	if ( (next->xo != CLK_XO_KEEP) && (current->osc == 1) )
		error();									// Sequence bug: XO change under a CPU that runs on it

	clk_select_xo(next->xo, current->hz);
	return clk_pll_start(1, next);
}


// Step 3, the gap at 12
static void reconfigure_mcu_clock_source(const clk_speed_t *next) {
	gpio_set_gpio_pin(AVR32_PIN_PX33);				// Gap starts
	pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCSEL_OSC0);
	if (!clk_pll_start(0, next))
		error();
	pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCCTRL_MCSEL_PLL0);
	gpio_clr_gpio_pin(AVR32_PIN_PX33);				// Gap ends
	pm_pll_disable(&AVR32_PM, 1);					// Scout off
}


// Pre-test, build with EXPERIMENT="-DFEATURE_CLOCK_TEST -DCLK_XO_PRETEST". Stays at 12 forever and swaps the
// XOs every 1.5 ms so the MCLK pin (PC04/XIN1) can be scoped during start-up and shut-down.
// PA22 is the trigger: rises when 24 is selected, falls when 22 is selected.
#ifdef CLK_XO_PRETEST
static void clk_xo_pretest(void) {
	while (1) {
		gpio_clr_gpio_pin(AVR32_PIN_PA23);			// 44.1 control off
		gpio_set_gpio_pin(AVR32_PIN_PA21);			// 48 control on
		gpio_set_gpio_pin(AVR32_PIN_PA22);
		cpu_delay_us(1500, FCPU_HZ_SLOW);

		gpio_clr_gpio_pin(AVR32_PIN_PA21);			// 48 control off
		gpio_set_gpio_pin(AVR32_PIN_PA23);			// 44.1 control on
		gpio_clr_gpio_pin(AVR32_PIN_PA22);
		cpu_delay_us(1500, FCPU_HZ_SLOW);
	}
}
#endif


// Lock pre-test, build with EXPERIMENT="-DFEATURE_CLOCK_TEST -DCLK_LOCK_PRETEST". Stays at 12 forever.
// Every 1.5 ms: PLL1 off, PA22 low, XO swap with no settle time, PLL1 on with the 45/49 settings (the same
// for both XOs), poll LOCK1, PA22 high as soon as LOCK1 is seen.
// PA22 low time = XO swap to LOCK1. The XO needs about 105-109 us to start (bench 2026-11-26), so a low time
// clearly above that says LOCK1 waits for a real clock. Shorter says LOCK1 is only a timer.
// Lock timeout: PA22 stays low for good.
#ifdef CLK_LOCK_PRETEST
static void clk_lock_pretest(void) {
	const clk_speed_t scout = {0, 1, 7, 2, CLK_XO_KEEP};	// OSC1 x 8/2, /2. Only the PLL fields are used
	U8 xo = CLK_XO_22;										// clock_test() starts with 24 on

	while (1) {
		pm_pll_disable(&AVR32_PM, 1);
		while (AVR32_PM.poscsr & AVR32_PM_POSCSR_LOCK1_MASK);	// No stale lock

		gpio_clr_gpio_pin(AVR32_PIN_PA22);
		clk_select_xo(xo, FCPU_HZ_SLOW);					// Old off, new on, no settle time
		if (!clk_pll_start(1, &scout))
			while (1);										// Timeout, PA22 stays low
		gpio_set_gpio_pin(AVR32_PIN_PA22);

		xo = (xo == CLK_XO_22) ? CLK_XO_24 : CLK_XO_22;
		cpu_delay_us(1500, FCPU_HZ_SLOW);
	}
}
#endif


static void clock_test(void) {
	const clk_speed_t *current_clock_source = &CLK_START;	// Still at 12 on OSC0 from pm_configure_clocks(&pm_freq_param_slow)
	const clk_speed_t *next_clock_source;
	U32 next = 0;
	U32 m = 1000;
	U32 n = ticks(current_clock_source->hz);
	U32 n_reset_value = n;

	// BCLK net: GCLK1 is never enabled here, keep PX35 a GPIO input. RX recovered MCLK off
	gpio_enable_gpio_pin(GCLK1);
	gpio_clr_gpio_pin(AVR32_PIN_PX22);

	// Start on 24, so the first switch to 45 is a real XO change
	gpio_clr_gpio_pin(AVR32_PIN_PA23);
	gpio_set_gpio_pin(AVR32_PIN_PA21);
	cpu_delay_ms(100, FCPU_HZ_SLOW);

	// OSC1 as external clock input, as in dxd_test(). It stays on, only the XO feeding it changes
	pm_enable_osc1_ext_clock(&AVR32_PM);
	pm_enable_clk1(&AVR32_PM, OSC1_STARTUP);

	#ifdef CLK_XO_PRETEST
		clk_xo_pretest();							// Never returns
	#endif
	#ifdef CLK_LOCK_PRETEST
		clk_lock_pretest();							// Never returns
	#endif

	// Set once for all speeds: no clock dividers, 1 flash wait state and high speed read mode (needed at 66,
	// harmless below)
	pm_cksel(&AVR32_PM, 0, 0, 0, 0, 0, 0);
	flashc_set_wait_state(1);
	#if (defined AVR32_FLASHC_210_H_INCLUDED)
		flashc_issue_command(AVR32_FLASHC_FCMD_CMD_HSEN, -1);
	#endif

	gpio_clr_gpio_pin(AVR32_PIN_PA22);
	gpio_clr_gpio_pin(AVR32_PIN_PX33);

	while (1) {
		next_clock_source = &CLK_SEQUENCE[next];
		next = (next + 1) % CLK_SEQUENCE_LEN;

		while (m--) {
			while (n--) {
				asm volatile ("nop");
				asm volatile ("nop");
				asm volatile ("nop");
				asm volatile ("nop");
			}
			n = n_reset_value;
			gpio_tgl_gpio_pin(AVR32_PIN_PA22);
		}
		m = 1000;									// 500 full periods, always starting with 0

		if (pll_is_stable(current_clock_source, next_clock_source)) {
			reconfigure_mcu_clock_source(next_clock_source);
			current_clock_source = next_clock_source;
			n_reset_value = ticks(current_clock_source->hz);
			n = n_reset_value;		
		}
		else {
			error();
		}
	}
}

#endif // FEATURE_CLOCK_TEST
