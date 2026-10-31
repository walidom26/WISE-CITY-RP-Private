/* ============================================================================
    WISE-CITY-RP  |  نظام الإدارة والعقوبات والبلاغات
    الوحدة: wc_admin.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

#define DIALOG_ADMIN_STATS       2000
#define DIALOG_ADMIN_LIST        2001
#define DIALOG_OWNER_MENU        2002
#define DIALOG_BANNED            2003

/*
 * ============================================================================
 *  WISE-CITY-RP | نظام الإدارة الشامل ولوحة التحكم
 *  ملف: Wise_City_Admin_System.pwn
 *  الإصدار: 1.0.0
 *  التوافق: open.mp | Pawn 3.10.12+ | MySQL R41-4+
 *  الوصف: نظام إداري متكامل مع 6 مستويات، إحصائيات، أوامر ميدانية، وحصانة المالك
 *  المالك الرئيسي: walid om
 * ============================================================================
 */

// ============================================================================
// 1. التعريفات والثوابت
// ============================================================================

#define ADMIN_SCRIPT_VERSION        "1.0.0"
#define MAX_ADMIN_LEVEL             6
#define OWNER_NAME                  "walid om"   // اسم العرض فقط (لا يُستخدم للتحقق)
// المالك يُعرَّف بمعرّف الحساب الثابت WC_OWNER_ACCOUNT_ID (wc_secrets.inc) وليس بالاسم
#define IS_OWNER_ACCOUNT(%0)        (pData[%0][pLoggedIn] && pData[%0][pID] == WC_OWNER_ACCOUNT_ID)
#define OWNER_TAG                   "[OWNER]"
#define FOUNDER_TAG                 "[FOUNDER]"

// مستويات الإدارة
#define ADMIN_HELPER                1    // [مساعد / Helper]
#define ADMIN_MODERATOR             2    // [مشرف / Moderator]
#define ADMIN_ADMIN                 3    // [أدمن / Admin]
#define ADMIN_SENIOR                4    // [كبير أدمنية / Senior Admin]
#define ADMIN_HEAD                  5    // [مدير النظام / Head Admin]
#define ADMIN_OWNER                 6    // [المالك / walid om]

// أنواع العقوبات
#define PUNISH_KICK                 0
#define PUNISH_BAN                  1
#define PUNISH_JAIL                 2
#define PUNISH_MUTE                 3
#define PUNISH_WARN                 4

// ألوان الرسائل الإدارية
#define COLOR_ADMIN_CHAT            0xFF6600FF
#define COLOR_ADMIN_ACTION          0x00CCFFFF
#define COLOR_ADMIN_WARN            0xFFAA00FF
#define COLOR_OWNER                 0xFFD700FF
#define COLOR_BAN                   0xFF0000FF
#define COLOR_KICK                  0xFF6600FF
#define COLOR_JAIL                  0x808080FF
#define COLOR_MUTE                  0x00FFAAFF

// أنواع الشات
#define CHAT_TYPE_NORMAL            0
#define CHAT_TYPE_ADMIN             1
#define CHAT_TYPE_VIP               2
#define CHAT_TYPE_FACTION           3

// ============================================================================
// 2. التضمينات
// ============================================================================


// ============================================================================
// 3. المتغيرات العامة
// ============================================================================


// بيانات الإدارة للاعب
enum E_ADMIN_DATA
{
    adID,                       // معرف قاعدة البيانات
    adLevel,                    // المستوى الإداري (0-6)
    adRankName[32],             // اسم الرتبة
    adTag[16],                  // التاغ المخصص
    bool:adDuty,                // هل في الخدمة؟
    bool:adHidden,              // هل مخفي؟
    adReportsHandled,           // عدد البلاغات التي تم التعامل معها
    adPunishmentsIssued,        // عدد العقوبات الصادرة
    adKicksIssued,              // عدد الطرد
    adBansIssued,               // عدد الحظر
    adJailsIssued,              // عدد السجن
    adMutesIssued,              // عدد الميوت
    adWarningsIssued,           // عدد التحذيرات
    adWeeklySeconds,            // عدد ثواني الخدمة الأسبوعية
    adTotalSeconds,             // إجمالي ثواني الخدمة
    adWarningsReceived,         // التحذيرات الموجهة له
    adLastLogin,                // آخر دخول
    adLoginTime,                // وقت بدء الجلسة
    adImmunity                  // مستوى الحصانة (0 = لا يوجد، 1 = ضد الطرد، 2 = كامل)
}
new AdminData[MAX_PLAYERS][E_ADMIN_DATA];

// بيانات السجن
enum E_JAIL_DATA
{
    bool:jJailed,
    jAdminID,
    jReason[128],
    jTimeLeft,
    Float:jPosX,
    Float:jPosY,
    Float:jPosZ
}
new JailData[MAX_PLAYERS][E_JAIL_DATA];

// بيانات الميوت
enum E_MUTE_DATA
{
    bool:mMuted,
    mAdminID,
    mReason[128],
    mTimeLeft
}
new MuteData[MAX_PLAYERS][E_MUTE_DATA];

// بيانات التحذيرات
enum E_WARN_DATA
{
    wID,
    wPlayerID,
    wAdminName[MAX_PLAYER_NAME],
    wReason[128],
    wDate[32]
}
new WarnData[MAX_PLAYERS][10][E_WARN_DATA];
new g_WarnCount[MAX_PLAYERS];

// بيانات الرتب
new const g_AdminRankNames[MAX_ADMIN_LEVEL + 1][] =
{
    "لاعب",
    "مساعد",
    "مشرف",
    "أدمن",
    "كبير أدمنية",
    "مدير النظام",
    "المالك"
};

new const g_AdminTags[MAX_ADMIN_LEVEL + 1][] =
{
    "",
    "[HELPER]",
    "[MODERATOR]",
    "[ADMIN]",
    "[SR.ADMIN]",
    "[HEAD ADMIN]",
    "[OWNER]"
};

new const g_AdminColors[MAX_ADMIN_LEVEL + 1] =
{
    0xFFFFFFFF,
    0x00FF00FF,     // مساعد - أخضر
    0x00FFFFFF,     // مشرف - سماوي
    0xFFFF00FF,     // أدمن - أصفر
    0xFF8800FF,     // كبير أدمنية - برتقالي
    0xFF00FFFF,     // مدير النظام - بنفسجي
    0xFFD700FF      // المالك - ذهبي
};

// مؤقتات
new g_AdminTimer;
new g_JailTimer;
new g_MuteTimer;
new g_AdminDutyTimer;

// إحصائيات عامة
new g_TotalAdminsOnline = 0;
new g_ServerStartTime = 0;

// ============================================================================
// 4. النماذج (Forward Declarations)
// ============================================================================

forward LoadAdminData(playerid);
forward SaveAdminData(playerid);
forward SaveAllAdminData();
forward OnAdminDataLoaded(playerid);
forward ApplyAdminPrivileges(playerid);
forward RemoveAdminPrivileges(playerid);
forward IsPlayerOwner(playerid);
forward HasAdminLevel(playerid, level);
forward SendAdminMessage(level, const msg[], color);
forward SendAdminAlert(const msg[], color);
forward LogAdminAction(playerid, const action[], targetid);
forward SendPunishmentLog(adminid, targetid, punishType, const reason[], time);
forward FormatAdminTag(playerid, output[], size);
forward UpdatePlayerTag(playerid);
forward OnAdminTimer();
forward OnJailTimer();
forward OnMuteTimer();
forward CheckPlayerImmunity(adminid, targetid);
forward GetAdminLevelName(level, output[], size);

// ============================================================================
// 5. تهيئة النظام
// ============================================================================

stock Admin_OnGameModeInit()
{


    print("[WISE-CITY-ADMIN] تم الاتصال بقاعدة البيانات الإدارية بنجاح.");

    // إنشاء الجداول
    CreateAdminTables();

    // بدء المؤقتات
    g_AdminTimer = SetTimer("OnAdminTimer", 1000, true);
    g_JailTimer = SetTimer("OnJailTimer", 1000, true);
    g_MuteTimer = SetTimer("OnMuteTimer", 1000, true);
    g_AdminDutyTimer = SetTimer("OnAdminDutyTimer", 1000, true);

    g_ServerStartTime = gettime();
    print("[WISE-CITY-ADMIN] تم تحميل نظام الإدارة الشامل بنجاح.");
    return 1;
}

