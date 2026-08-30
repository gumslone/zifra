<?php require __DIR__ . '/_cors.php'; ?>
<div class="col-md-10 row">
    <div class="offset-md-1 col-md-3">
        <h2 class="text-center">MQTT</h2>
        <hr>
        <div class="form-group">
            <label for="mqttServer">Server</label>
            <input type="text" class="form-control" id="mqttServer" minlength="7" maxlength="15" pattern="^((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$">
        </div>
        <div class="form-group">
            <label for="mqttPort">Port</label>
            <input type="number" class="form-control" id="mqttPort">
        </div>
        <div class="form-group">
            <label for="mqttUser">User</label>
            <input type="text" class="form-control" id="mqttUser" placeholder="Optional" autocomplete="off">
        </div>
        <div class="form-group">
            <label for="mqttPassword">Password</label>
            <input type="password" class="form-control" id="mqttPassword" placeholder="Optional" autocomplete="off">
        </div>
        <div class="form-group">
            <label for="mqttMasterTopic">Topic</label>
            <input type="text" class="form-control" id="mqttMasterTopic">
        </div>
        <div class="form-group">
            <label for="mqttMessage">Message</label>
            <input type="text" class="form-control" id="mqttMessage">
        </div>
        <div class="form-group">
            <label for="mqttFrequency">Data Frequency</label>
            <input type="number" class="form-control" id="mqttFrequency">
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="mqttRetained">
            <label class="form-check-label" for="mqttRetained">MQTT retained</label>
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="mqttActive">
            <label class="form-check-label" for="mqttActive">MQTT active</label>
        </div>
    </div>
    <div class="offset-md-1 col-md-3">
        <h2 class="text-center">HTTP</h2>
        <hr>
        <div class="form-group">
            <label for="httpGetURL">HTTP Get URL</label>
            <input type="url" class="form-control" id="httpGetURL" minlength="7" placeholder="https://example.com"
  pattern="[Hh][Tt][Tt][Pp][Ss]?:\/\/(?:(?:[a-zA-Z\u00a1-\uffff0-9]+-?)*[a-zA-Z\u00a1-\uffff0-9]+)(?:\.(?:[a-zA-Z\u00a1-\uffff0-9]+-?)*[a-zA-Z\u00a1-\uffff0-9]+)*(?:\.(?:[a-zA-Z\u00a1-\uffff]{2,}))(?::\d{2,5})?(?:\/[^\s]*)?">
        </div>
        <div class="form-group">
            <label for="httpGetFrequency">Data Frequency (seconds)</label>
            <input type="number" class="form-control" id="httpGetFrequency">
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="httpGetActive">
            <label class="form-check-label" for="httpGetActive">HTTP active</label>
        </div>
    </div>
    <div class="offset-md-1 col-md-3">
        <h2 class="text-center">HTTP</h2>
        <hr>
        <div class="form-group">
            <label for="httpPostURL">HTTP Post URL</label>
            <input type="url" class="form-control" id="httpPostURL" minlength="7" placeholder="https://example.com"
  pattern="[Hh][Tt][Tt][Pp][Ss]?:\/\/(?:(?:[a-zA-Z\u00a1-\uffff0-9]+-?)*[a-zA-Z\u00a1-\uffff0-9]+)(?:\.(?:[a-zA-Z\u00a1-\uffff0-9]+-?)*[a-zA-Z\u00a1-\uffff0-9]+)*(?:\.(?:[a-zA-Z\u00a1-\uffff]{2,}))(?::\d{2,5})?(?:\/[^\s]*)?">
        </div>
        <div class="form-group">
            <label for="httpPostFrequency">Data Frequency (seconds)</label>
            <input type="number" class="form-control" id="httpPostFrequency">
        </div>
        <div class="form-group">
            <label for="httpPostJson">Post Json</label>
            <input type="text" class="form-control" id="httpPostJson">
            <a href="javascript:void(0);" data-bs-toggle="modal" data-bs-target="#placeholders">Placeholders</a>
        </div>
        <div class="form-check form-switch">
            <input type="checkbox" class="form-check-input" id="httpPostActive">
            <label class="form-check-label" for="httpPostActive">HTTP active</label>
        </div>
    </div>
</div>


<?php require __DIR__ . '/_save_config.php'; ?>


