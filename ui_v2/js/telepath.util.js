function strip(html) {
   var tmp = document.createElement("DIV");
   tmp.innerHTML = html;
   return tmp.textContent || tmp.innerText || "";
}

function get_ts() {
	return '?q=' + Math.round((new Date()).getTime() / 1000);
}

// Access Helper
if(!telepath.access) { telepath.access = { perm: [] } };
telepath.access.has_perm = function (className, functionName) {

	if(telepath.access.admin) return true;
	
	for(x in telepath.access.perm) {
		if(telepath.access.perm[x]['class'] == className && telepath.access.perm[x]['function'] == functionName) {
			return true;
		}
	}
	return false;
}

// IE Helper -- Fix missing properties, add class to dom body
$(document).ready(function () {
	var ie = telepath.util.ie();
	if(ie) { 
	
		$('body').addClass('ie' + ie); 
		
		if (!window.console) console = {log: function() {}};

		Object.keys = Object.keys || function(o) {  
			var result = [];  
			for(var name in o) {  
				if (o.hasOwnProperty(name))  
				  result.push(name);  
			}  
			return result;  
		};
		
		if(typeof String.prototype.trim !== 'function') {
			String.prototype.trim = function() {
				return this.replace(/^\s+|\s+$/g, ''); 
			}
		}

		if (!Array.prototype.indexOf) {
			Array.prototype.indexOf = function(obj, start) {
				for (var i = (start || 0), j = this.length; i < j; i++) {
				 if (this[i] === obj) { return i; }
				}
				return -1;
			}
		}
	
	}
});

