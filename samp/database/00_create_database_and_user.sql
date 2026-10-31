-- نفّذه بحساب root. غيّر كلمة المرور ثم ضعها نفسها في gamemodes/includes/wc_secrets.inc
CREATE DATABASE IF NOT EXISTS `wise_city_db` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER IF NOT EXISTS 'wisecity_user'@'localhost' IDENTIFIED BY 'CHANGE_ME_STRONG_PASSWORD';
GRANT ALL PRIVILEGES ON `wise_city_db`.* TO 'wisecity_user'@'localhost';
FLUSH PRIVILEGES;
