// DO not allow to remove last IP range
function RangeRemove(that) {
	numRanges = $(that).parent().parent().children('.tele-ip-wrap').length;
	if (numRanges > 1)
	{	
		$(that).parent().remove();
	}
}

function RangeRemoveLB(that) {
  numRanges = $(that).parent().parent().children('.tele-ip-wrap-lb').length;
  if (numRanges > 1)
  {
    $(that).parent().remove();
  }
}
function old_getRangeUI(data) {

  if(!data) { data = ''; } else { data = data.trim() }

  var is_range = data.split('-').length > 1;

  var ipWrap   = $('<div>').addClass('tele-ip-wrap');
  var ipStart  = $('<div>').addClass('tele-ip').ip({ data: data.split('-')[0] });
  var ipDash   = $('<div>').addClass('tele-ip-dash').html('_');

  var ipEnd    = $('<div>').addClass('tele-ip').ip({ data: is_range ? data.split('-')[1] : '' });

  if(!is_range) {
    ipDash.hide();
    ipEnd.hide();
  }

  var ipAdd = $('<div>').addClass('tele-ip-add')
      .addClass('tele-icon')
      .addClass('tele-icon-plus')
      .hover(function () { $(this).addClass('hover'); },
      function () { $(this).removeClass('hover'); })
      .click(function () { that.limitRanges.append(getRangeUI()); });

  var ipRemove = $('<div>').addClass('tele-ip-remove')
      .addClass('tele-icon')
      .addClass('tele-icon-minus')
      .hover(function () { $(this).addClass('hover'); },
      function () { $(this).removeClass('hover'); })
      .click(function () { $(this).parent().remove(); });

  var ipToggle = $('<div>').toggleFlip({

    left_value: 'Single',
    right_value: 'Range',
    flip: function () {
      ipEnd.toggle();
      ipDash.toggle();
    },
    flipped: is_range,

  });

  ipWrap.append(ipAdd).append(ipRemove).append(ipToggle).append(ipStart).append(ipDash).append(ipEnd);
  return ipWrap;

}


function getRangeUI(data, container) {
					
	if(!data) { data = ''; }

  var is_range= data.from!=data.to;

	//var to = data.to?data.to:data.from;
						
	var ipWrap   = $('<div>').addClass('tele-ip-wrap');
	var ipStart  = $('<div>').addClass('tele-ip').ip({ data: data.from});
	var ipDash   = $('<div>').addClass('tele-ip-dash').html('_');
	
	var ipEnd    = $('<div>').addClass('tele-ip').ip({ data: !is_range?'':data.to});
	
	if(!is_range) {

		ipDash.hide();
		ipEnd.hide();
	}
	
	var ipAdd = $('<div>').addClass('tele-ip-add')
            .addClass('tele-icon')
							 .addClass('tele-icon-plus')
            .hover(function () { $(this).addClass('hover'); },
                function () { $(this).removeClass('hover'); })
							 .click(function () { container.append(getRangeUI()); });
	
	var ipRemove = $('<div>').addClass('tele-ip-remove')
            .addClass('tele-icon')
							 .addClass('tele-icon-minus')
            .hover(function () { $(this).addClass('hover'); },
                function () { $(this).removeClass('hover'); })
							 .click(function () { RangeRemove(this); /* $(this).parent().remove(); */ });
							 
	var ipToggle = $('<div>').toggleFlip({ 
	
		left_value: 'Single', 
		right_value: 'Range',
		flip: function () {
			ipEnd.toggle();
			ipDash.toggle();
		},
		flipped: is_range
	});

  
	ipWrap.append(ipAdd).append(ipRemove).append(ipToggle).append(ipStart).append(ipDash).append(ipEnd);
	return ipWrap;
	
}