// --------------------------------------------------
// إنشاء جداول الإدارة
// --------------------------------------------------
stock CreateAdminTables()
{
    // جدول بيانات الإدارة
    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `admin_data` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `player_id` INT(11) NOT NULL, \
        `username` VARCHAR(24) NOT NULL, \
        `admin_level` TINYINT(1) NOT NULL DEFAULT 0, \
        `rank_name` VARCHAR(32) NOT NULL DEFAULT 'لاعب', \
        `tag` VARCHAR(16) NOT NULL DEFAULT '', \
        `duty` TINYINT(1) NOT NULL DEFAULT 0, \
        `hidden` TINYINT(1) NOT NULL DEFAULT 0, \
        `reports_handled` INT(11) NOT NULL DEFAULT 0, \
        `punishments_issued` INT(11) NOT NULL DEFAULT 0, \
        `kicks_issued` INT(11) NOT NULL DEFAULT 0, \
        `bans_issued` INT(11) NOT NULL DEFAULT 0, \
        `jails_issued` INT(11) NOT NULL DEFAULT 0, \
        `mutes_issued` INT(11) NOT NULL DEFAULT 0, \
        `warnings_issued` INT(11) NOT NULL DEFAULT 0, \
        `weekly_seconds` INT(11) NOT NULL DEFAULT 0, \
        `total_seconds` INT(11) NOT NULL DEFAULT 0, \
        `warnings_received` INT(11) NOT NULL DEFAULT 0, \
        `immunity` TINYINT(1) NOT NULL DEFAULT 0, \
        `last_login` INT(11) NOT NULL DEFAULT 0, \
        `assigned_by` VARCHAR(24) NOT NULL DEFAULT '', \
        `assigned_at` INT(11) NOT NULL DEFAULT 0, \
        PRIMARY KEY (`id`), \
        UNIQUE KEY `unique_player` (`player_id`), \
        INDEX `idx_level` (`admin_level`), \
        INDEX `idx_duty` (`duty`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    // جدول سجل العقوبات
    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `admin_punishments` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `admin_id` INT(11) NOT NULL, \
        `admin_name` VARCHAR(24) NOT NULL, \
        `target_id` INT(11) NOT NULL, \
        `target_name` VARCHAR(24) NOT NULL, \
        `punishment_type` TINYINT(1) NOT NULL, \
        `reason` VARCHAR(255) NOT NULL, \
        `duration` INT(11) NOT NULL DEFAULT 0, \
        `punished_at` INT(11) NOT NULL, \
        `expires_at` INT(11) NOT NULL DEFAULT 0, \
        `active` TINYINT(1) NOT NULL DEFAULT 1, \
        PRIMARY KEY (`id`), \
        INDEX `idx_admin` (`admin_id`), \
        INDEX `idx_target` (`target_id`), \
        INDEX `idx_active` (`active`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    // جدول سجل الإجراءات الإدارية
    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `admin_logs` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `admin_id` INT(11) NOT NULL, \
        `admin_name` VARCHAR(24) NOT NULL, \
        `action` VARCHAR(255) NOT NULL, \
        `target_id` INT(11) NOT NULL DEFAULT -1, \
        `ip_address` VARCHAR(45) NOT NULL DEFAULT '', \
        `logged_at` INT(11) NOT NULL, \
        PRIMARY KEY (`id`), \
        INDEX `idx_admin` (`admin_id`), \
        INDEX `idx_time` (`logged_at`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    // جدول التحذيرات
    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `admin_warnings` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `player_id` INT(11) NOT NULL, \
        `admin_name` VARCHAR(24) NOT NULL, \
        `reason` VARCHAR(255) NOT NULL, \
        `warned_at` INT(11) NOT NULL, \
        PRIMARY KEY (`id`), \
        INDEX `idx_player` (`player_id`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    // جدول الحظر (Bans)
    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `admin_bans` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `username` VARCHAR(24) NOT NULL, \
        `ip_address` VARCHAR(45) NOT NULL, \
        `admin_name` VARCHAR(24) NOT NULL, \
        `reason` VARCHAR(255) NOT NULL, \
        `banned_at` INT(11) NOT NULL, \
        `expires_at` INT(11) NOT NULL DEFAULT 0, \
        `active` TINYINT(1) NOT NULL DEFAULT 1, \
        PRIMARY KEY (`id`), \
        UNIQUE KEY `unique_username` (`username`), \
        INDEX `idx_active` (`active`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    // جدول ساعات الأسبوع الإدارية
    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `admin_weekly_hours` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `admin_id` INT(11) NOT NULL, \
        `admin_name` VARCHAR(24) NOT NULL, \
        `week_start` INT(11) NOT NULL, \
        `seconds_online` INT(11) NOT NULL DEFAULT 0, \
        `reports_handled` INT(11) NOT NULL DEFAULT 0, \
        PRIMARY KEY (`id`), \
        INDEX `idx_admin_week` (`admin_id`, `week_start`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    print("[WISE-CITY-ADMIN] تم إنشاء جميع جداول الإدارة بنجاح.");
    return 1;
}

// ============================================================================
// 6. تحميل وحفظ بيانات الإدارة
// ============================================================================

public LoadAdminData(playerid)
{
    new query[256];
    mysql_format(g_SQL, query, sizeof(query),
        "SELECT * FROM `admin_data` WHERE `player_id` = %d LIMIT 1",
        pData[playerid][pID]
    );
    mysql_tquery(g_SQL, query, "OnAdminDataLoaded", "d", playerid);
    return 1;
}

public OnAdminDataLoaded(playerid)
{
    if (cache_num_rows() == 0)
    {
        // لا يوجد سجل إداري، اللاعب عادي
        AdminData[playerid][adLevel] = 0;
        AdminData[playerid][adImmunity] = 0;
        return 1;
    }

    AdminData[playerid][adID]                 = cache_get_field_content_int(0, "id", g_SQL);
    AdminData[playerid][adLevel]              = cache_get_field_content_int(0, "admin_level", g_SQL);
    cache_get_field_content(0, "rank_name", AdminData[playerid][adRankName], g_SQL, 32);
    cache_get_field_content(0, "tag", AdminData[playerid][adTag], g_SQL, 16);
    AdminData[playerid][adDuty]               = bool:cache_get_field_content_int(0, "duty", g_SQL);
    AdminData[playerid][adHidden]             = bool:cache_get_field_content_int(0, "hidden", g_SQL);
    AdminData[playerid][adReportsHandled]     = cache_get_field_content_int(0, "reports_handled", g_SQL);
    AdminData[playerid][adPunishmentsIssued]  = cache_get_field_content_int(0, "punishments_issued", g_SQL);
    AdminData[playerid][adKicksIssued]        = cache_get_field_content_int(0, "kicks_issued", g_SQL);
    AdminData[playerid][adBansIssued]         = cache_get_field_content_int(0, "bans_issued", g_SQL);
    AdminData[playerid][adJailsIssued]        = cache_get_field_content_int(0, "jails_issued", g_SQL);
    AdminData[playerid][adMutesIssued]        = cache_get_field_content_int(0, "mutes_issued", g_SQL);
    AdminData[playerid][adWarningsIssued]     = cache_get_field_content_int(0, "warnings_issued", g_SQL);
    AdminData[playerid][adWeeklySeconds]      = cache_get_field_content_int(0, "weekly_seconds", g_SQL);
    AdminData[playerid][adTotalSeconds]       = cache_get_field_content_int(0, "total_seconds", g_SQL);
    AdminData[playerid][adWarningsReceived]   = cache_get_field_content_int(0, "warnings_received", g_SQL);
    AdminData[playerid][adImmunity]           = cache_get_field_content_int(0, "immunity", g_SQL);

    AdminData[playerid][adLoginTime] = gettime();

    // التحقق من المالك
    if (IS_OWNER_ACCOUNT(playerid))
    {
        AdminData[playerid][adLevel] = ADMIN_OWNER;
        AdminData[playerid][adImmunity] = 2;
        format(AdminData[playerid][adRankName], 32, "المالك");
        format(AdminData[playerid][adTag], 16, OWNER_TAG);
        SendMessage(playerid, COLOR_OWNER, "[WISE-CITY] مرحباً بك يا مالك السيرفر walid om! تم تفعيل جميع صلاحياتك.");
    }

    ApplyAdminPrivileges(playerid);

    new string[128];
    format(string, sizeof(string), "[WISE-CITY] تم تحميل بياناتك الإدارية (المستوى: %d).", AdminData[playerid][adLevel]);
    SendMessage(playerid, COLOR_ADMIN_ACTION, string);

    return 1;
}

public SaveAdminData(playerid)
{
    if (pData[playerid][pID] == 0) return 0;
    if (AdminData[playerid][adLevel] == 0 && AdminData[playerid][adID] == 0) return 0;

    // حساب وقت الجلسة
    if (AdminData[playerid][adLoginTime] > 0)
    {
        new sessionTime = gettime() - AdminData[playerid][adLoginTime];
        AdminData[playerid][adTotalSeconds] += sessionTime;
        AdminData[playerid][adWeeklySeconds] += sessionTime;
        AdminData[playerid][adLoginTime] = gettime();
    }

    new query[1024];
    if (AdminData[playerid][adID] == 0)
    {
        // إدراج جديد
        mysql_format(g_SQL, query, sizeof(query),
            "INSERT INTO `admin_data` (`player_id`, `username`, `admin_level`, `rank_name`, `tag`, \
            `reports_handled`, `punishments_issued`, `kicks_issued`, `bans_issued`, `jails_issued`, \
            `mutes_issued`, `warnings_issued`, `weekly_seconds`, `total_seconds`, `warnings_received`, \
            `immunity`, `last_login`) VALUES (%d, '%e', %d, '%e', '%e', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
            pData[playerid][pID], pData[playerid][pName],
            AdminData[playerid][adLevel], AdminData[playerid][adRankName], AdminData[playerid][adTag],
            AdminData[playerid][adReportsHandled], AdminData[playerid][adPunishmentsIssued],
            AdminData[playerid][adKicksIssued], AdminData[playerid][adBansIssued],
            AdminData[playerid][adJailsIssued], AdminData[playerid][adMutesIssued],
            AdminData[playerid][adWarningsIssued], AdminData[playerid][adWeeklySeconds],
            AdminData[playerid][adTotalSeconds], AdminData[playerid][adWarningsReceived],
            AdminData[playerid][adImmunity], gettime()
        );
        mysql_tquery(g_SQL, query, "", "");
    }
    else
    {
        // تحديث
        mysql_format(g_SQL, query, sizeof(query),
            "UPDATE `admin_data` SET \
            `admin_level` = %d, `rank_name` = '%e', `tag` = '%e', `duty` = %d, `hidden` = %d, \
            `reports_handled` = %d, `punishments_issued` = %d, `kicks_issued` = %d, `bans_issued` = %d, \
            `jails_issued` = %d, `mutes_issued` = %d, `warnings_issued` = %d, \
            `weekly_seconds` = %d, `total_seconds` = %d, `warnings_received` = %d, \
            `immunity` = %d, `last_login` = %d \
            WHERE `id` = %d",
            AdminData[playerid][adLevel], AdminData[playerid][adRankName], AdminData[playerid][adTag],
            AdminData[playerid][adDuty] ? 1 : 0, AdminData[playerid][adHidden] ? 1 : 0,
            AdminData[playerid][adReportsHandled], AdminData[playerid][adPunishmentsIssued],
            AdminData[playerid][adKicksIssued], AdminData[playerid][adBansIssued],
            AdminData[playerid][adJailsIssued], AdminData[playerid][adMutesIssued],
            AdminData[playerid][adWarningsIssued], AdminData[playerid][adWeeklySeconds],
            AdminData[playerid][adTotalSeconds], AdminData[playerid][adWarningsReceived],
            AdminData[playerid][adImmunity], gettime(), AdminData[playerid][adID]
        );
        mysql_tquery(g_SQL, query, "", "");
    }

    return 1;
}

public SaveAllAdminData()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i) && AdminData[i][adLevel] > 0)
        {
            SaveAdminData(i);
        }
    }
    return 1;
}

// ============================================================================
// 7. دوال التحقق والصلاحيات
// ============================================================================

// --------------------------------------------------
// التحقق من كون اللاعب هو المالك walid om
// --------------------------------------------------
public IsPlayerOwner(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (IS_OWNER_ACCOUNT(playerid)) return 1;
    return 0;
}

// --------------------------------------------------
// التحقق من صلاحية إدارية بمستوى معين
// --------------------------------------------------
public HasAdminLevel(playerid, level)
{
    if (!IsPlayerConnected(playerid)) return 0;
    if (IsPlayerOwner(playerid)) return 1; // المالك دائماً يمتلك كل الصلاحيات
    return (AdminData[playerid][adLevel] >= level);
}

// --------------------------------------------------
// تطبيق صلاحيات الإدارة
// --------------------------------------------------
public ApplyAdminPrivileges(playerid)
{
    if (AdminData[playerid][adLevel] == 0) return 0;

    UpdatePlayerTag(playerid);

    // إشعار اللاعبين
    if (AdminData[playerid][adLevel] >= ADMIN_ADMIN && !AdminData[playerid][adHidden])
    {
        new string[128];
        format(string, sizeof(string), "[WISE-CITY] %s %s متواجد الآن في المدينة.",
            g_AdminTags[AdminData[playerid][adLevel]], pData[playerid][pName]);
        SendClientMessageToAll(g_AdminColors[AdminData[playerid][adLevel]], string);
    }

    g_TotalAdminsOnline++;
    return 1;
}

// --------------------------------------------------
// إزالة صلاحيات الإدارة
// --------------------------------------------------
public RemoveAdminPrivileges(playerid)
{
    if (AdminData[playerid][adLevel] > 0)
    {
        g_TotalAdminsOnline--;
    }
    return 1;
}

// --------------------------------------------------
// تنسيق التاغ الإداري
// --------------------------------------------------
public FormatAdminTag(playerid, output[], size)
{
    if (IsPlayerOwner(playerid))
    {
        format(output, size, "%s %s", OWNER_TAG, pData[playerid][pName]);
        return 1;
    }

    if (AdminData[playerid][adLevel] > 0)
    {
        if (AdminData[playerid][adTag][0] != EOS)
        {
            format(output, size, "%s %s", AdminData[playerid][adTag], pData[playerid][pName]);
        }
        else
        {
            format(output, size, "%s %s", g_AdminTags[AdminData[playerid][adLevel]], pData[playerid][pName]);
        }
        return 1;
    }

    format(output, size, "%s", pData[playerid][pName]);
    return 1;
}

// --------------------------------------------------
// تحديث تاغ اللاعب في السيرفر
// --------------------------------------------------
public UpdatePlayerTag(playerid)
{
    new tag[64];
    FormatAdminTag(playerid, tag, sizeof(tag));

    // تعيين التاغ الجديد
    SetPlayerName(playerid, tag);

    // إذا كان هناك نظام Score/Tag خاص، يمكن استخدامه هنا
    return 1;
}

// --------------------------------------------------
// الحصول على اسم الرتبة
// --------------------------------------------------
public GetAdminLevelName(level, output[], size)
{
    if (level < 0 || level > MAX_ADMIN_LEVEL)
    {
        format(output, size, "لاعب");
        return 1;
    }
    format(output, size, "%s", g_AdminRankNames[level]);
    return 1;
}

// ============================================================================
// 8. نظام الرسائل الإدارية
// ============================================================================

// --------------------------------------------------
// إرسال رسالة لجميع الأدمنية بمستوى معين
// --------------------------------------------------
public SendAdminMessage(level, const msg[], color)
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (AdminData[i][adLevel] >= level || IsPlayerOwner(i))
        {
            SendClientMessage(i, color, msg);
        }
    }
    return 1;
}

