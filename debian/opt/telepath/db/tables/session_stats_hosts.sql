CREATE TABLE IF NOT EXISTS `session_stats_hosts` (
	`id` int(10) unsigned NOT NULL AUTO_INCREMENT,
	`date_per_min` char(15) NOT NULL,
	`count` int(10) unsigned DEFAULT '0',
	`epoch` bigint(20) unsigned NOT NULL,
	`hostname` int(10) unsigned NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE KEY `date_per_min` (`epoch`,`hostname`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;