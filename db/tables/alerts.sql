CREATE TABLE IF NOT EXISTS `alerts` (
		`id` int(11) NOT NULL,
		`date` bigint(20) unsigned NOT NULL,
		`email` tinyint(1) NOT NULL DEFAULT '0',
		`syslog` tinyint(1) NOT NULL DEFAULT '0',
		`report` tinyint(1) NOT NULL DEFAULT '0',
		`SID` varchar(100) NOT NULL,
		`user` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
		`hostname` int(10) unsigned NOT NULL,
		`IP` varchar(20),
		`latitude` double NOT NULL DEFAULT 0,
		`longitude` double NOT NULL DEFAULT 0,
		`country` char(2) NOT NULL DEFAULT "",
		`city` varchar(100) NOT NULL DEFAULT "",
		`rule_group` int unsigned not null DEFAULT 0,
		`average_score` double NOT NULL DEFAULT 0,
		`counter` int(11),
		`counter_sig` int(11) NOT NULL DEFAULT '0',
		`subdomain_id` int(10) DEFAULT NULL,
		`anchor_type` tinyint(1) unsigned,
		UNIQUE KEY `id` (`id`),
		KEY `date_index` (`date`),
		KEY `subdomain_id` (`subdomain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;