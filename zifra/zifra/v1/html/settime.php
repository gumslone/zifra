<?php require __DIR__ . '/_cors.php'; ?>
<div class="col-md-10 row">
    <div class="col-md-3">
        <h2 class="text-center">Time</h2>
        <hr>
        <div class="form-group">
            <label for="ntpServer">NTP Server</label>
            <input type="text" class="form-control" id="ntpServer">
        </div>
        <div class="form-group">
            <label for="utcOffsetInSeconds">UTC Offset in seconds</label>
            <input type="number" class="form-control" id="utcOffsetInSeconds">
        </div>
        <br>
        <button type="button" class="btn btn-outline-success" onclick="SetBrowserOffset()">
        set offset based on browser time</button>
    </div>

    <div class="offset-md-1 col-md-3">
        <h2 class="text-center">Clock</h2>
        <hr>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="clock_12h">
            <label class="form-check-label" for="clock_12h">12h Clock</label>
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="clock_leading_hour_zero">
            <label class="form-check-label" for="clock_leading_hour_zero">Show leading zero for hours</label>
        </div>
    </div>

    <div class="offset-md-1 col-md-3">
        <h2 class="text-center">Nixie Sleep</h2>
        <hr>
        <div class="form-group">
            <label for="clock_sleep_start">Begin</label>
            <input type="time" class="form-control" id="clock_sleep_start">
        </div>
        <div class="form-group">
            <label for="clock_sleep_finish">End</label>
            <input type="time" class="form-control" id="clock_sleep_finish">
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="clock_sleep">
            <label class="form-check-label" for="clock_sleep">Sleep active?</label>
        </div>
    </div>
</div>

<?php require __DIR__ . '/_save_config.php'; ?>

<script>
    feather.replace();
    connectionStart();
</script>
