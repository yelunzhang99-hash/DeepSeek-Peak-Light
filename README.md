# DeepSeek Peak Light

A C3-Mini app that shows whether DeepSeek API prices are at peak or off-peak (off-peak is half price).

My Level 3 entry for the Carenuity Home Challenge.

## Hardware

- Carenuity C3-Mini (ESP32-C3)
- 0.66" OLED shield, 64x48, SSD1306, I2C address 0x3C, stacked on the C3-Mini (SDA = GPIO8, SCL = GPIO10)
- Onboard RGB LED (GPIO7) and BOOT button (GPIO9)

## What it shows

- OLED: Beijing time, `PEAK` or `OFF-PEAK`, and when the next change happens
- RGB LED: red = peak, green = off-peak, blue = connecting

Peak hours are Monday to Friday, 09:00-12:00 and 14:00-18:00 Beijing time (UTC 01:00-04:00 and 06:00-10:00), from the [DeepSeek pricing page](https://api-docs.deepseek.com/quick_start/pricing). Chinese public holidays, which are off-peak all day, are not handled.

## Usage

1. On first start, the board opens the WiFi network `PeakTimer`. Join it with a phone and enter your home WiFi on the page that opens, or browse to `192.168.4.1`.
2. The board saves the WiFi, gets the time over NTP and shows the status.
3. Hold BOOT for 3 seconds to forget the WiFi and set it up again.

If the OLED looks shifted or garbled after flashing other firmware, unplug USB once. The display keeps old settings until it loses power.

## Build

PlatformIO:

```
pio run -e c3_mini
python make_merged_bin.py <path to esptool.py>
```

The merged binary is written to `dist/` and flashes at offset `0x0`.

Arduino IDE: board "ESP32C3 Dev Module", USB CDC On Boot "Enabled". Install the libraries Adafruit SSD1306, Adafruit GFX, Adafruit NeoPixel and WiFiManager (tzapu).

## Libraries

- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306), [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library), [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel)
- [WiFiManager](https://github.com/tzapu/WiFiManager)
