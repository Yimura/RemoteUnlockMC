#include <concepts>
#include <driver/gpio.h>

template<gpio_num_t PinNumber, gpio_mode_t PinMode>
class GPIO
{
private:
public:
    GPIO()
    {
        gpio_set_direction(PinNumber, PinMode);
    }

    ~GPIO()
    {
        gpio_reset_pin(PinNumber);
    }

    int Read()
    requires(PinMode == GPIO_MODE_INPUT)
    {
        return gpio_get_level(PinNumber);
    }

    void Toggle(bool toggle)
    requires(PinMode == GPIO_MODE_OUTPUT)
    {
        gpio_set_level(PinNumber, toggle);
    }
};
