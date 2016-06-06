CREATE TABLE IF NOT EXISTS `long_term_rule_stats` (
		`anchor` char(30) NOT NULL,
		`counter` tinyint(4) NOT NULL,
		`date` bigint(20) unsigned NOT NULL,
		UNIQUE KEY `anchor` (`anchor`),
		KEY `date` (`date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;