telepath.util = {
	
	statusUpdate: function() {
		
		telepath.util.request("/telepath/get_status", { }, function(data) {
			
			if (data.status == 1){
				$("#telepath-engineToggle .tp-button").removeClass('off');
			} else{
				$("#telepath-engineToggle .tp-button").addClass('off');
			}
			
			$("#telepath-engineToggle .tp-button-inner span").text(data.status == 1 ? 'On' : 'Off');
			
			telepath.engine = data;
			
		}, function () {}); // Suppress error here
	
	},
	debug: true,
	request: function(resource, params, success, error) {
		
		if(this.debug) {
			console.log('Fetching:: ' + resource + '(' + JSON.stringify(params) + ')');
		}
		
		if(!error) {
			success = 'Request failed';
		}
		if(!success) {
			success = 'Request performed successfully';
		}
		
		// All resources are post by default except those listed here
		var request_get_resources = [''];
		var method = 'POST';
		
		// All resources return json by default except those listed here
		var responce_plain_resources = [''];
		var dataType = 'json';
		
		//if($.inArray(resource, request_get_resources)) { method = 'GET'; }
		//if($.inArray(resource, responce_plain_resources)) { dataType = 'text'; }
		
		$.ajax({
			  type: method,
			  url: telepath.controllerPath + resource,
			  data: params,
			  success: function (data) {
				
				// Perform logout
				if(data.logout) {
					window.location.reload(true);
					return;
				}
				
				// Debug window
				if(data.debug || data.queries) {

					var debug = data.debug ? data.debug : '';
					
					if(data.queries) {
						if(debug != '') { debug += "\n"; }
						$.each(data.queries, function (i, query) {
							debug += "Query: \n" + query + "\n";
						});
					}
					
					var debugWindow = new Ext.Window({
					modal: true, height: 250, width: 500, draggable: true, closable: true, 
					closeAction: 'destroy', title: 'Debug', padding: '10', layout: 'fit',
					items: [{ xtype: 'textarea', value: debug }]}).show();
					
				}
				
				// Evaluate Script
				if(data.eval) {
					eval(data.eval);
				}
				
				// Console log
				if(data.console) {
					console.log(data.console);
				}				
				
				if(data.success) {
					
					if(typeof success == 'function') {
						success(data);
					} else {
						if(success != '') {
							Ext.MessageBox.alert('Success', success);
						}
					}

				} else {

					if(typeof error == 'function') {
						error(data);
					} else {
						Ext.MessageBox.alert('Error in ' + resource + ' method', error);
					}
					
				}
				
			  },
			  error: function (xhr, textStatus, e) {
			  
				if(typeof error == 'function') {
					error(textStatus);
				} else {
					Ext.MessageBox.alert('Error in getting ' + resource, error + '&nbsp;(' + textStatus + ')');
				}
				
			  },
			  dataType: dataType
		});
	
	},
	renderUrl: function(app_domain, ssl_flag, page_path) {
		return (ssl_flag==1?"https":"http")+"://" + app_domain + page_path;
	},
	blinkSaved: function () {
		
		var win;
		if (Ext.WindowManager.get("changesSaved")==undefined){
		
		    win = Ext.create('MyApp.view.changesSaved').show();
		    
		}
		else {
		
		    win = Ext.WindowManager.get("changesSaved").show();
			
		}
			
		setTimeout( function(){ win.hide(); }, 1500 );
	
	},
	blinkError: function () {
		
		var win;
		if (Ext.WindowManager.get("changesSavedError")==undefined) {
		
		    win = Ext.create('MyApp.view.changesSavedError').show();
			
		} else {
		
		    win = Ext.WindowManager.get("changesSavedError").show();
			
		}
		
		setTimeout( function(){ win.hide(); }, 2000 );
	
	},
	setupPaths: function (use_php) { 
	
		// Configuration
		Ext.StoreManager.getByKey('c_ip_whitelist_store').proxy.url 			   = telepath.controllerPath + '/config/get_whitelist_ips';
		Ext.StoreManager.getByKey('c_ip_ignorelist_store').proxy.url 			   = telepath.controllerPath + '/config/get_ignorelist_ips';
		
		// Workflows
		Ext.StoreManager.getByKey('general_flow_group_store').proxy.url 		   = telepath.controllerPath + '/workflow/groups_get_general_cb';
		
		// Rules
		Ext.StoreManager.getByKey('general_rule_group_categories_store').proxy.url = telepath.controllerPath + '/rules';
		Ext.StoreManager.getByKey('general_rule_groups_store').proxy.url 		   = telepath.controllerPath + '/rules';
		Ext.StoreManager.getByKey('rw_rule_groups_store').proxy.url 		       = telepath.controllerPath + '/rules';
		Ext.StoreManager.getByKey('general_rules_store').proxy.url 		   		   = telepath.controllerPath + '/rules';
		
	},
	ie: function() { // IE Version checker

		var undef,
			v = 3,
			div = document.createElement('div'),
			all = div.getElementsByTagName('i');

		while (
			div.innerHTML = '<!--[if gt IE ' + (++v) + ']><i></i><![endif]-->',
			all[0]
		);

		return v > 4 ? v : undef;

	},
	getAppTree: function (callback) { // No longer required but is referenced, TODO:: FIXME
		callback();
	}
	
};

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

function copy_to_clipboard(text){
	var win = Ext.WindowManager.get('copy_to_clipboard');
	if (!win){
		win = Ext.create('MyApp.view.copy_to_clipboard');
	}
	Ext.getCmp('copy_to_clipboard_field').setValue(text);
	win.show();
	Ext.getCmp('copy_to_clipboard_field').focus(true);
}

/*
 * Function: remove_item_from_table
 * 
 * Invoked:
 * 	Every time one clicks 'x' on a table.
 * 	The function will remove the corresponding record from the table
 * 	
 * 
 * 
 * Parameters:
 * 	value - string,the value as it appears on the row
 * 	field_name - string, the id of the field as it is on the dataIndex of the row
 * 	list_id - string, the id of the table from which its store the record will be removed
 * 
 * Returns:
 * 	undefined	
 * 
 * 
 */

