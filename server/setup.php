<?php
session_start();
if (!isset($_SESSION['logged_in'])) {
    header('Location: login.php');
    exit();
}

$configFile = __DIR__ . '/settings.json';

// load existing config
$all = [];
if (file_exists($configFile)) {
    $all = json_decode(file_get_contents($configFile), true) ?: [];
}
if (!isset($all['boards'])) {
    $all['boards'] = [];
}

function default_board($id) {
    return [
        'board_id' => $id,
        'firmwareUpdate' => false,
        'loopDelay' => 10,
        'offset' => -598696,
        'scale' => -25.353687,
        'wifi_ssid' => '',
        'wifi_password' => ''
    ];
}

// update configuration if form submitted
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $boardId = intval($_POST['board_id'] ?? 1);
    $boardConfig = default_board($boardId);
    $boardConfig['firmwareUpdate'] = isset($_POST['firmwareUpdate']);
    $boardConfig['loopDelay'] = intval($_POST['loopDelay']);
    $boardConfig['offset'] = floatval($_POST['offset']);
    $boardConfig['scale'] = floatval($_POST['scale']);
    $boardConfig['wifi_ssid'] = $_POST['wifi_ssid'] ?? '';
    $boardConfig['wifi_password'] = $_POST['wifi_password'] ?? '';

    $updated = false;
    foreach ($all['boards'] as $i => $b) {
        if (($b['board_id'] ?? null) == $boardId) {
            $all['boards'][$i] = $boardConfig;
            $updated = true;
            break;
        }
    }
    if (!$updated) {
        $all['boards'][] = $boardConfig;
    }
    file_put_contents($configFile, json_encode($all, JSON_PRETTY_PRINT));
    $message = 'Zapisano konfigurację dla płytki ' . $boardId . '.';
}

// prepare board configs for display
$boards = [];
for ($id = 1; $id <= 4; $id++) {
    $boards[$id] = default_board($id);
    foreach ($all['boards'] as $b) {
        if (($b['board_id'] ?? null) == $id) {
            $boards[$id] = array_merge($boards[$id], $b);
            break;
        }
    }
}
?>
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <title>Ustawienia</title>
    <style>
        form { margin-bottom: 2rem; padding: 1rem; border: 1px solid #ccc; }
    </style>
</head>
<body>
<?php if(isset($message)) echo "<p>$message</p>"; ?>
<?php foreach($boards as $b): ?>
<form method="post">
    <h3>Płytka <?php echo $b['board_id']; ?></h3>
    <label>
        <input type="checkbox" name="firmwareUpdate" <?php if($b['firmwareUpdate']) echo 'checked'; ?>>
        firmwareUPDATE
    </label><br>
    <label>Delay pętli:
        <select name="loopDelay">
            <option value="10" <?php if($b['loopDelay']==10) echo 'selected'; ?>>10s</option>
            <option value="30" <?php if($b['loopDelay']==30) echo 'selected'; ?>>30s</option>
            <option value="60" <?php if($b['loopDelay']==60) echo 'selected'; ?>>60s</option>
            <option value="600" <?php if($b['loopDelay']==600) echo 'selected'; ?>>600s</option>
            <option value="3600" <?php if($b['loopDelay']==3600) echo 'selected'; ?>>3600s</option>
        </select>
    </label><br>
    <label>Offset:
        <input type="text" name="offset" value="<?php echo htmlspecialchars($b['offset']); ?>">
    </label><br>
    <label>Scale:
        <input type="text" name="scale" value="<?php echo htmlspecialchars($b['scale']); ?>">
    </label><br>
    <label>WiFi SSID:
        <input type="text" name="wifi_ssid" value="<?php echo htmlspecialchars($b['wifi_ssid']); ?>">
    </label><br>
    <label>WiFi Hasło:
        <input type="password" name="wifi_password" value="<?php echo htmlspecialchars($b['wifi_password']); ?>">
    </label><br>
    <input type="hidden" name="board_id" value="<?php echo $b['board_id']; ?>">
    <button type="submit">Zapisz</button>
</form>
<?php endforeach; ?>
<p><a href="index2.php">Powrót</a></p>
</body>
</html>
