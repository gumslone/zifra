/*
 * ZIFRA web UI v2 — core namespace: constants, pure helpers and a tiny
 * event bus. Every module hangs off window.ZIFRA and the files are
 * concatenated in order by js/javascript.php:
 *   core.js  -> constants, time math, events
 *   state.js -> config/info state, schema-driven apply/serialize
 *   view.js  -> markup templates, rendering, form binding
 *   audio.js -> melody preview
 *   app.js   -> websockets, wiring, boot
 */
window.ZIFRA = {};

(function (Z) {
    'use strict';

    Z.DAY_LETTERS = ['S', 'M', 'T', 'W', 'T', 'F', 'S'];
    Z.DAY_NAMES = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    Z.DAY_FULL = ['Sunday', 'Monday', 'Tuesday', 'Wednesday',
                  'Thursday', 'Friday', 'Saturday'];

    // Mirrors MELODIES in the firmware's melodies.h ([freq, ms]; freq 0 = rest)
    Z.MELODIES = [
        { name: 'Classic', notes: [[500, 180], [780, 180]] },
        { name: 'Double beep', notes: [[880, 150], [0, 150], [880, 150], [0, 600]] },
        { name: 'Siren', notes: [[400, 150], [520, 150], [660, 150], [780, 150], [660, 150], [520, 150]] },
        { name: 'Chime', notes: [[523, 200], [659, 200], [784, 200], [1047, 400], [0, 420]] },
        { name: 'Cuckoo', notes: [[659, 300], [523, 400], [0, 400], [659, 300], [523, 400], [0, 800]] },
        { name: 'Bugle', notes: [[392, 150], [523, 150], [659, 300], [523, 150], [392, 150], [659, 300], [0, 300]] },
        { name: 'Gentle beep', notes: [[988, 150], [0, 1000]] },
        { name: 'SOS', notes: [[880, 150], [0, 150], [880, 150], [0, 150], [880, 150], [0, 300],
                               [880, 450], [0, 150], [880, 450], [0, 150], [880, 450], [0, 300],
                               [880, 150], [0, 150], [880, 150], [0, 150], [880, 150], [0, 900]] }
    ];

    Z.ICONS = {
        home: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"></path><polyline points="9 22 9 12 15 12 15 22"></polyline></svg>',
        clock: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="9"></circle><polyline points="12 7 12 12 15.5 13.5"></polyline></svg>',
        bell: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M18 8A6 6 0 0 0 6 8c0 7-3 9-3 9h18s-3-2-3-9"></path><path d="M13.73 21a2 2 0 0 1-3.46 0"></path></svg>',
        cpu: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="4" width="16" height="16" rx="2"></rect><rect x="9" y="9" width="6" height="6"></rect><line x1="12" y1="1" x2="12" y2="4"></line><line x1="12" y1="20" x2="12" y2="23"></line><line x1="1" y1="12" x2="4" y2="12"></line><line x1="20" y1="12" x2="23" y2="12"></line></svg>',
        play: '<svg viewBox="0 0 24 24" fill="currentColor" stroke="none"><polygon points="7 4 20 12 7 20"></polygon></svg>',
        github: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M9 19c-5 1.5-5-2.5-7-3m14 6v-3.87a3.37 3.37 0 0 0-.94-2.61c3.14-.35 6.44-1.54 6.44-7A5.44 5.44 0 0 0 20 4.77 5.07 5.07 0 0 0 19.91 1S18.73.65 16 2.48a13.38 13.38 0 0 0-7 0C6.27.65 5.09 1 5.09 1A5.07 5.07 0 0 0 5 4.77 5.44 5.44 0 0 0 3.5 8.55c0 5.42 3.3 6.61 6.44 7A3.37 3.37 0 0 0 9 18.13V22"></path></svg>',
        chevron: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="9 18 15 12 9 6"></polyline></svg>',
        upload: '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path><polyline points="17 8 12 3 7 8"></polyline><line x1="12" y1="3" x2="12" y2="15"></line></svg>'
    };

    // ---- DOM shorthands ---------------------------------------------------

    Z.$ = (sel) => document.querySelector(sel);
    Z.all = (sel) => document.querySelectorAll(sel);
    Z.el = (id) => document.getElementById(id);

    // ---- event bus --------------------------------------------------------

    const listeners = {};
    Z.on = (name, fn) => { (listeners[name] = listeners[name] || []).push(fn); };
    Z.emit = (name, arg) => { (listeners[name] || []).forEach((fn) => fn(arg)); };

    // ---- time math --------------------------------------------------------

    const pad = (n) => (n < 10 ? '0' : '') + n;

    Z.time = {
        pad,

        // 'HH:MM' -> { h: 1-12, m, pm } (null when not a valid time)
        to12(t) {
            const p = String(t).split(':');
            const h = parseInt(p[0], 10);
            const m = parseInt(p[1], 10);
            if (isNaN(h) || isNaN(m)) return null;
            return { h: (h % 12) || 12, m, pm: h >= 12 };
        },

        // 12h parts -> 'HH:MM' ('' when out of range)
        from12(h, m, pm) {
            if (isNaN(h) || isNaN(m) || h < 1 || h > 12 || m < 0 || m > 59) return '';
            return pad((h % 12) + (pm ? 12 : 0)) + ':' + pad(m);
        },

        // 'HH:MM' formatted for the current clock mode ('9:41 PM' in 12h)
        format(t, is12h) {
            if (!is12h) return t;
            const v = Z.time.to12(t);
            return v ? v.h + ':' + pad(v.m) + ' ' + (v.pm ? 'PM' : 'AM') : t;
        },

        // hours exactly as the firmware displays them (CurrentTime::getHours)
        displayHours(h24, is12h) {
            if (!is12h) return h24;
            if (h24 > 12) return h24 - 12;
            if (h24 === 0) return 12;
            return h24;
        },

        // mirrors the firmware's sleep decision (sleep_logic.h); `now` is {h, m}
        sleepActive(cfg, now) {
            if (!cfg.clock_sleep || !cfg.clock_sleep_start || !cfg.clock_sleep_finish) return false;
            const hhmm = (s) => {
                const p = s.split(':');
                return parseInt(p[0], 10) * 100 + parseInt(p[1], 10);
            };
            const t = now.h * 100 + now.m;
            const start = hhmm(cfg.clock_sleep_start);
            const finish = hhmm(cfg.clock_sleep_finish);
            if (Math.floor(start / 100) > Math.floor(finish / 100)) {
                return t >= start || t <= finish; // window wraps past midnight
            }
            return t >= start && t <= finish;
        },

        // The clock's own time when the firmware reports it (info.clockTime,
        // "HH:MM" 24h), the browser's otherwise - so the dashboard mirrors
        // what the tube really shows, time-zone mistakes included.
        clockNow() {
            const t = Z.info && Z.info.clockTime;
            if (t) {
                const p = String(t).split(':');
                return { h: parseInt(p[0], 10), m: parseInt(p[1], 10) };
            }
            const d = new Date();
            return { h: d.getHours(), m: d.getMinutes() };
        },

        // the soonest active alarm: { mins, time, day } or null
        nextAlarm(alarms, now) {
            let best = null;
            alarms.forEach((a) => {
                if (!a.active || !a.time) return;
                const hm = a.time.split(':');
                for (let off = 0; off < 7; off++) {
                    const day = (now.getDay() + off) % 7;
                    if (a.weekdays[day] !== 1) continue;
                    const mins = off * 1440 +
                        parseInt(hm[0], 10) * 60 + parseInt(hm[1], 10) -
                        (off === 0 ? now.getHours() * 60 + now.getMinutes() : 0);
                    if (off === 0 && mins <= 0) continue;
                    if (best === null || mins < best.mins) best = { mins, time: a.time, day };
                }
            });
            return best;
        }
    };
})(window.ZIFRA);
