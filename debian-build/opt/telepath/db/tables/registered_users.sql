CREATE TABLE IF NOT EXISTS `registered_users` (
		`user` varchar(20) NOT NULL,
		`password` varchar(1024) DEFAULT NULL,
		UNIQUE KEY `user` (`user`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;