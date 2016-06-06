CREATE TABLE IF NOT EXISTS `user_groups` (
	`id` int(11) NOT NULL AUTO_INCREMENT,
	`name` varchar(50) NOT NULL,
	`SID` varchar(100) NOT NULL,
	`user_name` varchar(50) NOT NULL,
	`user_ip` varchar(20) NOT NULL,
	`app_name` varchar(50) NOT NULL,
	`case_status` varchar(10) NOT NULL,
	PRIMARY KEY (`id`),
	UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;