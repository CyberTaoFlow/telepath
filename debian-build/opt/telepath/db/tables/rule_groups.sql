CREATE TABLE IF NOT EXISTS `rule_groups` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci DEFAULT '-',
  `description` varchar(512) CHARACTER SET utf8 COLLATE utf8_unicode_ci DEFAULT '-',
  `score_numeric` int(3) DEFAULT '85',
  `score_literal` varchar(4) DEFAULT '',
  `action_email` tinyint(1) DEFAULT '0',
  `action_log` tinyint(1) DEFAULT '1',
  `action_syslog` tinyint(1) DEFAULT '0',
  `action_header_injection` tinyint(1) DEFAULT '0',
  `businessflow_id` int(11) DEFAULT '-1',
  `group_behavior_id` int(11) DEFAULT '0',
  `alert_param_ids` varchar(100) DEFAULT '',
  `category_id` int(11) DEFAULT '0',
  `email_recipient` char(255) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 COMMENT='Hold all the Rule Groups' AUTO_INCREMENT=76 ;