// --------------------------------------------------
// تنبيه لجميع الأدمنية
// --------------------------------------------------
public SendAdminAlert(const msg[], color)
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (AdminData[i][adLevel] >= ADMIN_HELPER || IsPlayerOwner(i))
        {
            SendClientMessage(i, color, msg);
        }
    }
    return 1;
}

// --------------------------------------------------
// تسجيل الإجراء الإداري
// --------------------------------------------------
public LogAdminAction(playerid, const action[], targetid)
{
    new query[512];
    new targetID = (targetid != -1) ? pData[targetid][pID] : -1;

    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `admin_logs` (`admin_id`, `admin_name`, `action`, `target_id`, `logged_at`) \
        VALUES (%d, '%e', '%e', %d, %d)",
        pData[playerid][pID],
        pData[playerid][pName],
        action,
        targetID,
        gettime()
    );
    mysql_tquery(g_SQL, query, "", "");
    return 1;
}

// --------------------------------------------------
// إرسال سجل عقوبة
// --------------------------------------------------
public SendPunishmentLog(adminid, targetid, punishType, const reason[], time)
{
    new typeName[16];
    switch (punishType)
    {
        case 0: format(typeName, sizeof(typeName), "KICK");
        case 1: format(typeName, sizeof(typeName), "BAN");
        case 2: format(typeName, sizeof(typeName), "JAIL");
        case 3: format(typeName, sizeof(typeName), "MUTE");
        default: format(typeName, sizeof(typeName), "WARN");
    }

    new tag[32];
    format(tag, sizeof(tag), "%s", g_AdminTags[AdminData[adminid][adLevel]]);

    new string[256];
    new targetName[MAX_PLAYER_NAME];
    if (targetid != -1) format(targetName, sizeof(targetName), "%s", pData[targetid][pName]);
    else format(targetName, sizeof(targetName), "N/A");

    format(string, sizeof(string),
        "[ADMIN-LOG] %s %s (%s) -> %s | السبب: %s | المدة: %d",
        tag,
        pData[adminid][pName],
        typeName,
        targetName,
        reason,
        time
    );
    SendAdminMessage(ADMIN_MODERATOR, string, COLOR_ADMIN_ACTION);
    return 1;
}

