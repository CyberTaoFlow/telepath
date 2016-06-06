CREATE TABLE IF NOT EXISTS `logs` (
		`msg_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
		`date` varchar(20) NOT NULL,
		`source` varchar(256) NOT NULL,
		`message` varchar(1024) NOT NULL,
		`event_type` varchar(20) NOT NULL,
		PRIMARY KEY (`msg_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=1;