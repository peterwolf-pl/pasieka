<?php
$dbhost = "localhost";
$dbuser = "_apibot";
$dbpass = "";
$dbname = "_apibot";

$conn = new mysqli($dbhost, $dbuser, $dbpass, $dbname);

if ($conn->connect_error) {
    die("Błąd połączenia z bazą: " . $conn->connect_error);
}
?>
