This project contains two parts:

- **client** – ESP32 sketch for the beehive scale
- **server** – PHP website with API endpoints

The setup page (`server/setup.php`) lets you configure WiFi credentials and
other settings. It now displays four separate forms so you can adjust each
board's configuration individually. All values are saved in
`server/settings.json`.

`server/settings.json` now holds configuration for four boards (IDs 1–4). Each
device reads its section based on its compiled `BOARD_ID` value.

`index2.php` mirrors the main `index.html` page but adds a link to the setup
form and allows you to choose the chart colour and type. The page displays four
charts so you can monitor up to four boards simultaneously.

To verify the PHP scripts you can run `php -l` on `server/setup.php`, `server/index2.php`, `server/data.php` and `server/api/add.php`.

The SQL script `server/sql/update_board_id.sql` adds the `board_id` column to the
`measurements` table and updates previous records so the new multi-board
features work correctly.
The script `server/sql/add_frequency_column.sql` adds a `frequency` column so you can
store microphone readings in Hz alongside weight measurements.

The ESP32 firmware reads an INMP441 digital microphone. Connect the module's
pins as follows:

- **VCC** to the ESP32's 3.3&nbsp;V
- **GND** to any ground
- **SCK (BCLK)** to GPIO26
- **WS** to GPIO25
- **SD** to GPIO33

The sketch sends the measured frequency in the `hz` field of the API request.

For environments where only the microphone is needed, a simplified
`microphone_esp8266.ino` sketch is provided for the ESP8266. Connect the
INMP441 pins like so:

- **VCC** to the ESP8266's 3.3&nbsp;V
- **GND** to any ground
- **SCK (BCLK)** to GPIO14 (D5)
- **WS** to GPIO15 (D8)
- **SD** to GPIO13 (D7)

These pins are fixed by the ESP8266 hardware, so the sketch does not set
them in code.

This firmware reads the microphone and periodically posts the frequency to the
same `add.php` endpoint without weight measurements. It relies on the
`core_esp8266_i2s.h` API from the ESP8266 Arduino core, so be sure the
appropriate board package is installed when compiling.

The charts on both public and admin pages now show weight deltas for the last
12, 24 and 72 hours above each plot.
