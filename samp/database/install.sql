-- ============================================================================
--  WISE-CITY-RP  |  قاعدة البيانات الموحّدة (نفّذ هذا الملف مرة واحدة)
--  1) نفّذ أولاً:  00_create_database_and_user.sql  بحساب root
--  2) ثم نفّذ هذا الملف:  mysql -u root -p wise_city_db < install.sql
--  كل الجداول بصيغة IF NOT EXISTS فلا خطر من إعادة التنفيذ.
-- ============================================================================
SET NAMES utf8mb4;

-- ============================================================================
--  الحسابات (users)
-- ============================================================================
CREATE TABLE IF NOT EXISTS `users` (
    `id`               INT UNSIGNED        NOT NULL AUTO_INCREMENT,
    `username`         VARCHAR(24)         NOT NULL,
    `password_hash`    VARCHAR(255)        NOT NULL COMMENT 'bcrypt',
    `email`            VARCHAR(100)        NOT NULL,
    `level`            INT UNSIGNED        NOT NULL DEFAULT 1,
    `money_dinar`      BIGINT UNSIGNED     NOT NULL DEFAULT 5000,
    `money_gold`       INT UNSIGNED        NOT NULL DEFAULT 0,
    `passport_status`  TINYINT UNSIGNED    NOT NULL DEFAULT 0 COMMENT '0 لا يوجد, 1 قيد المراجعة, 2 صادر, 3 موقوف/ملغي',
    `admin_level`      TINYINT UNSIGNED    NOT NULL DEFAULT 0 COMMENT '0 إلى 5',
    `character_skin`   SMALLINT UNSIGNED   NOT NULL DEFAULT 0,
    `spawn_location`   TINYINT UNSIGNED    NOT NULL DEFAULT 0 COMMENT '0 آخر موقع/المطار, 1 المنزل, 2 مقر الفصيل',
    `pos_x`            FLOAT               NOT NULL DEFAULT 1642.0,
    `pos_y`            FLOAT               NOT NULL DEFAULT -2333.5,
    `pos_z`            FLOAT               NOT NULL DEFAULT 13.5,
    `pos_angle`        FLOAT               NOT NULL DEFAULT 0.0,
    `interior_id`      SMALLINT UNSIGNED   NOT NULL DEFAULT 0,
    `virtual_world`    INT UNSIGNED        NOT NULL DEFAULT 0,
    `health`           FLOAT               NOT NULL DEFAULT 100.0,
    `armour`           FLOAT               NOT NULL DEFAULT 0.0,
    `faction_id`       TINYINT UNSIGNED    NOT NULL DEFAULT 0 COMMENT '0 مدني, 1 شرطة, 2 جيش, 3 حرس جمهوري, 4 إسعاف, 5 عصابات',
    `faction_rank`     TINYINT UNSIGNED    NOT NULL DEFAULT 0,
    `faction_warns`    TINYINT UNSIGNED    NOT NULL DEFAULT 0,
    `register_ip`      VARCHAR(45)         NOT NULL DEFAULT '',
    `last_ip`          VARCHAR(45)         NOT NULL DEFAULT '',
    `created_at`       TIMESTAMP           NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `last_login`       TIMESTAMP           NULL     DEFAULT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uq_users_username` (`username`),
    UNIQUE KEY `uq_users_email`    (`email`),
    KEY `idx_users_faction`        (`faction_id`, `faction_rank`),
    KEY `idx_users_admin`          (`admin_level`),
    CONSTRAINT `chk_users_admin_level` CHECK (`admin_level` BETWEEN 0 AND 6),
    CONSTRAINT `chk_users_faction_id`  CHECK (`faction_id`  BETWEEN 0 AND 5),
    CONSTRAINT `chk_users_passport`    CHECK (`passport_status` BETWEEN 0 AND 3)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ============================================================================
--  السيارات والمزادات
-- ============================================================================
CREATE TABLE IF NOT EXISTS `vehicles` (
    `id` INT(11) NOT NULL AUTO_INCREMENT,
    `owner_id` INT(11) NOT NULL DEFAULT 0,
    `model` INT(11) NOT NULL,
    `color1` INT(11) NOT NULL DEFAULT 1,
    `color2` INT(11) NOT NULL DEFAULT 1,
    `pos_x` FLOAT NOT NULL DEFAULT 0.0,
    `pos_y` FLOAT NOT NULL DEFAULT 0.0,
    `pos_z` FLOAT NOT NULL DEFAULT 0.0,
    `angle` FLOAT NOT NULL DEFAULT 0.0,
    `fuel` FLOAT NOT NULL DEFAULT 100.0,
    `damage` FLOAT NOT NULL DEFAULT 0.0,
    `plate` VARCHAR(16) NOT NULL DEFAULT 'WISE-CITY',
    `locked` TINYINT(1) NOT NULL DEFAULT 0,
    `insurance` TINYINT(1) NOT NULL DEFAULT 0,
    `for_sale` TINYINT(1) NOT NULL DEFAULT 0,
    `price` INT(11) NOT NULL DEFAULT 0,
    `currency` TINYINT(1) NOT NULL DEFAULT 0,
    `in_auction` TINYINT(1) NOT NULL DEFAULT 0,
    PRIMARY KEY (`id`),
    INDEX `idx_owner` (`owner_id`),
    INDEX `idx_for_sale` (`for_sale`),
    INDEX `idx_auction` (`in_auction`)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `auctions` (
    `id` INT(11) NOT NULL AUTO_INCREMENT,
    `vehicle_id` INT(11) NOT NULL,
    `seller_id` INT(11) NOT NULL,
    `start_price` INT(11) NOT NULL DEFAULT 0,
    `current_bid` INT(11) NOT NULL DEFAULT 0,
    `highest_bidder` INT(11) NOT NULL DEFAULT 0,
    `end_time` INT(11) NOT NULL DEFAULT 0,
    `status` TINYINT(1) NOT NULL DEFAULT 1,
    `receipt_id` VARCHAR(32) NOT NULL DEFAULT '',
    PRIMARY KEY (`id`),
    INDEX `idx_status` (`status`),
    INDEX `idx_vehicle` (`vehicle_id`)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
CREATE TABLE IF NOT EXISTS `auction_bids` (
    `id` INT(11) NOT NULL AUTO_INCREMENT,
    `auction_id` INT(11) NOT NULL,
    `bidder_id` INT(11) NOT NULL,
    `amount` INT(11) NOT NULL,
    `bid_time` INT(11) NOT NULL,
    PRIMARY KEY (`id`),
    INDEX `idx_auction` (`auction_id`),
    INDEX `idx_bidder` (`bidder_id`)
    ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
--  الفصائل والسجلات
-- ============================================================================
CREATE TABLE IF NOT EXISTS `faction_players` (
    `player_id` INT NOT NULL, `jail_minutes` INT NOT NULL DEFAULT 0,
    `is_president` TINYINT(1) NOT NULL DEFAULT 0, PRIMARY KEY (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `server_logs` (
    `id` INT NOT NULL AUTO_INCREMENT, `log_type` VARCHAR(32) NOT NULL,
    `player_id` INT NOT NULL DEFAULT -1, `player_name` VARCHAR(24) NOT NULL DEFAULT '',
    `log_message` TEXT NOT NULL, `log_time` INT NOT NULL,
    PRIMARY KEY (`id`), INDEX `idx_type` (`log_type`), INDEX `idx_time` (`log_time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `chat_logs` (
    `id` INT NOT NULL AUTO_INCREMENT, `player_id` INT NOT NULL,
    `player_name` VARCHAR(24) NOT NULL, `chat_type` TINYINT NOT NULL DEFAULT 0,
    `message` TEXT NOT NULL, `logged_at` INT NOT NULL,
    PRIMARY KEY (`id`), INDEX `idx_player` (`player_id`), INDEX `idx_time` (`logged_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
--  الإدارة
-- ============================================================================
-- ================================================================
-- 1. جدول بيانات الإدارة (admin_data)
-- ================================================================
CREATE TABLE IF NOT EXISTS `admin_data` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `player_id` INT(11) NOT NULL,
  `username` VARCHAR(24) NOT NULL,
  `admin_level` TINYINT(1) NOT NULL DEFAULT 0,
  `rank_name` VARCHAR(32) NOT NULL DEFAULT 'لاعب',
  `tag` VARCHAR(16) NOT NULL DEFAULT '',
  `duty` TINYINT(1) NOT NULL DEFAULT 0,
  `hidden` TINYINT(1) NOT NULL DEFAULT 0,
  `reports_handled` INT(11) NOT NULL DEFAULT 0,
  `punishments_issued` INT(11) NOT NULL DEFAULT 0,
  `kicks_issued` INT(11) NOT NULL DEFAULT 0,
  `bans_issued` INT(11) NOT NULL DEFAULT 0,
  `jails_issued` INT(11) NOT NULL DEFAULT 0,
  `mutes_issued` INT(11) NOT NULL DEFAULT 0,
  `warnings_issued` INT(11) NOT NULL DEFAULT 0,
  `weekly_seconds` INT(11) NOT NULL DEFAULT 0,
  `total_seconds` INT(11) NOT NULL DEFAULT 0,
  `warnings_received` INT(11) NOT NULL DEFAULT 0,
  `immunity` TINYINT(1) NOT NULL DEFAULT 0,
  `last_login` INT(11) NOT NULL DEFAULT 0,
  `assigned_by` VARCHAR(24) NOT NULL DEFAULT '',
  `assigned_at` INT(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `unique_player` (`player_id`),
  INDEX `idx_level` (`admin_level`),
  INDEX `idx_duty` (`duty`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ================================================================
-- 2. جدول العقوبات (admin_punishments)
-- ================================================================
CREATE TABLE IF NOT EXISTS `admin_punishments` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `admin_id` INT(11) NOT NULL,
  `admin_name` VARCHAR(24) NOT NULL,
  `target_id` INT(11) NOT NULL,
  `target_name` VARCHAR(24) NOT NULL,
  `punishment_type` TINYINT(1) NOT NULL,
  `reason` VARCHAR(255) NOT NULL,
  `duration` INT(11) NOT NULL DEFAULT 0,
  `punished_at` INT(11) NOT NULL,
  `expires_at` INT(11) NOT NULL DEFAULT 0,
  `active` TINYINT(1) NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  INDEX `idx_admin` (`admin_id`),
  INDEX `idx_target` (`target_id`),
  INDEX `idx_active` (`active`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ================================================================
-- 3. جدول سجل الإجراءات (admin_logs)
-- ================================================================
CREATE TABLE IF NOT EXISTS `admin_logs` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `admin_id` INT(11) NOT NULL,
  `admin_name` VARCHAR(24) NOT NULL,
  `action` VARCHAR(255) NOT NULL,
  `target_id` INT(11) NOT NULL DEFAULT -1,
  `ip_address` VARCHAR(45) NOT NULL DEFAULT '',
  `logged_at` INT(11) NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `idx_admin` (`admin_id`),
  INDEX `idx_time` (`logged_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ================================================================
-- 4. جدول التحذيرات (admin_warnings)
-- ================================================================
CREATE TABLE IF NOT EXISTS `admin_warnings` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `player_id` INT(11) NOT NULL,
  `admin_name` VARCHAR(24) NOT NULL,
  `reason` VARCHAR(255) NOT NULL,
  `warned_at` INT(11) NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `idx_player` (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ================================================================
-- 5. جدول الحظر (admin_bans)
-- ================================================================
CREATE TABLE IF NOT EXISTS `admin_bans` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `username` VARCHAR(24) NOT NULL,
  `ip_address` VARCHAR(45) NOT NULL,
  `admin_name` VARCHAR(24) NOT NULL,
  `reason` VARCHAR(255) NOT NULL,
  `banned_at` INT(11) NOT NULL,
  `expires_at` INT(11) NOT NULL DEFAULT 0,
  `active` TINYINT(1) NOT NULL DEFAULT 1,
  PRIMARY KEY (`id`),
  UNIQUE KEY `unique_username` (`username`),
  INDEX `idx_active` (`active`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ================================================================
-- 6. جدول ساعات الأسبوع الإدارية (admin_weekly_hours)
-- ================================================================
CREATE TABLE IF NOT EXISTS `admin_weekly_hours` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `admin_id` INT(11) NOT NULL,
  `admin_name` VARCHAR(24) NOT NULL,
  `week_start` INT(11) NOT NULL,
  `seconds_online` INT(11) NOT NULL DEFAULT 0,
  `reports_handled` INT(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  INDEX `idx_admin_week` (`admin_id`, `week_start`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ================================================================
-- 7. جدول إعدادات السيرفر (server_settings)
-- ================================================================
CREATE TABLE IF NOT EXISTS `server_settings` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `setting_key` VARCHAR(64) NOT NULL,
  `setting_value` TEXT NOT NULL,
  `updated_by` VARCHAR(24) NOT NULL DEFAULT '',
  `updated_at` INT(11) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE KEY `unique_key` (`setting_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ================================================================
-- 8. تعيين المالك الرئيسي walid om تلقائياً
-- ================================================================
-- بعد أن يسجل walid om دخوله مرة واحدة، قم بتنفيذ:
-- UPDATE `admin_data` SET `admin_level` = 6, `rank_name` = 'المالك', `tag` = '[OWNER]', `immunity` = 2 WHERE `username` = 'walid om';

-- ============================================================================
--  البلدية
-- ============================================================================
-- ============================================================
--  WISE-CITY-RP :: City Hall & Civic Documents
--  Database : wise_city_db
-- ============================================================

-- ------------------------------------------------------------
-- Player civic documents table
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `civic_documents` (
  `player_id`         INT(11)      NOT NULL,          -- matches `players.id`
  `has_id_card`       TINYINT(1)   NOT NULL DEFAULT 0,
  `has_passport`      TINYINT(1)   NOT NULL DEFAULT 0,
  `has_driver_license` TINYINT(1)  NOT NULL DEFAULT 0,
  `license_points`    TINYINT(3)   NOT NULL DEFAULT 12, -- 12 = clean record
  `license_suspended` TINYINT(1)   NOT NULL DEFAULT 0,
  `id_issued_at`      DATETIME     DEFAULT NULL,
  `passport_issued_at` DATETIME    DEFAULT NULL,
  `license_issued_at` DATETIME     DEFAULT NULL,
  `civic_job`         TINYINT(3)   NOT NULL DEFAULT 0,  -- 0 = none
  `civic_job_pay`     INT(11)      NOT NULL DEFAULT 0,
  `civic_job_started` DATETIME     DEFAULT NULL,
  PRIMARY KEY (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- City Hall transaction log (audit trail)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `cityhall_log` (
  `log_id`     INT(11)      NOT NULL AUTO_INCREMENT,
  `player_id`  INT(11)      NOT NULL,
  `action`     VARCHAR(32)  NOT NULL,   -- BUY_ID, BUY_PASSPORT, GET_LICENSE, JOB_PICK...
  `amount`     INT(11)      NOT NULL DEFAULT 0,
  `created_at` DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
--  الإسعاف
-- ============================================================================
-- ============================================================
--  WISE-CITY-RP :: EMS & Hospital System
--  Database : wise_city_db
-- ============================================================

-- ------------------------------------------------------------
-- EMS staff (linked to players)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `ems_staff` (
  `player_id`     INT(11)      NOT NULL,
  `badge_number`  VARCHAR(10)  NOT NULL,
  `rank`          TINYINT(3)   NOT NULL DEFAULT 1,  -- 1..6
  `on_duty`       TINYINT(1)   NOT NULL DEFAULT 0,
  `hired_at`      DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `total_revives` INT(11)      NOT NULL DEFAULT 0,
  `total_deaths`  INT(11)      NOT NULL DEFAULT 0,
  PRIMARY KEY (`player_id`),
  UNIQUE KEY `unq_badge` (`badge_number`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Medical records (persistent for every citizen)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `medical_records` (
  `player_id`       INT(11)      NOT NULL,
  `blood_type`      VARCHAR(4)   NOT NULL DEFAULT 'O+',
  `allergies`       VARCHAR(128) NOT NULL DEFAULT '',
  `chronic`         VARCHAR(128) NOT NULL DEFAULT '',
  `wounds`          INT(11)      NOT NULL DEFAULT 0,   -- active wound count
  `bleeding_level`  TINYINT(3)   NOT NULL DEFAULT 0,   -- 0..3
  `last_hospital`   DATETIME     DEFAULT NULL,
  `icu_visits`      INT(11)      NOT NULL DEFAULT 0,
  `onsite_revives`  INT(11)      NOT NULL DEFAULT 0,
  `deaths`          INT(11)      NOT NULL DEFAULT 0,
  PRIMARY KEY (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Emergency calls
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `ems_calls` (
  `id`            INT(11)      NOT NULL AUTO_INCREMENT,
  `caller_id`     INT(11)      NOT NULL,
  `caller_name`   VARCHAR(24)  NOT NULL,
  `category`      TINYINT(3)   NOT NULL DEFAULT 1,     -- 1 medical, 2 fire, 3 police
  `description`   VARCHAR(255) NOT NULL DEFAULT '',
  `pos_x`         FLOAT        NOT NULL DEFAULT 0,
  `pos_y`         FLOAT        NOT NULL DEFAULT 0,
  `pos_z`         FLOAT        NOT NULL DEFAULT 0,
  `assigned_to`   INT(11)      NOT NULL DEFAULT 0,
  `status`        TINYINT(3)   NOT NULL DEFAULT 0,     -- 0 pending, 1 accepted, 2 closed
  `created_at`    DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Hospital inventory (pharmacy + medical supply)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `hospital_inventory` (
  `item`         VARCHAR(32) NOT NULL,
  `quantity`     INT(11)     NOT NULL DEFAULT 0,
  `updated_at`   DATETIME    NOT NULL DEFAULT CURRENT_TIMESTAMP
                            ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`item`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Seed items
INSERT IGNORE INTO `hospital_inventory` (`item`,`quantity`) VALUES
  ('bandage',       500),
  ('medkit',        200),
  ('morphine',      150),
  ('antidote',       80),
  ('defibrillator',  20);

-- ------------------------------------------------------------
-- EMS action log (audit trail)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `ems_logs` (
  `log_id`     INT(11)     NOT NULL AUTO_INCREMENT,
  `player_id`  INT(11)     NOT NULL,
  `action`     VARCHAR(32) NOT NULL,  -- CALL_911, ACCEPT, REVIVE, HOSPITALIZE, DUTY_ON...
  `target_id`  INT(11)     NOT NULL DEFAULT 0,
  `details`    VARCHAR(255) NOT NULL DEFAULT '',
  `created_at` DATETIME    NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
--  العصابات
-- ============================================================================
-- ============================================================
--  WISE-CITY-RP :: Gangs, Luxury HQ, Turf Wars & Smoke Nades
--  Database : wise_city_db
-- ============================================================

-- ------------------------------------------------------------
-- Gangs
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `gangs` (
  `id`            INT(11)      NOT NULL AUTO_INCREMENT,
  `name`          VARCHAR(32)  NOT NULL,
  `tag`           VARCHAR(8)   NOT NULL,
  `color`         INT(11)      NOT NULL DEFAULT 0xFFFFFFFF,
  `leader_id`     INT(11)      NOT NULL DEFAULT 0,
  `funds`         INT(11)      NOT NULL DEFAULT 0,
  `level`         TINYINT(3)   NOT NULL DEFAULT 1,
  `wins`          INT(11)      NOT NULL DEFAULT 0,
  `losses`        INT(11)      NOT NULL DEFAULT 0,
  `created_at`    DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `unq_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Gang members
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `gang_members` (
  `player_id`   INT(11)      NOT NULL,
  `gang_id`     INT(11)      NOT NULL,
  `rank`        TINYINT(3)   NOT NULL DEFAULT 1,   -- 1 member, 2 officer, 3 leader
  `joined_at`   DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_id`),
  KEY `idx_gang` (`gang_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Luxury gang headquarters (inside + outside)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `gang_hqs` (
  `gang_id`       INT(11)      NOT NULL,
  `interior_id`   TINYINT(3)   NOT NULL DEFAULT 5,
  `ext_x`         FLOAT        NOT NULL DEFAULT 0,
  `ext_y`         FLOAT        NOT NULL DEFAULT 0,
  `ext_z`         FLOAT        NOT NULL DEFAULT 0,
  `int_x`         FLOAT        NOT NULL DEFAULT 225.7,
  `int_y`         FLOAT        NOT NULL DEFAULT 1022.0,
  `int_z`         FLOAT        NOT NULL DEFAULT 1084.0,
  `safe_money`    INT(11)      NOT NULL DEFAULT 0,
  `vault_weapons` INT(11)      NOT NULL DEFAULT 0,
  `luxury_level`  TINYINT(3)   NOT NULL DEFAULT 1, -- 1-5, unlocks cosmetics
  PRIMARY KEY (`gang_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Turfs (المناطق)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `turfs` (
  `id`          INT(11)      NOT NULL AUTO_INCREMENT,
  `name`        VARCHAR(48)  NOT NULL,
  `cx`          FLOAT        NOT NULL,
  `cy`          FLOAT        NOT NULL,
  `cz`          FLOAT        NOT NULL,
  `radius`      FLOAT        NOT NULL DEFAULT 80.0,
  `owner_gang`  INT(11)      NOT NULL DEFAULT 0,     -- 0 = neutral
  `locked`      TINYINT(1)   NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Turf wars (الاستيلاء على المناطق)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `turf_wars` (
  `id`             INT(11)      NOT NULL AUTO_INCREMENT,
  `turf_id`        INT(11)      NOT NULL,
  `attacker_gang`  INT(11)      NOT NULL,
  `defender_gang`  INT(11)      NOT NULL,
  `start_time`     INT(11)      NOT NULL,
  `end_time`       INT(11)      NOT NULL,
  `attacker_score` INT(11)      NOT NULL DEFAULT 0,
  `defender_score` INT(11)      NOT NULL DEFAULT 0,
  `winner_gang`    INT(11)      NOT NULL DEFAULT 0,
  `status`         TINYINT(1)   NOT NULL DEFAULT 1,  -- 1 = active
  PRIMARY KEY (`id`),
  KEY `idx_status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Smoke grenade inventory + turf playtime
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `smoke_grenades` (
  `player_id`      INT(11)      NOT NULL,
  `count`          INT(11)      NOT NULL DEFAULT 0,
  `turf_playtime`  INT(11)      NOT NULL DEFAULT 0, -- cumulative seconds inside turfs
  PRIMARY KEY (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Audit log for turf wars
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `turf_war_log` (
  `log_id`     INT(11)     NOT NULL AUTO_INCREMENT,
  `turf_id`    INT(11)     NOT NULL,
  `attacker`   INT(11)     NOT NULL,
  `defender`   INT(11)     NOT NULL,
  `winner`     INT(11)     NOT NULL,
  `score_a`    INT(11)     NOT NULL,
  `score_d`    INT(11)     NOT NULL,
  `ended_at`   DATETIME    NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
--  المنازل
-- ============================================================================
-- ============================================================
--  WISE-CITY-RP :: Housing & Real Estate System
--  Database : wise_city_db
-- ============================================================

CREATE TABLE IF NOT EXISTS `houses` (
  `id`              INT(11)      NOT NULL AUTO_INCREMENT,
  `owner_name`      VARCHAR(24)  NOT NULL DEFAULT 'None',
  `price`           INT(11)      NOT NULL DEFAULT 0,
  `interior_id`     TINYINT(3)   NOT NULL DEFAULT 0,
  `locked`          TINYINT(1)   NOT NULL DEFAULT 1,
  `rent_price`      INT(11)      NOT NULL DEFAULT 0,
  `is_rented`       TINYINT(1)   NOT NULL DEFAULT 0,
  `rent_due`        INT(11)      NOT NULL DEFAULT 0,
  `safe_money`      INT(11)      NOT NULL DEFAULT 0,
  `safe_drugs`      INT(11)      NOT NULL DEFAULT 0,
  `safe_weapons`    INT(11)      NOT NULL DEFAULT 0,
  `enter_x`         FLOAT        NOT NULL DEFAULT 0.0,
  `enter_y`         FLOAT        NOT NULL DEFAULT 0.0,
  `enter_z`         FLOAT        NOT NULL DEFAULT 0.0,
  `exit_x`          FLOAT        NOT NULL DEFAULT 0.0,
  `exit_y`          FLOAT        NOT NULL DEFAULT 0.0,
  `exit_z`          FLOAT        NOT NULL DEFAULT 0.0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Optional: log table for rent transactions
CREATE TABLE IF NOT EXISTS `rent_log` (
  `log_id`    INT(11)     NOT NULL AUTO_INCREMENT,
  `house_id`  INT(11)     NOT NULL,
  `owner`     VARCHAR(24) NOT NULL,
  `amount`    INT(11)     NOT NULL,
  `paid_at`   DATETIME    NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
--  الحانة والمسرح
-- ============================================================================
-- ============================================================
--  WISE-CITY-RP :: Tavern, Theater & Social System
--  Database : wise_city_db
-- ============================================================

-- ------------------------------------------------------------
-- Tavern drink & food menu (inventory)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `tavern_inventory` (
  `item_id`    INT(11)      NOT NULL AUTO_INCREMENT,
  `item_key`   VARCHAR(32)  NOT NULL,
  `item_name`  VARCHAR(64)  NOT NULL,
  `category`   TINYINT(3)   NOT NULL DEFAULT 1,  -- 1 drink, 2 food, 3 luxury
  `price`      INT(11)      NOT NULL DEFAULT 0,
  `stock`      INT(11)      NOT NULL DEFAULT 100,
  `alcohol`    TINYINT(3)   NOT NULL DEFAULT 0,  -- 0..3
  PRIMARY KEY (`item_id`),
  UNIQUE KEY `unq_key` (`item_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT IGNORE INTO `tavern_inventory` (`item_key`,`item_name`,`category`,`price`,`stock`,`alcohol`) VALUES
  ('whiskey',    'ويسكي فاخر',        1, 500,  200, 3),
  ('vodka',      'فودكا',              1, 400,  200, 3),
  ('champagne',  'شامبانيا',           3, 1500, 100, 2),
  ('beer',       'بيرة',               1, 150,  500, 1),
  ('coffee',     'قهوة',               1, 100,  500, 0),
  ('juice',      'عصير طازج',          1, 120,  500, 0),
  ('steak',      'ستيك مشوي',          2, 800,  150, 0),
  ('pasta',      'باستا إيطالية',      2, 600,  150, 0),
  ('dessert',    'حلوى الشوكولاتة',    2, 350,  200, 0);

-- ------------------------------------------------------------
-- Running tabs (bills at tavern)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `tavern_tabs` (
  `tab_id`      INT(11)      NOT NULL AUTO_INCREMENT,
  `player_id`   INT(11)      NOT NULL,
  `items_json`  TEXT         NOT NULL,
  `total`       INT(11)      NOT NULL DEFAULT 0,
  `status`      TINYINT(3)   NOT NULL DEFAULT 0,  -- 0 open, 1 paid
  `opened_at`   DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`tab_id`),
  KEY `idx_player` (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Theater event bookings
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `theater_events` (
  `event_id`     INT(11)      NOT NULL AUTO_INCREMENT,
  `organizer_id` INT(11)      NOT NULL,
  `title`        VARCHAR(96)  NOT NULL,
  `description`  TEXT         NOT NULL,
  `category`     TINYINT(3)   NOT NULL DEFAULT 1, -- 1 show, 2 meeting, 3 ceremony
  `ticket_price` INT(11)      NOT NULL DEFAULT 0,
  `max_seats`    INT(11)      NOT NULL DEFAULT 120,
  `sold_seats`   INT(11)      NOT NULL DEFAULT 0,
  `start_time`   INT(11)      NOT NULL,
  `status`       TINYINT(3)   NOT NULL DEFAULT 1, -- 1 scheduled, 2 live, 3 ended, 4 cancelled
  `created_at`   DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`event_id`),
  KEY `idx_status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Ticket reservations
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `theater_reservations` (
  `res_id`     INT(11)      NOT NULL AUTO_INCREMENT,
  `event_id`   INT(11)      NOT NULL,
  `player_id`  INT(11)      NOT NULL,
  `seats`      INT(11)      NOT NULL DEFAULT 1,
  `paid`       INT(11)      NOT NULL DEFAULT 0,
  `reserved_at` DATETIME    NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`res_id`),
  KEY `idx_event` (`event_id`),
  KEY `idx_player` (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Per-player social preferences
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `social_settings` (
  `player_id`      INT(11)      NOT NULL,
  `broadcast_optin` TINYINT(1)  NOT NULL DEFAULT 1,
  `vip_member`     TINYINT(1)   NOT NULL DEFAULT 0,
  `vip_expires`    INT(11)      NOT NULL DEFAULT 0,
  `emote_fav`      VARCHAR(32)  NOT NULL DEFAULT 'wave',
  PRIMARY KEY (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- Social interaction audit (optional)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `social_logs` (
  `log_id`     INT(11)      NOT NULL AUTO_INCREMENT,
  `player_id`  INT(11)      NOT NULL,
  `action`     VARCHAR(32)  NOT NULL,
  `target_id`  INT(11)      NOT NULL DEFAULT 0,
  `details`    VARCHAR(255) NOT NULL DEFAULT '',
  `created_at` DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ============================================================================
--  الهاتف والسوق السوداء
-- ============================================================================
-- ============================================================
--  WISE-CITY-RP :: Wise-Phone OS + Black Market
--  Database : wise_city_db
-- ============================================================

-- ------------------------------------------------------------
-- WISEGRAM (social media)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `wisegram_posts` (
  `post_id`     INT(11)      NOT NULL AUTO_INCREMENT,
  `author_id`   INT(11)      NOT NULL,
  `content`     VARCHAR(255) NOT NULL,
  `image_key`   VARCHAR(32)  NOT NULL DEFAULT '',
  `likes`       INT(11)      NOT NULL DEFAULT 0,
  `comments`    INT(11)      NOT NULL DEFAULT 0,
  `created_at`  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`post_id`),
  KEY `idx_author` (`author_id`),
  KEY `idx_created` (`created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `wisegram_likes` (
  `post_id`   INT(11) NOT NULL,
  `player_id` INT(11) NOT NULL,
  `liked_at`  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`post_id`,`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `wisegram_comments` (
  `comment_id` INT(11)      NOT NULL AUTO_INCREMENT,
  `post_id`    INT(11)      NOT NULL,
  `author_id`  INT(11)      NOT NULL,
  `content`    VARCHAR(200) NOT NULL,
  `created_at` DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`comment_id`),
  KEY `idx_post` (`post_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- WISECAB (taxi hailing)
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `wisecab_requests` (
  `req_id`      INT(11)      NOT NULL AUTO_INCREMENT,
  `passenger_id` INT(11)     NOT NULL,
  `driver_id`   INT(11)      NOT NULL DEFAULT 0,
  `pos_x`       FLOAT        NOT NULL DEFAULT 0,
  `pos_y`       FLOAT        NOT NULL DEFAULT 0,
  `pos_z`       FLOAT        NOT NULL DEFAULT 0,
  `dest_x`      FLOAT        NOT NULL DEFAULT 0,
  `dest_y`      FLOAT        NOT NULL DEFAULT 0,
  `dest_z`      FLOAT        NOT NULL DEFAULT 0,
  `fare`        INT(11)      NOT NULL DEFAULT 0,
  `status`      TINYINT(3)   NOT NULL DEFAULT 0, -- 0 pending, 1 accepted, 2 arrived, 3 done, 4 cancelled
  `created_at`  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`req_id`),
  KEY `idx_status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- CLASSIFIEDS marketplace
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `classifieds` (
  `ad_id`       INT(11)      NOT NULL AUTO_INCREMENT,
  `seller_id`   INT(11)      NOT NULL,
  `title`       VARCHAR(96)  NOT NULL,
  `description` VARCHAR(255) NOT NULL,
  `price`       INT(11)      NOT NULL,
  `category`    TINYINT(3)   NOT NULL DEFAULT 1, -- 1 vehicle, 2 property, 3 item, 4 service
  `status`      TINYINT(3)   NOT NULL DEFAULT 1, -- 1 active, 2 sold, 3 removed
  `created_at`  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`ad_id`),
  KEY `idx_status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- MOBILE BANKING
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `bank_accounts` (
  `player_id`     INT(11)      NOT NULL,
  `balance`       BIGINT(20)   NOT NULL DEFAULT 0,
  `pin_hash`      VARCHAR(64)  NOT NULL DEFAULT '',
  `updated_at`    DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `bank_transactions` (
  `tx_id`       INT(11)      NOT NULL AUTO_INCREMENT,
  `from_id`     INT(11)      NOT NULL DEFAULT 0,
  `to_id`       INT(11)      NOT NULL DEFAULT 0,
  `amount`      INT(11)      NOT NULL,
  `tx_type`     VARCHAR(16)  NOT NULL,   -- TRANSFER, DEPOSIT, WITHDRAW, PURCHASE
  `note`        VARCHAR(96)  NOT NULL DEFAULT '',
  `created_at`  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`tx_id`),
  KEY `idx_from` (`from_id`),
  KEY `idx_to` (`to_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `sms_messages` (
  `sms_id`     INT(11)      NOT NULL AUTO_INCREMENT,
  `to_id`      INT(11)      NOT NULL,
  `from_label` VARCHAR(48)  NOT NULL DEFAULT 'WISE-BANK',
  `body`       VARCHAR(255) NOT NULL,
  `is_read`    TINYINT(1)   NOT NULL DEFAULT 0,
  `created_at` DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`sms_id`),
  KEY `idx_to` (`to_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- ------------------------------------------------------------
-- BLACK MARKET
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS `blackmarket_locations` (
  `loc_id`      INT(11)      NOT NULL AUTO_INCREMENT,
  `name`        VARCHAR(64)  NOT NULL,
  `x`           FLOAT        NOT NULL,
  `y`           FLOAT        NOT NULL,
  `z`           FLOAT        NOT NULL,
  `interior`    TINYINT(3)   NOT NULL DEFAULT 0,
  `min_rank`    TINYINT(3)   NOT NULL DEFAULT 1, -- minimum criminal rank
  `active`      TINYINT(1)   NOT NULL DEFAULT 1,
  PRIMARY KEY (`loc_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `blackmarket_listings` (
  `listing_id`  INT(11)      NOT NULL AUTO_INCREMENT,
  `seller_id`   INT(11)      NOT NULL,
  `item_key`    VARCHAR(32)  NOT NULL,
  `quantity`    INT(11)      NOT NULL DEFAULT 1,
  `price`       INT(11)      NOT NULL,
  `location_id` INT(11)      NOT NULL DEFAULT 0,
  `status`      TINYINT(3)   NOT NULL DEFAULT 1, -- 1 active, 2 sold, 3 removed
  `created_at`  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`listing_id`),
  KEY `idx_status` (`status`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `blackmarket_channels` (
  `channel_id`  INT(11)      NOT NULL AUTO_INCREMENT,
  `gang_id`     INT(11)      NOT NULL DEFAULT 0,
  `name`        VARCHAR(32)  NOT NULL,
  `passphrase`  VARCHAR(64)  NOT NULL,
  `created_at`  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`channel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `blackmarket_messages` (
  `msg_id`     INT(11)      NOT NULL AUTO_INCREMENT,
  `channel_id` INT(11)      NOT NULL,
  `sender_id`  INT(11)      NOT NULL,
  `body`       VARCHAR(255) NOT NULL,
  `created_at` DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`msg_id`),
  KEY `idx_channel` (`channel_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `police_raids` (
  `raid_id`    INT(11)      NOT NULL AUTO_INCREMENT,
  `location_id` INT(11)     NOT NULL,
  `officer_id` INT(11)      NOT NULL,
  `raid_time`  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `success`    TINYINT(1)   NOT NULL DEFAULT 0,
  `seized_value` INT(11)    NOT NULL DEFAULT 0,
  PRIMARY KEY (`raid_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Seed black market locations
INSERT IGNORE INTO `blackmarket_locations` (name,x,y,z,interior,min_rank) VALUES
  ('المستودع المهجور',  -2078.5, -90.4, 35.3, 0, 1),
  ('الكهف السري',      -1400.0, 1500.0, 5.0, 0, 3),
  ('نادي القمار الخفي', 500.0,  -20.0, 1000.0, 3, 5);
