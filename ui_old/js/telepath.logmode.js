var logmode_timer = false;

function logmode_init(app_id) {
	
	var container = $(Ext.getCmp('c_logmode_dialog').body.dom);
	
	// Disable default drag and drop on the container
	$(container).bind('drop dragover', function (e) {
		e.preventDefault();
	});
	
	$('.fileupload-dragcontainer').on('dragleave', function (e) {
		$('.fileupload-dragcontainer').css('border', '2px dashed #ccc');
	});
	
	Ext.getCmp('c_logmode_dialog').disable();
	
	var loader_c = $('<div>').addClass('upload-loader');
	var loader_t = $('<h5>Please wait while loading upload controls</h5>');
	var loader_i = $('<img>').attr('src', 'images/loading.gif');
	
	loader_c.append(loader_t).append(loader_i);
	container.append(loader_c);
	
	$.get(telepath.controllerPath + '/logmode', function (html) {
		
		container.prepend(html);
		container.prepend('');
		
		loadResources([
				"lib/jquery.iframe-transport.js",
				"lib/jquery.fileupload.js",
				"lib/jquery.fileupload-process.js",
				"lib/jquery.fileupload-tmpl.min.js",
				"lib/jquery.fileupload-ui.js",
				"css/uploader.css"
			], function () {
				
				Ext.getCmp('c_logmode_start').disable();

				var url = telepath.controllerPath + '/logmode/upload';

				// Initialize the jQuery File Upload widget:
				$('#fileupload').fileupload({
					url: url,
					dropZone: $('.fileupload-dragcontainer'),
					dragover: function (e, data) {
					
						$('.fileupload-dragcontainer').css('border', '2px dashed black');
						data.dropEffect = 'move';
						data.preventDefault = false;
						
					}
					
				}).bind('fileuploadadd', function (e, data) {

					Ext.getCmp('c_logmode_start').enable();
					$('.fileupload-dragcontainer').css('height', 'auto');
					
				}).bind('fileuploadcompleted', function (e, data) {
				
					if(!logmode_timer) {
						logmode_timer = setTimeout(function () {
						
							logmode_parser_status_tick();
						
						}, 5000);
					}
					
				}).bind('fileuploadsubmit', function (e, data) {
					
					var logtype = Ext.getCmp('c_logmode_type').value;
					//var app_id  = $('select', data.context).val();
					
					var new_url = url + '?logtype=' + logtype + '&app_id=' + app_id;
					
					$("#fileupload").fileupload({ url: new_url });

				});
				
				setTimeout(function () {
				
					$('.upload-loader').remove();
					Ext.getCmp('c_logmode_dialog').enable();
					
				}, 500);
				
			});
			
	});
	
}

// Initialize controls
function logmode_start() {
	
	Ext.getCmp('c_logmode_start').disable();
	$(".fileupload-buttonbar .start").click();
	
}

function logmode_populate_apps() {
	// Not required
}

// Timer to poll for engine parse process
function logmode_parser_status_tick() {
	
	$.getJSON('index.php/logmode/status', function(data) {
		
		var files_uploaded  = data.length;
		
		var processed_current = 0;
		var processed_total   = data.length * 100;
		
		$.each(data, function(i, file) {
			processed_current	= processed_current + parseInt(file.processed);
		});
		
		console.log(processed_current + ' / ' + processed_total);
		
		// Update progress bar
		if(Ext.getCmp('c_logmode_progress_engine')) {
			Ext.getCmp('c_logmode_progress_engine').updateProgress(processed_current / processed_total);
		}
		
		// Reload on done
		if(processed_current / processed_total == 1) {
			$.getJSON('index.php/logmode/cleanup', function(data) {
				location.reload();
			});
		}
		
		// Reboot the timer if the dialog is still open
		if(Ext.getCmp('c_logmode_dialog')) {
		
			logmode_timer = setTimeout(function () {
							
				logmode_parser_status_tick();
							
			}, 5000);
		
		}
	
	});

}