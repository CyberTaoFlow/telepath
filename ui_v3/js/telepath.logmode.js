var logmode_timer = false;

// Helper functions for YEPNOPE loader script
var loadedResources = {};

function loadResources(resources, completeCallback) {
    // return only those which are not loaded yet
    resources = processResources(resources, completeCallback);
    yepnope({
        load: resources,
        complete: completeCallback
    });
}

function processResources(resources, completeCallback) {
    var notLoaded = [];
    for(var i = 0; i < resources.length; i++)
    {
        if(!loadedResources[resources[i]])
        {
            notLoaded.push(resources[i]);
        }
        loadedResources[resources[i]] = true;
    }

    if (notLoaded.length == 0) {
        completeCallback();
    }

    return notLoaded;
}

function logmode_init(app_id) {
	
	var that = this;
	
	this.maskEl = $('<div>').addClass('tele-overlay-mask').addClass('tele-overlay-mask-dialog');
	this.overlayEl = $('<div>').addClass('tele-overlay').addClass('tele-overlay-logmode');
	
	this.headerEl    = $('<div>').addClass('tele-overlay-header');
	this.contentEl   = $('<div>').addClass('tele-overlay-content');
	this.titleEl     = $('<div>').addClass('tele-overlay-title').html("Upload Logs");
	this.closeEl     = $('<a>').attr('href', '#').addClass('tele-overlay-close').addClass('tele-icon').addClass('tele-icon-close');
	this.textEl      = $('<div>').addClass('tele-dialog-text').html('');
	
	$('body').append(this.maskEl);
	$('body').append(this.overlayEl);
	
	this.overlayEl.append(this.headerEl).append(this.contentEl);
	this.headerEl.append(this.titleEl).append(this.closeEl);
		
	this.closeEl.click(function () {
		that.maskEl.remove();
		that.overlayEl.remove();
	}).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover'); });
	
	$( this.overlayEl ).draggable({ handle: this.headerEl });
	
	var container = this.contentEl;
	
	// Disable default drag and drop on the container
	$(container).bind('drop dragover', function (e) {
		e.preventDefault();
	});
	
	$('.fileupload-dragcontainer').on('dragleave', function (e) {
		$('.fileupload-dragcontainer').css('border', '2px dashed #ccc');
	});
	
	//Ext.getCmp('c_logmode_dialog').disable();
	container.append(telepath.loader);
	
	$.get(telepath.controllerPath + '/logmode', function (html) {
		
		loadResources([
				"js/lib/jquery.iframe-transport.js",
				"js/lib/jquery.fileupload.js",
				"js/lib/jquery.fileupload-process.js",
				"js/lib/jquery.fileupload-tmpl.min.js",
				"js/lib/jquery.fileupload-ui.js",
				"css/uploader.css"
			], function () {
				
				container.html(html);
				//Ext.getCmp('c_logmode_start').disable();

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

					//Ext.getCmp('c_logmode_start').enable();
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
					//Ext.getCmp('c_logmode_dialog').enable();
					
				}, 500);
				
			});
			
	});
	
}

// Initialize controls
function logmode_start() {
	
	//Ext.getCmp('c_logmode_start').disable();
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
		/*if(Ext.getCmp('c_logmode_progress_engine')) {
			Ext.getCmp('c_logmode_progress_engine').updateProgress(processed_current / processed_total);
		}*/
		
		// Reload on done
		if(processed_current / processed_total == 1) {
			$.getJSON('index.php/logmode/cleanup', function(data) {
				location.reload();
			});
		}
		
		// Reboot the timer if the dialog is still open
		/*if(Ext.getCmp('c_logmode_dialog')) {
		
			logmode_timer = setTimeout(function () {
							
				logmode_parser_status_tick();
							
			}, 5000);
		
		}*/
	
	});

}