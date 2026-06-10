;
var ipAddress = $(location).attr('hostname');
var pageName = 'dash';
var timeleft;
var rebootTimer;
var json;
var boardURL = 'https://tehybug.com/zifra/v1/';
var connection = null;

// Weekday config keys; the matching global arrays are created by setalarm.php
var ALARM_WEEKDAY_KEYS = ['alarm1Weekdays', 'alarm2Weekdays', 'alarm3Weekdays'];

$(function () {
    // highlight the clicked menu button
    $('.nav-link').click(function () {
        $('.nav-link').removeClass('active');
        $(this).addClass('active');
    });

    ChangePage('main', '#page');

    setTimeout(function () {
        connectionStart()
    }, 1000);
});

function connectionStart() {
    if (connection != null && connection.readyState != WebSocket.CLOSED) {
        connection.close();
    }

    connection = new WebSocket('ws://' + ipAddress + ':81/' + pageName);
    connection.onopen = function () {
        $("#connectionStatus").html("Online");
        $("#connectionStatus").removeClass("text-danger");
        $("#connectionStatus").addClass("text-success");

        if (pageName == 'setConfig') {
            connection.send(json);
            connection.close();
        }

        KeepAlive();
    }
    connection.onclose = function (e) {
        // Debug
        console.log('WebSocket connection close');
        $("#connectionStatus").html("Offline");
        $("#connectionStatus").removeClass("text-success");
        $("#connectionStatus").addClass("text-danger");
    }
    connection.onerror = function (error) {
        // Debug
        console.log('WebSocket Error ' + error);
        if (connection.readyState !== WebSocket.CLOSED) {
            connection.close();
        }
    }
    connection.onmessage = function (e) {
        // Debug
        console.log('WebSocket incoming message: ' + e.data);
        RefreshData(e.data)
    }

    function KeepAlive() {
        var timeout = 1000;
        if (connection.readyState == WebSocket.OPEN) {
            connection.send("KeepAlive");
        }
        setTimeout(KeepAlive, timeout);
    }
}

// Mark the selected weekdays of one alarm in its weekLine widget
function ApplyWeekdays(key, csv) {
    if (typeof window[key] === 'undefined') {
        return;
    }
    var days = csv.split(',');
    for (var i = 0; i < days.length; i++) {
        if (days[i] * 1 == 1) {
            window[key][i] = 1;
            $("#" + key).children().eq(i).addClass("selectedDay");
        }
    }
}

function RefreshData(input) {
    // only handle Json
    if (!input.startsWith("{")) {
        return;
    }

    var json = $.parseJSON(input);
    // Log Json
    if (json.log) {
        var logArea = $('#log');
        logArea.append("[" + json.log.timeStamp + "] " + json.log.function + ": " + json.log.message + "\n");
        logArea.scrollTop(logArea[0].scrollHeight);
    } else {
        $.each(json, function (key, val) {
            // Config Json
            if (
                pageName == 'settings' ||
                pageName == 'settime' ||
                pageName == 'setalarm' ||
                pageName == 'setsystem'
            ) {
                if (typeof val === 'boolean') {
                    $("#" + key).prop('checked', val);
                } else {
                    $("#" + key).val(val.toString());
                }
            }
            // SystemInfo Json
            if (pageName == 'main') {
                $("#" + key).html(val.toString());
            }

            if (ALARM_WEEKDAY_KEYS.indexOf(key) !== -1) {
                ApplyWeekdays(key, val);
            }
        });
    }
}

function SaveConfig() {
    var obj = {};
    // read all inputs
    $("input").each(function () {
        // Debug
        console.log('SaveConfig -> ID: ' + this.id + ', Val: ' + (this.type == 'checkbox' ? $(this)
            .prop('checked') : $(this).val()));

        if (this.type == 'checkbox') {
            obj[this.id] = $(this).prop('checked');
        } else {
            obj[this.id] = $(this).val();
        }
    });

    // read all selects
    $("select").each(function () {
        // Debug
        console.log('SaveConfig -> ID: ' + this.id + ', Val: ' + $(this).val());
        obj[this.id] = $(this).val();
    });

    // get alarm weekdays
    ALARM_WEEKDAY_KEYS.forEach(function (key) {
        if (typeof window[key] !== 'undefined') {
            obj[key] = window[key].join(',');
        }
    });

    json = JSON.stringify(obj);
    // Debug
    console.log(json);

    pageName = "setConfig";

    connectionStart();

    // Restart countdown etc.
    var timeout = 12000;
    StartCountDown(timeout / 1000);

    setTimeout(function () {
        $("#popup").modal('hide');
    }, timeout);

    setTimeout(function () {
        location.reload();
    }, timeout + 500);
}

function ChangePage(_pageName, destination) {
    pageName = _pageName;
    $(destination).load(boardURL + 'html/' + _pageName + '.php');
}

function ChangeContent(element, _pageName, destination) {
    pageName = _pageName;
    $(destination).load(boardURL + 'html/' + _pageName + '.php');
    $('.nav-link').removeClass('active');
    $(element).addClass('active');
}

function ChangeContentIframe(element, _pageName, destination) {
    pageName = _pageName;
    $(destination).html('<iframe src="/' + pageName + '" title="" width="100%" height="100%"></iframe>');
    $('.nav-link').removeClass('active');
    $(element).addClass('active');
}

// Countdown
function StartCountDown(_timeleft) {
    timeleft = _timeleft;
    rebootTimer = setInterval(function () {
        timeleft--;
        $("#countdowntimer").html(timeleft);
        if (timeleft <= 0)
            clearInterval(rebootTimer);
    }, 1000);
}

function SetBrowserOffset() {
    let seconds = -(new Date().getTimezoneOffset() * 60);
    $("#utcOffsetInSeconds").val(seconds);
    $('#popup').modal('show')

    SaveConfig();
}