// --------------------------------------------------
// فحص حصانة اللاعب المستهدف
// --------------------------------------------------
public CheckPlayerImmunity(adminid, targetid)
{
    // المالك محصن من كل شيء
    if (IsPlayerOwner(targetid))
    {
        SendMessage(adminid, COLOR_RED, "[خطأ] لا يمكنك تطبيق عقوبة على مالك السيرفر!");
        return 0;
    }

    // حصانة كاملة
    if (AdminData[targetid][adImmunity] == 2 && !IsPlayerOwner(adminid))
    {
        SendMessage(adminid, COLOR_RED, "[خطأ] هذا اللاعب لديه حصانة كاملة.");
        return 0;
    }

    // لا يمكن تطبيق عقوبة على أدمن بمستوى أعلى أو مساوٍ
    if (AdminData[targetid][adLevel] >= AdminData[adminid][adLevel] && !IsPlayerOwner(adminid))
    {
        SendMessage(adminid, COLOR_RED, "[خطأ] لا يمكنك تطبيق عقوبة على أدمن بمستوى أعلى أو مساوٍ لك.");
        return 0;
    }

    return 1;
}

// ============================================================================
// 9. المؤقتات
// ============================================================================

public OnAdminTimer()
{
    // تحديث إحصائيات الأدمنية
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (AdminData[i][adLevel] == 0) continue;

        // تسجيل الوقت الإداري كل ثانية
        if (AdminData[i][adDuty])
        {
            AdminData[i][adWeeklySeconds]++;
            AdminData[i][adTotalSeconds]++;
        }
    }
    return 1;
}

public OnJailTimer()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (!JailData[i][jJailed]) continue;

        JailData[i][jTimeLeft]--;

        new string[128];
        format(string, sizeof(string), "~r~السجن الإداري~n~~w~الوقت المتبقي: %d ثانية", JailData[i][jTimeLeft]);
        GameTextForPlayer(i, string, 1000, 3);

        if (JailData[i][jTimeLeft] <= 0)
        {
            // إطلاق سراح
            JailData[i][jJailed] = false;

            // إعادة اللاعب للموقع السابق
            SetPlayerPos(i, JailData[i][jPosX], JailData[i][jPosY], JailData[i][jPosZ]);
            SetPlayerInterior(i, 0);
            SetPlayerVirtualWorld(i, 0);

            SendMessage(i, COLOR_GREEN, "[WISE-CITY] تم إطلاق سراحك من السجن الإداري.");
            SendAdminAlert("انتهى وقت السجن الإداري لأحد اللاعبين.", COLOR_ADMIN_ACTION);
        }
    }
    return 1;
}

public OnMuteTimer()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (!MuteData[i][mMuted]) continue;

        MuteData[i][mTimeLeft]--;

        if (MuteData[i][mTimeLeft] <= 0)
        {
            MuteData[i][mMuted] = false;
            SendMessage(i, COLOR_GREEN, "[WISE-CITY] تم رفع الميوت عنك. يمكنك التحدث الآن.");
        }
    }
    return 1;
}

public OnAdminDutyTimer()
{
    // تحديث معلومات ساعات الأسبوع الإدارية في قاعدة البيانات (كل دقيقة)
    static counter = 0;
    counter++;
    if (counter >= 60)
    {
        counter = 0;
        for (new i = 0; i < MAX_PLAYERS; i++)
        {
            if (IsPlayerConnected(i) && AdminData[i][adLevel] > 0)
            {
                SaveAdminData(i);
            }
        }
    }
    return 1;
}

// ============================================================================
// 10. الأوامر الإدارية الشاملة
// ============================================================================

// --------------------------------------------------
// /makeadmin - تعيين أو رفع رتبة أدمن (حصري للمالك)
// --------------------------------------------------
CMD:makeadmin(playerid, params[])
{
    if (!IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر حصري لمالك السيرفر walid om فقط!");

    new targetid, level, customTag[16];
    if (sscanf(params, "uidS( )[16]", targetid, level, customTag))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /makeadmin [اللاعب] [المستوى 0-6] [التاغ المخصص اختياري]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (level < 0 || level > MAX_ADMIN_LEVEL)
        return SendMessage(playerid, COLOR_RED, "[خطأ] المستوى يجب أن يكون بين 0 و 6.");

    if (targetid == playerid && level < ADMIN_OWNER)
        return SendMessage(playerid, COLOR_RED, "[خطأ] لا يمكنك تعديل رتبتك الخاصة!");

    if (IS_OWNER_ACCOUNT(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] لا يمكن تعديل رتبة المالك!");

    new oldLevel = AdminData[targetid][adLevel];

    // تحديث المستوى
    AdminData[targetid][adLevel] = level;
    format(AdminData[targetid][adRankName], 32, "%s", g_AdminRankNames[level]);

    if (customTag[0] != EOS)
        format(AdminData[targetid][adTag], 16, "%s", customTag);
    else
        format(AdminData[targetid][adTag], 16, "%s", g_AdminTags[level]);

    // حفظ
    SaveAdminData(targetid);
    UpdatePlayerTag(targetid);

    // رسائل
    new string[256];
    if (level == 0)
    {
        format(string, sizeof(string), "[WISE-CITY] تم سحب الرتبة الإدارية من اللاعب %s.", pData[targetid][pName]);
        SendMessage(playerid, COLOR_GREEN, string);

        format(string, sizeof(string), "[WISE-CITY] تم سحب رتبتك الإدارية من قبل المالك.");
        SendMessage(targetid, COLOR_RED, string);

        SendAdminAlert(string, COLOR_ADMIN_ACTION);
    }
    else
    {
        format(string, sizeof(string),
            "[WISE-CITY] تم تعيين %s كـ %s (المستوى %d) بنجاح.",
            pData[targetid][pName], g_AdminRankNames[level], level);
        SendMessage(playerid, COLOR_GREEN, string);
        SendAdminAlert(string, COLOR_ADMIN_ACTION);

        format(string, sizeof(string),
            "[WISE-CITY] تم ترقيتك إلى %s (المستوى %d) من قبل المالك walid om.",
            g_AdminRankNames[level], level);
        SendMessage(targetid, COLOR_GOLD, string);
    }

    LogAdminAction(playerid, "makeadmin", targetid);

    // إشعار إداري
    printf("[ADMIN] %s -> makeadmin: %s | Old: %d | New: %d",
        pData[playerid][pName], pData[targetid][pName], oldLevel, level);

    return 1;
}

// --------------------------------------------------
// /a - شات الإدارة الداخلي
// --------------------------------------------------
CMD:a(playerid, params[])
{
    if (AdminData[playerid][adLevel] < ADMIN_HELPER && !IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر للأدمنية فقط.");

    if (isnull(params))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /a [الرسالة]");

    new string[256];
    new tag[64];
    FormatAdminTag(playerid, tag, sizeof(tag));

    format(string, sizeof(string), "[ADMIN-CHAT] %s: %s", tag, params);
    SendAdminMessage(ADMIN_HELPER, string, COLOR_ADMIN_CHAT);

    // تسجيل في الكونسول
    printf("[ADMIN-CHAT] %s: %s", pData[playerid][pName], params);

    return 1;
}

// --------------------------------------------------
// /goto - الانتقال للاعب
// --------------------------------------------------
CMD:goto(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_ADMIN))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية أدمن (المستوى 3) على الأقل.");

    new targetid;
    if (sscanf(params, "u", targetid))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /goto [اللاعب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (targetid == playerid)
        return SendMessage(playerid, COLOR_RED, "[خطأ] لا يمكنك الانتقال إلى نفسك.");

    // إذا كان اللاعب داخل سيارة
    if (IsPlayerInAnyVehicle(targetid))
    {
        new vehicleid = GetPlayerVehicleID(targetid);
        new seat = GetPlayerVehicleSeat(targetid);
        PutPlayerInVehicle(playerid, vehicleid, seat);
        SendMessage(playerid, COLOR_GREEN, "[WISE-CITY] تم الانتقال إلى اللاعب (داخل السيارة).");
    }
    else
    {
        new Float:x, Float:y, Float:z, interior, world;
        GetPlayerPos(targetid, x, y, z);
        interior = GetPlayerInterior(targetid);
        world = GetPlayerVirtualWorld(targetid);

        SetPlayerInterior(playerid, interior);
        SetPlayerVirtualWorld(playerid, world);
        SetPlayerPos(playerid, x + 1.0, y + 1.0, z);
        SendMessage(playerid, COLOR_GREEN, "[WISE-CITY] تم الانتقال إلى اللاعب.");
    }

    // إشعار اللاعب
    if (AdminData[targetid][adLevel] == 0 || AdminData[playerid][adLevel] >= ADMIN_ADMIN)
    {
        new string[128];
        format(string, sizeof(string), "[WISE-CITY] الأدمن %s انتقل إليك.", pData[playerid][pName]);
        SendMessage(targetid, COLOR_ADMIN_ACTION, string);
    }

    LogAdminAction(playerid, "goto", targetid);
    return 1;
}

// --------------------------------------------------
// /gethere - جلب اللاعب إلى موقعك
// --------------------------------------------------
CMD:gethere(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_ADMIN))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية أدمن (المستوى 3) على الأقل.");

    new targetid;
    if (sscanf(params, "u", targetid))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /gethere [اللاعب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (targetid == playerid)
        return SendMessage(playerid, COLOR_RED, "[خطأ] لا يمكنك جلب نفسك.");

    new Float:x, Float:y, Float:z, interior, world;
    GetPlayerPos(playerid, x, y, z);
    interior = GetPlayerInterior(playerid);
    world = GetPlayerVirtualWorld(playerid);

    SetPlayerInterior(targetid, interior);
    SetPlayerVirtualWorld(targetid, world);
    SetPlayerPos(targetid, x + 1.0, y + 1.0, z);

    SendMessage(playerid, COLOR_GREEN, "[WISE-CITY] تم جلب اللاعب إليك.");
    SendMessage(targetid, COLOR_ADMIN_ACTION, "[WISE-CITY] تم نقلك بواسطة أدمن.");

    LogAdminAction(playerid, "gethere", targetid);
    return 1;
}

// --------------------------------------------------
// /jail - السجن الإداري
// --------------------------------------------------
CMD:jail(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_ADMIN))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية أدمن (المستوى 3) على الأقل.");

    new targetid, minutes, reason[128];
    if (sscanf(params, "uiS( )[128]", targetid, minutes, reason))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /jail [اللاعب] [الدقائق] [السبب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (!CheckPlayerImmunity(playerid, targetid))
        return 1;

    if (minutes < 1 || minutes > 1440)
        return SendMessage(playerid, COLOR_RED, "[خطأ] المدة يجب أن تكون بين 1 و 1440 دقيقة.");

    if (isnull(reason))
        format(reason, sizeof(reason), "غير محدد");

    // حفظ الموقع الحالي
    GetPlayerPos(targetid, JailData[targetid][jPosX], JailData[targetid][jPosY], JailData[targetid][jPosZ]);

    // سجن اللاعب
    JailData[targetid][jJailed] = true;
    JailData[targetid][jAdminID] = pData[playerid][pID];
    format(JailData[targetid][jReason], 128, "%s", reason);
    JailData[targetid][jTimeLeft] = minutes * 60;

    // نقل للسجن (موقع داخلي)
    SetPlayerPos(targetid, 264.6288, 77.5743, 1001.0391);
    SetPlayerInterior(targetid, 6);
    SetPlayerVirtualWorld(targetid, 0);
    SetCameraBehindPlayer(targetid);

    // رسائل
    new string[256];
    format(string, sizeof(string),
        "[WISE-CITY] تم سجن اللاعب %s لمدة %d دقيقة | السبب: %s",
        pData[targetid][pName], minutes, reason);
    SendMessage(playerid, COLOR_GREEN, string);
    SendAdminAlert(string, COLOR_ADMIN_ACTION);

    format(string, sizeof(string),
        "[WISE-CITY] تم سجنك إدارياً لمدة %d دقيقة | السبب: %s",
        minutes, reason);
    SendMessage(targetid, COLOR_JAIL, string);

    // زيادة الإحصائيات
    AdminData[playerid][adJailsIssued]++;
    AdminData[playerid][adPunishmentsIssued]++;

    // تسجيل العقوبة
    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `admin_punishments` (`admin_id`, `admin_name`, `target_id`, `target_name`, \
        `punishment_type`, `reason`, `duration`, `punished_at`, `expires_at`, `active`) \
        VALUES (%d, '%e', %d, '%e', %d, '%e', %d, %d, %d, 1)",
        pData[playerid][pID], pData[playerid][pName],
        pData[targetid][pID], pData[targetid][pName],
        PUNISH_JAIL, reason, minutes * 60, gettime(), gettime() + (minutes * 60)
    );
    mysql_tquery(g_SQL, query, "", "");

    SendPunishmentLog(playerid, targetid, PUNISH_JAIL, reason, minutes);
    LogAdminAction(playerid, "jail", targetid);

    return 1;
}

