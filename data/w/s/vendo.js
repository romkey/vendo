$(function () {
    let colorPicker = new iro.ColorPicker('#color-picker-container', { width: 150 });
    let websocket;

    setupWebSocket();

    // 
    $.fn.serializeObject=function(){"use strict";var a={},b=function(b,c){var d=a[c.name];"undefined"!=typeof d&&d!==null?$.isArray(d)?d.push(c.value):a[c.name]=[d,c.value]:a[c.name]=c.value};return $.each(this.serializeArray(),b),a};

    function setupWebSocket() {
	let websocket_url = 'ws://' + location.hostname + '/ws';

	console.debug("websocket url", websocket_url);

	websocket = new WebSocket(websocket_url);

	websocket.onopen    = function() { console.log("websocket open"); };
	websocket.onclose   = function() { setTimeout(setupWebSocket, 500); console.log("websocket closed"); };
	websocket.onmessage = function(event) { console.log("websocket data"); console.log(event.data); };
    }

    function onColorChange(color, changes) {
	var rgb = color.rgb;

        $('body').css('background-color', color.hexString);

	let cmd = { rgb: { red: rgb.r, green: rgb.g, blue: rgb.b } };

	console.log(cmd);

	websocket.send(JSON.stringify(cmd));

        $('#r').val(rgb.r); $('#g').val(rgb.g); $('#b').val(rgb.b);

        console.log(color.hexString);
    }

    function update_page(data) {
	console.log('UPDATE PAGE', data);

        if(data["status"] === "off") {
            $('#animation option:selected').prop('selected', false); 
            $('#preset option:selected').prop('selected', false);
            $('body').css('background-color', '#000000');
        }

        $('#brightness').attr('value', data["brightness"]);
        if(data["animation"]) {
            $('#speed').attr('value', data["speed"]);
            $('#animation').val(data["animation"]); 
        }

        if(data["rgb"]) {
            $('#r').val(data["rgb"]["red"]); 
            $('#g').val(data["rgb"]["green"]); 
            $('#b').val(data["rgb"]["blue"]); 

            let rgb = "rgb(" + [ data["rgb"]["red"], data["rgb"]["green"], data["rgb"]["blue"] ].join(', ') + ")";

            $('body').css('background-color', rgb);
        }

        if(data["preset"])
            $('#preset').val(data["preset"]);
    }

    colorPicker.on('color:change', onColorChange);
    $('select').change(function() {
	let cmd = {};
	cmd[$(this).attr('name')] = $(this).val();

	websocket.send(JSON.stringify(cmd));
    });
	
    $(document).on('submit', 'form', function(event) {
        event.preventDefault();

	let cmd = {};

	let value = $(this).attr('data-value');
	let key = $(this).attr('data-role');

	if(value) {
	    cmd[key] = value;
	} else {
	    cmd[key] = $('input[name="' + key + '"]').val();

	    if($('input[name="' + key + '"]').attr('type') == 'number')
		cmd[key] = Number(cmd[key]);
	}

	console.log('command', cmd);

	websocket.send(JSON.stringify(cmd));
    }); 
}); 
