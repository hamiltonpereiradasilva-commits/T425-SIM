#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "t425_application.h"
bool t425_send_command(t425_application_t *a, const char *line, uint32_t now);
