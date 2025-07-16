<?php
session_start();
if (!isset($_SESSION['logged_in'])) {
    header('Location: login.php');
    exit();
}
if (isset($_GET['board'])) {
    $board = intval($_GET['board']);
    $db = new mysqli('localhost', 'DB_USER', 'DB_PASS', 'DB_NAME');
    if ($db->connect_errno) {
        header('Location: index2.php?error=export');
        exit();
    }
    $stmt = $db->prepare("SELECT timestamp, weight FROM ul_waga WHERE board = ? ORDER BY timestamp ASC");
    $stmt->bind_param('i', $board);
    $stmt->execute();
    $stmt->bind_result($timestamp, $weight);

    header('Content-Type: text/csv; charset=utf-8');
    header("Content-Disposition: attachment; filename=ula{$board}_history.csv");
    $output = fopen('php://output', 'w');
    fputcsv($output, ['Czas pomiaru', 'Waga (g)']);
    while ($stmt->fetch()) {
        fputcsv($output, [$timestamp, $weight]);
    }
    fclose($output);
    $stmt->close();
    $db->close();
    exit();
}
header('Location: index2.php?error=export');
exit();
?>
