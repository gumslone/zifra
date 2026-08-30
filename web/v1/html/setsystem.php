<?php require __DIR__ . '/_cors.php'; ?>
<div class="col-md-10 row">
    <div class="offset-md-1 col-md-5">
        <h2 class="text-center">System</h2>
        <hr>
        <div class="form-group">
            <label for="key">TeHyBug Key</label>
            <input type="text" class="form-control" id="key" minlength="7" maxlength="36">
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="am2320_sensor">
            <label class="form-check-label" for="am2320_sensor">AM2320 Active</label>
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="dht_sensor">
            <label class="form-check-label" for="dht_sensor">DHTXX Active</label>
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="ds18b20_sensor">
            <label class="form-check-label" for="ds18b20_sensor">DS18B20 Active</label>
        </div>
    </div>
</div>

<?php require __DIR__ . '/_save_config.php'; ?>

<script>
    feather.replace();
    connectionStart();

    // Only one sensor type can be active at a time
    $(function () {
        var sensorIds = ['#am2320_sensor', '#dht_sensor', '#ds18b20_sensor'];
        sensorIds.forEach(function (id) {
            $(id).change(function () {
                if (this.checked) {
                    sensorIds.forEach(function (other) {
                        if (other !== id) {
                            $(other).prop('checked', false);
                        }
                    });
                }
            });
        });
    });
</script>
