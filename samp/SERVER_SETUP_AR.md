# 🏙️ دليل تشغيل وإعداد سيرفر Wise City RP على خادم افتراضي خاص (VPS)

يقدم هذا الدليل خطوات عملية ومفصلة وشاملة لتثبيت وتشغيل سيرفر **Wise City RP** (المبني على مشغل **open.mp** وقاعدة بيانات **MySQL**) على خادم افتراضي بنظام لينكس (Ubuntu أو Debian)، مع شرح إعداد الجدار الناري وكيفية دخول اللاعبين عبر هواتف أندرويد (Android).

---

## 📋 المتطلبات الأساسية للخادم (VPS System Requirements)
* **نظام التشغيل الموصى به:** Ubuntu 22.04 LTS أو Ubuntu 24.04 LTS أو Debian 11/12 (64-bit).
* **المعالج:** 1 vCPU (يفضل 2 vCPU للأداء السلس).
* **الذاكرة العشوائية (RAM):** 1 جيجابايت كحد أدنى (يفضل 2 جيجابايت لاستيعاب MySQL ومشغل open.mp).
* **المساحة التخزينية:** 10 جيجابايت SSD.
* **صلاحيات الوصول:** حساب `root` أو مستخدم يمتلك صلاحيات `sudo`.

---

## 1️⃣ الخطوة الأولى: تحديث النظام وتثبيت المكتبات (32-Bit Libraries)
نواة خادم open.mp وإضافات SA-MP التقليدية مبنية بتقنية 32-bit، لذلك يجب تفعيل معمارية `i386` وتثبيت المكتبات المطلوبة:

```bash
sudo dpkg --add-architecture i386
sudo apt update && sudo apt upgrade -y
sudo apt install -y libc6:i386 libstdc++6:i386 zlib1g:i386 libssl-dev:i386 libssl3:i386 \
                    mariadb-server mariadb-client curl wget jq unzip git
```

---

## 2️⃣ الخطوة الثانية: إعداد قاعدة البيانات MariaDB / MySQL
1. تأكد من تشغيل خدمة قاعدة البيانات:
```bash
sudo systemctl enable --now mariadb
```

2. قم بتأمين تثبيت قاعدة البيانات (اختياري لكن موصى به):
```bash
sudo mysql_secure_installation
```

3. الدخول إلى MySQL كمسؤول وإنشاء قاعدة البيانات والمستخدم:
```bash
sudo mysql -u root
```

داخل موجه MySQL نفّذ الأوامر التالية (استبدل `YOUR_STRONG_PASSWORD` بكلمة مرور قوية خاصة بك):
```sql
CREATE DATABASE IF NOT EXISTS `wise_city_db` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER IF NOT EXISTS 'wisecity_user'@'localhost' IDENTIFIED BY 'YOUR_STRONG_PASSWORD';
GRANT ALL PRIVILEGES ON `wise_city_db`.* TO 'wisecity_user'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

4. استيراد جداول ومخطط السيرفر من ملف `install.sql`:
```bash
mysql -u wisecity_user -p'YOUR_STRONG_PASSWORD' wise_city_db < /path/to/samp/database/install.sql
```

---

## 3️⃣ الخطوة الثالثة: إعداد ملف الأسرار والتجميع (Compilation & Secrets)
1. افتح ملف `samp/gamemodes/includes/wc_secrets.inc`:
```c
#define WC_DB_HOST      "127.0.0.1"
#define WC_DB_USER      "wisecity_user"
#define WC_DB_PASS      "YOUR_STRONG_PASSWORD"   // كلمة المرور التي اخترتها في الخطوة 2
#define WC_DB_NAME      "wise_city_db"

