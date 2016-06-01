CREATE TABLE IF NOT EXISTS `request_scores_stats` (
		`id` int(10) unsigned NOT NULL AUTO_INCREMENT,
		`country` char(2) NOT NULL,
		`hostname` int(10) unsigned NOT NULL,
		`avg_score` enum('low','med','high') NOT NULL DEFAULT 'low',
		`date` date NOT NULL,
		`count` int(10) unsigned DEFAULT '0',
		PRIMARY KEY (`id`),
		UNIQUE KEY `country` (`country`,`hostname`,`avg_score`,`date`),
		KEY `date_index` (`date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;