CREATE TABLE IF NOT EXISTS `awl` (
  `username` varchar(100) NOT NULL DEFAULT '',
  `email` varchar(255) NOT NULL DEFAULT '',
  `ip` varchar(40) NOT NULL DEFAULT '',
  `count` int(11) DEFAULT '0',
  `totscore` float DEFAULT '0',
  `signedby` varchar(255) NOT NULL DEFAULT '',
  `lastupdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

ALTER TABLE `awl`
 ADD PRIMARY KEY (`username`,`email`,`signedby`,`ip`);