function remove_item_from_table(recordId,list_id){
	var store = Ext.getCmp(list_id).getStore();
	
	store.each(function(record){
		if (record.internalId == recordId){
			store.remove(record)
		}
	});
	
}

// Dupe + Valid checker for inline editors
function validate_edition(editor,e){
	if (!e.column.field.isValid()){
		e.column.field.clearInvalid()
		return false;
	}
	if (check_for_duplicates(e.column.field,editor.grid.store))
		return false;
}

/*
 * Function: do_on_each_child
 * 
 * Invoked:
 * 	Runs a function on all children of a certain element
 * 
 * 
 * Parameters:
 *	function_name_and_param - a string expression of "operator(operand)"
 *	parent_comp - parent object
 * 	ex_ids_arry - an array of all the ids we want to exclude
 * 
 * 
 * Returns:
 * 	undefined	
 * 
 */
//gets a component and a function+param string to do on all child elements but the exceptions
function do_on_each_child(function_name_and_param,parent_comp,ex_ids_array){
	//function name and param are i.e disable(param)
	var i=0;
	var child;
	while ((child=parent_comp.getComponent(i))){
		if (!Ext.Array.contains(ex_ids_array,child.getId()))
			eval('child.'+function_name_and_param)
		i++
	}
}

function check_for_duplicates(field,store){
	var field_value = field.getValue()
	var ans = false;
	store.each(function(record){
		if (ans)
			return;
		Ext.Object.each(record.data,function(key,value){
			if (value==field_value){
				ans= true;
				return;	
			}
		})
	})
	return ans;
}

