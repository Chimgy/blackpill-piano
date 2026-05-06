#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <stdint.h>


static const int16_t sine_table[256] = {
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602,
    6393, 7179, 7962, 8739, 9512, 10278, 11039, 11793,
    12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530,
    18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594,
    23170, 23731, 24279, 24811, 25329, 25832, 26319, 26790,
    27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971,
    32137, 32285, 32412, 32521, 32609, 32678, 32728, 32757,
    32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285,
    32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571,
    30273, 29956, 29621, 29268, 28898, 28510, 28105, 27683,
    27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868,
    18204, 17530, 16846, 16151, 15446, 14732, 14010, 13279,
    12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179,
    6393, 5602, 4808, 4011, 3212, 2410, 1608, 804,
    0, -804, -1608, -2410, -3212, -4011, -4808, -5602,
    -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530,
    -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
    -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
    -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
    -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971,
    -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285,
    -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
    -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
    -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
    -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868,
    -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
    -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179,
    -6393, -5602, -4808, -4011, -3212, -2410, -1608, -804
};

static void delay(uint32_t n) {
    for (uint32_t i = 0; i < n; i++) {
        __asm__("nop");
    }
}

int main(void) {
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);
    /* Enable PLLI2S: N=192, R=2 -> 96MHz I2S clock */
    RCC_PLLI2SCFGR = (2 << 28) | (192 << 6);
    RCC_CR |= RCC_CR_PLLI2SON;
    while (!(RCC_CR & RCC_CR_PLLI2SRDY));

    /* Select PLLI2S as I2S clock source */
    RCC_CFGR &= ~(1 << 23);

    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_SPI2);

    /* LED */
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE,
                    GPIO12 | GPIO13 | GPIO15);
    gpio_set_af(GPIOB, GPIO_AF5, GPIO12 | GPIO13 | GPIO15);

    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ,
                        GPIO12 | GPIO13 | GPIO15);
    
    SPI2_I2SCFGR = 0;
    SPI2_I2SPR   = 0;
    SPI2_I2SPR   = (31 << 0) | (0 << 8);
    SPI2_I2SCFGR = SPI_I2SCFGR_I2SMOD |
                   SPI_I2SCFGR_I2SCFG_MASTER_TRANSMIT |
                   SPI_I2SCFGR_I2SSTD_I2S_PHILIPS |
                   SPI_I2SCFGR_DATLEN_16BIT |
                   SPI_I2SCFGR_I2SE;
    
    /* Debug: blink out I2SCFGR value */
    uint32_t cfg = SPI2_I2SCFGR;
    uint32_t pr  = SPI2_I2SPR;
    
    /* Blink I2SMOD bit (bit 11) - should be set */
    if (cfg & (1 << 11)) {
        for (int i = 0; i < 6; i++) {
            gpio_toggle(GPIOC, GPIO13);
            delay(200000);
        }
    }
    
    /* Blink I2SE bit (bit 10) - should be set */  
    if (cfg & (1 << 10)) {
        for (int i = 0; i < 6; i++) {
            gpio_toggle(GPIOC, GPIO13);
            delay(2000000);
        }
    }

    /* Pause between checks */
    delay(2000000);

    /* Blink I2SCFG bits (9:8) - should be 10 for master transmit */
    uint32_t i2scfg = (cfg >> 8) & 0x3;
    for (uint32_t i = 0; i < i2scfg; i++) {
        gpio_toggle(GPIOC, GPIO13);
        delay(500000);
    }

    /* Check I2SE bit is actually set */
    if (SPI2_I2SCFGR & SPI_I2SCFGR_I2SE) {
        /* Fast blink = I2S enabled */
        for (int i = 0; i < 10; i++) {
            gpio_toggle(GPIOC, GPIO13);
            delay(100000);
        }
    } else {
        /* Slow blink = I2S not enabled */
        for (int i = 0; i < 3; i++) {
            gpio_toggle(GPIOC, GPIO13);
            delay(1000000);
        }
    }

    uint8_t idx = 0;
    while (1) {
        SPI2_DR = (uint16_t)sine_table[idx];
        SPI2_DR = (uint16_t)sine_table[idx];
        idx++;
    }

    return 0;
}