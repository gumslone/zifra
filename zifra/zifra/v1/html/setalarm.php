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
<div class="col-md-10 row">
<?php for ($i = 1; $i <= 3; $i++): ?>
    <div class="offset-md-1 col-md-3">
        <h2 class="text-center">Alarm <?php echo $i; ?></h2>
        <hr>
        <div class="form-group">
            <label for="alarm<?php echo $i; ?>Time">Alarm<?php echo $i; ?> Time</label>
            <input type="time" class="form-control" id="alarm<?php echo $i; ?>Time">
        </div>
        <div class="form-group">
            <div id="alarm<?php echo $i; ?>Weekdays" style="margin: 15px 0;"></div>
        </div>
        <div class="form-check form-switch" style="margin: 15px 0;">
            <input type="checkbox" class="form-check-input" id="alarm<?php echo $i; ?>Active">
            <label class="form-check-label" for="alarm<?php echo $i; ?>Active">Alarm<?php echo $i; ?> active</label>
        </div>
    </div>
<?php endfor; ?>
</div>

<?php require __DIR__ . '/_save_config.php'; ?>

<script>
    feather.replace();
    connectionStart();
</script>
