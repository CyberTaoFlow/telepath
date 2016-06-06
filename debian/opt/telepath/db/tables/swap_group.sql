CREATE TABLE IF NOT EXISTS `swap_group` (
	`group_behavior_id` int(15) NOT NULL,
	PRIMARY KEY (`group_behavior_id`),
	UNIQUE KEY `group_behavior_id` (`group_behavior_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;