// --------------------------------------------------
// /unjail - فك السجن
// --------------------------------------------------
CMD:unjail(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_ADMIN))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية أدمن (المستوى 3) على الأقل.");

    new targetid;
    if (sscanf(params, "u", targetid))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /unjail [اللاعب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (!JailData[targetid][jJailed])
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا اللاعب غير مسجون.");

    JailData[targetid][jJailed] = false;

    // إعادة للوضع الطبيعي
    SetPlayerPos(targetid, JailData[targetid][jPosX], JailData[targetid][jPosY], JailData[targetid][jPosZ]);
    SetPlayerInterior(targetid, 0);
    SetPlayerVirtualWorld(targetid, 0);
    SetCameraBehindPlayer(targetid);

    new string[128];
    format(string, sizeof(string), "[WISE-CITY] تم إطلاق سراح %s من السجن الإداري.", pData[targetid][pName]);
    SendMessage(playerid, COLOR_GREEN, string);
    SendMessage(targetid, COLOR_GREEN, "[WISE-CITY] تم إطلاق سراحك من السجن الإداري.");

    SendAdminAlert(string, COLOR_ADMIN_ACTION);
    LogAdminAction(playerid, "unjail", targetid);

    return 1;
}

// --------------------------------------------------
// /ban - الحظر
// --------------------------------------------------
CMD:ban(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_ADMIN))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية أدمن (المستوى 3) على الأقل.");

    new targetid, days, reason[128];
    if (sscanf(params, "uidS( )[128]", targetid, days, reason))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /ban [اللاعب] [الأيام 0=دائم] [السبب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (!CheckPlayerImmunity(playerid, targetid))
        return 1;

    if (days < 0 || days > 365)
        return SendMessage(playerid, COLOR_RED, "[خطأ] عدد الأيام يجب أن يكون بين 0 و 365 (0 = حظر دائم).");

    if (isnull(reason))
        format(reason, sizeof(reason), "غير محدد");

    // منع حظر الأدمنية من قِبل أدمن بمستوى أقل
    if (AdminData[targetid][adLevel] > 0 && AdminData[playerid][adLevel] <= AdminData[targetid][adLevel] && !IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] لا يمكنك حظر أدمن بمستوى مساوٍ أو أعلى.");

    new playerName[MAX_PLAYER_NAME], ip[16];
    GetPlayerName(targetid, playerName, MAX_PLAYER_NAME);
    GetPlayerIp(targetid, ip, 16);

    new expireTime = (days == 0) ? 0 : (gettime() + (days * 86400));

    // حفظ الحظر في قاعدة البيانات
    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `admin_bans` (`username`, `ip_address`, `admin_name`, `reason`, `banned_at`, `expires_at`, `active`) \
        VALUES ('%e', '%e', '%e', '%e', %d, %d, 1) \
        ON DUPLICATE KEY UPDATE `admin_name` = '%e', `reason` = '%e', `banned_at` = %d, `expires_at` = %d, `active` = 1",
        playerName, ip, pData[playerid][pName], reason, gettime(), expireTime,
        pData[playerid][pName], reason, gettime(), expireTime
    );
    mysql_tquery(g_SQL, query, "", "");

    // رسائل
    new string[256];
    format(string, sizeof(string),
        "[WISE-CITY] تم حظر اللاعب %s | المدة: %s | السبب: %s",
        playerName,
        (days == 0) ? "دائم" : "محدد",
        reason);
    SendAdminAlert(string, COLOR_BAN);

    format(string, sizeof(string),
        "[WISE-CITY] لقد تم حظرك من السيرفر | السبب: %s | %s",
        reason,
        (days == 0) ? "حظر دائم" : "سيتم رفع الحظر تلقائياً لاحقاً");
    SendMessage(targetid, COLOR_BAN, string);

    // طرد اللاعب
    Kick(targetid);

    // زيادة الإحصائيات
    AdminData[playerid][adBansIssued]++;
    AdminData[playerid][adPunishmentsIssued]++;

    // تسجيل العقوبة
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `admin_punishments` (`admin_id`, `admin_name`, `target_id`, `target_name`, \
        `punishment_type`, `reason`, `duration`, `punished_at`, `expires_at`, `active`) \
        VALUES (%d, '%e', %d, '%e', %d, '%e', %d, %d, %d, 1)",
        pData[playerid][pID], pData[playerid][pName],
        pData[targetid][pID], playerName,
        PUNISH_BAN, reason, (days == 0) ? -1 : (days * 86400), gettime(), expireTime
    );
    mysql_tquery(g_SQL, query, "", "");

    SendPunishmentLog(playerid, targetid, PUNISH_BAN, reason, days);
    LogAdminAction(playerid, "ban", targetid);

    return 1;
}

