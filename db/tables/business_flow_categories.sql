CREATE TABLE IF NOT EXISTS `business_flow_categories` (
		`id` int(11) NOT NULL AUTO_INCREMENT,
		`app_id` int(11) NOT NULL DEFAULT 0,
		`name` text CHARACTER SET utf8 COLLATE utf8_unicode_ci,
		PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=5 ;
