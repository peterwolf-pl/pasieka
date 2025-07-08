This project contains two parts:

- **client** – ESP32 sketch for the beehive scale
- **server** – PHP website with API endpoints

The setup page (`server/setup.php`) lets you configure WiFi credentials and the board ID stored in `server/settings.json`.

To verify the PHP scripts you can run `php -l` on `server/setup.php`, `server/index2.php`, `server/data.php` and `server/api/add.php`.
