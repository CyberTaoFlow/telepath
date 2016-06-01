CREATE TABLE IF NOT EXISTS `coordinates` (
		`cluster_id` int(11) NOT NULL DEFAULT 0,
		`hostname` int(10) unsigned NOT NULL DEFAULT 0,
		`x` double NOT NULL DEFAULT 0,
		`y` double NOT NULL DEFAULT 0
) ENGINE=MyISAM DEFAULT CHARSET=latin1;