CREATE TABLE IF NOT EXISTS `attribute_scores` (
		`att_id` bigint(20) unsigned NOT NULL,
		`RID` bigint(20) NOT NULL,
		`data` varchar(300) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
		`attribute_score_exponent` int(10) NOT NULL DEFAULT 0,
		`attribute_score_normal` double NOT NULL DEFAULT 0,
		`length_score_exponent` int(11) NOT NULL DEFAULT 0,
		UNIQUE KEY `att_id_rid` (`att_id`,`RID`),
		KEY `att_id` (`att_id`),
		KEY `rid` (`RID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
