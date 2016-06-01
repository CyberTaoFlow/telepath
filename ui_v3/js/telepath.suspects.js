telepath.suspects = {
	
	sort: 'date',
	dir: false,
	data: [],
	total: 0,
	searchString: '',

	rowFormatter: function(item,mode) {

		if (mode=='dashboard') {
			var identification = (item.user!='' ? {key: 'user', value: item.user } : { key: 'IP', value: item.ip_orig } );
			result = {
				raw: item,
				icon: 'suspect',
				time: item.ts,
				progbar: true,
				itemID: item.sid,
				progbarBig: item.progbarBig,
				checkable: item.checkable,
				checked: item.checked,
				count: item.count,
				progbarValue: item.ip_score,
				time: item.date,
				details: [
					{ key: 'country', value: item.country },
					{ key: 'host', value: grabNames(item.host) },
					identification
				]
			}
		}
		else {
			result = {
				raw: item,
				icon: 'suspect',
				time: item.ts,
				progbar: true,
				itemID: item.sid,
				progbarBig: item.progbarBig,
				checkable: item.checkable,
				checked: item.checked,
				count: item.count,
				progbarValue: item.ip_score,
				time: item.date,
				details: [
					{key: 'country', value: item.country},
					{key: 'IP', value: item.ip_orig},
					{key: 'city', value: item.city},
					{key: 'host', value: grabNames(item.host)},
					{key: 'alerts', value: item.alerts_count},
					{key: 'actions', value: item.actions_count},
					{key: 'cases', value: item.cases_count },
					{key: 'user', value: item.user }
				]
			}
		}

		/*if (item.business_action && item.business_action.length>0){
		 result.details.push({key: 'business_actions', value: item.business_actions[0].key})
		 }*/

		return result;
	},
	init: function () {
		
		var that = this;
		
		// Cleanup
		this.container = $('.tele-panel-suspects');
		this.container.empty();
		
		// Build UI
		this.panelTopBar  = $('<div>').addClass('tele-panel-topbar');
		this.panelTitle   = $('<div>').addClass('tele-panel-title');
		this.panelTopBar.append(this.panelTitle);
		this.panelTitle.html('Loading Suspects');
		this.container.append(this.panelTopBar);
		this.panelSubBar = $('<div>').addClass('tele-panel-subtitle');
		this.panelTopBarRight = $('<div>').addClass('tele-panel-topbar-right');
		this.panelTopBar.append(this.panelTopBarRight);
		this.container.append(this.panelSubBar);
		
		// DateRange
		var filterDateRange 	  = $('<div>').daterange({ 
			
			start: telepath.range.start, 
			end: telepath.range.end, 
			change: function(start, end) { 
			
				telepath.suspects.refresh(function () {	});
			
		}});

		// Sort filters
		var sortRadios = $('<div>').radios({
			title: 'Sort By',
			items: [
				{id: 'date', icon: 'time', tip: 'Time'},
				{id: 'count', icon: 'bars', tip: 'Count'}
			],
			selected: this.sort,
			callback: function(e, id) {
				if(that.sort == id) {
					that.dir = !that.dir;
				}
				that.sort = id;
				that.refresh();
			}
		});
		this.panelTopBarRight.append(sortRadios).append('<div class="tele-navsep"></div>');
		
		// Applications
		var filterApps		     = $('<div>').appSelect({ callback: function (app_id) {
			$('.tele-icon-application', filterApps).removeClass('tele-icon-application').addClass('tele-icon-loader');
			telepath.suspects.refresh(function () {
				$('.tele-icon-loader', filterApps).removeClass('tele-icon-loader').addClass('tele-icon-application');
			});
		}});
		
		// Append tools
		this.panelTopBarRight.append(filterDateRange).append('<div class="tele-navsep"></div>').append(filterApps);
		
		// Sub bar items
		// --------------------------------------
		
		// Select all cases
/*		var checkallEl = $('<a>').teleCheckbox({ callback: function (e) {
			$('.tele-suspects-alerts .tele-list li.tele-listitem').listitem("option", "checked", e.options.checked);
		}});
		this.panelSubBar.append(checkallEl);

		// Harmful
		var harmfulBtn = $('<div>').btn({ icon: 'harmful', text: 'Mark as Harmful', callback: function () { console.log('HH'); }});
		this.panelSubBar.append(harmfulBtn);
		
		this.panelSubBar.append('<div class="tele-navsep"></div>'); // Sep
		
		// Safe
		var safeBtn = $('<div>').btn({ icon: 'safe', text: 'Not harmful', callback: function () { console.log('NH'); }});
		this.panelSubBar.append(safeBtn);
		
		this.panelSubBar.append('<div class="tele-navsep"></div>'); // Sep
		
		// Archive cases
		var archiveBtn = $('<div>').btn({ icon: 'archive', text: 'Archive', callback: function () { console.log('Archive'); }});
		this.panelSubBar.append(archiveBtn);
		
		this.panelSubBar.append('<div class="tele-navsep"></div>'); // Sep*/

		
		// Search
		var searchSuspects = $('<div>').teleSearch({ callback: function (e, txt) {
			telepath.suspects.searchString = txt;
			//telepath.suspects.refresh(function () {	});
		}});

		
		var resetInput=$('<a>').addClass('icon-delete-input2').attr('id', 'remove-button').click(function(){
			$('.tele-panel-suspects .tele-search-input').val('');
			telepath.suspects.searchString = '';
			telepath.suspects.refresh();
			// console.log('Delete')
		});
		//searchSuspects.append(resetInput);

		this.panelSubBar.append(searchSuspects);
		// Load Data
		this.refresh();

		var typingTimer;                //timer identifier
		var doneTypingInterval = 1000;

		$(".tele-panel-suspects .tele-search-input").keyup('input', function () {
			clearTimeout(typingTimer);
			if ($('.tele-panel-suspects .tele-search-input').val()){
				typingTimer = setTimeout(function(){
					that.searchString = $('.tele-panel-suspects .tele-search-input').val();
					that.input();
				}, doneTypingInterval);
			}
		});

		$("#search-button").on("click", function (event) {
			that.searchString = '';
			$(".tele-panel-suspects .tele-search-input").prop("value", that.searchString);
			that.input();

		});

		if (that.searchString)
		{
			$('.tele-panel-suspects .tele-search-input').prop("value",that.searchString);

			that.input();
		}
		
	},

	input: function(){
		var that = this;
		var icon= $("#search-button");
		if (that.searchString.length>0)
			icon.addClass('icon-delete-input2').removeClass("tele-search-button");
		else
			icon.removeClass('icon-delete-input2').addClass("tele-search-button");

		that.refresh()

	},

	refresh: function (callback) {
		
		$('.tele-block, .tele-loader', this.container).remove();
		this.container.append(telepath.loader);
		
		var that = this;
		
		telepath.ds.get('/suspects/index', {
			sort: this.sort,
			dir: this.dir,
			search: this.searchString
		}, function (data) {
			
			that.count = data.count;
			that.data  = data.items;
			
			telepath.suspects.loadData();
			
			if(typeof(callback) == 'function') {
				callback();
			}
			
		});

	},
	_resize: function () {
		
		if($('.tele-panel-suspects').children().size() == 0) return;
		
		var height = $(window).height();
		
		$('.tele-body').css({ height: height });
		
		var offset = height - 
					 $('.tele-header').outerHeight() - 
					 $('.tele-panel-topbar').outerHeight() - 
					 $('.tele-panel-subtitle').outerHeight();
		
		$('.tele-panel-suspects .tele-block').height(offset - 20);
		$('.tele-panel-suspects .tele-block .tele-list').height(offset - 50);
		$('.tele-list').mCustomScrollbar("update");
		
	},
	loadData: function() {
		
		var that = this;
		
		// Cleanup
		$('.tele-block, .tele-loader', this.container).remove();
		this.suspectsList = $('<div>');
		this.container.append(this.suspectsList);



		// Init Suspects
		this.suspectsList.teleList({ context: "panel", data: this.data, searchkey: telepath.suspects.searchString,
			formatter: function(item) {

			//item.checkable = true;
			return telepath.suspects.rowFormatter(item);

		}, callbacks: { scroll: function (offset, callback) {

				telepath.ds.get('/suspects/index', {
					sort:   telepath.suspects.sort,
					dir:    telepath.suspects.dir,
					search: telepath.suspects.searchString,
					offset: offset,
					}, function (data) {
						callback(data);
				});
			}
		} });
		
		this.panelTitle.html( this.count + ' Sessions');
		
		// Resize hooks
		setTimeout(function () { that._resize(); }, 0);
		$(window).resize(function () { that._resize() });
		
	}
}
