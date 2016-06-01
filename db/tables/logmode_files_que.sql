CREATE TABLE IF NOT EXISTS `logmode_files_que` (
		`id` int(11) NOT NULL AUTO_INCREMENT,
		`user` text NOT NULL,
		`file` text NOT NULL,
		`processed` tinyint(4) NOT NULL,
		`type` enum('apache','iis') NOT NULL,
		`app_id` int(11) NOT NULL,
		PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;