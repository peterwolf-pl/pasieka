<?php
session_start();
if (!isset($_SESSION['logged_in'])) {
    http_response_code(403);
    exit();
}

include 'includes/config.php';

$result = $conn->query("SELECT * FROM measurements ORDER BY timestamp ASC");

$data = ['weights' => [], 'timestamps' => []];
while ($row = $result->fetch_assoc()) {
    $data['weights'][] = $row['weight'];
    $data['timestamps'][] = $row['timestamp'];
}

header('Content-Type: application/json');
echo json_encode($data);