// --------------------------------------------------
// /kick - الطرد
// --------------------------------------------------
CMD:kick(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_MODERATOR))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية مشرف (المستوى 2) على الأقل.");

    new targetid, reason[128];
    if (sscanf(params, "uS( )[128]", targetid, reason))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /kick [اللاعب] [السبب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (!CheckPlayerImmunity(playerid, targetid))
        return 1;

    if (isnull(reason))
        format(reason, sizeof(reason), "غير محدد");

    // رسائل
    new string[256];
    format(string, sizeof(string),
        "[WISE-CITY] تم طرد اللاعب %s | السبب: %s",
        pData[targetid][pName], reason);
    SendAdminAlert(string, COLOR_KICK);

    format(string, sizeof(string),
        "[WISE-CITY] لقد تم طردك من السيرفر | السبب: %s",
        reason);
    SendMessage(targetid, COLOR_KICK, string);

    // تسجيل قبل الطرد
    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `admin_punishments` (`admin_id`, `admin_name`, `target_id`, `target_name`, \
        `punishment_type`, `reason`, `duration`, `punished_at`, `active`) \
        VALUES (%d, '%e', %d, '%e', %d, '%e', 0, %d, 0)",
        pData[playerid][pID], pData[playerid][pName],
        pData[targetid][pID], pData[targetid][pName],
        PUNISH_KICK, reason, gettime()
    );
    mysql_tquery(g_SQL, query, "", "");

    // زيادة الإحصائيات
    AdminData[playerid][adKicksIssued]++;
    AdminData[playerid][adPunishmentsIssued]++;

    SendPunishmentLog(playerid, targetid, PUNISH_KICK, reason, 0);
    LogAdminAction(playerid, "kick", targetid);

    // طرد اللاعب
    Kick(targetid);

    return 1;
}

// --------------------------------------------------
// /setauctiontime - تحديد مواعيد المزادات الرسمية
// --------------------------------------------------
CMD:setauctiontime(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_HEAD) && !IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر حصري لمدير النظام والمالك (المستوى 5+).");

    new day, hour, minute;
    if (sscanf(params, "iii", day, hour, minute))
    {
        SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /setauctiontime [اليوم 0-6] [الساعة 0-23] [الدقيقة 0-59]");
        SendMessage(playerid, COLOR_WHITE, "[الأيام] 0=الأحد، 1=الإثنين، 2=الثلاثاء، 3=الأربعاء، 4=الخميس، 5=الجمعة، 6=السبت");
        return 1;
    }

    if (day < 0 || day > 6)
        return SendMessage(playerid, COLOR_RED, "[خطأ] اليوم يجب أن يكون بين 0 و 6.");

    if (hour < 0 || hour > 23)
        return SendMessage(playerid, COLOR_RED, "[خطأ] الساعة يجب أن تكون بين 0 و 23.");

    if (minute < 0 || minute > 59)
        return SendMessage(playerid, COLOR_RED, "[خطأ] الدقيقة يجب أن تكون بين 0 و 59.");

    // حفظ التوقيت في قاعدة البيانات
    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `server_settings` (`setting_key`, `setting_value`, `updated_by`, `updated_at`) \
        VALUES ('auction_schedule', '%d|%d|%d', '%e', %d) \
        ON DUPLICATE KEY UPDATE `setting_value` = '%d|%d|%d', `updated_by` = '%e', `updated_at` = %d",
        day, hour, minute, pData[playerid][pName], gettime(),
        day, hour, minute, pData[playerid][pName], gettime()
    );
    mysql_tquery(g_SQL, query, "", "");

    // إنشاء جدول الإعدادات إذا لم يكن موجوداً (يُنشأ مرة واحدة)
    // ملاحظة: يجب التأكد من وجود الجدول في CreateAdminTables

    new dayNames[7][] = {"الأحد", "الإثنين", "الثلاثاء", "الأربعاء", "الخميس", "الجمعة", "السبت"};
    new string[256];
    format(string, sizeof(string),
        "[WISE-CITY] تم تحديد موعد المزاد الرسمي: يوم %s الساعة %02d:%02d",
        dayNames[day], hour, minute);
    SendClientMessageToAll(COLOR_ADMIN_ACTION, string);

    SendAdminAlert(string, COLOR_ADMIN_ACTION);
    LogAdminAction(playerid, "setauctiontime", -1);

    return 1;
}

// --------------------------------------------------
// /adminstats - مراجعة إحصائيات أدمن
// --------------------------------------------------
CMD:adminstats(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_HELPER))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية إدارية على الأقل.");

    new targetid;
    if (sscanf(params, "u", targetid))
        targetid = playerid;

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (AdminData[targetid][adLevel] == 0 && !IsPlayerOwner(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا اللاعب ليس لديه رتبة إدارية.");

    new string[1024];
    new rankName[32];
    GetAdminLevelName(AdminData[targetid][adLevel], rankName, sizeof(rankName));

    new hours = AdminData[targetid][adTotalSeconds] / 3600;
    new minutes = (AdminData[targetid][adTotalSeconds] % 3600) / 60;

    new weeklyHours = AdminData[targetid][adWeeklySeconds] / 3600;
    new weeklyMinutes = (AdminData[targetid][adWeeklySeconds] % 3600) / 60;

    format(string, sizeof(string),
        "{FFD700}===== إحصائيات الأدمن: %s =====\n\
        {FFFFFF}الرتبة: {00FF00}%s {FFFFFF}(المستوى %d)\n\
        {FFFFFF}التاغ: {00CCFF}%s\n\
        {FFFFFF}الحالة: %s\n\
        {FFFFFF}الحصانة: %d\n\n\
        {FFAA00}--- الإحصائيات ---\n\
        {FFFFFF}البلاغات المنسقة: {00FF00}%d\n\
        {FFFFFF}العقوبات الصادرة: {FF0000}%d\n\
        {FFFFFF}  - الطرد: %d | الحظر: %d | السجن: %d\n\
        {FFFFFF}  - الميوت: %d | التحذيرات: %d\n\
        {FFFFFF}التحذيرات المستلمة: {FF0000}%d\n\n\
        {FFAA00}--- الوقت ---\n\
        {FFFFFF}الوقت الأسبوعي: {00FFFF}%d ساعة و %d دقيقة\n\
        {FFFFFF}الوقت الإجمالي: {00FFFF}%d ساعة و %d دقيقة",
        pData[targetid][pName],
        rankName,
        AdminData[targetid][adLevel],
        AdminData[targetid][adTag],
        AdminData[targetid][adDuty] ? "{00FF00}في الخدمة" : "{FF0000}خارج الخدمة",
        AdminData[targetid][adImmunity],
        AdminData[targetid][adReportsHandled],
        AdminData[targetid][adPunishmentsIssued],
        AdminData[targetid][adKicksIssued],
        AdminData[targetid][adBansIssued],
        AdminData[targetid][adJailsIssued],
        AdminData[targetid][adMutesIssued],
        AdminData[targetid][adWarningsIssued],
        AdminData[targetid][adWarningsReceived],
        weeklyHours, weeklyMinutes,
        hours, minutes
    );

    ShowPlayerDialog(playerid, DIALOG_ADMIN_STATS, DIALOG_STYLE_MSGBOX, "إحصائيات الأدمن", string, "حسناً", "");

    return 1;
}

// --------------------------------------------------
// /aduty - تبديل حالة الخدمة
// --------------------------------------------------
CMD:aduty(playerid, params[])
{
    if (AdminData[playerid][adLevel] < ADMIN_HELPER && !IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر للأدمنية فقط.");

    AdminData[playerid][adDuty] = !AdminData[playerid][adDuty];

    new string[128];
    if (AdminData[playerid][adDuty])
    {
        format(string, sizeof(string), "[WISE-CITY] أنت الآن في الخدمة الإدارية.");
        SendMessage(playerid, COLOR_GREEN, string);
        SendAdminAlert(string, COLOR_ADMIN_ACTION);
    }
    else
    {
        format(string, sizeof(string), "[WISE-CITY] أنت الآن خارج الخدمة الإدارية.");
        SendMessage(playerid, COLOR_YELLOW, string);
        SendAdminAlert(string, COLOR_ADMIN_ACTION);
    }

    SaveAdminData(playerid);
    LogAdminAction(playerid, "aduty", -1);

    return 1;
}

// --------------------------------------------------
// /ahide - إخفاء/إظهار الحالة الإدارية
// --------------------------------------------------
CMD:ahide(playerid, params[])
{
    if (AdminData[playerid][adLevel] < ADMIN_ADMIN && !IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية أدمن (المستوى 3) على الأقل.");

    AdminData[playerid][adHidden] = !AdminData[playerid][adHidden];

    new string[128];
    if (AdminData[playerid][adHidden])
        format(string, sizeof(string), "[WISE-CITY] أنت الآن مخفي عن اللاعبين.");
    else
        format(string, sizeof(string), "[WISE-CITY] أنت الآن ظاهر للاعبين.");

    SendMessage(playerid, COLOR_ADMIN_ACTION, string);
    LogAdminAction(playerid, "ahide", -1);

    return 1;
}

// --------------------------------------------------
// /mute - الميوت
// --------------------------------------------------
CMD:mute(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_MODERATOR))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية مشرف (المستوى 2) على الأقل.");

    new targetid, minutes, reason[128];
    if (sscanf(params, "uidS( )[128]", targetid, minutes, reason))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /mute [اللاعب] [الدقائق] [السبب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (!CheckPlayerImmunity(playerid, targetid))
        return 1;

    if (minutes < 1 || minutes > 1440)
        return SendMessage(playerid, COLOR_RED, "[خطأ] المدة يجب أن تكون بين 1 و 1440 دقيقة.");

    if (isnull(reason))
        format(reason, sizeof(reason), "غير محدد");

    MuteData[targetid][mMuted] = true;
    MuteData[targetid][mAdminID] = pData[playerid][pID];
    format(MuteData[targetid][mReason], 128, "%s", reason);
    MuteData[targetid][mTimeLeft] = minutes * 60;

    new string[256];
    format(string, sizeof(string),
        "[WISE-CITY] تم إسكات اللاعب %s لمدة %d دقيقة | السبب: %s",
        pData[targetid][pName], minutes, reason);
    SendMessage(playerid, COLOR_GREEN, string);
    SendAdminAlert(string, COLOR_MUTE);

    format(string, sizeof(string),
        "[WISE-CITY] تم إسكاتك لمدة %d دقيقة | السبب: %s",
        minutes, reason);
    SendMessage(targetid, COLOR_MUTE, string);

    AdminData[playerid][adMutesIssued]++;
    AdminData[playerid][adPunishmentsIssued]++;

    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `admin_punishments` (`admin_id`, `admin_name`, `target_id`, `target_name`, \
        `punishment_type`, `reason`, `duration`, `punished_at`, `expires_at`, `active`) \
        VALUES (%d, '%e', %d, '%e', %d, '%e', %d, %d, %d, 1)",
        pData[playerid][pID], pData[playerid][pName],
        pData[targetid][pID], pData[targetid][pName],
        PUNISH_MUTE, reason, minutes * 60, gettime(), gettime() + (minutes * 60)
    );
    mysql_tquery(g_SQL, query, "", "");

    SendPunishmentLog(playerid, targetid, PUNISH_MUTE, reason, minutes);
    LogAdminAction(playerid, "mute", targetid);

    return 1;
}

// --------------------------------------------------
// /unmute - رفع الميوت
// --------------------------------------------------
CMD:unmute(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_MODERATOR))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية مشرف (المستوى 2) على الأقل.");

    new targetid;
    if (sscanf(params, "u", targetid))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /unmute [اللاعب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (!MuteData[targetid][mMuted])
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا اللاعب غير مسكوت.");

    MuteData[targetid][mMuted] = false;

    new string[128];
    format(string, sizeof(string), "[WISE-CITY] تم رفع الميوت عن %s.", pData[targetid][pName]);
    SendMessage(playerid, COLOR_GREEN, string);
    SendMessage(targetid, COLOR_GREEN, "[WISE-CITY] تم رفع الميوت عنك.");

    SendAdminAlert(string, COLOR_ADMIN_ACTION);
    LogAdminAction(playerid, "unmute", targetid);

    return 1;
}

