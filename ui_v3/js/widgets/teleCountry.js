$.widget( "tele.teleCountry", {
 
    // Default options
    options: {
		data: {
			value: []
		}
    },
    _create: function() {
		this.element.addClass('tele-country-widget');
		this._update();
    },
    _setOption: function( key, value ) {
		this.options[ key ] = value;
		this._update();
    },
	getSelected: function () {
		
		var result = '';
		
		$('.tele-checkbox-checkbox.checked').each(function () {
			// fix undefined in country list, Yuli
			if ($(this).attr('dataid')) {
				result = result + $(this).attr('dataid') + ',';
			}
		});
		
		return result.substr(0, result.length - 1);
		
	},
    _update: function() {
	
		var that = this;
		
		var autoComplete = $('<div>');
		
		this.element.append(autoComplete);
		
		autoComplete.teleInput({ label: "Search countries:", labelCSS: {width: 120}});
		$('input', autoComplete).keyup(function () {
			var search = $(this).val().toLowerCase();
			if(search == '') {
				$(this).parents('div').find('.tele-country-list li').show();
				return;
			}
			$(this).parents('div').find('.tele-country-list li').each(function () {
				if($(this).text().toLowerCase().indexOf(search) > -1) {
					$(this).show();
				} else {
					$(this).hide();
				}
			});
			
		});
		
		/*
		var fList  = $('<ul>').addClass('tele-country-letters');
		
		var ascii_range = (function() {
		  var data = [];
		  while (data.length < 128) data.push(String.fromCharCode(data.length));
		  return function (start,stop) {
			start = start.charCodeAt(0);
			stop = stop.charCodeAt(0);
			return (start < 0 || start > 127 || stop < 0 || stop > 127) ? null : data.slice(start,stop+1);
		  };
		})();
		
		var allCountries = $('<li>').append($('<a>').html('All')).click(function () {
			$(this).parents('div').find('.tele-country-list li').show();
		});
		fList.append(allCountries);
		var range = ascii_range('A','Z');
		for(x in range) {
			var letter = $('<li>').append($('<a>').html(range[x])).click(function () {
				$(this).parents('div').find('.tele-country-list li').hide();
				$(this).parents('div').find('.tele-country-list .tele-country-letter-' + $(this).text()).show();
			});		
			fList.append(letter);
		}
	
		this.element.append(fList);
		*/
	
		var cList  = $('<ul>').addClass('tele-country-list');
		this.element.append(cList);
				
		that.options.selected = [];
		
		$.each(telepath.countries.map, function(code, text) {
			var checked = false;
			for(x in that.options.values) {
				if(that.options.values[x] == code) {
					checked = true;
				}
			}
			that.options.selected.push({ dataID: code, checked: checked });
		});
		
		$.each(telepath.countries.map, function (code, text) {
			
			var checked = false;
			for(x in that.options.values) {
				if(that.options.values[x] == code) {
					checked = true;
				}
			}
			
			var listEl = $('<li>').hover(function () { $(this).addClass('hover') }, function () { $(this).removeClass('hover') }).addClass('tele-country-letter-' + code.substr(0,1).toUpperCase());
			var cbEl   = $('<span>').teleCheckbox({ checked: checked, inputFirst: true, dataID: code, label: '<span class="flag flag-' + code + '"></span><span class="text">' + text + '</span>', callback: function () {
				var dataID = this.dataID;
				var checked = this.checked;
				$.each(that.options.selected, function(i, selection) {
					if(selection.dataID == dataID) {
						that.options.selected[i].checked = checked;
					}
				});
			}});
			
			listEl.append(cbEl);
			cList.append(listEl);
			
		});
		
		cList.mCustomScrollbar({
			scrollButtons:{	enable: false },
			advanced:{ updateOnContentResize: true }
		});
		
		
    }

});