function getStatusCodeRangeUI(data) {

  if (!data) {
    data = '';
  } else {
    data = data.trim()
  }

  var is_range = data.split('-').length > 1;

  var scWrap = $('<div>').addClass('tele-sc-wrap');
  var scStart = $('<div>').teleInput({
    value: data.split('-')[0],
    type: 'number',
    range: {min: 100, max: 600},
    step: 100,
    margintop: 1
  }).addClass('tele-sc');
  var scDash = $('<div>').addClass('tele-sc-dash').html('_');
  var scEnd = $('<div>').teleInput({
    value: !is_range ? '' : data.split('-')[1],
    type: 'number',
    range: {min: 100, max: 600}, step: 100, margintop: 1
  }).addClass('tele-sc');

  if (!is_range) {
    scDash.hide();
    scEnd.hide();
  }


  var scToggle = $('<div>').toggleFlip({

    left_value: 'Single',
    right_value: 'Range',
    flip: function () {
      scEnd.toggle();
      scDash.toggle();
    },
    flipped: is_range
  });


  scWrap.append(scToggle).append(scStart).append(scDash).append(scEnd);
  return scWrap;

}

function getRangeLB(data, container) {

  if(!data) { data = ''; }

  var is_range= data.from!=data.to;

  //var to = data.to?data.to:data.from;

  var ipWrap   = $('<div>').addClass('tele-ip-wrap-lb');
  var ipStart  = $('<div>').addClass('tele-ip').ip({ data: data.from});
  var ipDash   = $('<div>').addClass('tele-ip-dash').html('_');

  var ipEnd    = $('<div>').addClass('tele-ip').ip({ data: !is_range?'':data.to});

  if(!is_range) {

    ipDash.hide();
    ipEnd.hide();
  }

  var ipAdd = $('<div>').addClass('tele-ip-add')
      .addClass('tele-icon')
      .addClass('tele-icon-plus')
      .hover(function () { $(this).addClass('hover'); },
      function () { $(this).removeClass('hover'); })
      .click(function () { container.append(getRangeLB()); });

  var ipRemove = $('<div>').addClass('tele-ip-remove')
      .addClass('tele-icon')
      .addClass('tele-icon-minus')
      .hover(function () { $(this).addClass('hover'); },
      function () { $(this).removeClass('hover'); })
      .click(function () { RangeRemoveLB(this); /* $(this).parent().remove(); */ });

  var ipToggle = $('<div>').toggleFlip({

    left_value: 'Single',
    right_value: 'Range',
    flip: function () {
      ipEnd.toggle();
      ipDash.toggle();
    },
    flipped: is_range
  });


  ipWrap.append(ipAdd).append(ipRemove).append(ipToggle).append(ipStart).append(ipDash).append(ipEnd);
  return ipWrap;

}
				
function findAndRemove(array, property, value) {
   $.each(array, function(index, result) {
      if(result && result[property] && result[property] == value) {
          //Remove from array
          array.splice(index, 1);
      }    
   });
}

function grabNames(arr) {
	
	var result = '';
	$.each(arr, function(i, x) {
		result = result + x.key + ', ';
	});
	if(result.length > 0) { result = result.substr(0, result.length - 2); }
	return result;
		
}

function ip2long (IP) {
  // From: http://phpjs.org/functions
  // +   original by: Waldo Malqui Silva
  // +   improved by: Victor
  // +    revised by: fearphage (http://http/my.opera.com/fearphage/)
  // +    revised by: Theriault
  // *     example 1: ip2long('192.0.34.166');
  // *     returns 1: 3221234342
  // *     example 2: ip2long('0.0xABCDEF');
  // *     returns 2: 11259375
  // *     example 3: ip2long('255.255.255.256');
  // *     returns 3: false
  var i = 0;
  // PHP allows decimal, octal, and hexadecimal IP components.
  // PHP allows between 1 (e.g. 127) to 4 (e.g 127.0.0.1) components.
  IP = IP.match(/^([1-9]\d*|0[0-7]*|0x[\da-f]+)(?:\.([1-9]\d*|0[0-7]*|0x[\da-f]+))?(?:\.([1-9]\d*|0[0-7]*|0x[\da-f]+))?(?:\.([1-9]\d*|0[0-7]*|0x[\da-f]+))?$/i); // Verify IP format.
  if (!IP) {
    return false; // Invalid format.
  }
  // Reuse IP variable for component counter.
  IP[0] = 0;
  for (i = 1; i < 5; i += 1) {
    IP[0] += !! ((IP[i] || '').length);
    IP[i] = parseInt(IP[i]) || 0;
  }
  // Continue to use IP for overflow values.
  // PHP does not allow any component to overflow.
  IP.push(256, 256, 256, 256);
  // Recalculate overflow of last component supplied to make up for missing components.
  IP[4 + IP[0]] *= Math.pow(256, 4 - IP[0]);
  if (IP[1] >= IP[5] || IP[2] >= IP[6] || IP[3] >= IP[7] || IP[4] >= IP[8]) {
    return false;
  }
  return IP[1] * (IP[0] === 1 || 16777216) + IP[2] * (IP[0] <= 2 || 65536) + IP[3] * (IP[0] <= 3 || 256) + IP[4] * 1;
}

