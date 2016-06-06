CREATE TABLE IF NOT EXISTS `bad_ips` (
		`index` int(11) NOT NULL AUTO_INCREMENT,
		`ip_addr` text NOT NULL,
		`name` text NOT NULL,
		`description` varchar(1024) NOT NULL DEFAULT 'None',
		PRIMARY KEY (`index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;