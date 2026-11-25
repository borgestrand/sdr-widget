/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */

/* dxd_test.c - 20261124: minimal DXD (352.8k/384k, 32-bit) clock tree bring-up. Not production code.
 *
 * Build with EXPERIMENT=-DFEATURE_DXD_TEST. composite_widget.c includes this file, and main() calls
 * dxd_test() right after the early SPRX GPIO setup. It never returns: no FreeRTOS, no USB, no WM8804 setup.
 *
 * Clock tree in this test:
 *   OSC0 12 MHz crystal   - untouched, not used after the switch (USB is not started)
 *   OSC1 = MCLK on XIN1   - 24.576 MHz (48 family XO) or 22.5792 MHz (44.1 family XO), also wired straight to the DAC
 *   PLL0 <- OSC1          - VCO = OSC1 * (PLLMUL+1) / PLLDIV = OSC1 * 8/2 = 98.304 / 90.3168 MHz, output /2
 *   main = CPU = HSB = PBA = 2 x MCLK = 49.152 / 45.1584 MHz
 *   SSC master            - divided clock DIV = 1: TK = PBA/2 = 64 fs on PX23, TF = LRCK on PX27, 32-bit slots
 *   GCLK1 / PX35          - never enabled, pin kept as GPIO input, because PX35 and PX23 share the BCLK net
 *
 * Output: left = 1 kHz sine at -6 dBFS (918.75 Hz at 352.8k), right = digital zero. Hearing the tone on the
 * left tells us L/R is right. PX33 toggles at every DMA buffer reload (one sine period, about 1 ms).
 * Once per second the UART (57600 baud, as usual) prints the reload count and some PCM5142 status registers.
 */

#ifdef FEATURE_DXD_TEST

#include "flashc.h"
#include "I2C.h"


#ifndef DXD_TEST_RATE
	#define DXD_TEST_RATE		384000		// Or -DDXD_TEST_RATE=352800 in EXPERIMENT
#endif

#if (DXD_TEST_RATE == 384000)
	#define DXD_CPU_HZ			49152000	// 2 x 24.576 MHz
	#define DXD_TONE_SAMPLES	384			// 1 kHz
#elif (DXD_TEST_RATE == 352800)
	#define DXD_CPU_HZ			45158400	// 2 x 22.5792 MHz
	#define DXD_TONE_SAMPLES	384			// 918.75 Hz, same table
#else
	#error DXD_TEST_RATE must be 384000 or 352800
#endif

#define DXD_BUF_WORDS			(2 * DXD_TONE_SAMPLES)	// Interleaved L, R


static volatile S32 dxd_buf[DXD_BUF_WORDS];
static volatile U32 dxd_reloads = 0;


// Same idea as spk_pdca_int_handler() in taskAK5394A.c: point the reload register at the same buffer again
__attribute__((__interrupt__)) static void dxd_pdca_int_handler(void) {
	pdca_reload_channel(PDCA_CHANNEL_SSC_TX, (void *)dxd_buf, DXD_BUF_WORDS);
	gpio_tgl_gpio_pin(AVR32_PIN_PX33);
	dxd_reloads++;
}


// One sine period by recursion, s[n] = 2cos(t) s[n-1] - s[n-2], no libm. cos(t) and sin(t) by Taylor series
static void dxd_make_tone(void) {
	const double t = 2.0 * 3.14159265358979323846 / DXD_TONE_SAMPLES;
	const double t2 = t * t;
	const double c2 = 2.0 * (1.0 - t2/2.0 + t2*t2/24.0 - t2*t2*t2/720.0);
	double s0 = 0.0;
	double s1 = t - t*t2/6.0 + t*t2*t2/120.0;
	double s2;
	int i;

	for (i = 0; i < DXD_TONE_SAMPLES; i++) {
		dxd_buf[2*i] = (S32)(s0 * 1073741823.0);	// Left, -6 dBFS
		dxd_buf[2*i + 1] = 0;						// Right, digital zero
		s2 = c2 * s1 - s0;
		s0 = s1;
		s1 = s2;
	}
}


