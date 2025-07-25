<?php
include '../includes/config.php';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $json = file_get_contents('php://input');
    $data = json_decode($json, true);

if (isset($data['weight']) || isset($data['hz'])) {
    $weight = isset($data['weight']) ? floatval($data['weight']) : null;
    $hz     = isset($data['hz']) ? floatval($data['hz']) : null;
    $board  = isset($data['board']) ? intval($data['board']) : 1;

    $stmt = $conn->prepare("INSERT INTO measurements (weight, frequency, board_id) VALUES (?, ?, ?)");
    if($stmt){
        $stmt->bind_param("ddi", $weight, $hz, $board);
        if($stmt->execute()){
            echo json_encode(["status" => "success"]);
        }else{
            http_response_code(500);
            echo json_encode(["error" => $stmt->error]);
        }
    } else {
        http_response_code(500);
        echo json_encode(["error" => $conn->error]);
    }
} else {
    http_response_code(400);
    echo json_encode(["error" => "Missing data"]);
}
} else {
    http_response_code(405);
    echo json_encode(["error" => "Invalid method"]);
}
