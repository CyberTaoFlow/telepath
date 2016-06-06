CREATE TABLE IF NOT EXISTS `request_scores_group` (
		`group_behavior_id` int(11) NOT NULL,
		`RID` bigint(20) unsigned NOT NULL,
		`SID` varchar(100) NOT NULL,
		`seq_index` int(10) unsigned default 0 NOT NULL,
		`hostname` int(10) unsigned NOT NULL,
		`page_id` bigint(20) unsigned NOT NULL,
		`user_id` int(10) unsigned NOT NULL,
		`user_ip` varchar(18) NOT NULL,
		`date` bigint(20) unsigned default 0 NOT NULL,
		PRIMARY KEY (`RID`),
		UNIQUE KEY `SID` (`SID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;