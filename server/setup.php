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
    $config = [
        'firmwareUpdate' => $firmwareUpdate,
        'loopDelay' => $loopDelay
    ];
    file_put_contents($configFile, json_encode($config));
    $message = 'Zapisano konfigurację.';
} else {
    if (file_exists($configFile)) {
        $config = json_decode(file_get_contents($configFile), true);
        $firmwareUpdate = $config['firmwareUpdate'];
        $loopDelay = $config['loopDelay'];
    } else {
        $firmwareUpdate = false;
        $loopDelay = 10;
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
    <button type="submit">SETUP</button>
</form>
<p><a href="index2.php">Powrót</a></p>
</body>
</html>