<div class="modal fade" id="placeholders">
    <div class="modal-dialog modal-dialog-centered modal-lg">
        <div class="modal-content">
            <div class="modal-header bg-light">
                <h3 class="modal-title text-success">Config saved!</h3>
            </div>
            <div class="modal-body">
                        <div>
                            <b>TeHyBug Key</b><br/>
                            <code>%key%</code>
                            Unique identifier for your TeHyBug.
                        </div>
                        <div style="margin-top: 15px;">
                            <b>Temperature</b><br/>
                            <code>%temp%</code>
                            For temperature in &deg;Celsius.<br/>
                            <code>%temp_imp%</code>
                            For temperature in &deg;Fahrenheit.
                        </div>
                        <div style="margin-top: 15px;">
                            <b>Humidity</b><br/>
                            <code>%humi%</code>
                            For humidity in %.<br/>
                            Depends on sensor type in your TeHyBug.
                        </div>
                        <div style="margin-top: 15px;">
                            <b>Dew point</b><br/>
                            <code>%dew%</code>
                            For dew point in &deg;Celsius.<br/>
                            <code>%dew_imp%</code>
                            For dew point in &deg;Fahrenheit.<br/>
                            Works only with temperature and humidity TeHyBugs.
                        </div>
                        <div style="margin-top: 15px;">
                            <b>Barometric air pressure</b><br/>
                            <code>%qfe%</code>
                            For air pressure in hPa.<br/>
                            <code>%qfe_imp%</code>
                            For air pressure in inHg.<br/>
                            <code>%qnh%</code>
                            For air pressure in hPa.<br/>
                            <code>%qnh_imp%</code>
                            For air pressure in inHg.<br/>
                            Depends on sensor type in your TeHyBug.
                        </div>
                        <div style="margin-top: 15px;">
                            <b>Air quality</b><br/>
                            <code>%air%</code>
                            For air pressure in kOhms.<br/>
                            Depends on sensor type in your TeHyBug.
                        </div>
                        <div style="margin-top: 15px;">
                            <b>UV Light</b><br/>
                            <code>%uv%</code>
                            For UV light.<br/>
                            Depends on sensor type in your TeHyBug.
                        </div>
                        <div style="margin-top: 15px;">
                            <b>Ambient Light luminance</b><br/>
                            <code>%lux%</code>
                            For Ambient Light light.<br/>
                            Depends on sensor type in your TeHyBug.
                        </div>
                        
                        <div style="margin-top: 15px;">
                            <b>Chip id</b><br/>
                            <code>%chipid%</code>
                            esp8266 chip id.
                        </div>
                        
                        <div style="margin-top: 15px;">
                            <b>System voltage</b><br/>
                            <code>%vdd%</code>
                            system voltage in mV.
                        </div>
                        
                        
                        
                        <div style="margin-top: 15px;">
                            <b>HTTP url Examples:</b><br/>
                            <code>http://tehybug.com/track/?t=%temp%&h=%humi%&p=%qfe%&a=%air%&u=%uv%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a BME680 and VEML6070 sensor.<br/><br/>
                            <code>http://tehybug.com/track/?t=%temp%&h=%humi%&p=%qfe%&u=%uv%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a BME280 and VEML6070 sensor.<br/><br/>
                            <code>http://tehybug.com/track/?t=%temp%&h=%humi%&p=%qfe%&a=%air%&l=%lux%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a BME680 and MAX44009 sensor.<br/><br/>
                            <code>http://tehybug.com/track/?t=%temp%&h=%humi%&p=%qfe%&l=%lux%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a BME280 and MAX44009 sensor.<br/><br/>
                            <code>http://tehybug.com/track/?t=%temp%&h=%humi%&p=%qfe%&a=%air%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a BME680 sensor.<br/><br/>
                            <code>http://tehybug.com/track/?t=%temp%&h=%humi%&p=%qfe%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a BME280 sensor.<br/><br/>
                            <code>http://tehybug.com/track/?t=%temp%&p=%qfe%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a BMP280 sensor.<br/><br/>
                            <code>http://tehybug.com/track/?t=%temp%&h=%humi%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a DHT22 sensor.<br/><br/>
                            <code>http://tehybug.com/track/?t=%temp%&chipid=%chipid%&bug_key=%key%</code>
                            For TeHyBug with a DS18B20 sensor.
                        </div>
                        <div style="margin-top: 15px;">
                            <b>MQTT message or HTTP POST content json Examples:</b><br/>
                            <code>{"t":"%temp%","h":"%humi%","p":"%qfe%","a":"%air%","u":"%uv%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a BME680 and VEML6070 sensor.<br/><br/>
                            <code>{"t":"%temp%","h":"%humi%","p":"%qfe%","u":"%uv%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a BME280 and VEML6070 sensor.<br/><br/>
                            <code>{"t":"%temp%","h":"%humi%","p":"%qfe%","a":"%air%","l":"%lux%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a BME680 and MAX44009 sensor.<br/><br/>
                            <code>{"t":"%temp%","h":"%humi%","p":"%qfe%","l":"%lux%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a BME280 and MAX44009 sensor.<br/><br/>
                            <code>{"t":"%temp%","h":"%humi%","p":"%qfe%","a":"%air%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a BME680 sensor.<br/><br/>
                            <code>{"t":"%temp%","h":"%humi%","p":"%qfe%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a BME280 sensor.<br/><br/>
                            <code>{"t":"%temp%","p":"%qfe%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a BMP280 sensor.<br/><br/>
                            <code>{"t":"%temp%","h":"%humi%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a DHT22 sensor.<br/><br/>
                            <code>{"t":"%temp%","chip_id":"%chipid%","bug_key":"%key%"}</code>
                            For TeHyBug with a DS18B20 sensor.
                        </div>
                        
                        <div style="margin-top: 15px;">
                            <b>WEB output Examples:</b><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C&lt;br&gt;&lt;b&gt;Humidity:&lt;/b&gt; %humi% %&lt;br&gt;&lt;b&gt;Barometric air pressure:&lt;/b&gt; %qfe% hPa&lt;br&gt;&lt;b&gt;Air quality:&lt;/b&gt; %air% kOhms&lt;br&gt;&lt;b&gt;UV Light:&lt;/b&gt; %uv%</code>
                            For TeHyBug with a BME680 and VEML6070 sensor.<br/><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C&lt;br&gt;&lt;b&gt;Humidity:&lt;/b&gt; %humi% %&lt;br&gt;&lt;b&gt;Barometric air pressure:&lt;/b&gt; %qfe% hPa&lt;br&gt;&lt;b&gt;UV Light:&lt;/b&gt; %uv%</code>
                            For TeHyBug with a BME280 and VEML6070 sensor.<br/><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C&lt;br&gt;&lt;b&gt;Humidity:&lt;/b&gt; %humi% %&lt;br&gt;&lt;b&gt;Barometric air pressure:&lt;/b&gt; %qfe% hPa&lt;br&gt;&lt;b&gt;Air quality:&lt;/b&gt; %air% kOhms&lt;br&gt;&lt;b&gt;Ambient Light:&lt;/b&gt; %lux% Lux</code>
                            For TeHyBug with a BME680 and MAX44009 sensor.<br/><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C&lt;br&gt;&lt;b&gt;Humidity:&lt;/b&gt; %humi% %&lt;br&gt;&lt;b&gt;Barometric air pressure:&lt;/b&gt; %qfe% hPa&lt;br&gt;&lt;b&gt;Ambient Light:&lt;/b&gt; %lux% Lux</code>
                            For TeHyBug with a BME280 and MAX44009 sensor.<br/><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C&lt;br&gt;&lt;b&gt;Humidity:&lt;/b&gt; %humi% %&lt;br&gt;&lt;b&gt;Barometric air pressure:&lt;/b&gt; %qfe% hPa&lt;br&gt;&lt;b&gt;Air quality:&lt;/b&gt; %air% kOhms</code>
                            For TeHyBug with a BME680 sensor.<br/><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C&lt;br&gt;&lt;b&gt;Humidity:&lt;/b&gt; %humi% %&lt;br&gt;&lt;b&gt;Barometric air pressure:&lt;/b&gt; %qfe% hPa</code>
                            For TeHyBug with a BME280 sensor.<br/><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C&lt;br&gt;&lt;b&gt;Barometric air pressure:&lt;/b&gt; %qfe% hPa</code>
                            For TeHyBug with a BMP280 sensor.<br/><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C&lt;br&gt;&lt;b&gt;Humidity:&lt;/b&gt; %humi% %</code>
                            For TeHyBug with a DHT22 sensor.<br/><br/>
                            <code>&lt;b&gt;Temperature:&lt;/b&gt; %temp% &deg;C</code>
                            For TeHyBug with a DS18B20 sensor.
                            
                            <br/><br/><b>More placeholders for web output:</b><br/>
                            <code>%temp_values%</code>
                            For several measured temperature values in &deg;Celsius. (newest value first).<br/><br/>
                            
                            <code>%temp_imp_values%</code>
                            For several measured temperature values in &deg;Fahrenheit. (newest value first).<br/><br/>
                            <code>%humi_values%</code>
                            For several measured humidity values in %. (newest value first).<br/><br/>
                            
                            <code>%dew_values%</code>
                            For several measured dew point values in &deg;Celsius. (newest value first).<br/><br/>
                            
                            <code>%dew_imp_values%</code>
                            For several measured dew point values in &deg;Fahrenheit. (newest value first).<br/><br/>
                            
                            <code>%qfe_values%</code>
                            For several measured barometric air pressure values in hPa. (newest value first).<br/><br/>
                            
                            <code>%qfe_imp_values%</code>
                            For several measured barometric air pressure values in inHg. (newest value first).<br/><br/>
                            
                            <code>%air_values%</code>
                            For several measured air quality values. (newest value first).<br/><br/>
                            
                            <code>%uv_values%</code>
                            For several measured UV Light values. (newest value first).<br/><br/>
                            
                            <code>%lux_values%</code>
                            For several measured Ambient Light values. (newest value first).<br/><br/>
                            
                            
                            
                            <br/>
                            i.e. you can embed it in iframe.<br/>
                            You can also use the MQTT message examples for a json output format.
                        </div>		
                        
                        <div style="margin-top: 15px;">
                            More infos about TeHyBug can be found at tehybug.com or at the tindie store: https://www.tindie.com/stores/gumslone/
                        </div>

            </div>
        </div>
    </div>
</div>



<script>
    feather.replace();
    connectionStart();    
</script>