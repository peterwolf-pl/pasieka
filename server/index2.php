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
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=yes" />
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
            width: 100%;
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
    
    <header>
        <h1>🐝 Waga Ula</h1>
        <p>Monitoring masy ula w czasie rzeczywistym</p>
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
        <div class="mb-3 text-center">
            <label>Od: <input type="datetime-local" id="startRange"></label>
            <label class="ms-2">Do: <input type="datetime-local" id="endRange"></label>
            <button id="applyRange" class="btn btn-sm btn-primary ms-2">Pokaż</button>
            <button class="btn btn-sm btn-secondary ms-2 quick-range" data-hours="12">Ostatnie 12h</button>
            <button class="btn btn-sm btn-secondary ms-2 quick-range" data-hours="24">Ostatnie 24h</button>
            <button class="btn btn-sm btn-secondary ms-2 quick-range" data-hours="72">Ostatnie 72h</button>
        </div>
        <div class="row">

                <h5 id="delta1" class="text-center mb-2"></h5>
                <div class="chart-container"><canvas id="chart1"></canvas></div>
              </div>

              <div class="row">

                <h5 id="delta2" class="text-center mb-2"></h5>
                <div class="chart-container"><canvas id="chart2"></canvas></div>
            </div>
                    <div class="row">

                <h5 id="delta3" class="text-center mb-2"></h5>
                <div class="chart-container"><canvas id="chart3"></canvas></div>
            </div>
                          <div class="row">

                <h5 id="delta4" class="text-center mb-2"></h5>
                <div class="chart-container"><canvas id="chart4"></canvas></div>
            </div>
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

    const charts = {};
    const startInput = document.getElementById('startRange');
    const endInput = document.getElementById('endRange');
    const applyBtn = document.getElementById('applyRange');

    function setRange(hours){
        const now = new Date();
        endInput.value = now.toISOString().slice(0,16);
        const start = new Date(now.getTime() - hours * 60 * 60 * 1000);
        startInput.value = start.toISOString().slice(0,16);
    }

    document.querySelectorAll('.quick-range').forEach(btn => {
        btn.addEventListener('click', () => {
            setRange(parseInt(btn.dataset.hours,10));
            loadCharts();
        });
    });

    applyBtn.addEventListener('click', loadCharts);

    function loadCharts(){
        const start = new Date(startInput.value).getTime();
        const end = new Date(endInput.value).getTime();
        const now = Date.now();
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
                    const filteredWeights = [];
                    const filteredHz = [];
                    const filteredTimestamps = [];
                    data.timestamps.forEach((t, i) => {
                        const ts = new Date(t).getTime();
                        if (ts >= start && ts <= end) {
                            filteredWeights.push(data.weights[i]);
                            if(data.hz){
                                filteredHz.push(data.hz[i]);
                            }
                            filteredTimestamps.push(t);
                        }
                    });

                const lastWeight = data.weights[data.weights.length - 1];
                const deltaWithin = hours => {
                    const threshold = now - hours * 60 * 60 * 1000;
                    let earliest = lastWeight;
                    for (let i = data.timestamps.length - 1; i >= 0; i--) {
                        const ts = new Date(data.timestamps[i]).getTime();
                        if (ts >= threshold) {
                            earliest = data.weights[i];
                        } else {
                            break;
                        }
                    }
                    return lastWeight - earliest;
                };
                const delta12 = deltaWithin(12);
                const delta24 = deltaWithin(24);
                const delta72 = deltaWithin(72);
                document.getElementById(`delta${id}`).textContent =
                    `\u0394 12h: ${delta12.toFixed(2)} g | \u0394 24h: ${delta24.toFixed(2)} g | \u0394 72h: ${delta72.toFixed(2)} g`;

                const ctx = document.getElementById(`chart${id}`).getContext('2d');
                if(charts[id]) charts[id].destroy();
                charts[id] = new Chart(ctx, {
                    type: typeInput.value,
                    data: {
                        labels: filteredTimestamps,
                        datasets: [{
                            label: `Masa ula ${id} (g)`,
                            data: filteredWeights,
                            borderWidth: 2,
                            borderColor: colorInput.value,
                            backgroundColor: colorInput.value + '33',
                            yAxisID: 'y',
                            fill: true,
                            tension: 0.4,
                        }, {
                            label: `Częstotliwość ${id} (Hz)`,
                            data: filteredHz,
                            borderWidth: 2,
                            borderColor: 'orange',
                            backgroundColor: 'rgba(255,165,0,0.3)',
                            yAxisID: 'y1',
                            fill: false,
                            tension: 0.4,
                            type: 'line'
                        }]
                    },
                    options: {
                        responsive: true,
                        maintainAspectRatio: false,
                        scales: {
                            y: {
                                beginAtZero: false,
                                title: {display: true, text: 'Waga [g]'}
                            },
                            y1: {
                                beginAtZero: false,
                                position: 'right',
                                title: {display: true, text: 'Hz'}
                            },
                            x: {
                                ticks: {maxRotation: 90, minRotation: 45},
                                title: {display: true, text: 'Czas pomiaru'}
                            }
                        }
                    }
                });
            })
            .catch(err => console.error(err));
        });
    }

    setRange(24);
    loadCharts();
</script>
</body>
</html>