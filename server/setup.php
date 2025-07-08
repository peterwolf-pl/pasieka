<?php
session_start();
if (!isset($_SESSION['logged_in'])) {
    header('Location: login.php');
    exit();
}

$configFile = __DIR__ . '/settings.json';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $firmwareUpdate = isset($_POST['firmwareUpdate']);
    $loopDelay = intval($_POST['loopDelay']);
    $offset = floatval($_POST['offset']);
    $scale  = floatval($_POST['scale']);
    $ssid  = $_POST['wifi_ssid'] ?? '';
    $pass  = $_POST['wifi_password'] ?? '';
    $boardId = intval($_POST['board_id'] ?? 1);
    $config = [
        'firmwareUpdate' => $firmwareUpdate,
        'loopDelay' => $loopDelay,
        'offset' => $offset,
        'scale' => $scale,
        'wifi_ssid' => $ssid,
        'wifi_password' => $pass,
        'board_id' => $boardId
    ];
    file_put_contents($configFile, json_encode($config));
    $message = 'Zapisano konfigurację.';
} else {
    if (file_exists($configFile)) {
        $config = json_decode(file_get_contents($configFile), true);
        $firmwareUpdate = $config['firmwareUpdate'];
        $loopDelay = $config['loopDelay'];
        $offset = $config['offset'];
        $scale  = $config['scale'];
        $ssid   = $config['wifi_ssid'] ?? '';
        $pass   = $config['wifi_password'] ?? '';
        $boardId = $config['board_id'] ?? 1;
    } else {
        $firmwareUpdate = false;
        $loopDelay = 10;
        $offset = -598696;
        $scale  = -25.353687;
        $ssid = '';
        $pass = '';
        $boardId = 1;
    }
}
?>
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <title>Ustawienia</title>
</head>
<body>
<?php if(isset($message)) echo "<p>$message</p>"; ?>
<form method="post">
    <label>
        <input type="checkbox" name="firmwareUpdate" <?php if($firmwareUpdate) echo 'checked'; ?>>
        firmwareUPDATE
    </label><br>
    <label>Delay pętli:
        <select name="loopDelay">
            <option value="10" <?php if($loopDelay==10) echo 'selected'; ?>>10s</option>
            <option value="30" <?php if($loopDelay==30) echo 'selected'; ?>>30s</option>
            <option value="60" <?php if($loopDelay==60) echo 'selected'; ?>>60s</option>
            <option value="600" <?php if($loopDelay==600) echo 'selected'; ?>>600s</option>
            <option value="3600" <?php if($loopDelay==3600) echo 'selected'; ?>>3600s</option>
        </select>
    </label><br>
    <label>Offset:
        <input type="text" name="offset" value="<?php echo htmlspecialchars($offset); ?>">
    </label><br>
    <label>Scale:
        <input type="text" name="scale" value="<?php echo htmlspecialchars($scale); ?>">
    </label><br>
    <label>WiFi SSID:
        <input type="text" name="wifi_ssid" value="<?php echo htmlspecialchars($ssid); ?>">
    </label><br>
    <label>WiFi Hasło:
        <input type="password" name="wifi_password" value="<?php echo htmlspecialchars($pass); ?>">
    </label><br>
    <label>ID płytki:
        <input type="number" name="board_id" value="<?php echo htmlspecialchars($boardId); ?>" min="1">
    </label><br>
    <button type="submit">SETUP</button>
</form>
<p><a href="index2.php">Powrót</a></p>
</body>
</html>
