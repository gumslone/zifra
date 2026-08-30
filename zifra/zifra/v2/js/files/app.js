/*
 * ZIFRA web UI v2 — mobile-first single-page configuration app.
 * Vanilla JS, no dependencies. Talks the same websocket protocol as v1:
 *   ws://<clock>:81/main      -> system info + live log
 *   ws://<clock>:81/settime   -> config JSON on connect
 *   ws://<clock>:81/setConfig -> send config JSON, clock saves + applies it
 */
(function () {
    'use strict';

    var HOST = location.hostname;
    var DAY_LETTERS = ['S', 'M', 'T', 'W', 'T', 'F', 'S'];
    var DAY_NAMES = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];

    // Mirrors MELODIES in the firmware's melodies.h ([freq, ms]; freq 0 = rest)
    var MELODY_NAMES = ['Classic', 'Double beep', 'Siren', 'Chime'];
    var MELODY_NOTES = [
        [[500, 180], [780, 180]],
        [[880, 150], [0, 150], [880, 150], [0, 600]],
        [[400, 150], [520, 150], [660, 150], [780, 150], [660, 150], [520, 150]],
        [[523, 200], [659, 200], [784, 200], [1047, 400], [0, 420]]
    ];

    var ICONS = {
        home: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"></path><polyline points="9 22 9 12 15 12 15 22"></polyline></svg>',
        clock: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="9"></circle><polyline points="12 7 12 12 15.5 13.5"></polyline></svg>',
        bell: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M18 8A6 6 0 0 0 6 8c0 7-3 9-3 9h18s-3-2-3-9"></path><path d="M13.73 21a2 2 0 0 1-3.46 0"></path></svg>',
        cpu: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="4" width="16" height="16" rx="2"></rect><rect x="9" y="9" width="6" height="6"></rect><line x1="12" y1="1" x2="12" y2="4"></line><line x1="12" y1="20" x2="12" y2="23"></line><line x1="1" y1="12" x2="4" y2="12"></line><line x1="20" y1="12" x2="23" y2="12"></line></svg>',
        play: '<svg viewBox="0 0 24 24" fill="currentColor" stroke="none"><polygon points="7 4 20 12 7 20"></polygon></svg>',
        chevron: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="9 18 15 12 9 6"></polyline></svg>',
        upload: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path><polyline points="17 8 12 3 7 8"></polyline><line x1="12" y1="3" x2="12" y2="15"></line></svg>'
    };

    // ---- state ------------------------------------------------------------

    var cfg = {
        ntpServer: 'pool.ntp.org',
        utcOffsetInSeconds: 0,
        clock_12h: false,
        clock_leading_hour_zero: true,
        clock_sleep: false,
        clock_sleep_start: '',
        clock_sleep_finish: '',
        alarms: [
            { time: '', active: false, melody: 0, weekdays: [0, 0, 0, 0, 0, 0, 0] },
            { time: '', active: false, melody: 0, weekdays: [0, 0, 0, 0, 0, 0, 0] },
            { time: '', active: false, melody: 0, weekdays: [0, 0, 0, 0, 0, 0, 0] }
        ]
    };
    var info = {};

    // ---- markup -----------------------------------------------------------

    function tabHtml(id, icon, label) {
        return '<button class="tab" data-tab="' + id + '">' + ICONS[icon] +
               '<span>' + label + '</span></button>';
    }

    function alarmHtml(i) {
        var chips = '';
        for (var d = 0; d < 7; d++) {
            chips += '<button class="chip" data-alarm="' + i + '" data-day="' + d +
                     '" aria-label="' + DAY_NAMES[d] + '">' + DAY_LETTERS[d] + '</button>';
        }
        var options = '';
        for (var m = 0; m < MELODY_NAMES.length; m++) {
            options += '<option value="' + m + '">' + MELODY_NAMES[m] + '</option>';
        }
        return '' +
        '<div class="card alarm-card" id="alarmCard' + i + '">' +
          '<div class="alarm-head">' +
            '<input type="time" class="alarm-time" id="alarm' + i + 'Time">' +
            '<label class="switch"><input type="checkbox" id="alarm' + i + 'Active"><span class="knob"></span></label>' +
          '</div>' +
          '<div class="weekdays">' + chips + '</div>' +
          '<div class="sound-row">' +
            '<span class="value">Sound</span>' +
            '<select id="alarm' + i + 'Melody">' + options + '</select>' +
            '<button class="btn preview-btn" data-preview="' + i + '" aria-label="Preview sound">' + ICONS.play + '</button>' +
          '</div>' +
        '</div>';
    }

    function render() {
        document.getElementById('app').innerHTML = '' +
        '<div class="appbar">' +
          '<div class="brand">ZIFRA</div>' +
          '<div class="conn" id="conn"><span class="dot"></span><span id="connText">Connecting&hellip;</span></div>' +
        '</div>' +

        '<main>' +

        '<section class="screen active" data-screen="home">' +
          '<div class="card hero">' +
            '<div class="tube" id="tube"><span class="digit" id="tubeDigit">0</span><span class="tdot" id="tubeDot"></span></div>' +
            '<div class="hero-info">' +
              '<div class="hero-time" id="heroTime">--:--</div>' +
              '<div class="hint">spelled out one digit at a time on the IN-12B tube</div>' +
              '<div class="hero-sub">' + ICONS.bell + '<span id="nextAlarm">No alarm set</span></div>' +
            '</div>' +
          '</div>' +
          '<div class="card list-card">' +
            '<div class="row"><span class="value">Firmware</span><span class="label" data-info="gumboardVersion">&hellip;</span></div>' +
            '<div class="row"><span class="value">WiFi</span><span class="label" id="homeWifi">&hellip;</span></div>' +
            '<div class="row"><span class="value">IP address</span><span class="label" data-info="ipAddress">&hellip;</span></div>' +
            '<div class="row"><span class="value">Night sleep</span><span class="value lit" id="homeSleep">off</span></div>' +
          '</div>' +
          '<div class="card list-card">' +
            '<button class="link-row" data-goto="system">Event log ' + ICONS.chevron + '</button>' +
            '<button class="link-row" id="updateLink">Firmware update ' + ICONS.chevron + '</button>' +
          '</div>' +
        '</section>' +

        '<section class="screen" data-screen="clock">' +
          '<div class="card">' +
            '<div class="card-title">Time source</div>' +
            '<div class="field"><span class="field-label">NTP server</span><input type="text" id="ntpServer"></div>' +
            '<div class="field"><span class="field-label">UTC offset in seconds</span><input type="number" id="utcOffsetInSeconds" step="900"></div>' +
            '<button class="btn" id="phoneOffset">Set offset from this phone</button>' +
            '<div class="hint" style="margin-top:8px;">No automatic daylight-saving switch &mdash; adjust twice a year, or just tap the button.</div>' +
          '</div>' +
          '<div class="card">' +
            '<div class="card-title">Display</div>' +
            '<div class="row"><span class="label">12h clock<span class="sub">Hours as 1&ndash;12 instead of 0&ndash;23</span></span>' +
              '<label class="switch"><input type="checkbox" id="clock_12h"><span class="knob"></span></label></div>' +
            '<div class="row"><span class="label">Leading zero for hours<span class="sub">Off skips the 0 in 9:41 &mdash; time reads one digit sooner</span></span>' +
              '<label class="switch"><input type="checkbox" id="clock_leading_hour_zero"><span class="knob"></span></label></div>' +
          '</div>' +
          '<div class="card">' +
            '<div class="row" style="min-height:0;"><span class="card-title" style="margin:0;">Night sleep</span>' +
              '<label class="switch"><input type="checkbox" id="clock_sleep"><span class="knob"></span></label></div>' +
            '<div class="two-cols">' +
              '<div class="field"><span class="field-label">Begin</span><input type="time" id="clock_sleep_start"></div>' +
              '<div class="field"><span class="field-label">End</span><input type="time" id="clock_sleep_finish"></div>' +
            '</div>' +
            '<div class="hint">Tube off during the window (may span midnight) &mdash; saves IN-12B life. Top button wakes it for 3 minutes.</div>' +
          '</div>' +
          '<div class="save-area"><button class="btn primary" data-save>Save changes</button>' +
          '<div class="hint">Changes apply instantly &mdash; no restart</div></div>' +
        '</section>' +

        '<section class="screen" data-screen="alarms">' +
          alarmHtml(0) + alarmHtml(1) + alarmHtml(2) +
          '<div class="hint" style="padding:0 4px;">Rings its melody for up to 10 minutes. Press either button on the clock to mute.</div>' +
          '<div class="save-area"><button class="btn primary" data-save>Save changes</button>' +
          '<div class="hint">Changes apply instantly &mdash; no restart</div></div>' +
        '</section>' +

        '<section class="screen" data-screen="system">' +
          '<div class="card list-card">' +
            '<div class="row"><span class="value">Firmware version</span><span class="label" data-info="gumboardVersion">&hellip;</span></div>' +
            '<div class="row"><span class="value">Sketch size</span><span class="label" data-info="sketchSize">&hellip;</span></div>' +
            '<div class="row"><span class="value">Free sketch space</span><span class="label" data-info="freeSketchSpace">&hellip;</span></div>' +
            '<div class="row"><span class="value">WiFi RSSI</span><span class="label" data-info="wifiRSSI">&hellip;</span></div>' +
            '<div class="row"><span class="value">WiFi quality</span><span class="label" data-info="wifiQuality">&hellip;</span></div>' +
            '<div class="row"><span class="value">WiFi SSID</span><span class="label" data-info="wifiSSID">&hellip;</span></div>' +
            '<div class="row"><span class="value">IP address</span><span class="label" data-info="ipAddress">&hellip;</span></div>' +
            '<div class="row"><span class="value">Free heap</span><span class="label" data-info="freeHeap">&hellip;</span></div>' +
            '<div class="row"><span class="value">Chip ID</span><span class="label" data-info="chipID">&hellip;</span></div>' +
            '<div class="row"><span class="value">CPU frequency</span><span class="label" data-info="cpuFreqMHz">&hellip;</span></div>' +
            '<div class="row"><span class="value">Sleep mode active</span><span class="label" data-info="clock_sleep">&hellip;</span></div>' +
            '<div class="row"><span class="value">Weekday</span><span class="label" data-info="weekday">&hellip;</span></div>' +
          '</div>' +
          '<div class="card">' +
            '<div class="card-title">Event log</div>' +
            '<div class="console" id="log"></div>' +
            '<div class="hint">Streams live from the clock while this page is open.</div>' +
          '</div>' +
          '<div class="card">' +
            '<div class="card-title">Firmware update</div>' +
            '<div class="hint">Flash a new .bin over WiFi &mdash; no cables needed. The clock keeps its settings.</div>' +
            '<button class="btn" id="updateBtn" style="margin-top:10px;">' + ICONS.upload + '<span>Open the updater&hellip;</span></button>' +
          '</div>' +
        '</section>' +

        '</main>' +

        '<nav class="tabbar">' +
          tabHtml('home', 'home', 'Home') +
          tabHtml('clock', 'clock', 'Clock') +
          tabHtml('alarms', 'bell', 'Alarms') +
          tabHtml('system', 'cpu', 'System') +
        '</nav>' +

        '<div class="overlay" id="overlay"><div class="box">' +
          '<h3>Config saved</h3>' +
          '<p>Applied instantly &mdash; no restart needed.</p>' +
        '</div></div>';
    }

    // ---- helpers ----------------------------------------------------------

    function $(sel) { return document.querySelector(sel); }
    function all(sel) { return document.querySelectorAll(sel); }
    function el(id) { return document.getElementById(id); }

    function showScreen(name) {
        all('.screen').forEach(function (s) {
            s.classList.toggle('active', s.getAttribute('data-screen') === name);
        });
        all('.tab').forEach(function (t) {
            t.classList.toggle('active', t.getAttribute('data-tab') === name);
        });
        window.scrollTo(0, 0);
    }

    function setConn(online) {
        $('#conn').classList.toggle('online', online);
        el('connText').textContent = online ? 'Online' : 'Offline';
    }

    function pad(n) { return (n < 10 ? '0' : '') + n; }

    // ---- config <-> form --------------------------------------------------

    function applyConfig(json) {
        if (json.ntpServer !== undefined) cfg.ntpServer = String(json.ntpServer);
        if (json.utcOffsetInSeconds !== undefined) cfg.utcOffsetInSeconds = parseInt(json.utcOffsetInSeconds, 10) || 0;
        if (json.clock_12h !== undefined) cfg.clock_12h = !!json.clock_12h;
        if (json.clock_leading_hour_zero !== undefined) cfg.clock_leading_hour_zero = !!json.clock_leading_hour_zero;
        if (json.clock_sleep !== undefined) cfg.clock_sleep = !!json.clock_sleep;
        if (json.clock_sleep_start !== undefined) cfg.clock_sleep_start = String(json.clock_sleep_start);
        if (json.clock_sleep_finish !== undefined) cfg.clock_sleep_finish = String(json.clock_sleep_finish);
        for (var i = 0; i < 3; i++) {
            var n = i + 1;
            if (json['alarm' + n + 'Time'] !== undefined) cfg.alarms[i].time = String(json['alarm' + n + 'Time']);
            if (json['alarm' + n + 'Active'] !== undefined) cfg.alarms[i].active = !!json['alarm' + n + 'Active'];
            if (json['alarm' + n + 'Melody'] !== undefined) cfg.alarms[i].melody = parseInt(json['alarm' + n + 'Melody'], 10) || 0;
            if (json['alarm' + n + 'Weekdays'] !== undefined) {
                var parts = String(json['alarm' + n + 'Weekdays']).split(',');
                for (var d = 0; d < 7; d++) cfg.alarms[i].weekdays[d] = parts[d] === '1' ? 1 : 0;
            }
        }
        renderConfig();
    }

    function renderConfig() {
        el('ntpServer').value = cfg.ntpServer;
        el('utcOffsetInSeconds').value = cfg.utcOffsetInSeconds;
        el('clock_12h').checked = cfg.clock_12h;
        el('clock_leading_hour_zero').checked = cfg.clock_leading_hour_zero;
        el('clock_sleep').checked = cfg.clock_sleep;
        el('clock_sleep_start').value = cfg.clock_sleep_start;
        el('clock_sleep_finish').value = cfg.clock_sleep_finish;
        for (var i = 0; i < 3; i++) {
            el('alarm' + i + 'Time').value = cfg.alarms[i].time;
            el('alarm' + i + 'Active').checked = cfg.alarms[i].active;
            el('alarm' + i + 'Melody').value = cfg.alarms[i].melody;
            el('alarmCard' + i).classList.toggle('off', !cfg.alarms[i].active);
        }
        all('.chip').forEach(function (chip) {
            var a = parseInt(chip.getAttribute('data-alarm'), 10);
            var d = parseInt(chip.getAttribute('data-day'), 10);
            chip.classList.toggle('on', cfg.alarms[a].weekdays[d] === 1);
        });
        el('homeSleep').textContent = cfg.clock_sleep && cfg.clock_sleep_start && cfg.clock_sleep_finish
            ? cfg.clock_sleep_start + ' – ' + cfg.clock_sleep_finish : 'off';
        renderNextAlarm();
    }

    function readForm() {
        cfg.ntpServer = el('ntpServer').value;
        cfg.utcOffsetInSeconds = parseInt(el('utcOffsetInSeconds').value, 10) || 0;
        cfg.clock_12h = el('clock_12h').checked;
        cfg.clock_leading_hour_zero = el('clock_leading_hour_zero').checked;
        cfg.clock_sleep = el('clock_sleep').checked;
        cfg.clock_sleep_start = el('clock_sleep_start').value;
        cfg.clock_sleep_finish = el('clock_sleep_finish').value;
        for (var i = 0; i < 3; i++) {
            cfg.alarms[i].time = el('alarm' + i + 'Time').value;
            cfg.alarms[i].active = el('alarm' + i + 'Active').checked;
            cfg.alarms[i].melody = parseInt(el('alarm' + i + 'Melody').value, 10) || 0;
        }
    }

    function configPayload() {
        readForm();
        var out = {
            ntpServer: cfg.ntpServer,
            utcOffsetInSeconds: cfg.utcOffsetInSeconds,
            clock_12h: cfg.clock_12h,
            clock_leading_hour_zero: cfg.clock_leading_hour_zero,
            clock_sleep: cfg.clock_sleep,
            clock_sleep_start: cfg.clock_sleep_start,
            clock_sleep_finish: cfg.clock_sleep_finish
        };
        for (var i = 0; i < 3; i++) {
            var n = i + 1;
            out['alarm' + n + 'Time'] = cfg.alarms[i].time;
            out['alarm' + n + 'Active'] = cfg.alarms[i].active;
            out['alarm' + n + 'Melody'] = cfg.alarms[i].melody;
            out['alarm' + n + 'Weekdays'] = cfg.alarms[i].weekdays.join(',');
        }
        return out;
    }

    function renderNextAlarm() {
        var now = new Date();
        var best = null;
        for (var i = 0; i < 3; i++) {
            var a = cfg.alarms[i];
            if (!a.active || !a.time) continue;
            var hm = a.time.split(':');
            for (var off = 0; off < 7; off++) {
                var day = (now.getDay() + off) % 7;
                if (a.weekdays[day] !== 1) continue;
                var mins = off * 1440 + parseInt(hm[0], 10) * 60 + parseInt(hm[1], 10) -
                           (off === 0 ? now.getHours() * 60 + now.getMinutes() : 0);
                if (off === 0 && mins <= 0) continue;
                if (best === null || mins < best.mins) best = { mins: mins, time: a.time, day: day };
            }
        }
        el('nextAlarm').textContent = best
            ? 'Next alarm ' + best.time + ' · ' + DAY_NAMES[best.day] : 'No alarm set';
    }

    // ---- info + log -------------------------------------------------------

    function applyInfo(json) {
        Object.keys(json).forEach(function (k) { info[k] = json[k]; });
        all('[data-info]').forEach(function (node) {
            var k = node.getAttribute('data-info');
            if (info[k] !== undefined) node.textContent = String(info[k]);
        });
        if (info.wifiSSID !== undefined) {
            el('homeWifi').textContent = info.wifiSSID +
                (info.wifiQuality !== undefined ? ' · ' + info.wifiQuality + '%' : '');
        }
    }

    function addLog(entry) {
        var line = document.createElement('span');
        line.textContent = '[' + entry.timeStamp + '] ' + entry['function'] + ': ' + entry.message;
        var log = el('log');
        log.appendChild(line);
        while (log.childNodes.length > 200) log.removeChild(log.firstChild);
        log.scrollTop = log.scrollHeight;
    }

    function handleMessage(raw) {
        if (typeof raw !== 'string' || raw.charAt(0) !== '{') return;
        var json;
        try { json = JSON.parse(raw); } catch (e) { return; }
        if (json.log) { addLog(json.log); return; }
        if (json.gumboardVersion !== undefined || json.freeHeap !== undefined) { applyInfo(json); return; }
        applyConfig(json);
    }

    // ---- websockets -------------------------------------------------------

    var sockets = [];

    function openSocket(path) {
        if (!HOST) return;
        var ws;
        try { ws = new WebSocket('ws://' + HOST + ':81/' + path); } catch (e) { return; }
        sockets.push(ws);
        ws.onopen = function () { setConn(true); };
        ws.onmessage = function (e) { handleMessage(e.data); };
        ws.onclose = function () {
            sockets = sockets.filter(function (s) { return s !== ws; });
            if (sockets.length === 0) setConn(false);
            setTimeout(function () { openSocket(path); }, 3000);
        };
        ws.onerror = function () { try { ws.close(); } catch (e) {} };
    }

    setInterval(function () {
        sockets.forEach(function (ws) {
            if (ws.readyState === WebSocket.OPEN) ws.send('KeepAlive');
        });
    }, 1000);

    function saveConfig() {
        var payload = JSON.stringify(configPayload());
        renderConfig();
        var overlay = el('overlay');
        overlay.classList.add('show');
        setTimeout(function () { overlay.classList.remove('show'); }, 1800);
        if (!HOST) return;
        var ws = new WebSocket('ws://' + HOST + ':81/setConfig');
        ws.onopen = function () { ws.send(payload); ws.close(); };
    }

    // ---- melody preview (WebAudio approximation of the piezo) -------------

    var audioCtx = null;
    var previewOsc = null;

    function previewMelody(index) {
        if (previewOsc) {
            try { previewOsc.stop(); } catch (e) {}
            previewOsc = null;
        }
        var AC = window.AudioContext || window.webkitAudioContext;
        if (!AC) return;
        if (!audioCtx) audioCtx = new AC();
        if (audioCtx.resume) audioCtx.resume();
        var osc = audioCtx.createOscillator();
        var gain = audioCtx.createGain();
        osc.type = 'square';
        osc.connect(gain);
        gain.connect(audioCtx.destination);
        var notes = MELODY_NOTES[index] || MELODY_NOTES[0];
        var t = audioCtx.currentTime;
        gain.gain.setValueAtTime(0, t);
        for (var loop = 0; loop < 2; loop++) {
            for (var n = 0; n < notes.length; n++) {
                if (notes[n][0] > 0) {
                    osc.frequency.setValueAtTime(notes[n][0], t);
                    gain.gain.setValueAtTime(0.08, t);
                } else {
                    gain.gain.setValueAtTime(0, t);
                }
                t += notes[n][1] / 1000;
            }
        }
        gain.gain.setValueAtTime(0, t);
        osc.start();
        osc.stop(t + 0.05);
        previewOsc = osc;
    }

    // ---- tube demo (same timings as zifra_clock_timer.h) ------------------

    var SLOTS = [
        { digit: 0, dot: false, ms: 800 }, { off: true, ms: 200 },
        { digit: 1, dot: false, ms: 800 }, { off: true, ms: 700 },
        { digit: 2, dot: true, ms: 800 }, { off: true, ms: 200 },
        { digit: 3, dot: true, ms: 800 }, { off: true, ms: 1600 }
    ];

    function tubeStep(pos) {
        var slot = SLOTS[pos];
        var tube = el('tube');
        if (!tube) return;
        if (slot.off) {
            tube.classList.add('dark');
            el('tubeDot').classList.remove('lit');
        } else {
            var now = new Date();
            var digits = [Math.floor(now.getHours() / 10), now.getHours() % 10,
                          Math.floor(now.getMinutes() / 10), now.getMinutes() % 10];
            tube.classList.remove('dark');
            el('tubeDigit').textContent = digits[slot.digit];
            el('tubeDot').classList.toggle('lit', slot.dot);
        }
        setTimeout(function () { tubeStep((pos + 1) % SLOTS.length); }, slot.ms);
    }

    function tickHero() {
        var now = new Date();
        el('heroTime').textContent = pad(now.getHours()) + ':' + pad(now.getMinutes());
    }

    // ---- boot -------------------------------------------------------------

    function wire() {
        all('.tab').forEach(function (t) {
            t.addEventListener('click', function () { showScreen(t.getAttribute('data-tab')); });
        });
        all('[data-goto]').forEach(function (b) {
            b.addEventListener('click', function () { showScreen(b.getAttribute('data-goto')); });
        });
        all('.chip').forEach(function (chip) {
            chip.addEventListener('click', function () {
                var a = parseInt(chip.getAttribute('data-alarm'), 10);
                var d = parseInt(chip.getAttribute('data-day'), 10);
                cfg.alarms[a].weekdays[d] = cfg.alarms[a].weekdays[d] === 1 ? 0 : 1;
                chip.classList.toggle('on', cfg.alarms[a].weekdays[d] === 1);
                renderNextAlarm();
            });
        });
        for (var i = 0; i < 3; i++) {
            (function (i) {
                el('alarm' + i + 'Active').addEventListener('change', function (e) {
                    el('alarmCard' + i).classList.toggle('off', !e.target.checked);
                });
            })(i);
        }
        all('[data-save]').forEach(function (b) { b.addEventListener('click', saveConfig); });
        all('[data-preview]').forEach(function (b) {
            b.addEventListener('click', function () {
                var i = parseInt(b.getAttribute('data-preview'), 10);
                previewMelody(parseInt(el('alarm' + i + 'Melody').value, 10) || 0);
            });
        });
        el('phoneOffset').addEventListener('click', function () {
            el('utcOffsetInSeconds').value = -(new Date().getTimezoneOffset() * 60);
        });
        function openUpdater() { location.href = '/update'; }
        el('updateBtn').addEventListener('click', openUpdater);
        el('updateLink').addEventListener('click', openUpdater);
    }

    render();
    wire();
    renderConfig();
    setConn(false);
    tubeStep(0);
    tickHero();
    setInterval(tickHero, 10000);
    setInterval(renderNextAlarm, 60000);
    openSocket('main');
    openSocket('settime');

    // exposed for the demo page and screenshot harness
    window.zifraApp = {
        applyConfig: applyConfig,
        applyInfo: applyInfo,
        addLog: addLog,
        setConn: setConn,
        showScreen: showScreen,
        configPayload: configPayload
    };
})();
