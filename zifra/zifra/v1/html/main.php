<?php require __DIR__ . '/_cors.php'; ?>
<header class="navbar navbar-dark sticky-top bg-dark flex-md-nowrap p-0 shadow">
  <a class="navbar-brand col-md-3 col-lg-2 me-0 px-3" href="#">ZIFRA</a>
  <button class="navbar-toggler position-absolute d-md-none collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#sidebarMenu" aria-controls="sidebarMenu" aria-expanded="false" aria-label="Toggle navigation">
    <span class="navbar-toggler-icon"></span>
  </button>

  <ul class="navbar-nav px-3" style="margin-right:50px;">
      <li class="nav-item text-nowrap">
          <div class="text-center nav-link active">
              Connection: <span id="connectionStatus">Status Unknown</span>
          </div>
      </li>
  </ul>
</header>

<div class="container-fluid">
  <div class="row">
    <nav id="sidebarMenu" class="col-md-3 col-lg-2 d-md-block sidebar collapse">
      <div class="position-sticky pt-3">
        <ul class="nav flex-column">
          <li class="nav-item">
            <a class="nav-link active" aria-current="page" href="/">
              <span data-feather="home"></span>
              Dashboard
            </a>
          </li>
          <li class="nav-item">
            <a class="nav-link" href="javascript:void(0);" onclick="ChangeContent(this, 'settime', '#right-content');">
              <span data-feather="clock"></span>
              Clock Settings
            </a>
          </li>
          <li class="nav-item">
            <a class="nav-link" href="javascript:void(0);" onclick="ChangeContent(this, 'setalarm', '#right-content');">
              <span data-feather="bell"></span>
              Alarms
            </a>
          </li>
          <li class="nav-item">
            <a class="nav-link" href="javascript:void(0);" onclick="ChangeContentIframe(this, 'update', '#right-content');">
              <span data-feather="upload"></span>
              Firmware Update
            </a>
          </li>
        </ul>
      </div>
    </nav>

    <main class="col-md-9 ms-sm-auto col-lg-10 px-md-4 pt-3" id="right-content">

      <div class="row">
        <div class="col-md-6">
          <div class="panel">
            <h2>About this clock</h2>
            <hr>
            <p>
              ZIFRA is a clock built around a <b>single Soviet-era IN-12B nixie tube</b>.
              Instead of four tubes side by side, the one tube spells the time out
              <b>one digit at a time</b> &mdash; first the two hour digits, then the two
              minute digits.
            </p>
            <p class="mb-0">
              The clock fetches the time over WiFi from an NTP server. With WiFi switched
              off it falls back to its on-board DS3231 real-time clock (if installed).
              When connected, this interface is reachable at
              <a href="http://zifra.local">http://zifra.local</a>, and new firmware can be
              flashed from the <i>Firmware Update</i> page &mdash; no cables needed.
            </p>
          </div>

          <div class="panel">
            <h2>Buttons</h2>
            <hr>
            <table class="table table-sm mb-0">
              <tbody>
                <tr>
                  <td class="text-nowrap"><b>Front button</b> &mdash; press</td>
                  <td>Mute a ringing alarm (confirms with a beep).</td>
                </tr>
                <tr>
                  <td class="text-nowrap"><b>Top button</b> &mdash; press</td>
                  <td>Mute a ringing alarm and wake the tube from sleep mode for 3 minutes.</td>
                </tr>
                <tr>
                  <td class="text-nowrap"><b>Top button</b> &mdash; hold 10&nbsp;s</td>
                  <td>Switch WiFi on or off. The clock restarts; with WiFi off the DS3231 keeps the time.</td>
                </tr>
                <tr>
                  <td class="text-nowrap"><b>Front button</b> &mdash; hold 35&nbsp;s</td>
                  <td>Factory reset: erases all settings and WiFi credentials, then reboots into the
                      <code>ZIFRA-&hellip;</code> setup hotspot.</td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>

        <div class="col-md-6">
          <div class="panel">
            <h2>How to read the time</h2>
            <hr>
            <div class="row">
              <div class="col-4 text-center">
                <div class="nixie-tube" id="demoTube">
                  <span class="nixie-digit" id="demoDigit">0</span>
                  <span class="nixie-dot" id="demoDot"></span>
                </div>
                <div class="help-text">live demo,<br>your browser time</div>
              </div>
              <div class="col-8">
                <p>
                  The tube shows each digit for <b>0.8&nbsp;seconds</b> with a short dark
                  pause in between, and a slightly longer pause separating the hours from
                  the minutes:
                </p>
                <p class="text-center">
                  <b>H</b> &middot; <b>H</b> &nbsp;&nbsp; <b>M.</b> &middot; <b>M.</b>
                </p>
                <p>
                  While the <b>minute</b> digits are shown, the small <b>decimal point</b>
                  in the lower right of the tube glows &mdash; that is how you tell minutes
                  from hours.
                </p>
                <p class="mb-0">
                  After the last digit the tube stays dark for a moment, then the cycle
                  repeats (roughly every 5&frac12; seconds). With <i>leading zero</i>
                  disabled, an hour like 9:41 starts directly with the <b>9</b>.
                </p>
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="row">
        <div class="col-md-6">
          <div class="panel">
            <h2>System info</h2>
            <hr>
            <div class="table-responsive">
              <table class="table table-striped table-sm" id="table">
                <tbody>
                  <tr>
                    <td class="font-weight-bold">Firmware version:</td>
                    <td id="gumboardVersion">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">Sketch size:</td>
                    <td id="sketchSize">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">Free sketch space:</td>
                    <td id="freeSketchSpace">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">WiFi RSSI:</td>
                    <td id="wifiRSSI">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">WiFi quality:</td>
                    <td id="wifiQuality">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">WiFi SSID:</td>
                    <td id="wifiSSID">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">IP address:</td>
                    <td id="ipAddress">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">Free heap:</td>
                    <td id="freeHeap">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">Chip ID:</td>
                    <td id="chipID">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">CPU frequency (MHz):</td>
                    <td id="cpuFreqMHz">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">Sleep mode active:</td>
                    <td id="clock_sleep">Loading...</td>
                  </tr>
                  <tr>
                    <td class="font-weight-bold">Weekday:</td>
                    <td id="weekday">Loading...</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </div>
        </div>
        <div class="col-md-6">
          <div class="panel">
            <h2>Log</h2>
            <hr>
            <textarea class="form-control" id="log" rows="17" wrap="off" disabled="" style="font-size: small"></textarea>
            <div class="help-text">Live event log, streamed from the clock over a websocket.</div>
          </div>
        </div>
      </div>

      <script>
        connectionStart();

        // Live demo of the single-tube display sequence. Uses the same timing
        // as the firmware (see zifra_clock_timer.h) and the browser's time.
        (function () {
            var slots = [
                { digit: 0, dot: false, ms: 800 },  // hour, tens
                { off: true, ms: 200 },
                { digit: 1, dot: false, ms: 800 },  // hour, ones
                { off: true, ms: 700 },             // hours -> minutes pause
                { digit: 2, dot: true, ms: 800 },   // minute, tens (dot lit)
                { off: true, ms: 200 },
                { digit: 3, dot: true, ms: 800 },   // minute, ones (dot lit)
                { off: true, ms: 1600 }             // dark pause, then repeat
            ];
            var pos = 0;

            function timeDigits() {
                var now = new Date();
                var h = now.getHours(), m = now.getMinutes();
                return [Math.floor(h / 10), h % 10, Math.floor(m / 10), m % 10];
            }

            function step() {
                var tube = $('#demoTube');
                if (!tube.length) {
                    return; // dashboard content was replaced, stop the demo
                }
                var slot = slots[pos];
                if (slot.off) {
                    tube.addClass('nixie-off');
                    $('#demoDot').removeClass('lit');
                } else {
                    tube.removeClass('nixie-off');
                    $('#demoDigit').text(timeDigits()[slot.digit]);
                    $('#demoDot').toggleClass('lit', slot.dot);
                }
                pos = (pos + 1) % slots.length;
                setTimeout(step, slot.ms);
            }
            step();
        })();
      </script>
    </main>
  </div>
</div>
<script>
    feather.replace();
</script>
