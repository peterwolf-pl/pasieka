<?php
session_start();
if (!isset($_SESSION['logged_in'])) {
    http_response_code(403);
    exit();
}

include 'includes/config.php';

$board = isset($_GET['board']) ? intval($_GET['board']) : 1;
$stmt = $conn->prepare("SELECT weight, frequency, timestamp FROM measurements WHERE board_id=? ORDER BY timestamp ASC");
$stmt->bind_param("i", $board);
$stmt->execute();
$result = $stmt->get_result();

$data = ['weights' => [], 'hz' => [], 'timestamps' => []];
while ($row = $result->fetch_assoc()) {
    $data['weights'][] = $row['weight'];
    $data['hz'][] = $row['frequency'];
    $data['timestamps'][] = $row['timestamp'];
}

header('Content-Type: application/json');
echo json_encode($data);