// --------------------------------------------------
// /warn - التحذير
// --------------------------------------------------
CMD:warn(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_MODERATOR))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية مشرف (المستوى 2) على الأقل.");

    new targetid, reason[128];
    if (sscanf(params, "uS( )[128]", targetid, reason))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /warn [اللاعب] [السبب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (!CheckPlayerImmunity(playerid, targetid))
        return 1;

    if (isnull(reason))
        format(reason, sizeof(reason), "غير محدد");

    if (g_WarnCount[targetid] >= 10)
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا اللاعب وصل للحد الأقصى من التحذيرات (10).");

    new idx = g_WarnCount[targetid];
    WarnData[targetid][idx][wID] = idx + 1;
    WarnData[targetid][idx][wPlayerID] = pData[targetid][pID];
    format(WarnData[targetid][idx][wAdminName], MAX_PLAYER_NAME, "%s", pData[playerid][pName]);
    format(WarnData[targetid][idx][wReason], 128, "%s", reason);
    g_WarnCount[targetid]++;

    new string[256];
    format(string, sizeof(string),
        "[WISE-CITY] تم تحذير اللاعب %s | السبب: %s | عدد التحذيرات: %d/10",
        pData[targetid][pName], reason, g_WarnCount[targetid]);
    SendMessage(playerid, COLOR_GREEN, string);
    SendAdminAlert(string, COLOR_ADMIN_WARN);

    format(string, sizeof(string),
        "[WISE-CITY] تم تحذيرك | السبب: %s | عدد التحذيرات: %d/10",
        reason, g_WarnCount[targetid]);
    SendMessage(targetid, COLOR_ADMIN_WARN, string);

    AdminData[playerid][adWarningsIssued]++;
    AdminData[targetid][adWarningsReceived]++;

    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `admin_warnings` (`player_id`, `admin_name`, `reason`, `warned_at`) \
        VALUES (%d, '%e', '%e', %d)",
        pData[targetid][pID], pData[playerid][pName], reason, gettime()
    );
    mysql_tquery(g_SQL, query, "", "");

    // إذا وصل 5 تحذيرات، ميوت تلقائي 10 دقائق
    if (g_WarnCount[targetid] == 5)
    {
        MuteData[targetid][mMuted] = true;
        MuteData[targetid][mTimeLeft] = 600;
        format(MuteData[targetid][mReason], 128, "5 تحذيرات إدارية");
        SendMessage(targetid, COLOR_MUTE, "[WISE-CITY] تم إسكاتك تلقائياً لمدة 10 دقائق بسبب وصولك لـ 5 تحذيرات.");
    }

    // إذا وصل 10 تحذيرات، حظر تلقائي
    if (g_WarnCount[targetid] == 10)
    {
        new ip[16];
        GetPlayerIp(targetid, ip, 16);

        mysql_format(g_SQL, query, sizeof(query),
            "INSERT INTO `admin_bans` (`username`, `ip_address`, `admin_name`, `reason`, `banned_at`, `expires_at`, `active`) \
            VALUES ('%e', '%e', 'SYSTEM', 'وصل 10 تحذيرات إدارية', %d, %d, 1) \
            ON DUPLICATE KEY UPDATE `active` = 1, `banned_at` = %d, `expires_at` = %d",
            pData[targetid][pName], ip, gettime(), gettime() + 604800, gettime(), gettime() + 604800
        );
        mysql_tquery(g_SQL, query, "", "");

        Kick(targetid);
    }

    LogAdminAction(playerid, "warn", targetid);
    return 1;
}

// --------------------------------------------------
// /reports - عرض البلاغات
// --------------------------------------------------
CMD:reports(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_HELPER))
        return SendMessage(playerid, COLOR_RED, "[خطأ] تحتاج صلاحية إدارية على الأقل.");

    // نظام بلاغات بسيط - يمكن توسيعه لاحقاً
    SendMessage(playerid, COLOR_ADMIN_ACTION, "[WISE-CITY] لا توجد بلاغات نشطة حالياً.");
    return 1;
}

// --------------------------------------------------
// /adminon - عرض الأدمنية المتصلين
// --------------------------------------------------
CMD:adminon(playerid, params[])
{
    if (!HasAdminLevel(playerid, ADMIN_HELPER))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر للأدمنية فقط.");

    new string[1024];
    strcat(string, "{FFD700}===== الأدمنية المتصلون =====\n");

    new count = 0;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i)) continue;
        if (AdminData[i][adLevel] == 0 && !IsPlayerOwner(i)) continue;

        new rankName[32];
        GetAdminLevelName(AdminData[i][adLevel], rankName, sizeof(rankName));

        new line[128];
        format(line, sizeof(line), "{FFFFFF}%s %s | {00FF00}%s {FFFFFF}| %s\n",
            g_AdminTags[AdminData[i][adLevel]],
            pData[i][pName],
            rankName,
            AdminData[i][adDuty] ? "{00FF00}في الخدمة" : "{FF0000}خارج الخدمة"
        );
        strcat(string, line);
        count++;
    }

    if (count == 0)
        return SendMessage(playerid, COLOR_YELLOW, "[WISE-CITY] لا يوجد أدمنية متصلون حالياً.");

    ShowPlayerDialog(playerid, DIALOG_ADMIN_LIST, DIALOG_STYLE_MSGBOX, "الأدمنية المتصلون", string, "حسناً", "");
    return 1;
}

