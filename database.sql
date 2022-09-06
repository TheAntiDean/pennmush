-- --------------------------------------------------------
-- Host:                         192.168.2.1
-- Server version:               10.5.15-MariaDB-0+deb11u1 - Debian 11
-- Server OS:                    debian-linux-gnu
-- HeidiSQL Version:             12.1.0.6537
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


-- Dumping database structure for MUSH
CREATE DATABASE IF NOT EXISTS `MUSH` /*!40100 DEFAULT CHARACTER SET utf8mb4 */;
USE `MUSH`;

-- Dumping structure for table MUSH.object
CREATE TABLE IF NOT EXISTS `object` (
  `id` int(11) NOT NULL,
  `name` varchar(255) DEFAULT NULL,
  `locationObjId` int(11) DEFAULT NULL,
  `contentObjId` int(11) DEFAULT NULL,
  `exitObjId` int(11) DEFAULT NULL,
  `nextObjId` int(11) DEFAULT NULL,
  `parentObjId` int(11) DEFAULT NULL,
  `zoneObjId` int(11) DEFAULT NULL,
  `type` int(11) unsigned DEFAULT NULL,
  `powers` varchar(255) DEFAULT NULL,
  `warnings` varchar(255) DEFAULT NULL,
  `flags` varchar(255) DEFAULT NULL,
  `created` int(11) DEFAULT NULL,
  `modified` int(11) DEFAULT NULL,
  `pennies` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Data exporting was unselected.

-- Dumping structure for table MUSH.objectattrib
CREATE TABLE IF NOT EXISTS `objectattrib` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text DEFAULT NULL,
  `ownerId` int(11) NOT NULL,
  `flags` text NOT NULL,
  `derefs` int(11) NOT NULL,
  `objectId` int(11) NOT NULL,
  `value` text DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UKeyAtr` (`name`,`objectId`) USING HASH,
  KEY `FK_Obj` (`objectId`) USING BTREE,
  CONSTRAINT `FK_attribute_object` FOREIGN KEY (`objectId`) REFERENCES `object` (`id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=205921 DEFAULT CHARSET=utf8mb4;

-- Data exporting was unselected.

-- Dumping structure for table MUSH.objectlock
CREATE TABLE IF NOT EXISTS `objectlock` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` varchar(50) DEFAULT NULL,
  `creatorId` int(11) DEFAULT NULL,
  `flags` text DEFAULT NULL,
  `derefs` int(10) unsigned DEFAULT NULL,
  `objectId` int(10) NOT NULL,
  `boolexp` text DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `UKeyLock` (`type`,`objectId`) USING BTREE,
  KEY `ObjID` (`objectId`) USING BTREE,
  CONSTRAINT `FK_lock_object` FOREIGN KEY (`objectId`) REFERENCES `object` (`id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=7695 DEFAULT CHARSET=utf8mb4;

-- Data exporting was unselected.

/*!40103 SET TIME_ZONE=IFNULL(@OLD_TIME_ZONE, 'system') */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
