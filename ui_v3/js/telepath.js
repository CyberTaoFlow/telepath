$(document).ready(function () {

    telepath.main.init();

    $(document).click(function (e) {

        $('.tele-popup').each(function () {

            if($(this).hasClass('hook')) {

                if($(e.target).parents('.tele-popup').size() == 0 && $(e.target).parents('body').size() > 0) {
                    $('.tele-popup').remove();
                }

            } else {
                $(this).addClass('hook');
            }

        });

    });

});

telepath.loader = '<img class="loader" src="img/loader.gif">';
telepath.data = {};
telepath.appFilter = [{ key: -1, label: 'All Applications' }];

/*
 telepath.el_id = 0;
 telepath.next_id = function() {
 telepath.el_id++;
 return telepath.el_id;
 }
 */

telepath.main = {

    loadResources: function(callback) {

        yepnope({
            load: [
                // Widgets
                "css/widgets.css",
                "js/widgets/teleAppSelect.js",
                "js/widgets/teleInput.js",
                "js/widgets/telePassword.js",
                "js/widgets/teleCheckbox.js",
                "js/widgets/teleList.js",
                "js/widgets/teleForm.js",
                "js/widgets/teleTree.js",
                "js/widgets/teleRule.js",
                "js/widgets/teleRadios.js",
                "js/widgets/teleBrowse.js",
                "js/widgets/teleBrowser.js",
                "js/widgets/teleSelect.js",
                "js/widgets/teleMulti.js",
                "js/widgets/teleRange.js",
                "js/widgets/teleFile.js",
                "js/widgets/teleSearch.js",
                "js/widgets/teleRequest.js",
                "js/widgets/teleCountry.js",
                // Dropdown (JQuery)
                "js/lib/jquery.dropdown.js",
                "css/jquery.dropdown.css",
                // Bootstrap (Popover)
                "js/lib/bootstrap.min.js",
                "css/bootstrap.css",
                // Notifications
                "js/lib/jquery.pnotify.min.js",
                // Custom Scroll
                "js/lib/jquery.mousewheel.js",
                "js/lib/jquery.mCustomScrollbar.js",
                "css/jquery.mCustomScrollbar.css",
                // Map
                "js/lib/jquery.vmap.min.js",
                "js/lib/jquery.vmap.world.js",
                "js/lib/jquery.vmap.sampledata.js",
                "css/jqvmap.css",

                // Config Styles
                "css/config.css",

                // Scheduler
                "js/lib/jquery.weekcalendar.js",
                "css/jquery.weekcalendar.css",

                // Tree
                "js/lib/jstree.min.js",
                "js/lib/jstreegrid.js",
                "css/tree.css",

                // More code...
                "js/telepath.sessionflow.js?1",
                "js/telepath.handlers.js",
                "js/telepath.rule.js",
                "js/telepath.dialog.js",
                "js/telepath.ipaddress.js",
                "js/telepath.condition.js",
                "js/telepath.condition.select.js",
                "js/telepath.contextmenu.js",
                "js/telepath.logmode.js",

                // Config Scripts
                "js/telepath.config.account.js",
                "js/telepath.config.accounts.js",
                // Action
                "js/telepath.config.action.js",
                "js/telepath.config.actions.js",
                // Application
                "js/telepath.config.application.js?",
                "js/telepath.config.applications.js",
                "js/telepath.config.notifications.js",
                // Rule
                "js/telepath.config.rule.js",
                "js/telepath.config.rules.js",
                // Users
                "js/telepath.config.user.js",
                "js/telepath.config.users.js",
                "js/telepath.config.groups.js",
                // System
                "js/telepath.config.system.js"

            ],
            complete: function() {
                callback();
            }});

    },
    init: function () {


        // Load some stuff
        telepath.ds.get('/parameters/get_global_headers', {}, function (data) {
            telepath.global_headers = data.items;
            // console.log('GLOBAL HEADERS::');
            // console.log(telepath.global_headers);
        });
        telepath.ds.get('/rules/get_cmds', {}, function (data) {
            telepath.rule_cmds = data.items;
            // console.log('CMD EXEC::');
            // console.log(telepath.rule_cmds);
        });


        telepath.ds.get('/telepath/get_app_filter', {}, function (data) {

            telepath.app_filter = data.items;     // console.log('APP FILTER::');     console.log(telepath.app_filter);

            //if (data.ip == '81.218.185.126') {
            //     console.log('DEBUG FROM OFFICE');
            //}

        });
        telepath.ds.get('/telepath/get_time_range', {}, function (data) {
            telepath.range = data.items;
            // console.log('TIME RANGE::');
            // console.log(telepath.range);

        // Load resources
        telepath.main.loadResources(function () {

            // var el = $('<div>').teleMulti().addClass('HelloWorld');
            // $('body').append(el);
            // return;

            // Load templates
            $.getJSON(telepath.controllerPath + '/telepath/templates', function (data) {

                telepath.templates = data;
                telepath.ui.init();
                telepath.header.init();

                telepath.dashboard.init();
                $('.tele-nav-dashboard a').addClass('active');


                /*
                 var paramBrowse = $('<div>').teleBrowse();
                 $('body').prepend(paramBrowse);
                 */


                //$('.tele-content').append($('<div>').teleBrowse().openbrowser());

                /*setTimeout(function () {
                 $('.tele-nav-cases a').click();
                 $('.tele-panel-topbar .tele-button:first').click()
                 }, 1000);*/
                //telepath.casePanel.init(0);
                //telepath.suspects.init();
                //telepath.alerts.init();
                //telepath.ruleOverlay.addRule();

                // Debug Mode for Application Editor
                /*telepath.config.init();
                 setTimeout(function () {
                 $('.tele-icon-system').click();
                 }, 1500);*/

            });

        });

    });
        telepath.ds.get('/telepath/get_full_time_range', {}, function(data) {

            // Set global variable to hold the time of the first telepath data
            telepath.fullRangeStart = data.items.start;

        });
    }

};

