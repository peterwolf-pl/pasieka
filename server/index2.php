<?php
session_start();
if (!isset($_SESSION['logged_in'])) {
    header('Location: login.php');
    exit();
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>Pszczoły - pomiary wagi ula</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
<h2>Wykres masy ula</h2>
<canvas id="chart" width="600" height="400"></canvas>
<a href="logout.php">Wyloguj</a>

<script>
let chart;

function fetchAndUpdate() {
    fetch('data.php')
        .then(response => response.json())
        .then(data => {
            if (!chart) {
                const ctx = document.getElementById('chart').getContext('2d');
                chart = new Chart(ctx, {
                    type: 'line',
                    data: {
                        labels: data.timestamps,
                        datasets: [{
                            label: 'Waga ula (g)',
                            data: data.weights,
                            borderColor: 'green',
                            fill: false
                        }]
                    },
                    options: {
                        responsive: true,
                        scales: {
                            y: { beginAtZero: false }
                        }
                    }
                });
            } else {
                chart.data.labels = data.timestamps;
                chart.data.datasets[0].data = data.weights;
                chart.update();
            }
        });
}

fetchAndUpdate();
setInterval(fetchAndUpdate, 30000);
</script>
</body>
</html>
