CREATE TABLE IF NOT EXISTS `long_term_rules` (
		`mask_value` char(45) NOT NULL,
		`date` bigint(20) unsigned NOT NULL,
		UNIQUE KEY `mask_value` (`mask_value`),
		KEY `date` (`date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;