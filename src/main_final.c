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


int main(void) {
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_96MHZ]);

    /* PLLI2S is set up in the following clock pipeline 
       Crystal clock = 25Mhz -> M=25, N=192, R=2 -> 96 MHz clean for audio*/
    RCC_PLLI2SCFGR = (2 << 28) | (192 << 6) | 25;
    RCC_CR |= RCC_CR_PLLI2SON;
    while (!(RCC_CR & RCC_CR_PLLI2SRDY));

    /* Apply PLLI2S AS I2s Clock source */
    RCC_CFGR &= ~(1 << 23);

    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_SPI2);

    /* LED */
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO13);
    /* I2S Pins */
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, 
                    GPIO12 | GPIO13 | GPIO15);
    gpio_set_af(GPIOB, GPIO_AF5, GPIO12 | GPIO13 | GPIO15);

    SPI2_I2SCFGR = 0;
    SPI2_I2SPR   = 0;
    SPI2_I2SPR   = (31 << 0) | (0 << 8);
    SPI2_I2SCFGR = SPI_I2SCFGR_I2SMOD |
                (SPI_I2SCFGR_I2SCFG_MASTER_TRANSMIT << SPI_I2SCFGR_I2SCFG_LSB) |
                SPI_I2SCFGR_I2SSTD_I2S_PHILIPS |
                SPI_I2SCFGR_DATLEN_16BIT |
                SPI_I2SCFGR_I2SE;
    
    /* Blink 3 times on boot 
       for (int i = 0; i < 6; i++) {
        gpio_toggle(GPIOC, GPIO13);
        for (uint32_t d = 0; d < 500000; d++) __asm__("nop");
    }
    */
 

    /* Keep writing to force the peripheral to clock */
    uint8_t idx = 0;
    uint32_t counter = 0;
    while (1) {
        SPI2_DR = (uint16_t)sine_table[idx];
        SPI2_DR = (uint16_t)sine_table[idx];
        idx++;

        counter++; 
        if (counter >= 10000) {
            gpio_toggle(GPIOC, GPIO13);
            counter = 0;
        }
    }

    return 0;
}