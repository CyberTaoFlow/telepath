telepath.realtime = {
	initialized: false,
	data: {
		packets: [],
		loss: [],
		requests: []
	},
	offsets: {
		init: false,
		packets: 0,
		loss: 0,
		requests: 0
	},
	init: function () {
		
		console.log('Monitor Init');
		
		var totalPoints = 60;
		var now = new Date().getTime() - (totalPoints * updateInterval);
		
		for (var i = 0; i < totalPoints; i++) {
			var temp = [now += updateInterval, 0];
			telepath.realtime.data.packets.push(temp);
			telepath.realtime.data.loss.push(temp);
			telepath.realtime.data.requests.push(temp);
		}
		
		yepnope({
			load: [
				"lib/jquery.flot.js",
				"lib/jquery.flot.axislabels.js",
				"lib/jquery.flot.time.min.js",
				"lib/jquery.flot.symbol.min.js",
			],
			complete: function () {
			
				console.log('Monitor Init Done');
				telepath.realtime.initialized = true;
				telepath.realtime.expand();

			}
		});
	},
	expand: function () {
		if(!this.initialized) {
			this.init();
			return;
		}
		console.log('Monitor Expand');
		var dataset = [
			{ label: "Packets:", data: telepath.realtime.data.packets, lines: { fill: true, lineWidth: 1.2 }, color: "#00FF00" },
			{ label: "Dropped:", data: telepath.realtime.data.loss, lines: { lineWidth: 1.2}, color: "#FF0000", yaxis: 3 },
			{ label: "Requests:", data: telepath.realtime.data.requests, color: "#0044FF", bars: { show: true }, yaxis: 2 }
		];
		
		$.plot($("#d_realtime-innerCt"), dataset, options);
		setTimeout(GetData, updateInterval);
		
	},
	collapse: function () {
		console.log('Monitor Stop');
		$("#d_realtime-innerCt").empty();
	}
}

var updateInterval = 10000;

var options = {
    series: {
        lines: {
            lineWidth: 1.2
        }
    },
    xaxis: {
        mode: "time",
        tickFormatter: function (v, axis) {
            var date = new Date(v);
            var hours = date.getHours() < 10 ? "0" + date.getHours() : date.getHours();
            var minutes = date.getMinutes() < 10 ? "0" + date.getMinutes() : date.getMinutes();
            var seconds = date.getSeconds() < 10 ? "0" + date.getSeconds() : date.getSeconds();
            return hours + ":" + minutes + ":" + seconds;
        },
        axisLabel: "Time",
        axisLabelUseCanvas: true,
        axisLabelFontSizePixels: 12,
        axisLabelFontFamily: 'Verdana, Arial',
        axisLabelPadding: 10
    },
    yaxes: [
        {
            axisLabel: "Dropped",
            axisLabelUseCanvas: true,
            axisLabelFontSizePixels: 12,
            axisLabelFontFamily: 'Verdana, Arial',
            axisLabelPadding: 6
        }, {
            position: "right",
            axisLabel: "Requests",
            axisLabelUseCanvas: true,
            axisLabelFontSizePixels: 12,
            axisLabelFontFamily: 'Verdana, Arial',
            axisLabelPadding: 6
        }, {
            position: "right",
            axisLabel: "Packets",
            axisLabelUseCanvas: true,
            axisLabelFontSizePixels: 12,
            axisLabelFontFamily: 'Verdana, Arial',
            axisLabelPadding: 14
        }
    ],
    legend: {
        noColumns: 0,
        position:"nw"
    },
    grid: {      
        backgroundColor: { colors: ["#ffffff", "#EDF5FF"] }
    }
};

function GetData() {
    $.ajaxSetup({ cache: false });

    $.ajax({
        url: telepath.controllerPath + "/dashboard/get_realtime",
        dataType: 'json',
        success: function (_data) {	
			
			_data['capture.kernel_packets'] = parseInt(_data['capture.kernel_packets']);
			_data['capture.kernel_drops'] = parseInt(_data['capture.kernel_drops']);
			
			if(!telepath.realtime.offsets.init) {
				telepath.realtime.offsets.packets  = _data['capture.kernel_packets'];
				telepath.realtime.offsets.loss	   = _data['capture.kernel_drops'];
				telepath.realtime.offsets.requests = _data['cmd_set'];
			
				telepath.realtime.offsets.init = true;
				setTimeout(GetData, 1000);
				return;
			} 
			
			telepath.realtime.data.packets.shift();
			telepath.realtime.data.loss.shift();
			telepath.realtime.data.requests.shift();
			
			var now = new Date().getTime();
			
			telepath.realtime.data.packets.push([ now, _data['capture.kernel_packets'] - telepath.realtime.offsets.packets ]);
			telepath.realtime.data.loss.push([ now, _data['capture.kernel_drops'] - telepath.realtime.offsets.loss ]);
			telepath.realtime.data.requests.push([ now, _data.cmd_set - telepath.realtime.offsets.requests ]);
			
			var dataset = [
				{ label: "Dropped", data: telepath.realtime.data.loss, lines: { lineWidth: 1.2}, color: "#FF0000" },
				{ label: "Packets", data: telepath.realtime.data.packets, lines: { fill: true, lineWidth: 1.2 }, color: "#00FF00", yaxis: 3 },
				{ label: "Requests", data: telepath.realtime.data.requests, color: "#0044FF", yaxis: 2 }
			];
						
			$.plot($("#d_realtime-innerCt"), dataset, options);
			
			telepath.realtime.offsets.packets  = _data['capture.kernel_packets'];
			telepath.realtime.offsets.loss	   = _data['capture.kernel_drops'];
			telepath.realtime.offsets.requests = _data['cmd_set'];
		
			setTimeout(GetData, updateInterval);
		
		},
        error: function () {
            setTimeout(GetData, updateInterval);
        }
    });
}

var temp;