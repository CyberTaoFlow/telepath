CREATE TABLE IF NOT EXISTS `reputation_users` (
	`user_id` int(10) unsigned NOT NULL,
	`app_id` int(10) unsigned NOT NULL,
	`user` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
	`score` double NOT NULL,
	`counter` int(10) unsigned NOT NULL,
	PRIMARY KEY (`user_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;