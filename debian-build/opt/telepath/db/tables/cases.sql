CREATE TABLE IF NOT EXISTS `cases` (
	`id` int(11) NOT NULL AUTO_INCREMENT,
	`name` varchar(255),
	`user_id` int(11),
	`case_data` varchar(6000),
	`timestamp` bigint(20) unsigned,
	`favorite` tinyint(1) NOT NULL DEFAULT '0',
	`created` int(11),
	PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;