# Initial bench pinout

Use any GPIOs that are convenient and available on the chosen Pico carrier. The firmware API does not hard-code a board-specific connector.

Example:

```text
P00 GP2  TX  ───────► P01 GP3  RX
P00 GP3  RX  ◄─────── P01 GP2  TX
P00 GND     ───────── P01 GND
```

This is a point-to-point TTL/CMOS GPIO connection. Keep the first test short and use a common ground. Do not connect two outputs directly to each other.
