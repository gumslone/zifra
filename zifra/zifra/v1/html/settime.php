<?php
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Origin");
header('P3P: CP="CAO PSA OUR"'); // Makes IE to support cookies
header("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");

?>
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
        <button type="button" class="btn btn-outline-success"  onclick="SetBrowserOffset()">
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
</script>