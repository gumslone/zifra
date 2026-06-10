<?php require __DIR__ . '/_cors.php'; ?>
<script type="text/javascript">
    function selected(el)
    {
        let weekDays = [0, 0, 0, 0, 0, 0, 0];
        let weekdaysSelected = ($(el).weekLine('getSelected', 'indexes'));
        for (let i = 0; i < weekdaysSelected.length; i++)
        {
            weekDays[weekdaysSelected[i]] = 1;
        }
        return weekDays;
    }
    $(function () {
        let labels = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
        ALARM_WEEKDAY_KEYS.forEach(function (name) {
            window[name] = [0, 0, 0, 0, 0, 0, 0];
            $("#" + name).weekLine({
                mousedownSel: false,
                dayLabels: labels,
                onChange: function () {
                    window[name] = selected(this);
                }
            });
        });
    });
</script>
<div class="row pt-3">
    <div class="col-md-12">
        <div class="panel">
            <h2>Alarms</h2>
            <hr>
            <p class="help-text mb-0">
                The clock has three independent alarms. Each one rings at its set time on
                the selected weekdays &mdash; the buzzer plays an alternating two-tone
                signal for up to <b>10 minutes</b>. Press the front or top button on the
                clock to mute it. An alarm only fires when its switch is on and at least
                one weekday is selected.
            </p>
        </div>
    </div>
</div>
<div class="row">
<?php for ($i = 1; $i <= 3; $i++): ?>
    <div class="col-md-4">
        <div class="panel">
            <h2 class="text-center">Alarm <?php echo $i; ?></h2>
            <hr>
            <div class="form-group">
                <label for="alarm<?php echo $i; ?>Time">Time</label>
                <input type="time" class="form-control" id="alarm<?php echo $i; ?>Time">
            </div>
            <div class="form-group">
                <label>Weekdays</label>
                <div id="alarm<?php echo $i; ?>Weekdays" style="margin: 5px 0 15px;"></div>
                <div class="help-text">Click the days this alarm should ring on.</div>
            </div>
            <div class="form-check form-switch" style="margin: 15px 0 0;">
                <input type="checkbox" class="form-check-input" id="alarm<?php echo $i; ?>Active">
                <label class="form-check-label" for="alarm<?php echo $i; ?>Active">Alarm <?php echo $i; ?> active</label>
            </div>
        </div>
    </div>
<?php endfor; ?>
</div>

<?php require __DIR__ . '/_save_config.php'; ?>

<script>
    feather.replace();
    connectionStart();
</script>
