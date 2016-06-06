CREATE TABLE IF NOT EXISTS `filters` (
		`id` int(11) NOT NULL AUTO_INCREMENT,
		`filter_id` char(20) NOT NULL,
		`time` int(11) NOT NULL,
		`status` varchar(15) NOT NULL,
		`StatusInterval` int(11) NOT NULL,
		PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=3 ;