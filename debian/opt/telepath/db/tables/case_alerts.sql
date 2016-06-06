CREATE TABLE IF NOT EXISTS `case_alerts` (
	`case_id` int(11) unsigned NOT NULL,
	`alert_id` int(11) unsigned NOT NULL,
	`seen` tinyint(1) NOT NULL DEFAULT '0',
	UNIQUE KEY `case_id` (`case_id`,`alert_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;