// معرّف حساب المالك (أول لاعب يسجل دخوله يكون رقمه 1)
#define WC_OWNER_ACCOUNT_ID     1
```

2. قم بتجميع المود للحصول على ملف `wise_city.amx`:
* يمكنك التجميع سحابياً عبر سير العمل الخاص بنا في GitHub Actions (`Compile SAMP gamemode`) وتحميل ملف `wise_city.amx` الجاهز.
* أو التجميع يدوياً على السيرفر باستخدام أداة `pawncc 3.10.10`:
```bash
./pawncc wise_city.pwn -owise_city.amx -iincludes -imodules -i../libs/omp-stdlib -d3
```

---

## 4️⃣ الخطوة الرابعة: هيكلة مجلد مشغل open.mp وتثبيت الإضافات (Plugins)
قم بإنشاء مجلد السيرفر بالهيكلية الرسمية لـ open.mp:
```
samp-server/
├── omp-server          (الملف التنفيذي لمشغل open.mp)
├── config.json         (ملف الإعدادات الرئيسي)
├── gamemodes/
│   └── wise_city.amx   (المود المجمّع)
├── plugins/
│   ├── mysql.so        (إضافة MySQL R41-4)
│   ├── sscanf.so       (إضافة sscanf2)
│   ├── streamer.so     (إضافة Streamer)
│   └── samp-bcrypt.so  (إضافة bcrypt لتشفير كلمات المرور)
├── components/
│   └── pawncmd.so      (إضافة Pawn.CMD الخاصة بـ open.mp)
└── scriptfiles/
```

### تحميل الإضافات الرسمية من GitHub:
1. **مشغل open.mp:** من مستودع `openmultiplayer/open.mp/releases/latest` (حزمة `open.mp-linux-x86.tar.gz`).
2. **إضافة MySQL:** من مستودع `pBlueG/SA-MP-MySQL/releases/tag/R41-4` (حزمة `mysql-R41-4-Debian-static.tar.gz`).
3. **إضافة sscanf:** من مستودع `Y-Less/sscanf/releases/latest` (حزمة `sscanf-*-linux.tar.gz`).
4. **إضافة streamer:** من مستودع `samp-incognito/samp-streamer-plugin/releases/latest`.
5. **إضافة samp-bcrypt:** من مستودع `Sreyas-Sreelal/samp-bcrypt/releases/latest` (حزمة `samp-bcrypt-linux-x86.zip`).
6. **إضافة Pawn.CMD:** من مستودع `katursis/Pawn.CMD/releases/latest` (حزمة `pawncmd-*-omp-linux.tar.gz`).

---

## 5️⃣ الخطوة الخامسة: إعداد ملف `config.json`
أنشئ ملف `config.json` في مجلد السيرفر الرئيسي بالصيغة التالية:
```json
{
  "name": "WISE-CITY-RP | Arabic Roleplay",
  "language": "Arabic",
  "website": "wise-city-rp.com",
  "max_players": 100,
  "password": "",
  "game": { "mode": "Roleplay" },
  "network": { "port": 7777 },
  "rcon": { "enable": true, "password": "CHOOSE_STRONG_RCON_PASS" },
  "pawn": {
    "main_scripts": [ "wise_city 1" ],
    "legacy_plugins": [ "sscanf", "mysql", "samp-bcrypt", "streamer" ]
  }
}
```

---

## 6️⃣ الخطوة السادسة: ضبط الجدار الناري (Firewall Configuration)
يعتمد بروتوكول SA-MP / open.mp على منفذ **UDP 7777**. يجب فتح هذا المنفذ في جدار حماية السيرفر:

1. إذا كنت تستخدم `ufw`:
```bash
sudo ufw allow 7777/udp comment "Wise City SA-MP UDP Port"
sudo ufw reload
```

2. إذا كان الخادم على مزود سحابي (مثل AWS أو Oracle أو DigitalOcean أو Hetzner):
* تأكد من إضافة قاعدة في **Security Groups / Firewall**:
  * البروتوكول: **UDP**
  * المنفذ: **7777**
  * المصدر: **0.0.0.0/0 (Anywhere)**

---

## 7️⃣ الخطوة السابعة: تشغيل السيرفر في الخلفية ومراقبته
لتشغيل السيرفر باستمرار حتى بعد إغلاق جلسة SSH، استخدم إما `screen` أو `systemd`:

### الطريقة الأولى: باستخدام Screen
```bash
# إنشاء شاشة جديدة
screen -S wisecity

# الانتقال لمجلد السيرفر وتشغيله
cd /path/to/samp-server
./omp-server

# للخروج من الشاشة مع بقاء السيرفر يعمل: اضغط Ctrl + A ثم D
# للعودة إلى الشاشة لاحقاً:
screen -r wisecity
```

### الطريقة الثانية: باستخدام Systemd (موصى بها لإعادة التشغيل التلقائي)
أنشئ ملف الخدمة:
```bash
sudo nano /etc/systemd/system/wisecity.service
```
أضف المحتوى التالي:
```ini
[Unit]
Description=Wise City RP open.mp Server
After=network.target mariadb.service

[Service]
Type=simple
User=root
WorkingDirectory=/path/to/samp-server
ExecStart=/path/to/samp-server/omp-server
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```
ثم فعّل الخدمة:
```bash
sudo systemctl daemon-reload
sudo systemctl enable --now wisecity
sudo systemctl status wisecity
```

---

## 8️⃣ الخطوة الثامنة: كيفية اتصال اللاعبين عبر هواتف الأندرويد (Android Client)
يمكن للاعبين الاتصال بالسيرفر مباشرة من هواتفهم الذكية بنظام Android باتباع الخطوات التالية:

1. **تحميل تطبيق المشغل (Launcher):**
   * تحميل تطبيق **SA-MP Launcher** أو **MonetLoader** أو المشغل المخصص المتوفر على متجر Google Play.
2. **تجهيز ملفات اللعبة (Game Data):**
   * تثبيت ملفات GTA: San Andreas (النسخة المتوافقة v1.08 أو v2.00) داخل ذاكرة الهاتف `Android/data/com.rockstargames.gtasa/`.
3. **تطبيق مودات وتعديلات السيرفر:**
   * وضع مود السيارة المرفقة `comet.dff` (بورشه 911 GT3) في مجلد `samp/client_mods/porsche_911_gt3/` داخل مسار ملفات المشغل للاستمتاع بالمجسمات فائقة الدقة.
4. **إضافة السيرفر للمفضلة (Favorites):**
   * الدخول إلى خيار السيرفرات المفضلة (Favorites).
   * إدخال الآي بي (IP) الخاص بسيرفرك مع المنفذ:
     ```
     IP_YOUR_VPS:7777
     ```
5. **اختيار اسم شخصية واقعي (Roleplay Name):**
   * يجب كتابة اسم الشخصية بصيغة تقمص الأدوار: `Firstname_Lastname` (مثال: `Omar_Khaled`).
6. **الاتصال وبدء اللعب:**
   * الضغط على زر الاتصال (Connect)، وسيبدأ تحميل شاشات تسجيل الدخول وتوثيق الشخصية في مدينة Wise City.
