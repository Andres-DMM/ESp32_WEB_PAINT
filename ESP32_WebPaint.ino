/*
  ESP32 Web Paint - GC9A01A Round Display
  
  Web-based paint application for ESP32 with GC9A01A round display.
  Draw from your browser onto the display in any color.
  
  Connections (GC9A01A -> ESP32 Dev Kit V1):
    VCC   -> 5V
    GND   -> GND
    CS    -> GPIO 5
    DC    -> GPIO 2
    RST   -> GPIO 4
    MOSI  -> GPIO 23
    SCK   -> GPIO 18
    BL    -> 3.3V (or GPIO 22 via 100ohm resistor for PWM control)
  
  Required Libraries (Arduino IDE Library Manager):
    - Adafruit_GFX          by Adafruit
    - Adafruit_GC9A01A      by Adafruit
    - ArduinoJson           by Benoit Blanchon (v6)
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>

// ---- WiFi Configuration ----
const char* ssid = "CFM2";
const char* password = "Josue2424";

// ---- Display Pins ----
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

// ---- Display Settings ----
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);
WebServer server(80);

// ---- Display Colors ----
#define BLACK   0x0000
#define RED     0xF800
#define CYAN    0x07FF
#define WHITE   0xFFFF

// ---- Color Conversion ----
// Convert hex color string (e.g., "#ff0000") to RGB565
uint16_t hexToColor565(const char* hex) {
  if (hex[0] == '#') hex++;
  long num = strtol(hex, NULL, 16);
  uint8_t r = (num >> 16) & 0xFF;
  uint8_t g = (num >> 8) & 0xFF;
  uint8_t b = num & 0xFF;
  return tft.color565(r, g, b);
}

// ---- Web Handlers ----

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0,maximum-scale=1.0,user-scalable=no">
<title>ESP32 Paint</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{
  background:#1a1a2e;color:#fff;
  font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  display:flex;justify-content:center;align-items:center;
  min-height:100vh;margin:0;padding:16px
}
.container{
  background:#16213e;padding:24px;border-radius:24px;
  box-shadow:0 8px 32px rgba(0,0,0,.4);text-align:center;max-width:400px
}
h1{font-size:1.3em;margin-bottom:16px;color:#e0e0e0;letter-spacing:1px}
.canvas-wrap{
  display:inline-block;border-radius:50%;overflow:hidden;
  border:3px solid #0f3460;line-height:0;margin:0 auto
}
canvas{
  display:block;background:#000;width:280px;height:280px;
  touch-action:none;cursor:crosshair
}
.controls{display:flex;flex-wrap:wrap;gap:12px;justify-content:center;align-items:center;margin-top:16px}
.clr-wrap{display:flex;flex-direction:column;align-items:center;gap:4px}
.clr-wrap label{font-size:.7em;color:#888}
input[type="color"]{
  -webkit-appearance:none;width:44px;height:44px;border:2px solid #0f3460;
  border-radius:50%;cursor:pointer;background:none;padding:2px
}
input[type="color"]::-webkit-color-swatch-wrapper{padding:0}
input[type="color"]::-webkit-color-swatch{border:none;border-radius:50%}
.sz-wrap{display:flex;flex-direction:column;align-items:center;gap:4px}
.sz-wrap label{font-size:.7em;color:#888}
input[type="range"]{width:90px;accent-color:#e94560;cursor:pointer}
.sz-val{font-size:.8em;color:#aaa;min-width:20px}
.btn{
  background:#0f3460;color:#fff;border:none;padding:10px 18px;
  border-radius:10px;cursor:pointer;font-size:.85em;
  transition:background .2s;font-weight:600
}
.btn:hover{background:#1a4a7a}
.btn-danger{background:#e94560}
.btn-danger:hover{background:#c73650}
.status{font-size:.75em;margin-top:12px;padding:4px 12px;border-radius:8px;display:inline-block}
.status-ok{color:#4ecca3}
.status-err{color:#ff6b6b}
.status-sending{color:#ffd93d}
</style>
</head>
<body>
<div class="container">
  <h1>ESP32 PAINT</h1>
  <div class="canvas-wrap">
    <canvas id="c" width="240" height="240"></canvas>
  </div>
  <div class="controls">
    <div class="clr-wrap">
      <label>COLOR</label>
      <input type="color" id="clr" value="#ffffff">
    </div>
    <div class="sz-wrap">
      <label>SIZE</label>
      <input type="range" id="sz" min="1" max="20" value="3">
      <span class="sz-val" id="szVal">3</span>
    </div>
    <div>
      <button class="btn btn-danger" id="clearBtn">CLEAR</button>
    </div>
  </div>
  <div class="status status-ok" id="status">Connected</div>
</div>
<script>
(function(){
const c=document.getElementById('c'),ctx=c.getContext('2d');
const clr=document.getElementById('clr'),sz=document.getElementById('sz');
const szVal=document.getElementById('szVal'),status=document.getElementById('status');
const clearBtn=document.getElementById('clearBtn');

let drawing=false,lastX=0,lastY=0;
let curColor='#ffffff',curSize=3;
let pts=[];
let timer=null;

function pos(e){
  const t=e.touches?e.touches[0]:e;
  const r=c.getBoundingClientRect();
  return{
    x:Math.round((t.clientX-r.left)*(c.width/r.width)),
    y:Math.round((t.clientY-r.top)*(c.height/r.height))
  };
}

function drawDot(x,y){
  ctx.beginPath();ctx.arc(x,y,curSize/2,0,Math.PI*2);
  ctx.fillStyle=curColor;ctx.fill();
}

function drawLine(x1,y1,x2,y2){
  ctx.beginPath();ctx.moveTo(x1,y1);ctx.lineTo(x2,y2);
  ctx.strokeStyle=curColor;ctx.lineWidth=curSize;
  ctx.lineCap='round';ctx.lineJoin='round';ctx.stroke();
}

function sendBatch(){
  if(!pts.length)return;
  status.textContent='Sending...';status.className='status status-sending';
  const body={color:curColor,size:curSize,points:pts};
  const batch=pts.slice();pts=[];
  fetch('/draw',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
  .then(r=>{status.textContent='Connected';status.className='status status-ok'})
  .catch(()=>{status.textContent='Disconnected';status.className='status status-err'});
}

function start(e){e.preventDefault();
  const p=pos(e);drawing=true;lastX=p.x;lastY=p.y;pts=[[p.x,p.y]];
  drawDot(p.x,p.y);
}

function move(e){e.preventDefault();
  if(!drawing)return;
  const p=pos(e);drawLine(lastX,lastY,p.x,p.y);
  lastX=p.x;lastY=p.y;pts.push([p.x,p.y]);
  if(!timer)timer=setTimeout(()=>{timer=null;sendBatch();},80);
}

function stop(e){drawing=false;
  if(timer){clearTimeout(timer);timer=null;}
  sendBatch();
}

c.addEventListener('mousedown',start);
c.addEventListener('mousemove',move);
c.addEventListener('mouseup',stop);
c.addEventListener('mouseleave',stop);
c.addEventListener('touchstart',start,{passive:false});
c.addEventListener('touchmove',move,{passive:false});
c.addEventListener('touchend',stop);

clr.addEventListener('input',e=>{curColor=e.target.value});
sz.addEventListener('input',e=>{curSize=parseInt(e.target.value);szVal.textContent=curSize});

clearBtn.addEventListener('click',()=>{
  ctx.fillStyle='#000';ctx.fillRect(0,0,240,240);
  fetch('/clear',{method:'POST'}).catch(()=>{});
});

ctx.fillStyle='#000';ctx.fillRect(0,0,240,240);
})();
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void handleDraw() {
  String body = server.arg("plain");
  if (body.length() == 0) {
    server.send(400, "text/plain", "Empty body");
    return;
  }

  DynamicJsonDocument doc(12288);
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  const char* colorStr = doc["color"] | "#ffffff";
  int brushSize = doc["size"] | 3;
  uint16_t color = hexToColor565(colorStr);
  int r = (brushSize + 1) / 2;
  if (r < 1) r = 1;

  JsonArray points = doc["points"].as<JsonArray>();
  if (points.isNull()) {
    server.send(400, "text/plain", "Missing points");
    return;
  }

  for (JsonArray pt : points) {
    int x = pt[0];
    int y = pt[1];
    x = constrain(x, 0, TFT_WIDTH - 1);
    y = constrain(y, 0, TFT_HEIGHT - 1);
    tft.fillCircle(x, y, r, color);
  }

  server.send(200, "text/plain", "OK");
}

void handleClear() {
  tft.fillScreen(BLACK);
  server.send(200, "text/plain", "OK");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// ---- Setup ----
void setup() {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(BLACK);

  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(25, 80);
  tft.println("ESP32");
  tft.setCursor(20, 110);
  tft.println("PAINT");
  tft.setTextSize(1);
  tft.setCursor(30, 150);
  tft.println("Connecting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 60) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    tft.fillScreen(BLACK);
    tft.setTextSize(1);
    tft.setTextColor(WHITE);
    tft.setCursor(15, 70);
    tft.println("ESP32 Paint Ready");
    tft.setCursor(15, 100);
    tft.println("Open browser to:");
    tft.setCursor(15, 120);
    tft.setTextColor(CYAN);
    tft.println(WiFi.localIP().toString().c_str());
    delay(4000);
    tft.fillScreen(BLACK);
  } else {
    Serial.println("\nFailed to connect!");
    tft.fillScreen(BLACK);
    tft.setTextColor(RED);
    tft.setTextSize(1);
    tft.setCursor(15, 110);
    tft.println("WiFi Failed!");
    tft.setCursor(15, 130);
    tft.println("Check credentials");
    delay(5000);
    ESP.restart();
  }

  server.on("/", handleRoot);
  server.on("/draw", HTTP_POST, handleDraw);
  server.on("/clear", HTTP_POST, handleClear);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Server started");
}

// ---- Main Loop ----
void loop() {
  server.handleClient();
}
