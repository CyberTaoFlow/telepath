
telepath.header = {

	getSearchOpts: function () {
	
	},
	init: function () {

		var that = this;
		// Start Build Header

		// LOGO
		this.logo = '<div class="tele-logo"><img src="img/logo.png" alt="Hybrid Security Telepath" /></div>';

		// NAV
		this.nav = $('<ul>').addClass('tele-nav');
		this.navItems = [
			{id: 'dashboard', label: 'Dashboard'},
			{id: 'cases', label: 'Cases'},
			{id: 'alerts', label: 'Alerts'},
			{id: 'suspects', label: 'Suspects'}
			//{ id: 'reports',   label: 'Reports'   }
		];
		$.each(this.navItems, function (i, item) {
			if (telepath.access.admin || telepath.access.perm[item.label + '_get']) {
				var itemEl = $('<li>').addClass('tele-nav-' + item.id).append($('<a>').attr('href', '#').append('<span class="tele-nav-icon">').append(item.label)); //	
			}
			;

			$(that.nav).append(itemEl);
		});

		// SEARCH -- Perhaps move this code to search panel?

		this.headerRight = $('<div>').addClass('tele-header-right');
		this.search = $('<div>').addClass('tele-search-top');
		this.searchInput = $('<input>').addClass('tele-search-input');
		this.searchIcon = $('<a href="#">').addClass('tele-search-icon').html('Search');
		this.searchDD = $('<a href="#">').addClass('tele-search-dropdown').html('');

		this.search.append(this.searchInput).append(this.searchIcon).append(this.searchDD);
		this.headerRight.append(this.search);

		// Hook for icon click
		$(this.searchIcon).click(function (e) {
			telepath.search.init(telepath.header.searchInput.val());
			telepath.ui.resize();
		});

		// Hook for enter key
		$(this.searchInput).keydown(function (e) {
			if (e.keyCode == 13) {
				telepath.search.init(telepath.header.searchInput.val());
				telepath.ui.resize();
			}
		});

		// Hook for dropdown arrow
		this.searchDD.click(function () {

			if ($('.tele-search-filters').size() > 0) {
				$('.tele-search-filters').remove();
				return;
			}

			that.searchWrap = $('<div>').addClass('tele-search-filters').addClass('tele-popup');

			var title = $('<div>').addClass('tele-title-1').html('What to search');
			that.searchWrap.append(title);
			that.headerRight.append(that.searchWrap);
			telepath.search.printTypes(that.searchWrap);

		});

		
		$('.tele-header').append(this.logo).append(this.nav).append(this.headerRight);
		// End Build Header
		
		
		telepath.header.bindHooks();
		
		// Bind resize event
		$(window).resize(function () { telepath.header.resize(); });
		telepath.header.resize();
		
		// Notifications
		this.notifications = $('<div>').notifications();
		
		// Config Button
		this.configDiv = $('<div>').addClass('tele-config');
		this.configCmd = $('<a>').attr('href', '#').addClass('tele-icon').addClass('tele-icon-config');
		this.configDiv.append(this.configCmd);
		
		this.configCmd.click(function () {
			
			// Cleanup other panels
			$('.tele-panel').empty().hide();
			$("#file-upload").hide();
			$('.tele-nav a.active').removeClass('active');
			
			// Trigger this panel
			var id = 'config';
			telepath[id].init();
			$('.tele-panel-' + id).show();
			$(this).addClass('active');
	
		}).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover'); });
		
		this.logoutCmd = $('<a>').attr('href', '#').addClass('tele-icon').addClass('tele-icon-logout').click(function () {
			
			telepath.ds.get('/auth/logout', {}, function(data) { 
				location.reload(true);
			}, 'Error logging out.');
		
		}).hover(function () { $(this).addClass('hover'); }, function () { $(this).removeClass('hover'); });
		
		// Append
		$('.tele-header-right').append(this.notifications).append(this.configDiv).append(this.logoutCmd);
		
	},
	bindHooks: function () {
		
		/* Navigation */
		$('.tele-nav a').click(function () {
		
			var id = $(this).parent().attr('class').split('-');

			if(id[0] == 'tele' && id[1] == 'nav' && id[2] != '') {
				id = id[2];
			} else {
				return false;
			}
			
			if(id == 'dashboard' && telepath.dashboard.loading) {
				return;
			}
			
			$('.tele-panel').empty().hide().removeClass('active');
			$("#file-upload").hide();
			$('.tele-panel-' + id).show().addClass('active');
			telepath.ui.resize();
			telepath[id].init();
			
		
			telepath.header.configCmd.removeClass('active');
			$('.tele-nav a.active').removeClass('active');
			$(this).addClass('active');
			
			setTimeout(function () {
				$('.tele-popup, .popover').remove();
			}, 100);
			
		});
		
	},
	resize: function () {
		
		// Nav
		var width = $(window).width();
		
		$('.tele-logo').css({ width: 200, overflow: 'hidden' });
		
		if(width < 1300) {
		
			$(this.searchInput).css({ width: 100 });
			
			if(width < 1200) {
				if(width < 1000) {
					$('.tele-logo').css({ width: 65, overflow: 'hidden' });
					$('.tele-nav li').css({ margin: '0px 0px' });
				} else {
					$('.tele-nav li').css({ margin: '0px 10px' });
				}
				$('.tele-logo').css({ width: 65 });
			} else {
				$('.tele-nav li').css({ margin: '0px 25px' });
			}
			
		} else {
			$(this.search).css({input:'{padding: 5px 75px 5px 20px}' });
		
			$(this.searchInput).css({ width: 140 });

			$('.tele-nav li').css({ margin: '0px 25px' });
			
		}
		
		var navMargin = (width - $('.tele-header-right').width() - $('.tele-logo').width() - $('.tele-nav').width()) / 2;
		$('.tele-nav').css('marginLeft', navMargin);

		if(width < 801) {
			$('.tele-nav').css('marginLeft', 20);
			$('.tele-panel-subtitle').css('height', '100px !important');
			$('.tele-panel-title').css('position', 'absolute');
		}
		
	}
}