// --------------------------------------------------
// /settag - تحديد تاغ مخصص (للمالك فقط)
// --------------------------------------------------
CMD:settag(playerid, params[])
{
    if (!IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر حصري للمالك walid om.");

    new targetid, tag[16];
    if (sscanf(params, "us[16]", targetid, tag))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /settag [اللاعب] [التاغ]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (AdminData[targetid][adLevel] == 0)
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا اللاعب ليس لديه رتبة إدارية.");

    format(AdminData[targetid][adTag], 16, "%s", tag);
    SaveAdminData(targetid);
    UpdatePlayerTag(targetid);

    new string[256];
    format(string, sizeof(string), "[WISE-CITY] تم تعيين تاغ '%s' للاعب %s.", tag, pData[targetid][pName]);
    SendMessage(playerid, COLOR_GREEN, string);
    SendMessage(targetid, COLOR_OWNER, string);

    LogAdminAction(playerid, "settag", targetid);
    return 1;
}

// --------------------------------------------------
// /owner - قائمة أوامر المالك
// --------------------------------------------------
CMD:owner(playerid, params[])
{
    if (!IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر حصري لمالك السيرفر.");

    new string[1024];
    format(string, sizeof(string),
        "{FFD700}===== أوامر المالك walid om =====\n\n\
        {FFFFFF}/makeadmin [اللاعب] [المستوى] [التاغ] - تعيين/تعديل رتبة إدارية\n\
        {FFFFFF}/settag [اللاعب] [التاغ] - تعيين تاغ مخصص\n\
        {FFFFFF}/setauctiontime - تحديد مواعيد المزادات\n\
        {FFFFFF}/ownermoney [اللاعب] [المبلغ] - تحويل أموال\n\
        {FFFFFF}/ownerweapons [اللاعب] - إعطاء أسلحة كاملة\n\
        {FFFFFF}/ownerskin [اللاعب] [SkinID] - تغيير سكن\n\
        {FFFFFF}/ownertp [اللاعب] - الانتقال الفوري\n\
        {FFFFFF}/setimmunity [اللاعب] [0-2] - تحديد الحصانة\n\
        {FFFFFF}/ownercar [اللاعب] [الموديل] - إعطاء سيارة\n\n\
        {FFD700}جميع الصلاحيات مفعّلة لك بشكل مطلق."
    );
    ShowPlayerDialog(playerid, DIALOG_OWNER_MENU, DIALOG_STYLE_MSGBOX, "قائمة المالك", string, "حسناً", "");
    return 1;
}

// --------------------------------------------------
// /setimmunity - تحديد الحصانة
// --------------------------------------------------
CMD:setimmunity(playerid, params[])
{
    if (!IsPlayerOwner(playerid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر حصري للمالك.");

    new targetid, level;
    if (sscanf(params, "ui", targetid, level))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /setimmunity [اللاعب] [0-2]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (level < 0 || level > 2)
        return SendMessage(playerid, COLOR_RED, "[خطأ] المستوى يجب أن يكون 0، 1، أو 2.");

    AdminData[targetid][adImmunity] = level;
    SaveAdminData(targetid);

    new string[128];
    format(string, sizeof(string), "[WISE-CITY] تم تعيين الحصانة للاعب %s إلى المستوى %d.", pData[targetid][pName], level);
    SendMessage(playerid, COLOR_GREEN, string);
    SendMessage(targetid, COLOR_OWNER, string);

    LogAdminAction(playerid, "setimmunity", targetid);
    return 1;
}

// ============================================================================
// 11. أحداث اللاعب
// ============================================================================

stock Admin_OnPlayerConnect(playerid)
{
    // إعادة تعيين بيانات الإدارة
    AdminData[playerid][adID] = 0;
    AdminData[playerid][adLevel] = 0;
    AdminData[playerid][adImmunity] = 0;
    AdminData[playerid][adDuty] = false;
    AdminData[playerid][adHidden] = false;
    AdminData[playerid][adLoginTime] = 0;
    AdminData[playerid][adTag][0] = EOS;
    format(AdminData[playerid][adRankName], 32, "لاعب");

    // تحميل البيانات بعد تحميل بيانات اللاعب الأساسية
    // (يتم استدعاء LoadAdminData من OnPlayerDataLoaded في نظام اللاعبين)

    // فحص الحظر
    CheckPlayerBan(playerid);
    return 1;
}

stock Admin_OnPlayerDisconnect(playerid, reason)
{
    if (AdminData[playerid][adLevel] > 0 || IsPlayerOwner(playerid))
    {
        SaveAdminData(playerid);
        RemoveAdminPrivileges(playerid);
    }

    // إعادة تعيين
    AdminData[playerid][adLevel] = 0;
    JailData[playerid][jJailed] = false;
    MuteData[playerid][mMuted] = false;
    return 1;
}

// --------------------------------------------------
// فحص الحظر عند الدخول
// --------------------------------------------------
forward CheckPlayerBan(playerid);
public CheckPlayerBan(playerid)
{
    new playerName[MAX_PLAYER_NAME], ip[16];
    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);
    GetPlayerIp(playerid, ip, 16);

    new query[256];
    mysql_format(g_SQL, query, sizeof(query),
        "SELECT * FROM `admin_bans` WHERE `active` = 1 AND (`username` = '%e' OR `ip_address` = '%e') \
        AND (`expires_at` = 0 OR `expires_at` > %d) LIMIT 1",
        playerName, ip, gettime()
    );
    mysql_tquery(g_SQL, query, "OnPlayerBanCheck", "d", playerid);
    return 1;
}

forward OnPlayerBanCheck(playerid);
public OnPlayerBanCheck(playerid)
{
    if (cache_num_rows() > 0)
    {
        new reason[128], adminName[MAX_PLAYER_NAME], expiresAt;
        cache_get_field_content(0, "reason", reason, g_SQL, 128);
        cache_get_field_content(0, "admin_name", adminName, g_SQL, MAX_PLAYER_NAME);
        expiresAt = cache_get_field_content_int(0, "expires_at", g_SQL);

        new string[512];
        if (expiresAt == 0)
        {
            format(string, sizeof(string),
                "{FF0000}أنت محظور من السيرفر بشكل دائم!\n\
                {FFFFFF}السبب: %s\n\
                {FFFFFF}بواسطة: %s\n\n\
                {FFFF00}إذا كنت تعتقد أن هذا خطأ، تواصل مع الإدارة.",
                reason, adminName);
        }
        else
        {
            new remaining = (expiresAt - gettime()) / 3600;
            format(string, sizeof(string),
                "{FF0000}أنت محظور من السيرفر!\n\
                {FFFFFF}السبب: %s\n\
                {FFFFFF}بواسطة: %s\n\
                {FFFFFF}الوقت المتبقي: %d ساعة\n\n\
                {FFFF00}إذا كنت تعتقد أن هذا خطأ، تواصل مع الإدارة.",
                reason, adminName, remaining);
        }

        ShowPlayerDialog(playerid, DIALOG_BANNED, DIALOG_STYLE_MSGBOX, "أنت محظور", string, "خروج", "");
        SetTimerEx("KickPlayerDelayed", 5000, false, "d", playerid);
    }
    return 1;
}

forward KickPlayerDelayed(playerid);
public KickPlayerDelayed(playerid)
{
    if (IsPlayerConnected(playerid)) Kick(playerid);
    return 1;
}

// ============================================================================
// 12. الحوارات
// ============================================================================


stock Admin_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch (dialogid)
    {
        case DIALOG_BANNED:
        {
            Kick(playerid);
        }
    }
    return 0;
}

// ============================================================================
// 13. معالجة الشات لمنع الميوت
// ============================================================================

stock Admin_OnPlayerText(playerid, text[])
{
    // فحص الميوت
    if (MuteData[playerid][mMuted])
    {
        new string[128];
        format(string, sizeof(string), "[WISE-CITY] أنت مسكوت! الوقت المتبقي: %d ثانية.", MuteData[playerid][mTimeLeft]);
        SendMessage(playerid, COLOR_MUTE, string);
        return 0;
    }

    // إذا بدأ النص بـ @، إرسال في شات الإدارة
    if (text[0] == '@')
    {
        if (AdminData[playerid][adLevel] < ADMIN_HELPER && !IsPlayerOwner(playerid))
            return 0;

        new string[256], tag[64];
        FormatAdminTag(playerid, tag, sizeof(tag));
        format(string, sizeof(string), "[ADMIN-CHAT] %s: %s", tag, text[1]);
        SendAdminMessage(ADMIN_HELPER, string, COLOR_ADMIN_CHAT);
        return 0;
    }

    return 1;
}

// ============================================================================
// 14. نهاية السكريبت
// ============================================================================

stock Admin_OnGameModeExit()
{
    // حفظ جميع بيانات الأدمنية
    SaveAllAdminData();

    // إغلاق الاتصال

    KillTimer(g_AdminTimer);
    KillTimer(g_JailTimer);
    KillTimer(g_MuteTimer);
    KillTimer(g_AdminDutyTimer);

    print("[WISE-CITY-ADMIN] تم إغلاق نظام الإدارة وحفظ جميع البيانات.");
    return 1;
}

/*
 * ============================================================================
 *  WISE-CITY-RP | نهاية ملف نظام الإدارة
 *  Wise_City_Admin_System.pwn
 *  المالك الرئيسي: walid om
 * ============================================================================
 */