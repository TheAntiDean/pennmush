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

-- Dumping structure for table MUSH.attribute
CREATE TABLE IF NOT EXISTS `attribute` (
  `id` int(11) NOT NULL,
  `name` text DEFAULT NULL,
  `ownerId` int(11) NOT NULL,
  `flags` text NOT NULL,
  `derefs` int(11) NOT NULL,
  `objectId` int(11) NOT NULL,
  `hash` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY `UKeyAtr` (`name`,`objectId`) USING HASH,
  KEY `FK_Obj` (`objectId`) USING BTREE,
  CONSTRAINT `FK_attribute_object` FOREIGN KEY (`objectId`) REFERENCES `object` (`id`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Data exporting was unselected.

-- Dumping structure for table MUSH.lock
CREATE TABLE IF NOT EXISTS `lock` (
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Data exporting was unselected.

-- Dumping structure for table MUSH.object
CREATE TABLE IF NOT EXISTS `object` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text DEFAULT NULL,
  `locationObjId` int(11) DEFAULT NULL,
  `contentObjId` int(11) DEFAULT NULL,
  `exitObjId` int(11) DEFAULT NULL,
  `nextObjId` int(11) DEFAULT NULL,
  `parentObjId` int(11) DEFAULT NULL,
  `zoneObjId` int(11) DEFAULT NULL,
  `type` int(11) unsigned DEFAULT NULL,
  `powers` text DEFAULT NULL,
  `warnings` text DEFAULT NULL,
  `flags` text DEFAULT NULL,
  `created` int(11) DEFAULT NULL,
  `modified` int(11) DEFAULT NULL,
  `pennies` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Data exporting was unselected.

/*!40103 SET TIME_ZONE=IFNULL(@OLD_TIME_ZONE, 'system') */;
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;


  rc1 = SQLExecDirect(
    hstmt,
    "select id, name, locationObjId, contentObjId, exitObjId, nextObjId, parentObjId, type, powers, warnings,\
  flags, created, modified, pennies from object where id=( ? )",
    SQL_NTS);

  SQLBindParameter(hstmt,           /* Statement handle */
                   1,               /* Column number 1 */
                   SQL_PARAM_INPUT, /* This is an input parameter */
                   SQL_C_LONG,      /* This is an integer in C */
                   SQL_INTEGER,     /* Destination column is varchar */
                   strlen(id),      /* Length of the parameter */
                   0,               /* No scale specifier */
                   id,              /* The data itself */
                   0,               /* Maximum length (default 0) */
                   &orind1);        /* Null-terminated string */

snprintf(buff, BUFFER_LEN, "INSERT INTO object (id, name, locationObjId, contentObjId, exitObjId, nextObjId, parentObjId, type, powers, warnings,\
  flags, created, modified, pennies) VALUES (%i, \"%s\", %i, %i,%i, %i,%i, %i,%i, %i,%i, %i,%i, %i) 
  ON DUPLICATE KEY UPDATE name=\"%s\", locationObjId=%i, contentObjId=%i, exitObjId=%i, nextObjId=%i,\
  parentObjId=%i, type=%i, powers=%i, warnings=%i, flags=%i, created=%i, modified=%i, pennies=%i", 
  objID, 
  DBObj.name,
  DBObj.location,
  DBObj.exits,
  DBObj.next,
  DBObj.parent,
  DBObj.zone,
  DBObj.type,
  DBObj.powers,
  DBObj.warnings,
  DBObj.flags,
  DBObj.creation_time,
  DBObj.modification_time,
  DBObj.penn,
  DBObj.location,
  DBObj.exits,
  DBObj.next,
  DBObj.parent,
  DBObj.zone,
  DBObj.type,
  DBObj.powers,
  DBObj.warnings,
  DBObj.flags,
  DBObj.creation_time,
  DBObj.modification_time,
  DBObj.penn);

 name=@name, locationObjId=@location, contentObjId=@content, exitObjId=@exit, nextObjId=@next,\
parentObjId=@parent, zoneObjId=@zone, type=@type, powers=@powers, warnings=@warnings, flags=@flags, created=@created, modified=@modified, pennies=@pennies

snprintf(buff, BUFFER_LEN
"INSERT INTO attribute (name, ownerId, flags, derefs, objectId, value) VALUES (\"%s\", %i, \"%s\", %i, %i, \"%s") \
ON DUPLICATE KEY UPDATE ownerId=%i, flags=\"%s\", derefs=%i, value=\"%s\"",
AL_NAME(list), Owner(AL_CREATOR(list)), atrflag_to_string(AL_FLAGS(list)),AL_DEREFS(list),ObjID, atr_value(list)

Owner(AL_CREATOR(list)), atrflag_to_string(AL_FLAGS(list)),AL_DEREFS(list),atr_value(list));


  `id` int(11) NOT NULL,
  `name` text DEFAULT NULL,
  `ownerId` int(11) NOT NULL,
  `flags` text NOT NULL,
  `derefs` int(11) NOT NULL,
  `objectId` int(11) NOT NULL,


ATTR_FOR_EACH (i, list) {
snprintf(buff, BUFFER_LEN
"INSERT INTO attribute (name, ownerId, flags, derefs, objectId, value) VALUES (\"%s\", %i, \"%s\", %i, %i, \"%s") \
ON DUPLICATE KEY UPDATE ownerId=%i, flags=\"%s\", derefs=%i, value=\"%s\"",
AL_NAME(list), Owner(AL_CREATOR(list)), atrflag_to_string(AL_FLAGS(list)),AL_DEREFS(list),ObjID, atr_value(list)

Owner(AL_CREATOR(list)), atrflag_to_string(AL_FLAGS(list)),AL_DEREFS(list),atr_value(list));
  }


  ODBCParamInt   (&hstmt, 3, &DBObj->location);
  ODBCParamInt   (&hstmt, 4, &DBObj->contents);
  ODBCParamInt   (&hstmt, 5, &DBObj->exits);
  ODBCParamInt   (&hstmt, 6, &DBObj->next);
  ODBCParamInt   (&hstmt, 7, &DBObj->parent);
  ODBCParamInt   (&hstmt, 8, &DBObj->zone);
  ODBCParamInt   (&hstmt, 9, &DBObj->type);

      ODBCParamInt(&hstmt, 13, &DBObj->creation_time);
    ODBCParamInt(&hstmt, 14, &DBObj->modification_time);
    ODBCParamInt(&hstmt, 15, &DBObj->penn);


SQLRETURN ODBCParamInt(SQLHSTMT *hstmt,  SQLINTEGER idx, SQLINTEGER value) {
  
        SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,
                         11, 0, &objID, sizeof(objID), NULL);
        SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 11,
                       0, &objID, sizeof(objID), NULL);

