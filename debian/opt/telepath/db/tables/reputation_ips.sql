CREATE TABLE IF NOT EXISTS `reputation_ips` (
	`user_ip` int(11) NOT NULL,
	`score` double NOT NULL,
	`counter` int(10) unsigned NOT NULL,
	PRIMARY KEY (  `user_ip` )
) ENGINE=MyISAM DEFAULT CHARSET=latin1;