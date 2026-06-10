<?php
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, GET, OPTIONS");
header("Access-Control-Allow-Headers: Origin");
header('P3P: CP="CAO PSA OUR"'); // Makes IE to support cookies
header("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");

?>
<script type="text/javascript">
    var alarm1Weekdays = [0, 0, 0, 0, 0, 0, 0];
    var alarm2Weekdays = [0, 0, 0, 0, 0, 0, 0];
    var alarm3Weekdays = [0, 0, 0, 0, 0, 0, 0];
    function selected(el)
    {
        let weekDays = [0, 0, 0, 0, 0, 0, 0];
        let weekdaysSelected = ($(el).weekLine('getSelected', 'indexes'));
        for(let i=0; i<weekdaysSelected.length; i++)
        {
            weekDays[weekdaysSelected[i]] = 1;
        }
        return weekDays;
    }
    $(function () {
        let labels = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
        // Return selected days as indexes
        let weekcal1 = $("#alarm1Weekdays").weekLine({
            mousedownSel: false,
            dayLabels:labels,
                onChange: function () {
                    alarm1Weekdays = selected(this);
                }
        });
        let weekcal2 = $("#alarm2Weekdays").weekLine({
            mousedownSel: false,
            dayLabels: labels,
                onChange: function () {
                    alarm2Weekdays = selected(this);
                }
        });
        let weekcal3 = $("#alarm3Weekdays").weekLine({
            mousedownSel: false,
            dayLabels: labels,
                onChange: function () {
                    alarm3Weekdays = selected(this);
                }
        });
    });
</script>
<div class="col-md-10 row">
    
    <div class="offset-md-1 col-md-3">
        
        <h2 class="text-center">Alarm 1</h2>
        <hr>
        <div class="form-group">
            <label for="alarm1Time">Alarm1 Time</label>
            <input type="time" class="form-control" id="alarm1Time">
        </div>
        <div class="form-group">
            <div id="alarm1Weekdays" style="margin: 15px 0;"></div>
        </div>
        <div class="form-check form-switch"  style="margin: 15px 0;">
            <input type="checkbox" class="form-check-input" id="alarm1Active">
            <label class="form-check-label" for="alarm1Active">Alarm1 active</label>
        </div>
        

    </div>
    <div class="offset-md-1 col-md-3">
        
        <h2 class="text-center">Alarm 2</h2>
        <hr>
        <div class="form-group">
            <label for="alarm2Time">Alarm2 Time</label>
            <input type="time" class="form-control" id="alarm2Time">
        </div>
        <div class="form-group">
            <div id="alarm2Weekdays" style="margin: 15px 0;"></div>
        </div>
        <div class="form-check form-switch"  style="margin: 15px 0;">
            <input type="checkbox" class="form-check-input" id="alarm2Active">
            <label class="form-check-label" for="alarm2Active">Alarm2 active</label>
        </div>
        

    </div>
    <div class="offset-md-1 col-md-3">
        
        <h2 class="text-center">Alarm 3</h2>
        <hr>
        
        <div class="form-group">
            <label for="alarm3Time">Alarm3 Time</label>
            <input type="time" class="form-control" id="alarm3Time">
        </div>
        <div class="form-group">
            <div id="alarm3Weekdays" style="margin: 15px 0;"></div>
        </div>
        <div class="form-check form-switch"  style="margin: 15px 0;">
            <input type="checkbox" class="form-check-input" id="alarm3Active">
            <label class="form-check-label" for="alarm3Active">Alarm3 active</label>
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