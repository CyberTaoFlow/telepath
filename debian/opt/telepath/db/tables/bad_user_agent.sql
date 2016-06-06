CREATE TABLE IF NOT EXISTS `bad_user_agent` (
		`index` int(11) NOT NULL AUTO_INCREMENT,
		`user_agent` text NOT NULL,
		`name` text NOT NULL,
		`description` varchar(1024) NOT NULL DEFAULT 'None',
		PRIMARY KEY (`index`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;