#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-TW">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 ➔ Mega2560 平衡車 PID 即時波形與控制台</title>
  <style>
    :root {
      --bg: linear-gradient(135deg, #0f172a 0%, #1e1b4b 100%);
      --card: rgba(30, 41, 59, 0.85);
      --border: rgba(255, 255, 255, 0.12);
      --accent: #38bdf8;
      --red: #ef4444;
      --green: #22c55e;
      --purple: #c084fc;
      --text: #f8fafc;
      --sub: #94a3b8;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background: var(--bg); color: var(--text); min-height: 100vh; padding: 16px; display: flex; flex-direction: column; align-items: center; }
    .header { text-align: center; margin-bottom: 14px; }
    .header h1 { font-size: 1.4rem; font-weight: 700; background: linear-gradient(to right, #38bdf8, #c084fc); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
    .status-badge { display: inline-block; padding: 4px 14px; border-radius: 20px; font-size: 0.85rem; font-weight: 600; margin-top: 4px; background: rgba(56, 189, 248, 0.15); color: var(--accent); border: 1px solid var(--accent); }
    .grid { width: 100%; max-width: 500px; display: flex; flex-direction: column; gap: 14px; }
    .card { background: var(--card); backdrop-filter: blur(12px); border: 1px solid var(--border); border-radius: 16px; padding: 16px; box-shadow: 0 8px 32px rgba(0,0,0,0.4); }
    .card-title { font-size: 0.95rem; color: var(--sub); margin-bottom: 10px; display: flex; justify-content: space-between; align-items: center; font-weight: 600; }
    
    .chart-container { position: relative; width: 100%; height: 180px; background: #0b1329; border-radius: 10px; border: 1px solid #1e293b; overflow: hidden; }
    canvas { width: 100%; height: 100%; display: block; }
    .chart-legend { display: flex; justify-content: space-around; margin-top: 8px; font-size: 0.78rem; font-weight: 600; }
    .leg-item { display: flex; align-items: center; gap: 4px; }
    .leg-dot { width: 10px; height: 10px; border-radius: 50%; display: inline-block; }

    .control-group { margin-bottom: 12px; }
    .control-label { display: flex; justify-content: space-between; font-size: 0.9rem; margin-bottom: 4px; }
    .val { font-weight: 700; color: var(--accent); font-family: monospace; }
    input[type=range] { width: 100%; height: 8px; border-radius: 4px; background: #334155; outline: none; -webkit-appearance: none; }
    input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 22px; height: 22px; border-radius: 50%; background: var(--accent); cursor: pointer; }
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    button { padding: 12px; border: none; border-radius: 10px; font-size: 0.95rem; font-weight: 700; cursor: pointer; color: white; transition: transform 0.1s; }
    button:active { transform: scale(0.96); }
    .btn-start { background: linear-gradient(135deg, #16a34a, #22c55e); }
    .btn-stop { background: linear-gradient(135deg, #dc2626, #ef4444); }
  </style>
</head>
<body>
  <div class="header">
    <h1>ESP32 ➔ Mega2560 平衡車控制台</h1>
    <div id="btStatus" class="status-badge">藍芽連線檢查中...</div>
  </div>

  <div class="grid">
    <!-- 即時 PID 姿態動態波形圖表 -->
    <div class="card">
      <div class="card-title">
        <span>即時 PID 姿態動態波形 (Real-Time Chart)</span>
        <span id="liveVals" style="font-size:0.8rem; color:var(--accent);">Angle: 0.0°</span>
      </div>
      <div class="chart-container">
        <canvas id="chartCanvas"></canvas>
      </div>
      <div class="chart-legend">
        <div class="leg-item"><span class="leg-dot" style="background:#ef4444;"></span> 即時傾角 (Angle)</div>
        <div class="leg-item"><span class="leg-dot" style="background:#38bdf8;"></span> 目標角度 (Target)</div>
        <div class="leg-item"><span class="leg-dot" style="background:#22c55e;"></span> 馬達 PWM 輸出</div>
      </div>
    </div>

    <!-- PID 滑桿調參 -->
    <div class="card">
      <div class="card-title">無線 PID 即時調參 (Slider Controls)</div>

      <div class="control-group">
        <div class="control-label"><span>Kp (比例增益)</span><span id="kpDisp" class="val">20.0</span></div>
        <input type="range" min="0" max="100" step="1" value="20" onchange="sendCmd('P' + this.value); document.getElementById('kpDisp').innerText = this.value;">
      </div>

      <div class="control-group">
        <div class="control-label"><span>Kd (微分阻尼)</span><span id="kdDisp" class="val">0.5</span></div>
        <input type="range" min="0" max="10" step="0.1" value="0.5" onchange="sendCmd('D' + this.value); document.getElementById('kdDisp').innerText = this.value;">
      </div>

      <div class="control-group">
        <div class="control-label"><span>Ki (積分增益)</span><span id="kiDisp" class="val">0.0</span></div>
        <input type="range" min="0" max="5" step="0.1" value="0" onchange="sendCmd('I' + this.value); document.getElementById('kiDisp').innerText = this.value;">
      </div>

      <div class="control-group">
        <div class="control-label"><span>Target Angle (目標角度)</span><span id="targetDisp" class="val">23.0°</span></div>
        <input type="range" min="10" max="35" step="0.2" value="23" onchange="sendCmd('T' + this.value); document.getElementById('targetDisp').innerText = this.value + '°';">
      </div>
    </div>

    <!-- 操控按鈕 -->
    <div class="card">
      <div class="btn-grid">
        <button class="btn-start" onclick="sendCmd('1')">啟動平衡 (1)</button>
        <button class="btn-stop" onclick="sendCmd('0')">緊急停止 (0)</button>
        <button style="background:#6366f1; grid-column:span 2;" onclick="sendCmd('?')">查詢目前所有參數 (?)</button>
      </div>
    </div>
  </div>

  <script>
    const canvas = document.getElementById('chartCanvas');
    const ctx = canvas.getContext('2d');

    const MAX_POINTS = 50;
    let angleData = new Array(MAX_POINTS).fill(23.0);
    let targetData = new Array(MAX_POINTS).fill(23.0);
    let motorData = new Array(MAX_POINTS).fill(0);

    function resizeCanvas() {
      canvas.width = canvas.parentElement.clientWidth;
      canvas.height = canvas.parentElement.clientHeight;
    }
    window.addEventListener('resize', resizeCanvas);
    resizeCanvas();

    function drawChart() {
      const w = canvas.width;
      const h = canvas.height;
      ctx.clearRect(0, 0, w, h);

      // 背景網格
      ctx.strokeStyle = '#1e293b';
      ctx.lineWidth = 1;
      for (let y = 0; y < h; y += 30) {
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
      }

      const centerY = h / 2;
      const angleScale = 4.0;
      const motorScale = 0.5;

      // Target Angle Line
      ctx.strokeStyle = '#38bdf8';
      ctx.lineWidth = 2;
      ctx.setLineDash([4, 4]);
      ctx.beginPath();
      for (let i = 0; i < MAX_POINTS; i++) {
        const x = (i / (MAX_POINTS - 1)) * w;
        const y = centerY - (targetData[i] - 23.0) * angleScale;
        if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();
      ctx.setLineDash([]);

      // Motor PWM Line
      ctx.strokeStyle = '#22c55e';
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      for (let i = 0; i < MAX_POINTS; i++) {
        const x = (i / (MAX_POINTS - 1)) * w;
        const y = centerY - motorData[i] * motorScale;
        if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();

      // Current Angle Line
      ctx.strokeStyle = '#ef4444';
      ctx.lineWidth = 2.5;
      ctx.beginPath();
      for (let i = 0; i < MAX_POINTS; i++) {
        const x = (i / (MAX_POINTS - 1)) * w;
        const y = centerY - (angleData[i] - 23.0) * angleScale;
        if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }

    function sendCmd(cmd) {
      const controller = new AbortController();
      const timeoutId = setTimeout(() => controller.abort(), 1000);
      fetch(`/api/send?cmd=${encodeURIComponent(cmd)}`, { signal: controller.signal })
        .finally(() => clearTimeout(timeoutId));
    }

    let isPolling = false;
    function pollStatus() {
      if (isPolling) return;
      isPolling = true;

      const controller = new AbortController();
      const timeoutId = setTimeout(() => controller.abort(), 1000);

      fetch('/api/status', { signal: controller.signal })
        .then(res => res.json())
        .then(data => {
          const badge = document.getElementById('btStatus');
          if (data.connected) {
            badge.innerText = "藍芽連線成功 (98:D3:31:F4:20:B8)";
            badge.style.borderColor = "#22c55e";
            badge.style.color = "#22c55e";
          } else {
            badge.innerText = "藍芽連線中...";
            badge.style.borderColor = "#f59e0b";
            badge.style.color = "#f59e0b";
          }

          // 直接推送純數值，無需字串解析
          if (data.angle !== undefined) {
            angleData.push(data.angle); angleData.shift();
            targetData.push(data.target); targetData.shift();
            motorData.push(data.motor); motorData.shift();

            document.getElementById('liveVals').innerText = `Angle: ${data.angle.toFixed(1)}° | PWM: ${data.motor}`;
            requestAnimationFrame(drawChart);
          }
        })
        .catch(err => {})
        .finally(() => {
          clearTimeout(timeoutId);
          isPolling = false;
        });
    }

    setInterval(pollStatus, 250);
    drawChart();
  </script>
</body>
</html>
)rawliteral";

#endif
