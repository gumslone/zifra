<?php
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Origin");
header('P3P: CP="CAO PSA OUR"'); // Makes IE to support cookies
header("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");

?>
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


<div class="col-md-12 text-center">
    <hr>
</div>
<div class="col-md-12 text-center">
    <button type="button" class="btn btn-outline-success"  onclick="SaveConfig()" data-bs-toggle="modal" data-bs-target="#popup"><span data-feather="save"></span> Save
        Config</button>
</div>

<div class="modal fade" id="popup">
    <div class="modal-dialog modal-dialog-centered modal-lg">
        <div class="modal-content">
            <div class="modal-header bg-light">
                <h3 class="modal-title text-success">Config saved!</h3>
            </div>
            <div class="modal-body">
                <h5>System will be restarted, please wait <span id="countdowntimer">12 </span> seconds to reload!
                </h5>
            </div>
        </div>
    </div>
</div>


<script>
    feather.replace();
    connectionStart();  
    
    $(function () {

        $("#am2320_sensor").change(function() {
            if(this.checked) {
                //Do stuff
                $('#ds18b20_sensor').prop('checked', false);
                $('#dht_sensor').prop('checked', false);
            }
        });
        $("#dht_sensor").change(function() {
            if(this.checked) {
                //Do stuff
                $('#am2320_sensor').prop('checked', false);
                $('#ds18b20_sensor').prop('checked', false);
            }
        });
        $("#ds18b20_sensor").change(function() {
            if(this.checked) {
                //Do stuff
                $('#dht_sensor').prop('checked', false);
                $('#dht_sensor').prop('checked', false);
                $('#am2320_sensor').prop('checked', false);
            }
        });
        
        
    });
</script>