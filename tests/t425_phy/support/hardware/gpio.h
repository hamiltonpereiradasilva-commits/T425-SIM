#ifndef T425_TEST_GPIO_H
#define T425_TEST_GPIO_H
#include <stdbool.h>
typedef unsigned int uint;
#define GPIO_IN false
void gpio_init(uint pin);
void gpio_set_dir(uint pin, bool out);
bool gpio_get(uint pin);
#endif
