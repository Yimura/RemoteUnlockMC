#pragma once
#include <led_strip.h>

namespace RemoteUnlock
{
    struct Color
    {
        uint8_t m_Red;
        uint8_t m_Green;
        uint8_t m_Blue;
    };

    template<gpio_num_t PinNumber>
    class OnboardLed
    {
    private:
        led_strip_config_t m_StripConfig;
        led_strip_rmt_config_t m_RmtConfig;
        led_strip_handle_t m_LedStrip;

        Color m_Colors;

        uint8_t m_Brightness;

    public:
        OnboardLed();
        virtual ~OnboardLed();

        void SetColor(Color colors, uint8_t brightness = 255);
        void SetColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness = 255);
    };

    template<gpio_num_t PinNumber>
    inline OnboardLed<PinNumber>::OnboardLed()
    {
        m_StripConfig = {
            .strip_gpio_num         = PinNumber,
            .max_leds               = 1,
            .led_model              = LED_MODEL_WS2812,
            .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
            .flags                  = {.invert_out = false},
        };
        m_RmtConfig = {
            .clk_src           = RMT_CLK_SRC_DEFAULT,
            .resolution_hz     = 10 * 1000 * 1000,
            .mem_block_symbols = 64,
            .flags             = {.with_dma = false},
        };

        led_strip_new_rmt_device(&m_StripConfig, &m_RmtConfig, &m_LedStrip);
        led_strip_clear(m_LedStrip);
    }

    template<gpio_num_t PinNumber>
    inline OnboardLed<PinNumber>::~OnboardLed()
    {
        led_strip_clear(m_LedStrip);
    }

    template<gpio_num_t PinNumber>
    inline void OnboardLed<PinNumber>::SetColor(Color colors, uint8_t brightness)
    {
        float normalize = 255.f / static_cast<float>(brightness);
        float red       = static_cast<float>(colors.m_Red) / normalize;
        float green     = static_cast<float>(colors.m_Green) / normalize;
        float blue      = static_cast<float>(colors.m_Blue) / normalize;

        led_strip_set_pixel(m_LedStrip, 0, red, green, blue);
        led_strip_refresh(m_LedStrip);
    }

    template<gpio_num_t PinNumber>
    inline void OnboardLed<PinNumber>::SetColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness)
    {
        SetColor({red, green, blue}, brightness);
    }

    constexpr Color WHITE = {255, 255, 255};
    constexpr Color RED   = {255, 0, 0};
    constexpr Color GREEN = {0, 255, 0};
    constexpr Color BLUE  = {0, 0, 255};
} // namespace RemoteUnlock
