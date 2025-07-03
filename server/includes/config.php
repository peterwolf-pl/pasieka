<?php
$dbhost = "localhost";
$dbuser = "srv51934_apibot";
$dbpass = "KQ9atgTFpEsHDvDHCPZT";
$dbname = "srv51934_apibot";

$conn = new mysqli($dbhost, $dbuser, $dbpass, $dbname);

if ($conn->connect_error) {
    die("Błąd połączenia z bazą: " . $conn->connect_error);
}
?>
