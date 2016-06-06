CREATE TABLE IF NOT EXISTS `whitelist_ip` (
	`user_ip` char(40) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
	UNIQUE KEY `user_ip` (`user_ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;