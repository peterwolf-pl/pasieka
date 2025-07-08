This project contains two parts:

- **client** – ESP32 sketch for the beehive scale
- **server** – PHP website with API endpoints

The setup page (`server/setup.php`) lets you configure WiFi credentials and the
board ID stored in `server/settings.json`.

`index2.php` mirrors the main `index.html` page but adds a link to the setup
form and allows you to choose the chart colour and type. The page displays four
charts so you can monitor up to four boards simultaneously.

To verify the PHP scripts you can run `php -l` on `server/setup.php`, `server/index2.php`, `server/data.php` and `server/api/add.php`.
