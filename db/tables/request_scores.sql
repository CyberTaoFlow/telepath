CREATE TABLE IF NOT EXISTS `request_scores` (
	`production` tinyint(1) NOT NULL DEFAULT 0,
	`RID` bigint(20) NOT NULL,
	`SID` varchar(100) NOT NULL DEFAULT "N/A",
	`seq_index` int(5) unsigned zerofill NOT NULL DEFAULT 0,
	`SID_status` enum('valid','invalid','unknown'),
	`hostname` int(10) unsigned NOT NULL,
	`page_id` int(11) unsigned NOT NULL,
	`status_code` int(3) unsigned NOT NULL,
	`protocol` enum('HTTP','HTTPS') NOT NULL DEFAULT 'HTTP',
	`method` ENUM( 'GET', 'POST', 'HEAD', 'OPTIONS', 'PUT', 'DELETE', 'TRACE', 'CONNECT', 'PROPFIND', 'PROPPATCH', 'MKCOL', 'COPY', 'MOVE', 'LOCK', 'UNLOCK', 'POLL', 'REPORT', 'SUBSCRIBE', 'BMOVE', 'SEARCH' ) NOT NULL DEFAULT 'GET',
	`AgentID` int(11) unsigned NOT NULL DEFAULT '1',
	`user_id` int(10) unsigned,
	`user_ip` char(18) NOT NULL,
	`country` char(2) CHARACTER SET latin1,
	`city` varchar(40),
	`latitude` double NOT NULL DEFAULT 0,
	`longitude` double NOT NULL DEFAULT 0,
	`query_score` double NOT NULL DEFAULT 0,
	`flow_score` double NOT NULL DEFAULT 0,
	`landing_normal` double NOT NULL DEFAULT 0,
	`geo_normal` double NOT NULL DEFAULT 0,
	`avg_score` double NOT NULL DEFAULT 0,
	`date` bigint(20) unsigned,
	`subdomain_id` int(10),
	PRIMARY KEY (`RID`,`date`),
	KEY `rs_page_id` (`page_id`),
	KEY `SID` (`SID`(100)),
	KEY `date_index` (`date`),
	KEY `avg_score` (`avg_score`),
	KEY `user_ip_index` (`user_ip`),
	KEY `user_index` (`user_id`),
	KEY `method_index` (`method`),
	KEY `country_index` (`country`),
	KEY `city_index` (`city`),
	KEY `hostname` (`hostname`),
	KEY `subdomain_id` (`subdomain_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
