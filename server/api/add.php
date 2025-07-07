<?php
include '../includes/config.php';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $json = file_get_contents('php://input');
    $data = json_decode($json, true);

    if (isset($data['weight'])) {
        $weight = floatval($data['weight']);
        $board  = isset($data['board']) ? intval($data['board']) : 1;

        $stmt = $conn->prepare("INSERT INTO measurements (weight, board_id) VALUES (?, ?)");
        $stmt->bind_param("di", $weight, $board);
        $stmt->execute();

        echo json_encode(["status" => "success"]);
    } else {
        http_response_code(400);
        echo json_encode(["error" => "Missing weight"]);
    }
} else {
    http_response_code(405);
    echo json_encode(["error" => "Invalid method"]);
}
