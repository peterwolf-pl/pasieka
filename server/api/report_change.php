<?php
include '../includes/config.php';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $json = file_get_contents('php://input');
    $data = json_decode($json, true);
    $logFile = __DIR__ . '/../change_log.txt';
    $entry = date('c') . ' ' . json_encode($data) . "\n";
    file_put_contents($logFile, $entry, FILE_APPEND);
    echo json_encode(['status' => 'received']);
} else {
    http_response_code(405);
    echo json_encode(['error' => 'Invalid method']);
}
?>
