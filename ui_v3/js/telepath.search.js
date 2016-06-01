telepath.search = {

    results: {},
    defaults: {
        'application': true,
        'pages': true,
        'attributes': true,
        'requests': true,
        'suspects': true,
        'alerts': true,
        'cases': true,
        'request_data': true,
        'users': false
    },
    sort: 'date',
    dir: false,

    options: false,
    searchTypes: [
        {id: 'cases', label: 'Request Data', desc: 'Search Cases'},					// Scope
        {id: 'alerts', label: 'Applications', desc: 'Search Alerts'},					// Scope
        {id: 'suspects', label: 'Request Data', desc: 'Search suspects'},				// Scope
        {id: 'requests', label: 'Requests', desc: 'Search Requests'},					// Scope
        {id: 'request_data', label: 'Request Data', desc: 'Search request data'}, 	// Variant TODO:: See performance cost to have these on by default
        {id: 'application', label: 'Applications', desc: 'Search domain names'},		// Variant
        {id: 'pages', label: 'Applications', desc: 'Search application page names'},	// Variant
        {id: 'attributes', label: 'Applications', desc: 'Search attribute names'},	// Variant
        {id: 'users', label: 'Request Data', desc: 'Search web application users'},	// Variant
        {id: 'users', label: 'Request Data', desc: 'Search in countries and cities'}	// Variant
    ],


    printTypes: function (element) {

        var that = this;

        /*if(!telepath.access.perm.Cases_get|| !telepath.access.admin) {
         that.searchTypes.push({id: 'cases', label: 'Request Data', desc: 'Search Cases' });
         this.defaults.cases=false;
         }


         if (!telepath.access.perm.Alerts_get|| !telepath.access.admin){
         that.searchTypes.push({ id: 'alerts', label: 'Applications', desc: 'Search Alerts' });
         this.defaults.alerts=false;
         }

         if (!telepath.access.perm.Suspects_get|| !telepath.access.admin){
         that.searchTypes.push({ id: 'suspects', label: 'Request Data', desc: 'Search suspects' });
         this.defaults.suspects=false;
         }*/

        if (this.options === false) {
            this.options = this.defaults; // USE $.extend
        }


        $.each(that.searchTypes, function (i, data) {

            var wrap = $('<div>').addClass('tele-search-filter').attr('id', 'tele-search-filter-' + data.id);
            //var title = $('<div>').addClass('tele-title-2').html(data.label);

            var cb = $('<div>').addClass('tele-search-filter').teleCheckbox({
                label: data.desc,
                checked: that.options[data.id] ? that.options[data.id] : false,
                callback: function (widget) {

                    that.options[data.id] = widget.options.checked;
                    // console.log(that.options);

                }
            });

            wrap.append(cb);
            element.append(wrap);

        });

        that.buttonsEl = $('<div>').addClass('tele-form-buttons');
        that.applyBtn = $('<a href="#" class="tele-button tele-button-apply">Apply</a>');
        that.cancelBtn = $('<a href="#" class="tele-button tele-button-cancel">Cancel</a>');

        that.buttonsEl.append(that.applyBtn).append(that.cancelBtn);
        element.append(that.buttonsEl);

        // BIND Validate
        that.applyBtn.click(function () {
            $('.tele-search-filters').remove();
        });

        // BIND Cancel -- Simply reload
        that.cancelBtn.click(function () {
            that.options = that.defaults; // USE $.extend
            $('.tele-search-filters').remove();
        });

    },

    searchStr: '',
    init: function (searchStr) {

        this.searchStr = searchStr;

        // Reset nav states, clear panels, set container
        telepath.header.configCmd.removeClass('active');
        $('.tele-nav a.active').removeClass('active');
        $('.tele-panel').empty().hide().removeClass('active');
        $("#file-upload").hide();
        $('.tele-panel-search').show().addClass('active');
        this.container = $('.tele-panel-search');
        this.container.empty();

        this.initPanel();

    },
    initPanel: function () {

        var that = this;

        // Build UI
        this.panelTopBar = $('<div>').addClass('tele-panel-topbar');
        this.panelTitle = $('<div>').addClass('tele-panel-title');
        this.panelTopBar.append(this.panelTitle);
        this.panelTitle.html('Search results');
        this.container.append(this.panelTopBar);
        this.panelSubBar = $('<div>').addClass('tele-panel-subtitle');
        this.panelTopBarRight = $('<div>').addClass('tele-panel-topbar-right');
        this.panelTopBar.append(this.panelTopBarRight);
        this.container.append(this.panelSubBar);

        // Sort filters
        var sortRadios = $('<div>').radios({
            title: 'Sort By',
            items: [
                {id: 'date', icon: 'time', tip: 'Time'},
                {id: 'count', icon: 'bars', tip: 'Count'},
                //{id: 'score', icon: 'alerts', tip: 'Score'}
            ],
            selected: this.sort,
            callback: function(e, id) {
                if(that.sort == id) {
                    that.dir = !that.dir;
                }
                that.sort = id;

                telepath.search.refresh(function () {
                });
            }
        });


        // DateRange
        var filterDateRange = $('<div>').daterange({

            start: telepath.range.start,
            end: telepath.range.end,
            change: function (start, end) {

                telepath.range.start = start;
                telepath.range.end = end;

                //that.boll=false;
                //Reset tab result count to 0
                $('.tele-search-tab span').html('0');
                telepath.search.refresh(function () {
                });

            }
        });

        // Applications
        var filterApps = $('<div>').appSelect({
            callback: function (app_id) {
                $('.tele-icon-application', filterApps).removeClass('tele-icon-application').addClass('tele-icon-loader');
                telepath.search.refresh(function () {
                    $('.tele-icon-loader', filterApps).removeClass('tele-icon-loader').addClass('tele-icon-application');
                });
            }
        });

        // Append tools
        this.panelTopBarRight.append(sortRadios).append('<div class="tele-navsep"></div>').append(filterDateRange).append('<div class="tele-navsep"></div>').append(filterApps);

        // TABS
        // --------------------------------

        // Tab Containers
        this.tabsEl = $('<div>').addClass('tabs');
        this.tabsUl = $('<ul>');
        this.tabsEl.append(this.tabsUl);

        // Tab Declaration
        var tabs = [
            {id: 'alerts', text: 'Alerts'},
            {id: 'cases', text: 'Cases'},
            {id: 'suspects', text: 'Suspects'},
            {id: 'requests', text: 'Normal'},
        ];

        // Tab Print
        for (x in tabs) {

            var tab = tabs[x];
            var tabEl = $('<div>').attr('id', 'tele-search-' + tab.id);
            // Show Loading
            //tabEl.append('<img class="loader" src="img/loader.gif">');
            var tabLi = $('<li>');
            var tabCount = $('<span>').html('0');
            var tabA = $('<a>').attr('href', '#tele-search-' + tab.id).append(tab.text + '&nbsp;(').append(tabCount).append(')').attr('rel', tab.id).addClass('tele-search-tab');
            tabLi.append(tabA);
            this.tabsUl.append(tabLi);
            this.tabsEl.append(tabEl);

        }

        // Append our tabs
        this.container.append(this.tabsEl);

        this.tabsEl.height(
            $(window).height() -
            $('.tele-header').height() -
            $('.tele-panel-topbar').height() -
            $('.tele-panel-subtitle').height() - 60
        );

        // Init tabs
        this.tabsEl.tabs({

            heightStyle: 'fill',
            autoHeight: false,
            animate: false,
            activate: function (event, ui) {

                // Hook to activate panels and populate data
                var id = ui.newPanel.selector.split('-')[2];
                that.container = $(ui.newPanel.selector);
                // empty container, Yuli
                var teleBlock = $(ui.newPanel.selector + ' .tele-block');
                teleBlock.remove();
                //alert('empty!');
                //that.container.empty();

                switch (id) {

                    case 'cases':
                        that.showCasesTab(ui.newPanel.selector);
                        break;
                    case 'alerts':
                        that.showAlertsTab(ui.newPanel.selector);
                        break;
                    case 'suspects':
                        that.showSuspectsTab(ui.newPanel.selector);
                        break;
                    case 'requests':
                        that.showRequestsTab(ui.newPanel.selector);
                        break;

                }

            },
            create: function (event, ui) {


                // Hook to activate panels and populate data
                /*var id = ui.newPanel.selector.split('-')[2];
                 that.container = $(ui.newPanel.selector);
                 // empty container, Yuli
                 var teleBlock = $(ui.newPanel.selector + ' .tele-block' );
                 teleBlock.remove();*/
            }

        });


        this.tabsEl.tabs({collapsible: true, active: false });

        this.panelSubBar.append(this.tabsUl);

        this.refresh();

    },


    refresh: function () {

        // console.log('Starting Search..');

        if (!this.searchStr){
            $.each(['alerts', 'cases', 'suspects', 'requests'], function (i, type) {
                var container = $('#tele-search-'+type);
                container.empty();
                container.append($('<p>').text("No search string defined"));

            });
            this.tabsEl.tabs({collapsible: false} );
            return false;
        }

        $('.ui-tabs').append(telepath.loader);

        // Cleanup
        var that = this;
        this.results = {};


        // Fallback to defaults
        if (this.options === false) {
            this.options = this.defaults; // USE $.extend
        }

        // Quick country conversion
        if (telepath.countries.n2a(this.searchStr) != '00') {
            this.searchStr = telepath.countries.n2a(this.searchStr);
            $('.tele-search-input').val(this.searchStr);
            this.countryFlag = true;
        } else {
            this.countryFlag = false;
        }

        // Collect Settings
        var searchSettingsObj = {
            search: this.searchStr,
            options: this.options,
            range: telepath.range,
            apps: telepath.appFilter,
            is_country: this.countryFlag,
            sort: this.sort,
            dir: this.dir
        };


        // Loop our types and send out search requests for different types of data

        that.count=0;

        $.each(['alerts', 'cases', 'suspects', 'requests'], function (i, type) {

            if (that.options[type]) {

                telepath.ds.get('/search/' + type, searchSettingsObj, function (data) {
                    // remove loading image, Yuli
                    that.count++;
                    that.container = $('#tele-search-' + type);
                    that.container.empty();
                    if (!data.items || data.items.length == 0) {
                        var p = $('<p>').text("No results");
                        that.container.append(p);
                        if (that.count==4)
                            that.selectTab();
                        return;
                    }

                    that.results[type] = data.items;
                    $('.tele-search-tab[rel="' + type + '"] span').html(data.total);

                    if (that.count==4)
                        that.selectTab();

                }, function (data) {
                    // error handler
                    that.count++;
                    if (that.count==4)
                        that.selectTab();
                    that.container = $('#tele-search-' + type);
                    that.container.empty();
                    var p = $('<p>').text(data['error']);
                    that.container.append(p);
                });


            }
            else {
                that.count++;
                if (that.count==4)
                    that.selectTab();
                that.container = $('#tele-search-' + type);
                that.container.empty();
                var p = $('<p>').text("No select option " + type);
                that.container.append(p);
            }

        });

        // Seperate formatter from each panel or make it available globally
        // Properly format the 4 types of data
        // Mark search terms as a listitem widget input
        // Do charts / graphs / maps where acceptable
        // Profit?

    },

    selectTab: function () {

        var that = this;

        //telepath.loader.remove();
        that.tabsEl.children('.tele-loader').remove();

        var found = false;

        $.each(['alerts', 'cases', 'suspects', 'requests'], function (i, type) {

            if (!found) {

                if (that.results[type] && that.results[type].length > 0) {

                    var select;

                    switch (type) {
                        case 'alerts':
                            select = 0;
                            that.showAlertsTab();
                            break;
                        case 'cases':
                            select = 1;
                            that.showCasesTab();
                            break;
                        case 'suspects':
                            select = 2;
                            that.showSuspectsTab();
                            break;
                        case 'requests':
                            select = 3;
                            that.showRequestsTab();
                            break;

                    }
                    that.tabsEl.tabs({active: select});

                    found = true;
                }
            }
        });
        that.tabsEl.tabs({collapsible: false});
    },

    showCasesTab: function () {

        // Create List
        this.list = $('<div>').addClass('tele-cases-block');
        $('#tele-search-cases').append(this.list);

        // Init List
        this.list.teleList({
            data: this.results.cases,
            searchkey: this.searchStr,
            formatter: function (item) {

                if (item._source) {
                    item = item._source
                }
                // Just in case defaults
                if (!item.checkable) {
                    item.checkable = false;
                }
                if (!item.favorites) {
                    item.favorites = false;
                    item.favorite = false;
                }

                var case_names = '';
                $.each(item.cases_names, function (i, x) {
                    case_names = case_names + x.key + ' ,'
                });
                case_names = case_names.substr(0, case_names.length - 2);

                var result = {
                    raw: item,
                    checkable: false,
                    favorites: false,
                    favorite: false,
                    itemID: item.sid,
                    icon: 'case',
                    title: case_names,
                    count: item.count,
                    details: [
                        {key: 'IP', value: item.ip_orig},
                        {key: 'country', value: item.country},
                        {key: 'city', value: item.city},
                        {key: 'host', value: grabNames(item.host)},
                        {key: 'alerts', value: item.alerts_count},
                        {key: 'actions', value: item.actions_count},
                    ]
                };
                return result;

            }
        });

    },
    showAlertsTab: function () {
        if (!this.results.alerts)
            return;
        // Create List
        this.list = $('<div>').addClass('tele-alerts-block');
        $('#tele-search-alerts').append(this.list);

        //this.list.teleList({data: this.results.alerts, searchkey: this.searchStr});
        // Init List
        this.list.teleList({
            data: this.results.alerts,
            searchkey: this.searchStr,
            formatter: function (item) {

                //item.checkable = true;
                return telepath.alert.rowFormatter(item);

            }
        });

    },
    showSuspectsTab: function () {
        if (!this.results.suspects)
            return;
        // Create List
        this.list = $('<div>').addClass('tele-suspects-block');
        $('#tele-search-suspects').append(this.list);

        // Init Suspects
        this.list.teleList({data: this.results.suspects, searchkey: this.searchStr,
        formatter: function(item){
           // item.checkable = true;
            return telepath.suspects.rowFormatter(item);
        }});

    },
    showRequestsTab: function () {
        if (!this.results.requests)
            return;
        // Create List
        this.list = $('<div>').addClass('tele-requests-block');
        $('#tele-search-requests').append(this.list);

        // Init Suspects
        this.list.teleList({data: this.results.requests, searchkey: this.searchStr,
            formatter: function(item){
             //   item.checkable = true;
                return telepath.suspects.rowFormatter(item);
            }});

    }

}
