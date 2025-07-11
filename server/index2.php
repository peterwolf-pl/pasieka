<?php
session_start();
if (!isset($_SESSION['logged_in'])) {
    header('Location: login.php');
    exit();
}
?>
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
    <meta name="apple-mobile-web-app-capable" content="yes">
    <title>Waga Ula | pszczol.one.pl</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body {
            background: #f8f9fa;
            font-family: 'Segoe UI', sans-serif;
            margin: 0;
            padding: 0;
        }
        .chart-container {
            position: relative;
            margin: auto;
            height: 80vh;
            width: 95vw;
            max-width: 100%;
        }
        header {
            background-color: #198754;
            color: white;
            padding: 1rem;
            text-align: center;
            border-bottom: 3px solid #145c32;
            position: relative;
        }
        .setup-link {
            position: absolute;
            right: 1rem;
            top: -1.5rem;
        }
        .footer {
            text-align: center;
            margin-top: 2rem;
            font-size: 0.9rem;
            color: #6c757d;
        }
    </style>
</head>
<body>
    <div class="setup-link"><a href="setup.php">Ustawienia</a> | <a href="logout.php">Wyloguj</a></div>
    <header>
        <h1>🐝 Waga Ula</h1>
        <p>Monitoring masy ula i w czasie rzeczywistym</p>
    </header>

    <div class="container mt-4">
        <div class="mb-3">
            <label>Kolor linii: <input type="color" id="lineColor" value="#198754"></label>
            <label class="ms-3">Typ wykresu:
                <select id="chartType">
                    <option value="line">Linia</option>
                    <option value="bar">Słupki</option>
                </select>
            </label>
            <button id="saveStyle" class="btn btn-sm btn-primary ms-3">Zapisz</button>
<a href="https://pszczol.one.pl/setup.php">setup</a>     
</div>
        <div class="row">
            <div class="col-6 mb-4">
                <div class="chart-container"><canvas id="chart1"></canvas></div>
            </div>   </div>
            
              <div class="row">
            <div class="col-6 mb-4">
                <div class="chart-container"><canvas id="chart2"></canvas></div>
            </div>   </div>
                    <div class="row">
            <div class="col-6 mb-4">
                <div class="chart-container"><canvas id="chart3"></canvas></div>
            </div>   </div>
                          <div class="row">
            <div class="col-6 mb-4">
                <div class="chart-container"><canvas id="chart4"></canvas></div>
            </div>   </div>
        </div>
    </div>

    <div class="footer">&copy; 2025 pszczol.one.pl by peterwolf.pl </div>
<script>
    const boards = [1,2,3,4];
    const colorInput = document.getElementById('lineColor');
    const typeInput = document.getElementById('chartType');
    const savedColor = localStorage.getItem('chartColor');
    const savedType = localStorage.getItem('chartType');
    if(savedColor) colorInput.value = savedColor;
    if(savedType) typeInput.value = savedType;

    document.getElementById('saveStyle').addEventListener('click', () => {
        localStorage.setItem('chartColor', colorInput.value);
        localStorage.setItem('chartType', typeInput.value);
        location.reload();
    });

    boards.forEach(id => {
        fetch(`data.php?board=${id}`)
            .then(res => {
                if(res.status === 403){
                    window.location.href = 'login.php';
                    throw new Error('Unauthorized');
                }
                return res.json();
            })
            .then(data => {
                const ctx = document.getElementById(`chart${id}`).getContext('2d');
                new Chart(ctx, {
                    type: typeInput.value,
                    data: {
                        labels: data.timestamps,
                        datasets: [{
                            label: `Masa ula ${id} (g)`,
                            data: data.weights,
                            borderWidth: 2,
                            borderColor: colorInput.value,
                            backgroundColor: colorInput.value + '33',
                            fill: true,
                            tension: 0.4,
                        }]
                    },
                    options: {
                        responsive: true,
                        maintainAspectRatio: false,
                        scales: {
                            y: {
                                beginAtZero: false,
                                title:{display:true,text:'Waga [g]'}
                            },
                            x: {
                                ticks:{maxRotation:90,minRotation:45},
                                title:{display:true,text:'Czas pomiaru'}
                            }
                        }
                    }
                });
            })
            .catch(err => console.error(err));
    });
</script>
</body>
</html>