// Write one PCM5142 register on both possible chip addresses, without FreeRTOS. A missing second chip just NACKs
static void dxd_dac_write(U8 reg, U8 val) {
	U8 dev_data[2] = {reg, val};

	gpio_set_gpio_pin(AVR32_PIN_PX17);				// I2C enable for DAC
	cpu_delay_us(500, DXD_CPU_HZ);
	twi_write_out(PCM5142_DEV_ADR, dev_data, 2);
	twi_write_out(PCM5142_DEV_ADR_SECONDARY, dev_data, 2);
	gpio_clr_gpio_pin(AVR32_PIN_PX17);				// I2C disable for DAC
	cpu_delay_us(500, DXD_CPU_HZ);
}


// Read one PCM5142 register from the first chip, 0xEE if I2C fails (0xEE is not a likely register value)
static U8 dxd_dac_read(U8 reg) {
	U8 dev_data[1] = {reg};

	gpio_set_gpio_pin(AVR32_PIN_PX17);				// I2C enable for DAC
	cpu_delay_us(500, DXD_CPU_HZ);
	if (twi_write_out(PCM5142_DEV_ADR, dev_data, 1) == TWI_SUCCESS) {
		if (twi_read_in(PCM5142_DEV_ADR, dev_data, 1) != TWI_SUCCESS)
			dev_data[0] = 0xEE;
	}
	else
		dev_data[0] = 0xEE;
	gpio_clr_gpio_pin(AVR32_PIN_PX17);				// I2C disable for DAC
	cpu_delay_us(500, DXD_CPU_HZ);
	return dev_data[0];
}


static void dxd_print_reg(U8 reg) {
	print_dbg_char(' ');
	print_dbg_char_hex(reg);
	print_dbg_char(':');
	print_dbg_char_hex(dxd_dac_read(reg));
}


static const pdca_channel_options_t DXD_PDCA_OPTIONS = {
	.addr = (void *)dxd_buf,
	.pid = AVR32_PDCA_PID_SSC_TX,
	.size = DXD_BUF_WORDS,
	.r_addr = (void *)dxd_buf,
	.r_size = DXD_BUF_WORDS,
	.transfer_size = PDCA_TRANSFER_SIZE_WORD
};


static const gpio_map_t DXD_SSC_TX_GPIO_MAP = {
	{SSC_TX_CLOCK,		SSC_TX_CLOCK_FUNCTION},		// PX23, now an output: BCLK
	{SSC_TX_DATA,		SSC_TX_DATA_FUNCTION},
	{SSC_TX_FRAME_SYNC,	SSC_TX_FRAME_SYNC_FUNCTION}	// PX27, LRCK
};


