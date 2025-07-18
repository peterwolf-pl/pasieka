<?php
session_start();
$password = 'laser123'; // ustaw swoje hasło!

if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    if ($_POST['password'] == $password) {
        $_SESSION['logged_in'] = true;
        header('Location: index2.php');
        exit();
    } else {
        echo "Nieprawidłowe hasło";
    }
}
?>
<center>

<h1>zaloguj się do pasieki</h1>
<form method="post">
    <input type="password" name="password" placeholder="Hasło" required>
    <button type="submit">Zaloguj</button>
</form>
</center>
