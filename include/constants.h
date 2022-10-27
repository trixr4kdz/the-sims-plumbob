#define CONTROL_PIN D1

// Set Access Point creds
#ifndef AP_SSID
#define AP_SSID "trix-sims"
#define AP_PSK "plumbob-69420"
#endif

const int LED_COUNT = 13;

// In case something goes wrong...
const bool DEBUG_MODE = false;

// TODO: Fix this outdated html/js/css...
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>The Sims Plumbob LED Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html { font-family: Arial; display: inline-block; text-align: center; }
      body { margin: 0px auto; padding-bottom: 15px; }
      .slider { margin: 0px auto; flex-grow: 1; }
    </style>
  </head>
  <body>
    <h2>The Sims Plumbob LED Controller</h2>
    <div align="center">
      <div id="num_clients"></div>
      <label for="num_clients">Clients: </label><br>

      <label for="powerState">Power: </label>
      <input type="checkbox" id="powerState" name="powerState" checked onchange="togglePower(this)" />
      <br>

      <label for="slider">Color: </label>
      <input type="range" class="slider" name="slider" min="0" max="255" step="5" value="0" onchange="handleSlider(this)" />
      <br>

      <label for="brightness">Brightness: </label>
      <input type="range" class="slider" name="brightness" min="0" max="100" value="100" onchange="handleBrightness(this)" />
      <br>

      <form action="" onsubmit="handleUnsupervisedMode()">
        <label for="unsupervisedMode">Unsupervised Mode: </label>
        <input type="text" name="duration" value="1" id="duration"> min</input>
        <input type="submit" value="Unsupervised Mode" />
      </form>
    </div>
    <script>
      function togglePower(e) {
        let xhr = new XMLHttpRequest();
        if (e.checked) {
          xhr.open("GET", "/power?state=1", true);
        } else {
          xhr.open("GET", "/power?state=0", true);
        }
        xhr.send();
      }

      function handleSlider(e) {
        let xhr = new XMLHttpRequest();
        xhr.open("GET", "/color?value=" + e.value, true);
        xhr.send();
        e.preventDefault();
      }

      function handleBrightness(e) {
        let xhr = new XMLHttpRequest();
        xhr.open("GET", "/brightness?value=" + e.value, true);
        xhr.send();
        e.preventDefault();
      }

      function handleUnsupervisedMode() {
        let xhr = new XMLHttpRequest();
        let duration = document.getElementById("duration").value;
        xhr.open("GET", "/unsupervised?duration=" + duration, true);
        xhr.send();
      }
    </script>
  </body>
</html>)rawliteral";