static void dxd_test(void) {
	volatile avr32_ssc_t *dxd_ssc = &AVR32_SSC;

	// Still at 12 MHz on OSC0 from pm_configure_clocks(&pm_freq_param_slow)

	// 1. BCLK net: GCLK1 is never enabled in this build. Keep PX35 as a GPIO input so only PX23 drives the net
	gpio_enable_gpio_pin(GCLK1);

	// 2. XO for the chosen family, the same pins as mobo_xo_select()
	#if (DXD_TEST_RATE == 384000)
		gpio_set_gpio_pin(AVR32_PIN_PA21);			// 48 control
		gpio_clr_gpio_pin(AVR32_PIN_PA23);			// 44.1 control
	#else
		gpio_set_gpio_pin(AVR32_PIN_PA23);			// 44.1 control
		gpio_clr_gpio_pin(AVR32_PIN_PA21);			// 48 control
	#endif
	gpio_clr_gpio_pin(AVR32_PIN_PX22);				// Disable RX recovered MCLK
	cpu_delay_ms(100, FCPU_HZ_SLOW);				// Let the XO settle

	// 3. OSC1 as external clock input, as in AK5394A_task_init()
	pm_enable_osc1_ext_clock(&AVR32_PM);
	pm_enable_clk1(&AVR32_PM, OSC1_STARTUP);		// Waits for OSC1 ready

	// 4. PLL0 from OSC1: (7+1)/2 = x4 into the 80-180 MHz VCO range, then /2
	pm_pll_setup(&AVR32_PM, 0,	// pll
		7,						// mul
		2,						// div
		1,						// osc: OSC1
		16);					// lockcount
	pm_pll_set_option(&AVR32_PM, 0,	// pll
		1,						// pll_freq: VCO range 80-180 MHz
		1,						// pll_div2
		0);						// pll_wbwdisable
	pm_pll_enable(&AVR32_PM, 0);
	pm_wait_for_pll0_locked(&AVR32_PM);

	// 5. No dividers: CPU = HSB = PBA = PBB. Flash wait state as in pm_configure_clocks(), then switch
	pm_cksel(&AVR32_PM, 0, 0, 0, 0, 0, 0);
	flashc_set_wait_state(1);
	#if (defined AVR32_FLASHC_210_H_INCLUDED)
		if (DXD_CPU_HZ > AVR32_FLASHC_HSEN_FWS_1_MAX_FREQ)
			flashc_issue_command(AVR32_FLASHC_FCMD_CMD_HSEN, -1);
		else
			flashc_issue_command(AVR32_FLASHC_FCMD_CMD_HSDIS, -1);
	#endif
	pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCCTRL_MCSEL_PLL0);

	// 6. Now running at 2 x MCLK
	init_dbg_rs232(DXD_CPU_HZ);
	print_dbg("\nDXD test ");
	print_dbg_ulong(DXD_TEST_RATE);
	print_dbg(" CPU=PBA ");
	print_dbg_ulong(DXD_CPU_HZ);
	print_dbg_char('\n');

	// 7. I2C: WM8804 out of reset first, as in main(), its reset state drives SDA as a GPIO
	INTC_init_interrupts();
	wm8804_reset(WM8804_RESET_END);
	cpu_delay_ms(10, DXD_CPU_HZ);
	twi_init();				// Speed is tuned for 66 MHz PBA, so it runs at about 49/66 of that: slower, not faster

	// 8. PCM5142: 32-bit I2S, as pcm5142_init() does with FEATURE_ALT3_32BIT. Everything else stays at reset values
	dxd_dac_write(0x00, 0x00);		// Page 0
	dxd_dac_write(0x02, 0x10);		// Standby request
	dxd_dac_write(0x28, 0x03);		// I2S, 32 bits
	dxd_dac_write(0x02, 0x00);		// Exit standby
	print_dbg("DAC 28:");
	print_dbg_char_hex(dxd_dac_read(0x28));		// Expect 03
	print_dbg_char('\n');

	// 9. Data and DMA
	dxd_make_tone();
	INTC_register_interrupt((__int_handler)&dxd_pdca_int_handler, AVR32_PDCA_IRQ_1, AVR32_INTC_INT0);

	// 10. SSC as I2S master. ssc_i2s_init() SSC_I2S_MODE_STEREO_OUT is Atmel's own master path, not the TK pin input
	// path we use for production: DIV = (PBA + bitrate) / (2 x bitrate) = 1 with bitrate = 2 x 32 x fs = PBA/2
	gpio_enable_module(DXD_SSC_TX_GPIO_MAP, sizeof(DXD_SSC_TX_GPIO_MAP) / sizeof(DXD_SSC_TX_GPIO_MAP[0]));
	ssc_i2s_init(dxd_ssc, DXD_TEST_RATE, 32, 32, SSC_I2S_MODE_STEREO_OUT, DXD_CPU_HZ);
	print_dbg("SSC CMR:");
	print_dbg_hex(dxd_ssc->cmr);				// Expect 00000001
	print_dbg_char('\n');

	// Attempt at L/R order: stop TX, let the DMA put the first left word in THR, then start TX. Not verified,
	// the tone on the left channel will tell
	dxd_ssc->cr = AVR32_SSC_CR_TXDIS_MASK;
	pdca_init_channel(PDCA_CHANNEL_SSC_TX, &DXD_PDCA_OPTIONS);
	pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_TX);
	pdca_enable(PDCA_CHANNEL_SSC_TX);
	Enable_global_interrupt();
	dxd_ssc->cr = AVR32_SSC_CR_TXEN_MASK;

	// 11. The ampmeter: total DMA reloads (should grow by 1000 per second at 384k, about 919 at 352.8k) and
	// PCM5142 status. Registers to look up in the PCM5142 datasheet: 5B-5F clock detection and errors, 76 power state
	while (1) {
		cpu_delay_ms(1000, DXD_CPU_HZ);
		print_dbg_ulong(dxd_reloads);
		dxd_print_reg(0x5B);
		dxd_print_reg(0x5C);
		dxd_print_reg(0x5D);
		dxd_print_reg(0x5E);
		dxd_print_reg(0x5F);
		dxd_print_reg(0x76);
		print_dbg_char('\n');
	}
}

#endif // FEATURE_DXD_TEST
