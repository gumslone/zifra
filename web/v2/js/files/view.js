/*
 * Markup and rendering. Screens are data (Z.SCREENS) — the shell, tabbar
 * and navigation are generated from the list — and the config form is
 * bound to the state through per-kind fill/read handlers driven by the
 * FIELDS schema in state.js.
 */
(function (Z) {
    'use strict';

    const { el, all, ICONS, DAY_LETTERS, DAY_NAMES, DAY_FULL } = Z;
    const cfg = Z.cfg;

    // ---- templates --------------------------------------------------------

    const alarmHtml = (i) => {
        const chips = DAY_LETTERS.map((letter, d) =>
            `<button class="chip" data-alarm="${i}" data-day="${d}" title="${DAY_FULL[d]}" aria-label="${DAY_FULL[d]}">${letter}</button>`
        ).join('');
        const options = Z.MELODIES.map((m, idx) =>
            `<option value="${idx}">${m.name}</option>`
        ).join('');
        return `
        <div class="card alarm-card" id="alarmCard${i}">
          <div class="alarm-head">
            ${timeInputHtml('alarm' + i + 'Time', 'alarm-time')}
            <label class="switch"><input type="checkbox" id="alarm${i}Active"><span class="knob"></span></label>
          </div>
          <div class="weekdays">${chips}</div>
          <div class="sound-row">
            <span class="value">Sound</span>
            <select id="alarm${i}Melody">${options}</select>
            <button class="btn preview-btn" data-preview="${i}" aria-label="Preview sound">${ICONS.play}</button>
          </div>
        </div>`;
    };

    // The native time input always follows the browser locale, so the 12h
    // clock setting gets its own hour/minute/AM-PM editor; values are
    // converted back to 24h HH:MM before they reach the firmware.
    const timeInputHtml = (id, cls = '') => `
        <input type="time" id="${id}" class="${cls}">
        <span class="time12 ${cls ? cls + '-12' : ''}" id="${id}_12" hidden>
          <input type="number" id="${id}_h" min="1" max="12" inputmode="numeric" placeholder="--">
          <span class="colon">:</span>
          <input type="number" id="${id}_m" min="0" max="59" inputmode="numeric" placeholder="--">
          <select id="${id}_ap"><option value="AM">AM</option><option value="PM">PM</option></select>
        </span>`;

    const infoRow = (label, key) =>
        `<div class="row"><span class="value">${label}</span><span class="label" data-info="${key}">&hellip;</span></div>`;

    const saveArea = `
        <div class="save-area"><button class="btn primary" data-save>Save changes</button>
        <div class="hint">Changes apply instantly &mdash; no restart</div></div>`;

    const homeHtml = () => `
        <div class="card hero">
          <div class="tube" id="tube"><span class="digit" id="tubeDigit">0</span><span class="tdot" id="tubeDot"></span></div>
          <div class="hero-info">
            <div class="hero-time" id="heroTime">--:--</div>
            <div class="hint">spelled out one digit at a time on the IN-12B tube</div>
            <div class="hero-sub">${ICONS.bell}<span id="nextAlarm">No alarm set</span></div>
          </div>
        </div>
        <div class="card list-card">
          ${infoRow('Firmware', 'gumboardVersion')}
          <div class="row"><span class="value">WiFi</span><span class="label" id="homeWifi">&hellip;</span></div>
          ${infoRow('IP address', 'ipAddress')}
          <div class="row"><span class="value">Night sleep</span><span class="value lit" id="homeSleep">off</span></div>
        </div>
        <div class="card list-card">
          <button class="link-row" data-goto="system">Event log ${ICONS.chevron}</button>
          <button class="link-row" id="updateLink">Firmware update ${ICONS.chevron}</button>
          <a class="link-row" href="https://github.com/gumslone/zifra" target="_blank" rel="noopener">
            <span class="lead-icon">${ICONS.github}ZIFRA on GitHub</span>${ICONS.chevron}</a>
        </div>`;

    const clockHtml = () => `
        <div class="card">
          <div class="card-title">Time source</div>
          <div class="field"><span class="field-label">NTP server</span><input type="text" id="ntpServer"></div>
          <div class="field"><span class="field-label">UTC offset in seconds</span><input type="number" id="utcOffsetInSeconds" step="900"></div>
          <div class="field"><span class="field-label">Summer/winter time</span><select id="dstMode">
            <option value="0">Off</option>
            <option value="1">Automatic (Europe)</option>
            <option value="2">Automatic (USA)</option>
          </select></div>
          <button class="btn" id="phoneOffset">Set offset from this phone</button>
          <div class="hint" style="margin-top:8px;">With automatic switching, the offset is the winter (standard) time &mdash; the clock adds the summer hour by itself.</div>
        </div>
        <div class="card">
          <div class="card-title">Display</div>
          <div class="row"><span class="label">12h clock<span class="sub">Hours as 1&ndash;12 instead of 0&ndash;23</span></span>
            <label class="switch"><input type="checkbox" id="clock_12h"><span class="knob"></span></label></div>
          <div class="row"><span class="label">Leading zero for hours<span class="sub">Off skips the 0 in 9:41 &mdash; time reads one digit sooner</span></span>
            <label class="switch"><input type="checkbox" id="clock_leading_hour_zero"><span class="knob"></span></label></div>
        </div>
        <div class="card">
          <div class="row" style="min-height:0;"><span class="card-title" style="margin:0;">Night sleep</span>
            <label class="switch"><input type="checkbox" id="clock_sleep"><span class="knob"></span></label></div>
          <div class="two-cols">
            <div class="field"><span class="field-label">Begin</span>${timeInputHtml('clock_sleep_start')}</div>
            <div class="field"><span class="field-label">End</span>${timeInputHtml('clock_sleep_finish')}</div>
          </div>
          <div class="hint">Tube off during the window (may span midnight) &mdash; saves IN-12B life. Top button wakes it for 3 minutes.</div>
        </div>
        ${saveArea}`;

    const alarmsHtml = () => `
        ${cfg.alarms.map((a, i) => alarmHtml(i)).join('')}
        <div class="card">
          <div class="row" style="min-height:0;">
            <span class="label">Auto-stop<span class="sub">A ringing alarm gives up after this long</span></span>
            <div class="minutes-field"><input type="number" id="alarmTimeoutMinutes" min="1" max="120" step="1"><span>min</span></div>
          </div>
        </div>
        <div class="hint" style="padding:0 4px;">Rings its melody until muted or the auto-stop time passes. Press either button on the clock to mute.</div>
        ${saveArea}`;

    const systemHtml = () => `
        <div class="card list-card">
          ${infoRow('Firmware version', 'gumboardVersion')}
          ${infoRow('Sketch size', 'sketchSize')}
          ${infoRow('Free sketch space', 'freeSketchSpace')}
          ${infoRow('WiFi RSSI', 'wifiRSSI')}
          ${infoRow('WiFi quality', 'wifiQuality')}
          ${infoRow('WiFi SSID', 'wifiSSID')}
          ${infoRow('IP address', 'ipAddress')}
          ${infoRow('Free heap', 'freeHeap')}
          ${infoRow('Chip ID', 'chipID')}
          ${infoRow('CPU frequency', 'cpuFreqMHz')}
          ${infoRow('Sleep mode active', 'clock_sleep')}
          ${infoRow('Weekday', 'weekday')}
        </div>
        <div class="card">
          <div class="card-title">Event log</div>
          <div class="console" id="log"></div>
          <div class="hint">Streams live from the clock while this page is open.</div>
        </div>
        <div class="card">
          <div class="card-title">Firmware update</div>
          <div class="hint">Flash a new .bin over WiFi &mdash; no cables needed. The clock keeps its settings.</div>
          <label class="file-pick" id="fwPick"><input type="file" id="fwFile" accept=".bin" hidden><span id="fwName">Choose firmware .bin&hellip;</span></label>
          <button class="btn" id="fwFlash" disabled>${ICONS.upload}<span>Flash firmware</span></button>
          <div class="progress" hidden id="fwBar"><div id="fwFill"></div></div>
          <div class="hint" id="fwMsg" style="margin-top:8px;">Don't unplug the clock while flashing.</div>
        </div>
        <div class="card">
          <div class="card-title">Update password</div>
          <div class="hint">Optional: without one, anyone on your WiFi can flash firmware. The user name is <strong>zifra</strong>.</div>
          <div class="field"><span class="field-label">New password</span><input type="password" id="otaPassword" autocomplete="new-password" placeholder="Leave empty to keep the current one"></div>
          <div class="hint" id="otaState"></div>
          <div class="save-area" style="margin-top:8px;">
            <button class="btn primary" data-save>Save changes</button>
            <button class="btn" id="otaClear">Remove password</button>
          </div>
        </div>`;

    Z.SCREENS = [
        { id: 'home', icon: 'home', label: 'Home', html: homeHtml },
        { id: 'clock', icon: 'clock', label: 'Clock', html: clockHtml },
        { id: 'alarms', icon: 'bell', label: 'Alarms', html: alarmsHtml },
        { id: 'system', icon: 'cpu', label: 'System', html: systemHtml }
    ];

    Z.render = () => {
        const screens = Z.SCREENS.map((s, i) =>
            `<section class="screen${i === 0 ? ' active' : ''}" data-screen="${s.id}">${s.html()}</section>`
        ).join('');
        const tabs = Z.SCREENS.map((s) =>
            `<button class="tab" data-tab="${s.id}">${ICONS[s.icon]}<span>${s.label}</span></button>`
        ).join('');

        document.getElementById('app').innerHTML = `
        <div class="appbar">
          <div class="brand">ZIFRA</div>
          <div class="conn" id="conn"><span class="dot"></span><span id="connText">Connecting&hellip;</span></div>
        </div>
        <main>${screens}</main>
        <nav class="tabbar">${tabs}</nav>
        <div class="overlay" id="overlay"><div class="box">
          <h3>Config saved</h3>
          <p>Applied instantly &mdash; no restart needed.</p>
        </div></div>`;
    };

    // ---- navigation and status --------------------------------------------

    Z.showScreen = (name) => {
        all('.screen').forEach((s) => s.classList.toggle('active', s.getAttribute('data-screen') === name));
        all('.tab').forEach((t) => t.classList.toggle('active', t.getAttribute('data-tab') === name));
        window.scrollTo(0, 0);
    };

    Z.setConn = (online) => {
        Z.$('#conn').classList.toggle('online', online);
        el('connText').textContent = online ? 'Online' : 'Offline';
    };

    // ---- time editors (24h input or 12h composite) ------------------------

    const fillTimeInput = (id, value) => {
        el(id).value = value;
        const v = Z.time.to12(value);
        el(id + '_h').value = v ? v.h : '';
        el(id + '_m').value = v ? Z.time.pad(v.m) : '';
        el(id + '_ap').value = v && v.pm ? 'PM' : 'AM';
        el(id).hidden = cfg.clock_12h;
        el(id + '_12').hidden = !cfg.clock_12h;
    };

    const readTimeInput = (id, is12) => {
        if (!is12) return el(id).value;
        const h = parseInt(el(id + '_h').value, 10);
        const m = parseInt(el(id + '_m').value, 10);
        return Z.time.from12(h, m, el(id + '_ap').value === 'PM');
    };

    // Every time editor on the page, so a 12h toggle can convert them all
    const timeInputIds = () => ['clock_sleep_start', 'clock_sleep_finish']
        .concat(cfg.alarms.map((a, i) => 'alarm' + i + 'Time'));

    // Re-renders the editors after the 12h switch flipped: reads each one
    // through the previous mode (that is what it still shows), then fills
    // it for the new mode - unsaved edits survive the flip.
    Z.switchTimeInputs = (wasIs12) => {
        timeInputIds().forEach((id) => {
            const value = readTimeInput(id, wasIs12);
            if (id.indexOf('alarm') === 0) {
                cfg.alarms[parseInt(id.charAt(5), 10)].time = value;
            } else {
                cfg[id] = value;
            }
            fillTimeInput(id, value);
        });
    };

    // ---- schema-driven form binding ---------------------------------------

    const BIND = {
        string: {
            fill: (f) => { el(f.key).value = cfg[f.key]; },
            read: (f) => el(f.key).value
        },
        int: {
            fill: (f) => { el(f.key).value = cfg[f.key]; },
            read: (f) => {
                const v = parseInt(el(f.key).value, 10) || f.def;
                return f.min !== undefined ? Math.max(f.min, v) : v;
            }
        },
        bool: {
            fill: (f) => { el(f.key).checked = cfg[f.key]; },
            read: (f) => el(f.key).checked
        },
        time: {
            fill: (f) => fillTimeInput(f.key, cfg[f.key]),
            read: (f) => readTimeInput(f.key, el('clock_12h').checked)
        },
        password: {
            fill: (f) => { el(f.key).value = ''; }, // never echo it back
            read: (f) => el(f.key).value
        },
        flag: {
            fill: () => {
                el('otaState').textContent = cfg.otaPasswordSet
                    ? 'A password is set - flashing asks for it.'
                    : 'No password set.';
                el('otaClear').hidden = !cfg.otaPasswordSet;
            },
            read: (f) => cfg[f.key]
        }
    };

    Z.renderConfig = () => {
        Z.FIELDS.forEach((f) => BIND[f.kind].fill(f));
        cfg.alarms.forEach((alarm, i) => {
            fillTimeInput('alarm' + i + 'Time', alarm.time);
            el('alarm' + i + 'Active').checked = alarm.active;
            el('alarm' + i + 'Melody').value = alarm.melody;
            el('alarmCard' + i).classList.toggle('off', !alarm.active);
        });
        all('.chip').forEach((chip) => {
            const a = parseInt(chip.getAttribute('data-alarm'), 10);
            const d = parseInt(chip.getAttribute('data-day'), 10);
            chip.classList.toggle('on', cfg.alarms[a].weekdays[d] === 1);
        });
        Z.renderSleepRow();
        Z.renderNextAlarm();
        Z.tickHero();
    };

    Z.readForm = () => {
        // clock_12h first: the time fields read through it
        cfg.clock_12h = el('clock_12h').checked;
        Z.FIELDS.forEach((f) => { cfg[f.key] = BIND[f.kind].read(f); });
        cfg.alarms.forEach((alarm, i) => {
            alarm.time = readTimeInput('alarm' + i + 'Time', cfg.clock_12h);
            alarm.active = el('alarm' + i + 'Active').checked;
            alarm.melody = parseInt(el('alarm' + i + 'Melody').value, 10) || 0;
        });
    };

    // ---- dashboard renderers ----------------------------------------------

    Z.renderSleepRow = () => {
        const node = el('homeSleep');
        if (!(cfg.clock_sleep && cfg.clock_sleep_start && cfg.clock_sleep_finish)) {
            node.textContent = 'off';
            return;
        }
        const fmt = (t) => Z.time.format(t, cfg.clock_12h);
        node.textContent = Z.time.sleepActive(cfg, Z.time.clockNow())
            ? 'asleep until ' + fmt(cfg.clock_sleep_finish)
            : fmt(cfg.clock_sleep_start) + ' – ' + fmt(cfg.clock_sleep_finish);
    };

    Z.renderNextAlarm = () => {
        const best = Z.time.nextAlarm(cfg.alarms, new Date());
        el('nextAlarm').textContent = best
            ? 'Next alarm ' + best.time + ' · ' + DAY_NAMES[best.day] : 'No alarm set';
    };

    Z.renderInfo = () => {
        const info = Z.info;
        all('[data-info]').forEach((node) => {
            const k = node.getAttribute('data-info');
            if (info[k] === undefined) return;
            const v = String(info[k]);
            if (k === 'ipAddress' && /^[0-9a-fA-F.:]+$/.test(v)) {
                // clickable: opens the clock directly by IP
                node.textContent = '';
                const a = document.createElement('a');
                a.href = 'http://' + v + '/';
                a.target = '_blank';
                a.rel = 'noopener';
                a.textContent = v;
                node.appendChild(a);
            } else {
                node.textContent = v;
            }
        });
        if (info.wifiSSID !== undefined) {
            el('homeWifi').textContent = info.wifiSSID +
                (info.wifiQuality !== undefined ? ' · ' + info.wifiQuality + '%' : '');
        }
        // the clock's own time may have ticked over
        Z.tickHero();
        Z.renderSleepRow();
    };

    Z.addLog = (entry) => {
        const line = document.createElement('span');
        line.textContent = '[' + entry.timeStamp + '] ' + entry['function'] + ': ' + entry.message;
        const log = el('log');
        log.appendChild(line);
        while (log.childNodes.length > 200) log.removeChild(log.firstChild);
        log.scrollTop = log.scrollHeight;
    };

    // ---- hero time + tube demo (same timings as zifra_clock_timer.h) ------

    const SLOTS = [
        { digit: 0, dot: false, ms: 800 }, { off: true, ms: 200 },
        { digit: 1, dot: false, ms: 800 }, { off: true, ms: 700 },
        { digit: 2, dot: true, ms: 800 }, { off: true, ms: 200 },
        { digit: 3, dot: true, ms: 800 }, { off: true, ms: 1600 }
    ];

    // hours and minutes as the tube shows them right now
    const tubeNow = () => {
        const now = Z.time.clockNow();
        return { h: Z.time.displayHours(now.h, cfg.clock_12h), m: now.m };
    };

    Z.tickHero = () => {
        const now = tubeNow();
        const hh = (now.h < 10 && !cfg.clock_leading_hour_zero) ? String(now.h) : Z.time.pad(now.h);
        el('heroTime').textContent = hh + ':' + Z.time.pad(now.m);
    };

    Z.tubeStep = (pos) => {
        const slot = SLOTS[pos];
        const tube = el('tube');
        if (!tube) return;
        if (Z.time.sleepActive(cfg, Z.time.clockNow())) {
            // the real tube is dark right now - show that
            tube.classList.add('dark');
            el('tubeDot').classList.remove('lit');
            setTimeout(() => Z.tubeStep(pos), 5000);
            return;
        }
        const now = tubeNow();
        // with the leading zero off the firmware skips the first digit's slots
        if (pos <= 1 && now.h < 10 && !cfg.clock_leading_hour_zero) {
            Z.tubeStep(2);
            return;
        }
        if (slot.off) {
            tube.classList.add('dark');
            el('tubeDot').classList.remove('lit');
        } else {
            const digits = [Math.floor(now.h / 10), now.h % 10,
                            Math.floor(now.m / 10), now.m % 10];
            tube.classList.remove('dark');
            el('tubeDigit').textContent = digits[slot.digit];
            el('tubeDot').classList.toggle('lit', slot.dot);
        }
        setTimeout(() => Z.tubeStep((pos + 1) % SLOTS.length), slot.ms);
    };
})(window.ZIFRA);
