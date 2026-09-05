/*
 * Websockets, event wiring and boot. Talks the same protocol as v1:
 *   ws://<clock>:81/main      -> system info + live log
 *   ws://<clock>:81/settime   -> config JSON on connect
 *   ws://<clock>:81/setConfig -> send config JSON, clock saves + applies it
 */
(function (Z) {
    'use strict';

    const { el, all } = Z;
    const cfg = Z.cfg;
    const HOST = location.hostname;

    // ---- websockets -------------------------------------------------------

    let sockets = [];

    const handleMessage = (raw) => {
        if (typeof raw !== 'string' || raw.charAt(0) !== '{') return;
        let json;
        try { json = JSON.parse(raw); } catch (e) { return; }
        if (json.log) { Z.addLog(json.log); return; }
        if (json.gumboardVersion !== undefined || json.freeHeap !== undefined) {
            Z.applyInfo(json);
            return;
        }
        Z.applyConfig(json);
    };

    const openSocket = (path) => {
        if (!HOST) return;
        let ws;
        try { ws = new WebSocket('ws://' + HOST + ':81/' + path); } catch (e) { return; }
        sockets.push(ws);
        ws.onopen = () => Z.setConn(true);
        ws.onmessage = (e) => handleMessage(e.data);
        ws.onclose = () => {
            sockets = sockets.filter((s) => s !== ws);
            if (sockets.length === 0) Z.setConn(false);
            setTimeout(() => openSocket(path), 3000);
        };
        ws.onerror = () => { try { ws.close(); } catch (e) { /* closing */ } };
    };

    setInterval(() => {
        sockets.forEach((ws) => {
            if (ws.readyState === WebSocket.OPEN) ws.send('KeepAlive');
        });
    }, 4000);

    // Sends a (partial) config to the clock; it saves, applies and
    // broadcasts the result back to every open page.
    const sendConfig = (payload) => {
        const overlay = el('overlay');
        overlay.classList.add('show');
        setTimeout(() => overlay.classList.remove('show'), 1800);
        if (!HOST) return;
        const ws = new WebSocket('ws://' + HOST + ':81/setConfig');
        ws.onopen = () => { ws.send(JSON.stringify(payload)); ws.close(); };
    };

    const saveConfig = () => {
        const payload = Z.configPayload();
        Z.renderConfig();
        sendConfig(payload);
    };

    // ---- event wiring -----------------------------------------------------

    const wire = () => {
        all('.tab').forEach((t) => {
            t.addEventListener('click', () => Z.showScreen(t.getAttribute('data-tab')));
        });
        all('[data-goto]').forEach((b) => {
            b.addEventListener('click', () => Z.showScreen(b.getAttribute('data-goto')));
        });
        all('.chip').forEach((chip) => {
            chip.addEventListener('click', () => {
                const a = parseInt(chip.getAttribute('data-alarm'), 10);
                const d = parseInt(chip.getAttribute('data-day'), 10);
                cfg.alarms[a].weekdays[d] = cfg.alarms[a].weekdays[d] === 1 ? 0 : 1;
                chip.classList.toggle('on', cfg.alarms[a].weekdays[d] === 1);
                Z.renderNextAlarm();
            });
        });
        cfg.alarms.forEach((alarm, i) => {
            el('alarm' + i + 'Active').addEventListener('change', (e) => {
                el('alarmCard' + i).classList.toggle('off', !e.target.checked);
            });
        });
        all('[data-save]').forEach((b) => b.addEventListener('click', saveConfig));
        all('[data-preview]').forEach((b) => {
            b.addEventListener('click', () => {
                const i = parseInt(b.getAttribute('data-preview'), 10);
                Z.previewMelody(parseInt(el('alarm' + i + 'Melody').value, 10) || 0);
            });
        });
        el('clock_12h').addEventListener('change', () => {
            const nowOn = el('clock_12h').checked;
            cfg.clock_12h = nowOn;
            Z.switchTimeInputs(!nowOn);
            Z.tickHero();
        });
        el('otaClear').addEventListener('click', () => sendConfig({ otaPassword: '' }));
        el('clock_leading_hour_zero').addEventListener('change', () => {
            cfg.clock_leading_hour_zero = el('clock_leading_hour_zero').checked;
            Z.tickHero();
        });
        el('phoneOffset').addEventListener('click', () => {
            // With automatic DST the stored offset must be the winter
            // (standard) offset, which is the smaller of the Jan/Jul values.
            const now = new Date();
            let mins = -now.getTimezoneOffset();
            if (parseInt(el('dstMode').value, 10)) {
                const jan = -(new Date(now.getFullYear(), 0, 1).getTimezoneOffset());
                const jul = -(new Date(now.getFullYear(), 6, 1).getTimezoneOffset());
                mins = Math.min(jan, jul);
            }
            el('utcOffsetInSeconds').value = mins * 60;
        });
        el('updateLink').addEventListener('click', () => Z.showScreen('system'));
        wireUpdater();
    };

    // Flashes a firmware .bin straight from the System page: the app is
    // served by the clock itself, so POST /update is same-origin. The
    // standalone page at /update stays as a fallback.
    const wireUpdater = () => {
        const file = el('fwFile');
        const flash = el('fwFlash');
        const msg = el('fwMsg');
        file.addEventListener('change', () => {
            if (!file.files.length) return;
            el('fwName').textContent = file.files[0].name;
            el('fwPick').classList.add('picked');
            flash.disabled = false;
        });
        flash.addEventListener('click', () => {
            if (!file.files.length) return;
            flash.disabled = true;
            el('fwBar').hidden = false;
            msg.className = 'hint';
            msg.textContent = 'Flashing… don’t unplug the clock.';
            const fd = new FormData();
            fd.append('update', file.files[0]);
            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/update');
            xhr.upload.onprogress = (e) => {
                if (e.lengthComputable) el('fwFill').style.width = Math.round(e.loaded / e.total * 100) + '%';
            };
            xhr.onload = () => {
                if (xhr.status === 200 && xhr.responseText.indexOf('Success') >= 0) {
                    el('fwFill').style.width = '100%';
                    msg.className = 'hint ok';
                    msg.textContent = 'Update done — the clock is rebooting and this page reconnects by itself.';
                } else {
                    msg.className = 'hint bad';
                    msg.textContent = 'Update failed — the clock kept its old firmware. Check the .bin and try again.';
                    flash.disabled = false;
                }
            };
            xhr.onerror = () => {
                msg.className = 'hint bad';
                msg.textContent = 'Connection lost — if the flash finished, the clock is rebooting.';
                flash.disabled = false;
            };
            xhr.send(fd);
        });
    };

    // ---- boot -------------------------------------------------------------

    Z.on('config', Z.renderConfig);
    Z.on('info', Z.renderInfo);
    Z.on('readForm', Z.readForm);

    Z.render();
    wire();
    Z.renderConfig();
    Z.setConn(false);
    Z.tubeStep(0);
    Z.tickHero();
    setInterval(Z.tickHero, 10000);
    setInterval(() => { Z.renderNextAlarm(); Z.renderSleepRow(); }, 60000);
    openSocket('main');
    openSocket('settime');

    // exposed for the demo page and screenshot harness
    window.zifraApp = {
        applyConfig: Z.applyConfig,
        applyInfo: Z.applyInfo,
        addLog: Z.addLog,
        setConn: Z.setConn,
        showScreen: Z.showScreen,
        configPayload: Z.configPayload
    };
})(window.ZIFRA);
