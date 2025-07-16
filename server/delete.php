<?php
session_start();
if (!isset($_SESSION['logged_in'])) {
    header('Location: login.php');
    exit();
}
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['board'])) {
    $board = intval($_POST['board']);
    // --- Ustawienia bazy ---
    $db = new mysqli('localhost', 'DB_USER', 'DB_PASS', 'DB_NAME');
    if ($db->connect_errno) {
        header('Location: index2.php?error=delete');
        exit();
    }
    // Usuwanie tylko rekordów danego ula
    $stmt = $db->prepare("DELETE FROM ul_waga WHERE board = ?");
    $stmt->bind_param('i', $board);
    $stmt->execute();
    $stmt->close();
    $db->close();
    header('Location: index2.php?deleted=' . $board);
    exit();
}
header('Location: index2.php?error=delete');
exit();
?>
