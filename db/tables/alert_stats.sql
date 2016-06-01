CREATE TABLE IF NOT EXISTS `alert_stats` (
		`id` int(10) unsigned NOT NULL AUTO_INCREMENT,
		`country` char(2),
		`hostname` int(10) unsigned,
		`average_score` enum('low','med','high') NOT NULL DEFAULT 'low',
		`rule_group` int unsigned NOT NULL DEFAULT 0,
		`date` date,
		`count` int(10) unsigned DEFAULT 0,
		`epoch` bigint(20) unsigned NOT NULL DEFAULT 0,
		PRIMARY KEY (`id`),
		UNIQUE KEY `country` (`country`,`hostname`,`rule_group`,`average_score`,`date`),
		KEY `date_index` (`date`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=78 ;