telepath.ui = {

    init: function() {

        $(window).resize(function () {
            telepath.ui.resize();
        });

        $(window).click(function (e) {
            if($('.popover').size() > 0) {
                if($(e.target).parents('.popover').size() > 0 || $(e.target).hasClass('popover')) {

                } else {
                    $('.popover').remove();
                }
            }
        });

        var that = this;

        this.mainEl    = $('<div>').addClass('tele-body');
        this.headerEl  = $('<div>').addClass('tele-header');
        this.contentEl = $('<div>').addClass('tele-content');

        this.mainEl.append(this.headerEl).append(this.contentEl);
        $('body').append(this.mainEl);

        this.panels = ['dashboard', 'cases', 'alerts', 'suspects', 'reports', 'config', 'search'];

        $.each(this.panels, function(i, panel) {
            var panelEl = $('<div>').addClass('tele-panel').addClass('tele-panel-' + panel);
            that.contentEl.append(panelEl);
        });

        $('.tele-panel-dashboard').addClass('active');

        $(window).resize(function () {
            telepath.ui.resize();
        });

        telepath.ui.resize();

    },
    resize: function () {

        var active = $('.tele-panel.active');
        $.each(this.panels, function(i, x) {
            if($(active).hasClass('tele-panel-' + x)) {
                active = x;
                return false;
            }
        });

        // Containers
        $('.tele-body').height($(window).height());

        if(active == 'dashboard') {
            $('.tele-content').css({ height: '890px' });
            $('body').css({ overflow: 'scroll-x' });
        } else {
            $('.tele-content').css({ height: $(window).height() - $('.tele-header').height() - 50 });

            // remove this because scrolling is not enabled in setting (Moshe)
            //$('body').css({ overflow: 'hidden' });
        }


    }

}