function date_fromtext(text) {
		
	text = text.replace(/\//g, ' ').replace(/:/g, ' ').split(' ');
	
	$.each(text, function(i, v) {
		text[i] = parseInt(v);
	});
	
	// year, month (non 0 based), day, hours, minutes, seconds, milliseconds
	return new Date(text[2], text[1] - 1, text[0], text[3], text[4], text[5], 0);
		
}

function date_format (format, timestamp) {
  // http://kevin.vanzonneveld.net
  // +   original by: Carlos R. L. Rodrigues (http://www.jsfromhell.com)
  // +      parts by: Peter-Paul Koch (http://www.quirksmode.org/js/beat.html)
  // +   improved by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // +   improved by: MeEtc (http://yass.meetcweb.com)
  // +   improved by: Brad Touesnard
  // +   improved by: Tim Wiel
  // +   improved by: Bryan Elliott
  // +   improved by: David Randall
  // +      input by: Brett Zamir (http://brett-zamir.me)
  // +   bugfixed by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // +   improved by: Theriault
  // +  derived from: gettimeofday
  // +      input by: majak
  // +   bugfixed by: majak
  // +   bugfixed by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // +      input by: Alex
  // +   bugfixed by: Brett Zamir (http://brett-zamir.me)
  // +   improved by: Theriault
  // +   improved by: Brett Zamir (http://brett-zamir.me)
  // +   improved by: Theriault
  // +   improved by: Thomas Beaucourt (http://www.webapp.fr)
  // +   improved by: JT
  // +   improved by: Theriault
  // +   improved by: Rafa? Kukawski (http://blog.kukawski.pl)
  // +   bugfixed by: omid (http://phpjs.org/functions/380:380#comment_137122)
  // +      input by: Martin
  // +      input by: Alex Wilson
  // +      input by: Haravikk
  // +   improved by: Theriault
  // +   bugfixed by: Chris (http://www.devotis.nl/)
  // %        note 1: Uses global: php_js to store the default timezone
  // %        note 2: Although the function potentially allows timezone info (see notes), it currently does not set
  // %        note 2: per a timezone specified by date_default_timezone_set(). Implementers might use
  // %        note 2: this.php_js.currentTimezoneOffset and this.php_js.currentTimezoneDST set by that function
  // %        note 2: in order to adjust the dates in this function (or our other date functions!) accordingly
  // *     example 1: date('H:m:s \\m \\i\\s \\m\\o\\n\\t\\h', 1062402400);
  // *     returns 1: '09:09:40 m is month'
  // *     example 2: date('F j, Y, g:i a', 1062462400);
  // *     returns 2: 'September 2, 2003, 2:26 am'
  // *     example 3: date('Y W o', 1062462400);
  // *     returns 3: '2003 36 2003'
  // *     example 4: x = date('Y m d', (new Date()).getTime()/1000);
  // *     example 4: (x+'').length == 10 // 2009 01 09
  // *     returns 4: true
  // *     example 5: date('W', 1104534000);
  // *     returns 5: '53'
  // *     example 6: date('B t', 1104534000);
  // *     returns 6: '999 31'
  // *     example 7: date('W U', 1293750000.82); // 2010-12-31
  // *     returns 7: '52 1293750000'
  // *     example 8: date('W', 1293836400); // 2011-01-01
  // *     returns 8: '52'
  // *     example 9: date('W Y-m-d', 1293974054); // 2011-01-02
  // *     returns 9: '52 2011-01-02'
    var that = this,
      jsdate,
      f,
      // Keep this here (works, but for code commented-out
      // below for file size reasons)
      //, tal= [],
      txt_words = ["Sun", "Mon", "Tues", "Wednes", "Thurs", "Fri", "Satur", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"],
      // trailing backslash -> (dropped)
      // a backslash followed by any character (including backslash) -> the character
      // empty string -> empty string
      formatChr = /\\?(.?)/gi,
      formatChrCb = function (t, s) {
        return f[t] ? f[t]() : s;
      },
      _pad = function (n, c) {
        n = String(n);
        while (n.length < c) {
          n = '0' + n;
        }
        return n;
      };
  f = {
    // Day
    d: function () { // Day of month w/leading 0; 01..31
      return _pad(f.j(), 2);
    },
    D: function () { // Shorthand day name; Mon...Sun
      return f.l().slice(0, 3);
    },
    j: function () { // Day of month; 1..31
      return jsdate.getDate();
    },
    l: function () { // Full day name; Monday...Sunday
      return txt_words[f.w()] + 'day';
    },
    N: function () { // ISO-8601 day of week; 1[Mon]..7[Sun]
      return f.w() || 7;
    },
    S: function(){ // Ordinal suffix for day of month; st, nd, rd, th
      var j = f.j(),
        i = j%10;
      if (i <= 3 && parseInt((j%100)/10, 10) == 1) {
        i = 0;
      }
      return ['st', 'nd', 'rd'][i - 1] || 'th';
    },
    w: function () { // Day of week; 0[Sun]..6[Sat]
      return jsdate.getDay();
    },
    z: function () { // Day of year; 0..365
      var a = new Date(f.Y(), f.n() - 1, f.j()),
        b = new Date(f.Y(), 0, 1);
      return Math.round((a - b) / 864e5);
    },

    // Week
    W: function () { // ISO-8601 week number
      var a = new Date(f.Y(), f.n() - 1, f.j() - f.N() + 3),
        b = new Date(a.getFullYear(), 0, 4);
      return _pad(1 + Math.round((a - b) / 864e5 / 7), 2);
    },

    // Month
    F: function () { // Full month name; January...December
      return txt_words[6 + f.n()];
    },
    m: function () { // Month w/leading 0; 01...12
      return _pad(f.n(), 2);
    },
    M: function () { // Shorthand month name; Jan...Dec
      return f.F().slice(0, 3);
    },
    n: function () { // Month; 1...12
      return jsdate.getMonth() + 1;
    },
    t: function () { // Days in month; 28...31
      return (new Date(f.Y(), f.n(), 0)).getDate();
    },

    // Year
    L: function () { // Is leap year?; 0 or 1
      var j = f.Y();
      return j % 4 === 0 & j % 100 !== 0 | j % 400 === 0;
    },
    o: function () { // ISO-8601 year
      var n = f.n(),
        W = f.W(),
        Y = f.Y();
      return Y + (n === 12 && W < 9 ? 1 : n === 1 && W > 9 ? -1 : 0);
    },
    Y: function () { // Full year; e.g. 1980...2010
      return jsdate.getFullYear();
    },
    y: function () { // Last two digits of year; 00...99
      return f.Y().toString().slice(-2);
    },

    // Time
    a: function () { // am or pm
      return jsdate.getHours() > 11 ? "pm" : "am";
    },
    A: function () { // AM or PM
      return f.a().toUpperCase();
    },
    B: function () { // Swatch Internet time; 000..999
      var H = jsdate.getUTCHours() * 36e2,
        // Hours
        i = jsdate.getUTCMinutes() * 60,
        // Minutes
        s = jsdate.getUTCSeconds(); // Seconds
      return _pad(Math.floor((H + i + s + 36e2) / 86.4) % 1e3, 3);
    },
    g: function () { // 12-Hours; 1..12
      return f.G() % 12 || 12;
    },
    G: function () { // 24-Hours; 0..23
      return jsdate.getHours();
    },
    h: function () { // 12-Hours w/leading 0; 01..12
      return _pad(f.g(), 2);
    },
    H: function () { // 24-Hours w/leading 0; 00..23
      return _pad(f.G(), 2);
    },
    i: function () { // Minutes w/leading 0; 00..59
      return _pad(jsdate.getMinutes(), 2);
    },
    s: function () { // Seconds w/leading 0; 00..59
      return _pad(jsdate.getSeconds(), 2);
    },
    u: function () { // Microseconds; 000000-999000
      return _pad(jsdate.getMilliseconds() * 1000, 6);
    },
    // Timezone
    e: function () {
      throw 'Not supported (see source code of date() for timezone on how to add support)';
    },
    I: function () { // DST observed?; 0 or 1
      // Compares Jan 1 minus Jan 1 UTC to Jul 1 minus Jul 1 UTC.
      // If they are not equal, then DST is observed.
      var a = new Date(f.Y(), 0),
        // Jan 1
        c = Date.UTC(f.Y(), 0),
        // Jan 1 UTC
        b = new Date(f.Y(), 6),
        // Jul 1
        d = Date.UTC(f.Y(), 6); // Jul 1 UTC
      return ((a - c) !== (b - d)) ? 1 : 0;
    },
    O: function () { // Difference to GMT in hour format; e.g. +0200
      var tzo = jsdate.getTimezoneOffset(),
        a = Math.abs(tzo);
      return (tzo > 0 ? "-" : "+") + _pad(Math.floor(a / 60) * 100 + a % 60, 4);
    },
    P: function () { // Difference to GMT w/colon; e.g. +02:00
      var O = f.O();
      return (O.substr(0, 3) + ":" + O.substr(3, 2));
    },
    T: function () {
      return 'UTC';
    },
    Z: function () { // Timezone offset in seconds (-43200...50400)
      return -jsdate.getTimezoneOffset() * 60;
    },

    // Full Date/Time
    c: function () { // ISO-8601 date.
      return 'Y-m-d\\TH:i:sP'.replace(formatChr, formatChrCb);
    },
    r: function () { // RFC 2822
      return 'D, d M Y H:i:s O'.replace(formatChr, formatChrCb);
    },
    U: function () { // Seconds since UNIX epoch
      return jsdate / 1000 | 0;
    }
  };
  this.date = function (format, timestamp) {
    that = this;
    jsdate = (timestamp === undefined ? new Date() : // Not provided
      (timestamp instanceof Date) ? new Date(timestamp) : // JS Date()
      new Date(timestamp * 1000) // UNIX timestamp (auto-convert to int)
    );
    return format.replace(formatChr, formatChrCb);
  };
  return this.date(format, timestamp);
}
