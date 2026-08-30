<?php require __DIR__ . '/_cors.php'; ?>
<div class="row pt-3">
    <div class="col-md-4">
        <div class="panel">
            <h2>Time source</h2>
            <hr>
            <p class="help-text">
                With WiFi on, the clock fetches the time from an NTP server on the
                internet. With WiFi off, the on-board DS3231 real-time clock keeps the
                time instead.
            </p>
            <div class="form-group">
                <label for="ntpServer">NTP server</label>
                <input type="text" class="form-control" id="ntpServer">
                <div class="help-text">Hostname of the time server. The default
                    <code>pool.ntp.org</code> works worldwide.</div>
            </div>
            <div class="form-group">
                <label for="utcOffsetInSeconds">UTC offset in seconds</label>
                <input type="number" class="form-control" id="utcOffsetInSeconds">
                <div class="help-text">Your time zone as an offset from UTC, e.g.
                    <code>3600</code> for UTC+1 or <code>-18000</code> for UTC&minus;5.
                    There is no automatic daylight-saving switch &mdash; adjust the offset
                    twice a year, or simply press the button below.</div>
            </div>
            <button type="button" class="btn btn-nixie" onclick="SetBrowserOffset()">
            Set offset from browser time</button>
        </div>
    </div>

    <div class="col-md-4">
        <div class="panel">
            <h2>Display</h2>
            <hr>
            <p class="help-text">
                How the single tube spells out the time.
            </p>
            <div class="form-check form-switch">
                <input type="checkbox" class="form-check-input" id="clock_12h">
                <label class="form-check-label" for="clock_12h">12h clock</label>
            </div>
            <div class="help-text">
                Show hours as 1&ndash;12 instead of 0&ndash;23.
            </div>
            <div class="form-check form-switch" style="margin-top: 15px;">
                <input type="checkbox" class="form-check-input" id="clock_leading_hour_zero">
                <label class="form-check-label" for="clock_leading_hour_zero">Show leading zero for hours</label>
            </div>
            <div class="help-text">
                When on, 9:41 is shown as <b>0</b>&nbsp;<b>9</b>&nbsp;<b>4.</b>&nbsp;<b>1.</b> &mdash;
                when off, the meaningless leading zero is skipped and the cycle starts
                directly with the <b>9</b>, so you get the time one digit sooner.
            </div>
        </div>
    </div>

    <div class="col-md-4">
        <div class="panel">
            <h2>Night sleep</h2>
            <hr>
            <p class="help-text">
                Switches the tube off during a daily quiet window &mdash; typically at
                night. Nixie tubes have a finite lifetime, so sleeping 8 hours a day
                makes the IN-12B last noticeably longer. Press the top button to wake
                the display for 3 minutes while it sleeps.
            </p>
            <div class="form-group">
                <label for="clock_sleep_start">Begin</label>
                <input type="time" class="form-control" id="clock_sleep_start">
            </div>
            <div class="form-group">
                <label for="clock_sleep_finish">End</label>
                <input type="time" class="form-control" id="clock_sleep_finish">
                <div class="help-text">The window may span midnight, e.g.
                    22:00&nbsp;&rarr;&nbsp;06:00.</div>
            </div>
            <div class="form-check form-switch">
                <input type="checkbox" class="form-check-input" id="clock_sleep">
                <label class="form-check-label" for="clock_sleep">Sleep active</label>
            </div>
        </div>
    </div>
</div>

<?php require __DIR__ . '/_save_config.php'; ?>

<script>
    feather.replace();
    connectionStart();
</script>
