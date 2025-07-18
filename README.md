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
