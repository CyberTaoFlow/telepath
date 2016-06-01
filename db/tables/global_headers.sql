CREATE TABLE IF NOT EXISTS `global_headers` (
		`app_id` int(11) NOT NULL,
		`page_id` int(11) NOT NULL,
		`att_id` int(11) NOT NULL,
		`att_name` varchar(256) NOT NULL,
		UNIQUE KEY `att_id` (`att_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;