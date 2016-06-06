CREATE TABLE IF NOT EXISTS `business_flow` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `group_name` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `app_id` int(11) NOT NULL,
  `category_id` int(11) NOT NULL,
  `pages` varchar(16000) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `page_id` int(11) unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;
