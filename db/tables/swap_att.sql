CREATE TABLE IF NOT EXISTS `swap_att` (
		`att_id` bigint(20) unsigned NOT NULL,
		PRIMARY KEY (`att_id`),
		UNIQUE KEY `att_id` (`att_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;