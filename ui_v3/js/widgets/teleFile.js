$.widget( "tele.teleFile", {
 
    // Default options.
    options: {
		label: false,
		type: 'file',
		data: ''
    },
    _create: function() {
        this.element.addClass( "tele-file" );
		this.element.addClass( "tele-browse" );
        this._update();
    },
    _setOption: function( key, value ) {
        this.options[ key ] = value;
        this._update();
    },
    _update: function() {
		
		var that = this;
		this.element.empty();
		
		// Append label
		/*if(this.options.label) {
			this.label = $('<label>').addClass('tele-input-label').html(this.options.label);
			this.element.append(this.label);
		} else {
			this.element.addClass('no-label');
		}*/
		
		this.input  = $('<div>').teleInput({ label: this.options.label, value: this.options.value }).click(function () {
			that.file.click();
		});
		this.button = $('<div>').btn({ text: 'Browse', callback: function () {
			that.file.click();
		}});
		
		this.file   = $('<input type="file">').css('opacity','0');
		this.element.append(this.input).append(this.button).append(this.file);
		
		this.file.change(function (e) {
			
			// console.log('FILECHANGE');
			
			var files = this.files;
			if(!files.length) {
				$('input', that.input).css({ borderColor: 'red' });
				return false;
			}
			var file = files[0];
			window.file = file;
			
			that.input.teleInput('option', 'value', file.name + ' (' + parseInt(file.size / 1024) + 'KB)');
			
			var type = that.options.type;
			
			// Quick'n'dirty certificate validation
			if(type == 'certificate' || type == 'privatekey') {
			
				if(file.size > 1024 * 10) {
				
					$('input', that.input).css({ borderColor: 'red' });
					$('input', that.input).css({ borderColor: 'red' }).val('');
					
					if(type == 'privatekey') {
						telepath.dialog({ type: 'alert', title: 'Telepath', msg: 'Not a valid Private Key file' });
					}
					if(type == 'certificate') {
						telepath.dialog({ type: 'alert', title: 'Telepath', msg: 'Not a valid Certificate file' });
					}
					
					$('input', that.input).data('file', '');
					
					return false;
					
				} else {
					
					var reader = new FileReader();
					reader.onloadend = function(evt) {
					
					  if (evt.target.readyState == FileReader.DONE) {
						
						var data = evt.target.result;
						
						if(type == 'privatekey'  && data.toUpperCase().indexOf('PRIVATE KEY') == -1) {
							$('input', that.input).css({ borderColor: 'red' }).val('');
							telepath.dialog({ type: 'alert', title: 'Telepath', msg: 'Not a valid Private Key file' });
							$('input', that.input).data('file', '');
							return false;
						} else {
							$('input', that.input).data('file', data);
						}
						
						if(type == 'certificate' && data.toUpperCase().indexOf('CERTIFICATE') == -1) {
							$('input', that.input).css({ borderColor: 'red' });
							$('input', that.input).css({ borderColor: 'red' }).val('');
							telepath.dialog({ type: 'alert', title: 'Telepath', msg: 'Not a valid Certificate file' });
							$('input', that.input).data('file', '');
							return false;
						} else {
							$('input', that.input).data('file', data);
						}
						
						that.data = data;
						
					  }
					  
					};
					
					reader.readAsBinaryString(file);
					
				}
				
			}
			
			
			
			
		});
		
		/*
		this.button = 
		this.element.append(this.range);*/
		
    }

});