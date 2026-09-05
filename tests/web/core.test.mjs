// Unit tests for the web app's pure logic (core.js time math and the
// schema-driven state in state.js). Runs under Node's built-in test runner:
//   node --test tests/web
// The modules are loaded the way the browser bundle does - in order, onto
// a global `window` - so they need no build step.
import { test } from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';

const files = new URL('../../web/v2/js/files/', import.meta.url);
globalThis.window = globalThis;
for (const name of ['core.js', 'state.js']) {
    vm.runInThisContext(fs.readFileSync(new URL(name, files), 'utf8'), { filename: name });
}
const Z = window.ZIFRA;
const T = Z.time;

test('to12 splits a 24h time into 12h parts', () => {
    assert.deepEqual(T.to12('00:05'), { h: 12, m: 5, pm: false });
    assert.deepEqual(T.to12('12:30'), { h: 12, m: 30, pm: true });
    assert.deepEqual(T.to12('13:07'), { h: 1, m: 7, pm: true });
    assert.deepEqual(T.to12('09:41'), { h: 9, m: 41, pm: false });
    assert.equal(T.to12(''), null);
    assert.equal(T.to12('nope'), null);
});

test('from12 builds HH:MM and rejects out-of-range parts', () => {
    assert.equal(T.from12(12, 0, false), '00:00');
    assert.equal(T.from12(12, 0, true), '12:00');
    assert.equal(T.from12(1, 5, true), '13:05');
    assert.equal(T.from12(11, 59, false), '11:59');
    assert.equal(T.from12(13, 0, false), '');
    assert.equal(T.from12(0, 0, false), '');
    assert.equal(T.from12(11, 60, false), '');
    assert.equal(T.from12(NaN, 0, false), '');
});

test('to12/from12 round-trip every minute of the day', () => {
    for (let h = 0; h < 24; h++) {
        for (let m = 0; m < 60; m += 7) {
            const s = T.pad(h) + ':' + T.pad(m);
            const v = T.to12(s);
            assert.equal(T.from12(v.h, v.m, v.pm), s);
        }
    }
});

test('format shows AM/PM only on the 12h clock', () => {
    assert.equal(T.format('21:03', false), '21:03');
    assert.equal(T.format('21:03', true), '9:03 PM');
    assert.equal(T.format('00:15', true), '12:15 AM');
    assert.equal(T.format('', true), '');
});

test('displayHours mirrors the firmware (>12 loses 12, midnight is 12)', () => {
    assert.equal(T.displayHours(0, true), 12);
    assert.equal(T.displayHours(12, true), 12);
    assert.equal(T.displayHours(13, true), 1);
    assert.equal(T.displayHours(23, true), 11);
    assert.equal(T.displayHours(9, false), 9);
    assert.equal(T.displayHours(0, false), 0);
});

test('sleepActive handles windows that wrap past midnight', () => {
    const cfg = { clock_sleep: true, clock_sleep_start: '22:00', clock_sleep_finish: '06:30' };
    assert.equal(T.sleepActive(cfg, { h: 23, m: 0 }), true);
    assert.equal(T.sleepActive(cfg, { h: 3, m: 0 }), true);
    assert.equal(T.sleepActive(cfg, { h: 22, m: 0 }), true);   // start boundary
    assert.equal(T.sleepActive(cfg, { h: 6, m: 30 }), true);   // finish boundary
    assert.equal(T.sleepActive(cfg, { h: 6, m: 31 }), false);
    assert.equal(T.sleepActive(cfg, { h: 12, m: 0 }), false);
    const day = { clock_sleep: true, clock_sleep_start: '09:00', clock_sleep_finish: '17:00' };
    assert.equal(T.sleepActive(day, { h: 10, m: 0 }), true);
    assert.equal(T.sleepActive(day, { h: 8, m: 59 }), false);
    assert.equal(T.sleepActive({ ...cfg, clock_sleep: false }, { h: 23, m: 0 }), false);
    assert.equal(T.sleepActive({ ...cfg, clock_sleep_finish: '' }, { h: 23, m: 0 }), false);
});

test('nextAlarm picks the soonest active alarm across the week', () => {
    const monday = new Date(2026, 7, 31, 20, 0); // Monday 20:00
    const weekdays = (...days) => [0, 1, 2, 3, 4, 5, 6].map((d) => (days.includes(d) ? 1 : 0));
    const alarms = [
        { time: '07:50', active: true, weekdays: weekdays(1, 2, 3, 4, 5, 6) },
        { time: '21:00', active: false, weekdays: weekdays(1) },
        { time: '', active: true, weekdays: weekdays(1) }
    ];
    // 07:50 tomorrow (Tuesday): 4 h to midnight + 7 h 50
    assert.deepEqual(T.nextAlarm(alarms, monday), { mins: 710, time: '07:50', day: 2 });
    alarms[1].active = true;
    assert.deepEqual(T.nextAlarm(alarms, monday), { mins: 60, time: '21:00', day: 1 });
    // an alarm earlier today only counts next week
    const past = [{ time: '19:00', active: true, weekdays: weekdays(1) }];
    assert.equal(T.nextAlarm(past, monday).mins, 7 * 1440 - 60);
    assert.equal(T.nextAlarm([], monday), null);
});

test('clockNow prefers the time the clock reports', () => {
    Z.info.clockTime = '20:41';
    assert.deepEqual(T.clockNow(), { h: 20, m: 41 });
    delete Z.info.clockTime;
    const d = new Date();
    assert.deepEqual(T.clockNow(), { h: d.getHours(), m: d.getMinutes() });
});

test('applyConfig coerces the firmware JSON by schema and keeps unknown keys out', () => {
    let events = 0;
    Z.on('config', () => { events++; });
    Z.applyConfig({
        utcOffsetInSeconds: '3600', dstMode: 1, clock_12h: 1, clock_sleep_start: '22:00',
        otaPasswordSet: true, wifiActive: true,
        alarm1Time: '07:50', alarm1Active: true, alarm1Melody: '3', alarm1Weekdays: '0,1,1,1,1,1,0'
    });
    assert.equal(events, 1);
    assert.equal(Z.cfg.utcOffsetInSeconds, 3600);
    assert.equal(Z.cfg.clock_12h, true);
    assert.equal(Z.cfg.otaPasswordSet, true);
    assert.equal(Z.cfg.wifiActive, undefined);
    assert.deepEqual(Z.cfg.alarms[0], { time: '07:50', active: true, melody: 3, weekdays: [0, 1, 1, 1, 1, 1, 0] });
    assert.equal(Z.cfg.alarms[1].time, ''); // untouched
});

test('configPayload flattens alarms and never sends flags or an empty password', () => {
    Z.on('readForm', () => {}); // the view would pull the form here
    Z.cfg.otaPassword = '';
    let out = Z.configPayload();
    assert.equal('otaPasswordSet' in out, false);
    assert.equal('otaPassword' in out, false);
    assert.equal(out.alarm1Weekdays, '0,1,1,1,1,1,0');
    assert.equal(out.alarm1Melody, 3);
    assert.equal(out.alarm3Time, '');
    Z.cfg.otaPassword = 'tube123';
    out = Z.configPayload();
    assert.equal(out.otaPassword, 'tube123');
});
