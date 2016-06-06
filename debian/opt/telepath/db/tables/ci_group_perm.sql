CREATE TABLE IF NOT EXISTS `ci_group_perm` (
  `id` int(9) NOT NULL AUTO_INCREMENT,
  `group_id` int(5) unsigned NOT NULL,
  `perm_id` int(5) unsigned NOT NULL,
  `value` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `group_id` (`group_id`),
  KEY `perm_id` (`perm_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1;