function date_format(format, timestamp) {
  //  discuss at: http://phpjs.org/functions/date/
  // original by: Carlos R. L. Rodrigues (http://www.jsfromhell.com)
  // original by: gettimeofday
  //    parts by: Peter-Paul Koch (http://www.quirksmode.org/js/beat.html)
  // improved by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // improved by: MeEtc (http://yass.meetcweb.com)
  // improved by: Brad Touesnard
  // improved by: Tim Wiel
  // improved by: Bryan Elliott
  // improved by: David Randall
  // improved by: Theriault
  // improved by: Theriault
  // improved by: Brett Zamir (http://brett-zamir.me)
  // improved by: Theriault
  // improved by: Thomas Beaucourt (http://www.webapp.fr)
  // improved by: JT
  // improved by: Theriault
  // improved by: Rafa? Kukawski (http://blog.kukawski.pl)
  // improved by: Theriault
  //    input by: Brett Zamir (http://brett-zamir.me)
  //    input by: majak
  //    input by: Alex
  //    input by: Martin
  //    input by: Alex Wilson
  //    input by: Haravikk
  // bugfixed by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // bugfixed by: majak
  // bugfixed by: Kevin van Zonneveld (http://kevin.vanzonneveld.net)
  // bugfixed by: Brett Zamir (http://brett-zamir.me)
  // bugfixed by: omid (http://phpjs.org/functions/380:380#comment_137122)
  // bugfixed by: Chris (http://www.devotis.nl/)
  //        note: Uses global: php_js to store the default timezone
  //        note: Although the function potentially allows timezone info (see notes), it currently does not set
  //        note: per a timezone specified by date_default_timezone_set(). Implementers might use
  //        note: this.php_js.currentTimezoneOffset and this.php_js.currentTimezoneDST set by that function
  //        note: in order to adjust the dates in this function (or our other date functions!) accordingly
  //   example 1: date('H:m:s \\m \\i\\s \\m\\o\\n\\t\\h', 1062402400);
  //   returns 1: '09:09:40 m is month'
  //   example 2: date('F j, Y, g:i a', 1062462400);
  //   returns 2: 'September 2, 2003, 2:26 am'
  //   example 3: date('Y W o', 1062462400);
  //   returns 3: '2003 36 2003'
  //   example 4: x = date('Y m d', (new Date()).getTime()/1000);
  //   example 4: (x+'').length == 10 // 2009 01 09
  //   returns 4: true
  //   example 5: date('W', 1104534000);
  //   returns 5: '53'
  //   example 6: date('B t', 1104534000);
  //   returns 6: '999 31'
  //   example 7: date('W U', 1293750000.82); // 2010-12-31
  //   returns 7: '52 1293750000'
  //   example 8: date('W', 1293836400); // 2011-01-01
  //   returns 8: '52'
  //   example 9: date('W Y-m-d', 1293974054); // 2011-01-02
  //   returns 9: '52 2011-01-02'

  var that = this;
  var jsdate, f;
  // Keep this here (works, but for code commented-out below for file size reasons)
  // var tal= [];
  var txt_words = [
    'Sun', 'Mon', 'Tues', 'Wednes', 'Thurs', 'Fri', 'Satur',
    'January', 'February', 'March', 'April', 'May', 'June',
    'July', 'August', 'September', 'October', 'November', 'December'
  ];
  // trailing backslash -> (dropped)
  // a backslash followed by any character (including backslash) -> the character
  // empty string -> empty string
  var formatChr = /\\?(.?)/gi;
  var formatChrCb = function(t, s) {
    return f[t] ? f[t]() : s;
  };
  var _pad = function(n, c) {
    n = String(n);
    while (n.length < c) {
      n = '0' + n;
    }
    return n;
  };
  f = {
    // Day
    d: function() { // Day of month w/leading 0; 01..31
      return _pad(f.j(), 2);
    },
    D: function() { // Shorthand day name; Mon...Sun
      return f.l()
        .slice(0, 3);
    },
    j: function() { // Day of month; 1..31
      return jsdate.getDate();
    },
    l: function() { // Full day name; Monday...Sunday
      return txt_words[f.w()] + 'day';
    },
    N: function() { // ISO-8601 day of week; 1[Mon]..7[Sun]
      return f.w() || 7;
    },
    S: function() { // Ordinal suffix for day of month; st, nd, rd, th
      var j = f.j();
      var i = j % 10;
      if (i <= 3 && parseInt((j % 100) / 10, 10) == 1) {
        i = 0;
      }
      return ['st', 'nd', 'rd'][i - 1] || 'th';
    },
    w: function() { // Day of week; 0[Sun]..6[Sat]
      return jsdate.getDay();
    },
    z: function() { // Day of year; 0..365
      var a = new Date(f.Y(), f.n() - 1, f.j());
      var b = new Date(f.Y(), 0, 1);
      return Math.round((a - b) / 864e5);
    },

    // Week
    W: function() { // ISO-8601 week number
      var a = new Date(f.Y(), f.n() - 1, f.j() - f.N() + 3);
      var b = new Date(a.getFullYear(), 0, 4);
      return _pad(1 + Math.round((a - b) / 864e5 / 7), 2);
    },

    // Month
    F: function() { // Full month name; January...December
      return txt_words[6 + f.n()];
    },
    m: function() { // Month w/leading 0; 01...12
      return _pad(f.n(), 2);
    },
    M: function() { // Shorthand month name; Jan...Dec
      return f.F()
        .slice(0, 3);
    },
    n: function() { // Month; 1...12
      return jsdate.getMonth() + 1;
    },
    t: function() { // Days in month; 28...31
      return (new Date(f.Y(), f.n(), 0))
        .getDate();
    },

    // Year
    L: function() { // Is leap year?; 0 or 1
      var j = f.Y();
      return j % 4 === 0 & j % 100 !== 0 | j % 400 === 0;
    },
    o: function() { // ISO-8601 year
      var n = f.n();
      var W = f.W();
      var Y = f.Y();
      return Y + (n === 12 && W < 9 ? 1 : n === 1 && W > 9 ? -1 : 0);
    },
    Y: function() { // Full year; e.g. 1980...2010
      return jsdate.getFullYear();
    },
    y: function() { // Last two digits of year; 00...99
      return f.Y()
        .toString()
        .slice(-2);
    },

    // Time
    a: function() { // am or pm
      return jsdate.getHours() > 11 ? 'pm' : 'am';
    },
    A: function() { // AM or PM
      return f.a()
        .toUpperCase();
    },
    B: function() { // Swatch Internet time; 000..999
      var H = jsdate.getUTCHours() * 36e2;
      // Hours
      var i = jsdate.getUTCMinutes() * 60;
      // Minutes
      var s = jsdate.getUTCSeconds(); // Seconds
      return _pad(Math.floor((H + i + s + 36e2) / 86.4) % 1e3, 3);
    },
    g: function() { // 12-Hours; 1..12
      return f.G() % 12 || 12;
    },
    G: function() { // 24-Hours; 0..23
      return jsdate.getHours();
    },
    h: function() { // 12-Hours w/leading 0; 01..12
      return _pad(f.g(), 2);
    },
    H: function() { // 24-Hours w/leading 0; 00..23
      return _pad(f.G(), 2);
    },
    i: function() { // Minutes w/leading 0; 00..59
      return _pad(jsdate.getMinutes(), 2);
    },
    s: function() { // Seconds w/leading 0; 00..59
      return _pad(jsdate.getSeconds(), 2);
    },
    u: function() { // Microseconds; 000000-999000
      return _pad(jsdate.getMilliseconds() * 1000, 6);
    },

    // Timezone
    e: function() { // Timezone identifier; e.g. Atlantic/Azores, ...
      // The following works, but requires inclusion of the very large
      // timezone_abbreviations_list() function.
      /*              return that.date_default_timezone_get();
       */
      throw 'Not supported (see source code of date() for timezone on how to add support)';
    },
    I: function() { // DST observed?; 0 or 1
      // Compares Jan 1 minus Jan 1 UTC to Jul 1 minus Jul 1 UTC.
      // If they are not equal, then DST is observed.
      var a = new Date(f.Y(), 0);
      // Jan 1
      var c = Date.UTC(f.Y(), 0);
      // Jan 1 UTC
      var b = new Date(f.Y(), 6);
      // Jul 1
      var d = Date.UTC(f.Y(), 6); // Jul 1 UTC
      return ((a - c) !== (b - d)) ? 1 : 0;
    },
    O: function() { // Difference to GMT in hour format; e.g. +0200
      var tzo = jsdate.getTimezoneOffset();
      var a = Math.abs(tzo);
      return (tzo > 0 ? '-' : '+') + _pad(Math.floor(a / 60) * 100 + a % 60, 4);
    },
    P: function() { // Difference to GMT w/colon; e.g. +02:00
      var O = f.O();
      return (O.substr(0, 3) + ':' + O.substr(3, 2));
    },
    T: function() { // Timezone abbreviation; e.g. EST, MDT, ...
      // The following works, but requires inclusion of the very
      // large timezone_abbreviations_list() function.
      /*              var abbr, i, os, _default;
      if (!tal.length) {
        tal = that.timezone_abbreviations_list();
      }
      if (that.php_js && that.php_js.default_timezone) {
        _default = that.php_js.default_timezone;
        for (abbr in tal) {
          for (i = 0; i < tal[abbr].length; i++) {
            if (tal[abbr][i].timezone_id === _default) {
              return abbr.toUpperCase();
            }
          }
        }
      }
      for (abbr in tal) {
        for (i = 0; i < tal[abbr].length; i++) {
          os = -jsdate.getTimezoneOffset() * 60;
          if (tal[abbr][i].offset === os) {
            return abbr.toUpperCase();
          }
        }
      }
      */
      return 'UTC';
    },
    Z: function() { // Timezone offset in seconds (-43200...50400)
      return -jsdate.getTimezoneOffset() * 60;
    },

    // Full Date/Time
    c: function() { // ISO-8601 date.
      return 'Y-m-d\\TH:i:sP'.replace(formatChr, formatChrCb);
    },
    r: function() { // RFC 2822
      return 'D, d M Y H:i:s O'.replace(formatChr, formatChrCb);
    },
    U: function() { // Seconds since UNIX epoch
      return jsdate / 1000 | 0;
    }
  };
  this.date = function(format, timestamp) {
    that = this;
    jsdate = (timestamp === undefined ? new Date() : // Not provided
      (timestamp instanceof Date) ? new Date(timestamp) : // JS Date()
        new Date(timestamp * 1000) // UNIX timestamp (auto-convert to int)
      );
    return format.replace(formatChr, formatChrCb);
  };
  return this.date(format, timestamp);
}

// Use the browser's built-in functionality to quickly and safely escape the
// string
function escapeHtml(str) {
  var div = document.createElement('div');
  div.appendChild(document.createTextNode(str));
  return div.innerHTML;
};

// UNSAFE with unsafe strings; only use on previously-escaped ones!
function unescapeHtml(escapedStr) {
  var div = document.createElement('div');
  div.innerHTML = escapedStr;
  var child = div.childNodes[0];
  return child ? child.nodeValue : '';
};
