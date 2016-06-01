CREATE TABLE IF NOT EXISTS `swap_path` (
	`app_id` int(11) NOT NULL,
	PRIMARY KEY (`app_id`),
	UNIQUE KEY `app_id` (`app_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;