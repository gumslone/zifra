;
var ipAddress = $(location).attr('hostname');
var pageName = 'dash';
var devMode = false;
var timeleft;
var rebootTimer;
var json;
var boardURL = 'https://tehybug.com/zifra/v1/';


if (ipAddress.includes('localhost')) {
    devMode = true;
}

$(function () {
    // Akive Menu Button select 
    $('.nav-link').click(function () {
        $('.nav-link').removeClass('active');
        $(this).addClass('active');
    })
    //ChangePage("dash");
    //$('#page').html('works');
    ChangePage('main', '#page');

    var script = document.createElement('script');
    script.src = 'https://cdn.jsdelivr.net/npm/feather-icons@4.28.0/dist/feather.min.js';
    script.type = 'text/javascript';
    document.getElementsByTagName('head')[0].appendChild(script);
    setTimeout(function () {
        connectionStart()
    }, 1000);
});

var connection = null;

function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}

function connectionStart() {
    if (connection != null && connection.readyState != WebSocket.CLOSED) {
        connection.close();
    }
    var wsServer = ipAddress;

    connection = new WebSocket('ws://' + wsServer + ':81/' + pageName);
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
        console.log('WebSocket incomming message: ' + e.data);
        RefershData(e.data)
    }

    function KeepAlive() {
        var timeout = 1000;
        if (connection.readyState == WebSocket.OPEN) {
            connection.send("KeepAlive");
        }
        setTimeout(KeepAlive, timeout);
    }
}

function RefershData(input) {
    // Prüfen ob es ein Json ist
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

            if (key == "alarm1Weekdays" && typeof alarm1Weekdays !== 'undefined') {
                let alarm1Weekdays_arr = val.split(',');
                for (let i = 0; i < alarm1Weekdays_arr.length; i++) {
                    if (alarm1Weekdays_arr[i] * 1 == 1) {
                        alarm1Weekdays[i] = 1;
                        $("#alarm1Weekdays").children().eq(i).addClass("selectedDay");
                    }
                }
            }
            if (key == "alarm2Weekdays" && typeof alarm2Weekdays !== 'undefined') {
                let alarm2Weekdays_arr = val.split(',');
                for (let i = 0; i < alarm2Weekdays_arr.length; i++) {
                    if (alarm2Weekdays_arr[i] * 1 == 1) {
                        alarm2Weekdays[i] = 1;
                        $("#alarm2Weekdays").children().eq(i).addClass("selectedDay");
                    }
                }
            }
            if (key == "alarm3Weekdays" && typeof alarm3Weekdays !== 'undefined') {
                let alarm3Weekdays_arr = val.split(',');
                for (let i = 0; i < alarm3Weekdays_arr.length; i++) {
                    if (alarm3Weekdays_arr[i] * 1 == 1) {
                        alarm3Weekdays[i] = 1;
                        $("#alarm3Weekdays").children().eq(i).addClass("selectedDay");
                    }
                }
            }
        });
    }
}

function SaveConfig() {
    var obj = {};
    // read all imputs
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
    if (typeof alarm1Weekdays !== 'undefined')
        obj["alarm1Weekdays"] = (alarm1Weekdays.join(','));
    if (typeof alarm2Weekdays !== 'undefined')
        obj["alarm2Weekdays"] = (alarm2Weekdays.join(','));
    if (typeof alarm3Weekdays !== 'undefined')
        obj["alarm3Weekdays"] = (alarm3Weekdays.join(','));


    json = JSON.stringify(obj);
    // Debug
    console.log(json);

    pageName = "setConfig";

    connectionStart();

    // Restart Countdown usw.
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

function isNullOrWhitespace(input) {

    if (typeof input === 'undefined' || input == null) {
        return true;
    }
    return input.replace(/\s/g, '').length < 1;
}

function SetBrowserOffset()
{
    let seconds = -(new Date().getTimezoneOffset() * 60);
    $("#utcOffsetInSeconds").val(seconds);
    $('#popup').modal('show')

    SaveConfig();
}