<div class="col-md-12 text-center">
    <hr>
</div>
<div class="col-md-12 text-center">
    <button type="button" class="btn btn-nixie" onclick="SaveConfig()" data-bs-toggle="modal" data-bs-target="#popup"><span data-feather="save"></span> Save
        Config</button>
    <div class="help-text">Saving restarts the clock; it is back online after a few seconds.</div>
</div>

<div class="modal fade" id="popup">
    <div class="modal-dialog modal-dialog-centered modal-lg">
        <div class="modal-content">
            <div class="modal-header">
                <h3 class="modal-title">Config saved!</h3>
            </div>
            <div class="modal-body">
                <h5>The clock is restarting &mdash; this page reloads in <span id="countdowntimer">12</span> seconds.
                </h5>
            </div>
        </div>
    </div>
</div>
