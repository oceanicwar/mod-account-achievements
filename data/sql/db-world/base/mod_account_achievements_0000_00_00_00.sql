CREATE TABLE IF NOT EXISTS `account_achievements_whitelist` (
  `achievement_id` int NOT NULL,
  `comment` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT '',
  PRIMARY KEY (`achievement_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;