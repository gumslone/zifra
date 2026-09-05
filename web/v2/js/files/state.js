/*
 * Config and info state. The FIELDS schema drives everything: defaults,
 * parsing incoming config JSON, form fill/read (view.js) and the payload
 * sent back to the clock — adding a config key is one schema line plus
 * its markup.
 */
(function (Z) {
    'use strict';

    Z.ALARM_COUNT = 3;

    // kind: string | int | bool | time (the 12h/24h time editor)
    //       password (write-only: sent only when typed)
    //       flag (read-only: reported by the clock, never sent back)
    Z.FIELDS = [
        { key: 'ntpServer', kind: 'string', def: 'pool.ntp.org' },
        { key: 'utcOffsetInSeconds', kind: 'int', def: 0 },
        { key: 'dstMode', kind: 'int', def: 0 },
        { key: 'alarmTimeoutMinutes', kind: 'int', def: 10, min: 1 },
        { key: 'clock_12h', kind: 'bool', def: false },
        { key: 'clock_leading_hour_zero', kind: 'bool', def: true },
        { key: 'clock_sleep', kind: 'bool', def: false },
        { key: 'clock_sleep_start', kind: 'time', def: '' },
        { key: 'clock_sleep_finish', kind: 'time', def: '' },
        { key: 'otaPassword', kind: 'password', def: '' },
        { key: 'otaPasswordSet', kind: 'flag', def: false }
    ];

    const PARSE = {
        string: (v) => String(v),
        int: (v) => parseInt(v, 10) || 0,
        bool: (v) => !!v,
        time: (v) => String(v),
        password: (v) => String(v),
        flag: (v) => !!v
    };

    const newAlarm = () => ({ time: '', active: false, melody: 0, weekdays: [0, 0, 0, 0, 0, 0, 0] });

    const cfg = {};
    Z.FIELDS.forEach((f) => { cfg[f.key] = f.def; });
    cfg.alarms = [];
    for (let i = 0; i < Z.ALARM_COUNT; i++) cfg.alarms.push(newAlarm());

    Z.cfg = cfg;
    Z.info = {};

    // Merges a config JSON from the clock into the state (missing keys keep
    // their value) and notifies the view.
    Z.applyConfig = (json) => {
        Z.FIELDS.forEach((f) => {
            if (json[f.key] !== undefined) cfg[f.key] = PARSE[f.kind](json[f.key]);
        });
        cfg.alarms.forEach((alarm, i) => {
            const p = 'alarm' + (i + 1);
            if (json[p + 'Time'] !== undefined) alarm.time = String(json[p + 'Time']);
            if (json[p + 'Active'] !== undefined) alarm.active = !!json[p + 'Active'];
            if (json[p + 'Melody'] !== undefined) alarm.melody = parseInt(json[p + 'Melody'], 10) || 0;
            if (json[p + 'Weekdays'] !== undefined) {
                const parts = String(json[p + 'Weekdays']).split(',');
                for (let d = 0; d < 7; d++) alarm.weekdays[d] = parts[d] === '1' ? 1 : 0;
            }
        });
        Z.emit('config');
    };

    Z.applyInfo = (json) => {
        Object.keys(json).forEach((k) => { Z.info[k] = json[k]; });
        Z.emit('info');
    };

    // The flat JSON the firmware's setConfig expects
    Z.configPayload = () => {
        Z.emit('readForm'); // let the view pull the current form values first
        const out = {};
        Z.FIELDS.forEach((f) => {
            if (f.kind === 'flag') return;
            if (f.kind === 'password' && !cfg[f.key]) return; // empty = keep
            out[f.key] = cfg[f.key];
        });
        cfg.alarms.forEach((alarm, i) => {
            const p = 'alarm' + (i + 1);
            out[p + 'Time'] = alarm.time;
            out[p + 'Active'] = alarm.active;
            out[p + 'Melody'] = alarm.melody;
            out[p + 'Weekdays'] = alarm.weekdays.join(',');
        });
        return out;
    };
})(window.ZIFRA);
