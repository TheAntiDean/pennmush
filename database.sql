-- MySQL dump 10.13  Distrib 8.0.30, for Linux (x86_64)
--
-- Host: localhost    Database: mush
-- ------------------------------------------------------
-- Server version	8.0.30-0ubuntu0.22.04.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `attributes`
--

DROP TABLE IF EXISTS `attributes`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `attributes` (
  `attrib_id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(500) DEFAULT NULL,
  `owner` int NOT NULL,
  `flags` text,
  `derefs` int unsigned DEFAULT NULL,
  `value` text,
  `obj` int DEFAULT NULL,
  PRIMARY KEY (`attrib_id`),
  UNIQUE KEY `ATTR_UNIQ` (`obj`,`name`),
  KEY `ATTR_OBJ` (`obj`),
  CONSTRAINT `ATTR_OBJ` FOREIGN KEY (`obj`) REFERENCES `object` (`dbref`)
) ENGINE=InnoDB AUTO_INCREMENT=27625 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci ROW_FORMAT=DYNAMIC;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `locks`
--

DROP TABLE IF EXISTS `locks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `locks` (
  `lockid` int NOT NULL AUTO_INCREMENT,
  `type` varchar(200) DEFAULT NULL,
  `creator` int DEFAULT NULL,
  `flags` text,
  `derefs` int unsigned DEFAULT NULL,
  `obj` int DEFAULT NULL,
  `boolexp` text,
  PRIMARY KEY (`lockid`),
  UNIQUE KEY `lockid_UNIQUE` (`lockid`),
  UNIQUE KEY `OwnerLock` (`obj`,`type`),
  KEY `key_obj` (`obj`)
) ENGINE=InnoDB AUTO_INCREMENT=822 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `object`
--

DROP TABLE IF EXISTS `object`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `object` (
  `dbref` int NOT NULL,
  `name` text,
  `location` int DEFAULT NULL,
  `contents` int DEFAULT NULL,
  `exits` int DEFAULT NULL,
  `next` int DEFAULT NULL,
  `parent` int DEFAULT NULL,
  `owner` int DEFAULT NULL,
  `zone` int DEFAULT NULL,
  `type` int unsigned DEFAULT NULL,
  `flags` text,
  `powers` text,
  `warnings` text,
  `created` int DEFAULT NULL,
  `modified` int DEFAULT NULL,
  `pennies` int DEFAULT NULL,
  PRIMARY KEY (`dbref`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2022-08-20 10:00:28
