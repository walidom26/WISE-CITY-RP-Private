/* ============================================================================
    WISE-CITY-RP  |  نظام الحسابات: التسجيل وتسجيل الدخول (bcrypt)
    الوحدة: wc_auth.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

/* =====================================================================
    WISE-CITY-RP  |  نظام التسجيل وتسجيل الدخول (Register & Login)
    ---------------------------------------------------------------------
    المتطلبات (Dependencies):
      - open.mp  (open.mp.inc)
      - pawn-mysql  : a_mysql   (BlueG MySQL plugin  R41-4)
      - samp_bcrypt : samp_bcrypt (Southclaws samp-bcrypt  v2.x)

    قبل التشغيل نفّذ الملف:  wise_city_users_update.sql

    ملاحظات الأمان المطبّقة:
      1) تشفير كلمة المرور بـ bcrypt (Cost 12) داخل Thread منفصل.
      2) كل استعلام SQL يستخدم mysql_format مع %e / %d / %f (لا يوجد SQL Injection).
      3) كلمة المرور لا تدخل أبداً في أي استعلام ولا في format ولا في السجلات.
      4) نظام مراحل (Stage) يمنع إرسال Dialog مزوّر من عميل معدّل.
      5) نظام Session لإبطال أي Callback قديم بعد خروج اللاعب وإعادة استخدام الـ ID.
      6) حد أقصى 3 محاولات دخول + مهلة 90 ثانية للتسجيل/الدخول.
      7) منع الحركة والشات والأوامر قبل تسجيل الدخول.
      8) مسح كلمة المرور المؤقتة والهاش من الذاكرة فور انتهاء الحاجة إليها.
      9) admin_level يُقرأ فقط ولا يُكتب من اللعبة (حماية من الكتابة بقيمة قديمة).
   ===================================================================== */


/* ---------------------------------------------------------------------
    الإعدادات
   --------------------------------------------------------------------- */

#define WC_BCRYPT_COST          12
#define PASSWORD_HASH_LEN       64
#define MIN_PASSWORD_LEN        8
#define MAX_PASSWORD_LEN        32
#define MAX_EMAIL_LEN           100
#define MAX_LOGIN_ATTEMPTS      3
#define AUTH_TIMEOUT_MS         90000
#define AUTOSAVE_INTERVAL_MS    300000
#define MAX_MONEY_VALUE         2000000000
#define REQUIRE_RP_NAME         true        // يشترط اسم بصيغة Firstname_Lastname

#define COLOR_ERROR             0xFF4C4CFF
#define COLOR_SUCCESS           0x4CFF4CFF
#define COLOR_INFO              0x4C9CFFFF
#define COLOR_GOLD              0xFFD700FF

/* ---------------------------------------------------------------------
    معرّفات الـ Dialogs (يجب أن تبقى متتالية)
   --------------------------------------------------------------------- */
enum
{
    DIALOG_LOGIN = 1000,
    DIALOG_REGISTER_PASSWORD,
    DIALOG_REGISTER_CONFIRM,
    DIALOG_REGISTER_EMAIL
}

/* ---------------------------------------------------------------------
    بيانات اللاعب
   --------------------------------------------------------------------- */
enum E_PLAYER
{
    pID,
    pName[MAX_PLAYER_NAME + 1],
    pHash[PASSWORD_HASH_LEN],
    pTempPassword[MAX_PASSWORD_LEN + 1],
    pTempEmail[MAX_EMAIL_LEN + 1],
    pStage,
    pLoginAttempts,
    pAuthTimer,

    bool:pBusy,
    bool:pLoggedIn,
    bool:pNewAccount,
    bool:pSpawned,
    bool:pFirstSpawn,

    pLevel,
    pMoney,
    pMoneyGold,
    pPassport,
    pAdmin,
    pSkin,
    pSpawnType,
    pFaction,
    pRank,
    pWarns,

    Float:pPosX,
    Float:pPosY,
    Float:pPosZ,
    Float:pPosA,
    pInterior,
    pWorld,
    Float:pHealth,
    Float:pArmour
}

new g_Session[MAX_PLAYERS];
new pData[MAX_PLAYERS][E_PLAYER];

/* ---------------------------------------------------------------------
    نقاط الظهور (عدّل الإحداثيات حسب خريطة السيرفر)
    الفصائل: 0 مدني، 1 شرطة، 2 جيش، 3 حرس جمهوري، 4 إسعاف، 5 عصابات
   --------------------------------------------------------------------- */
new const Float:g_DefaultSpawn[4] = {1642.0, -2333.5, 13.5, 0.0};

new const Float:g_FactionSpawns[6][4] =
{
    {1642.0,  -2333.5,  13.5,  0.0},     // 0 : مدني  (المطار)
    {1554.5,  -1675.6,  16.2, 90.0},     // 1 : شرطة
    {-217.8,    979.2,  19.5,  0.0},     // 2 : جيش
    {1481.0,  -1750.0,  15.4,  0.0},     // 3 : حرس جمهوري
    {1177.5,  -1323.8,  14.0,270.0},     // 4 : إسعاف
    {2495.3,  -1688.7,  14.0,  0.0}      // 5 : عصابات
};

new const g_FactionNames[][] =
{
    "مدني", "شرطة", "جيش", "حرس جمهوري", "إسعاف", "عصابات"
};

new const g_PassportNames[][] =
{
    "لا يوجد", "قيد المراجعة", "صادر", "موقوف / ملغي"
};

new const g_WeakPasswords[][] =
{
    "12345678", "123456789", "1234567890", "11111111", "password",
    "password1", "qwerty123", "abc12345", "iloveyou1", "admin1234"
};

/* ---------------------------------------------------------------------
    Forward Declarations
   --------------------------------------------------------------------- */
forward OnAccountChecked(playerid, session);
forward OnEmailChecked(playerid, session);
forward OnPasswordHashed(playerid);
forward OnPlayerRegistered(playerid, session);
forward OnPasswordVerified(playerid, bool:success);
forward OnPlayerDataLoaded(playerid, session);
forward OnPlayerHomeLoaded(playerid, session);
forward OnAuthTimeout(playerid, session);
forward DelayedKick(playerid, session);
forward AutoSaveAll();

/* =====================================================================
    دوال مساعدة
   ===================================================================== */

stock ResetPlayerData(playerid)
{
    static const empty[E_PLAYER];
    pData[playerid] = empty;
    return 1;
}

stock bool:IsSessionValid(playerid, session)
{
    if (playerid < 0 || playerid >= MAX_PLAYERS) return false;
    if (!IsPlayerConnected(playerid)) return false;
    return (g_Session[playerid] == session);
}

stock KickWithReason(playerid, const reason[])
{
    pData[playerid][pBusy]  = true;     // تجاهل أي رد بعد هذه النقطة
    pData[playerid][pStage] = 0;
    SendClientMessage(playerid, COLOR_ERROR, reason);
    SetTimerEx("DelayedKick", 700, false, "dd", playerid, g_Session[playerid]);
    return 1;
}

stock StopAuthTimer(playerid)
{
    if (pData[playerid][pAuthTimer] != 0)
    {
        KillTimer(pData[playerid][pAuthTimer]);
        pData[playerid][pAuthTimer] = 0;
    }
    return 1;
}

/* ---------------------------------------------------------------------
    التحقق من الاسم / كلمة المرور / البريد
   --------------------------------------------------------------------- */

// صيغة الاسم المطلوبة: Firstname_Lastname (حروف إنجليزية فقط)
stock bool:IsValidRPName(const name[])
{
    new len = strlen(name);
    new underscore = -1;

    if (len < 5) return false;

    for (new i = 0; i < len; i++)
    {
        new c = name[i];

        if (c == '_')
        {
            if (underscore != -1) return false;
            underscore = i;
        }
        else if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
        {
            return false;
        }
    }

    if (underscore < 2 || underscore > len - 3) return false;
    if (name[0] < 'A' || name[0] > 'Z') return false;
    if (name[underscore + 1] < 'A' || name[underscore + 1] > 'Z') return false;

    return true;
}

stock bool:IsPasswordStrong(const password[], const name[], error[], errorSize = sizeof(error))
{
    new len = strlen(password);
    new bool:hasLetter = false;
    new bool:hasDigit  = false;

    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN)
    {
        format(error, errorSize, "{FF4C4C}طول كلمة المرور يجب أن يكون بين %d و %d خانة.", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
        return false;
    }

    for (new i = 0; i < len; i++)
    {
        new c = password[i];

        if (c <= ' ')
        {
            format(error, errorSize, "{FF4C4C}كلمة المرور لا يجوز أن تحتوي على مسافات.");
            return false;
        }
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) hasLetter = true;
        if (c >= '0' && c <= '9') hasDigit = true;
    }

    if (!hasLetter || !hasDigit)
    {
        format(error, errorSize, "{FF4C4C}يجب أن تحتوي كلمة المرور على حروف إنجليزية وأرقام معاً.");
        return false;
    }

    if (strfind(password, name, true) != -1)
    {
        format(error, errorSize, "{FF4C4C}كلمة المرور لا يجوز أن تحتوي على اسم شخصيتك.");
        return false;
    }

    for (new i = 0; i < sizeof(g_WeakPasswords); i++)
    {
        if (!strcmp(password, g_WeakPasswords[i], true))
        {
            format(error, errorSize, "{FF4C4C}كلمة المرور هذه شائعة وضعيفة جداً، اختر غيرها.");
            return false;
        }
    }

    return true;
}

stock bool:IsValidEmail(const email[])
{
    new len = strlen(email);
    new at  = -1;
    new dot = -1;

    if (len < 6 || len > MAX_EMAIL_LEN) return false;

    for (new i = 0; i < len; i++)
    {
        new c = email[i];

        if (c == '@')
        {
            if (at != -1 || i == 0) return false;
            at = i;
        }
        else if (c == '.')
        {
            if (i == 0 || i == len - 1) return false;
            if (at != -1) dot = i;
        }
        else if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')
                 || c == '_' || c == '-' || c == '+'))
        {
            return false;
        }
    }

    if (at == -1 || dot == -1) return false;
    if (dot < at + 2)          return false;   // لازم يوجد نطاق قبل النقطة
    if (dot > len - 3)         return false;   // امتداد النطاق حرفان على الأقل

    return true;
}

/* ---------------------------------------------------------------------
    واجهات الـ Dialogs
   --------------------------------------------------------------------- */
stock ShowAuthDialog(playerid, dialogid, style, const caption[], const info[], const button1[], const button2[])
{
    pData[playerid][pStage] = dialogid;
    ShowPlayerDialog(playerid, dialogid, style, caption, info, button1, button2);
    return 1;
}

stock ShowLoginDialog(playerid, const notice[] = "")
{
    new info[512];

    format(info, sizeof(info),
        "{FFFFFF}مرحباً بعودتك إلى {FFD700}WISE-CITY-RP\n\n\
         {FFFFFF}الحساب: {4C9CFF}%s\n\
         {FFFFFF}أدخل كلمة المرور للمتابعة:\n\n%s",
        pData[playerid][pName], notice);

    ShowAuthDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD,
        "WISE-CITY-RP | تسجيل الدخول", info, "دخول", "خروج");
    return 1;
}

stock ShowRegisterPasswordDialog(playerid, const notice[] = "")
{
    new info[768];

    format(info, sizeof(info),
        "{FFFFFF}أهلاً بك في {FFD700}WISE-CITY-RP\n\n\
         {FFFFFF}الحساب {4C9CFF}%s {FFFFFF}غير مسجّل، أنشئ كلمة مرور لحسابك:\n\n\
         {AAAAAA}- من %d إلى %d خانة\n\
         - حروف إنجليزية وأرقام معاً\n\
         - بدون مسافات وبدون اسم شخصيتك\n\n%s",
        pData[playerid][pName], MIN_PASSWORD_LEN, MAX_PASSWORD_LEN, notice);

    ShowAuthDialog(playerid, DIALOG_REGISTER_PASSWORD, DIALOG_STYLE_PASSWORD,
        "WISE-CITY-RP | إنشاء حساب", info, "التالي", "خروج");
    return 1;
}

stock ShowRegisterConfirmDialog(playerid, const notice[] = "")
{
    new info[256];

    format(info, sizeof(info),
        "{FFFFFF}أعد كتابة كلمة المرور للتأكيد:\n\n%s", notice);

    ShowAuthDialog(playerid, DIALOG_REGISTER_CONFIRM, DIALOG_STYLE_PASSWORD,
        "WISE-CITY-RP | تأكيد كلمة المرور", info, "التالي", "خروج");
    return 1;
}

stock ShowRegisterEmailDialog(playerid, const notice[] = "")
{
    new info[384];

    format(info, sizeof(info),
        "{FFFFFF}أدخل بريدك الإلكتروني (يُستخدم لاسترجاع الحساب):\n\n%s", notice);

    ShowAuthDialog(playerid, DIALOG_REGISTER_EMAIL, DIALOG_STYLE_INPUT,
        "WISE-CITY-RP | البريد الإلكتروني", info, "تسجيل", "خروج");
    return 1;
}

stock PrepareAuthScreen(playerid)
{
    TogglePlayerSpectating(playerid, true);
    SetPlayerCameraPos(playerid, 1500.0, -1700.0, 120.0);
    SetPlayerCameraLookAt(playerid, 1481.0, -1750.0, 20.0);
    return 1;
}

stock HandleFailedLogin(playerid)
{
    pData[playerid][pBusy] = false;
    pData[playerid][pLoginAttempts]++;

    new left = MAX_LOGIN_ATTEMPTS - pData[playerid][pLoginAttempts];

    if (left <= 0)
    {
        KickWithReason(playerid, "تجاوزت الحد الأقصى لمحاولات الدخول، تم طردك.");
        return 1;
    }

    new notice[96];
    format(notice, sizeof(notice), "{FF4C4C}كلمة المرور خاطئة! المحاولات المتبقية: %d", left);
    ShowLoginDialog(playerid, notice);
    return 1;
}

/* ---------------------------------------------------------------------
    الحفظ
   --------------------------------------------------------------------- */

// تحديث النسخة المخزّنة في الذاكرة من القيم الحيّة قبل الحفظ
stock UpdateCachedState(playerid)
{
    Core_SyncPlayer(playerid);   // مزامنة بيانات الفصيل وغيرها قبل الحفظ

    new money = GetPlayerMoney(playerid);

    if (money < 0)               money = 0;
    if (money > MAX_MONEY_VALUE) money = MAX_MONEY_VALUE;
    pData[playerid][pMoney] = money;

    // لا نحفظ الإحداثيات إلا إذا كان اللاعب ظهر فعلاً في العالم
    if (pData[playerid][pSpawned])
    {
        GetPlayerPos(playerid, pData[playerid][pPosX], pData[playerid][pPosY], pData[playerid][pPosZ]);
        GetPlayerFacingAngle(playerid, pData[playerid][pPosA]);
        GetPlayerHealth(playerid, pData[playerid][pHealth]);
        GetPlayerArmour(playerid, pData[playerid][pArmour]);
        pData[playerid][pInterior] = GetPlayerInterior(playerid);
        pData[playerid][pWorld]    = GetPlayerVirtualWorld(playerid);
    }
    return 1;
}

stock BuildSaveQuery(playerid, query[], size)
{
    mysql_format(g_SQL, query, size,
        "UPDATE `users` SET \
            `level` = %d, \
            `money_dinar` = %d, \
            `money_gold` = %d, \
            `passport_status` = %d, \
            `character_skin` = %d, \
            `spawn_location` = %d, \
            `faction_id` = %d, \
            `faction_rank` = %d, \
            `faction_warns` = %d, \
            `pos_x` = %f, \
            `pos_y` = %f, \
            `pos_z` = %f, \
            `pos_angle` = %f, \
            `interior_id` = %d, \
            `virtual_world` = %d, \
            `health` = %f, \
            `armour` = %f \
         WHERE `id` = %d",
        pData[playerid][pLevel],
        pData[playerid][pMoney],
        pData[playerid][pMoneyGold],
        pData[playerid][pPassport],
        pData[playerid][pSkin],
        pData[playerid][pSpawnType],
        pData[playerid][pFaction],
        pData[playerid][pRank],
        pData[playerid][pWarns],
        pData[playerid][pPosX],
        pData[playerid][pPosY],
        pData[playerid][pPosZ],
        pData[playerid][pPosA],
        pData[playerid][pInterior],
        pData[playerid][pWorld],
        pData[playerid][pHealth],
        pData[playerid][pArmour],
        pData[playerid][pID]);
    return 1;
}

stock SavePlayerData(playerid)
{
    if (!pData[playerid][pLoggedIn] || pData[playerid][pID] <= 0) return 0;

    new query[1024];

    UpdateCachedState(playerid);
    BuildSaveQuery(playerid, query, sizeof(query));
    mysql_tquery(g_SQL, query);
    return 1;
}

/* ---------------------------------------------------------------------
    تحميل بيانات اللاعب بعد نجاح الدخول
   --------------------------------------------------------------------- */
stock LoadPlayerData(playerid)
{
    new query[768];

    pData[playerid][pBusy] = true;

    mysql_format(g_SQL, query, sizeof(query),
        "SELECT `level`, \
                LEAST(`money_dinar`, %d) AS `money_dinar`, \
                LEAST(`money_gold`, %d)  AS `money_gold`, \
                `passport_status`, `admin_level`, `character_skin`, `spawn_location`, \
                `faction_id`, `faction_rank`, `faction_warns`, \
                `pos_x`, `pos_y`, `pos_z`, `pos_angle`, \
                `interior_id`, `virtual_world`, `health`, `armour` \
         FROM `users` WHERE `id` = %d LIMIT 1",
        MAX_MONEY_VALUE, MAX_MONEY_VALUE, pData[playerid][pID]);

    mysql_tquery(g_SQL, query, "OnPlayerDataLoaded", "dd", playerid, g_Session[playerid]);
    return 1;
}

// إذا كان الموقع المحفوظ فارغاً نستخدم نقطة المطار
stock ApplyDefaultSpawnIfEmpty(playerid)
{
    if (pData[playerid][pPosX] == 0.0 && pData[playerid][pPosY] == 0.0 && pData[playerid][pPosZ] == 0.0)
    {
        pData[playerid][pPosX] = g_DefaultSpawn[0];
        pData[playerid][pPosY] = g_DefaultSpawn[1];
        pData[playerid][pPosZ] = g_DefaultSpawn[2];
        pData[playerid][pPosA] = g_DefaultSpawn[3];
        pData[playerid][pInterior] = 0;
        pData[playerid][pWorld]    = 0;
    }
    return 1;
}

stock FinishLogin(playerid)
{
    StopAuthTimer(playerid);

    pData[playerid][pLoggedIn]      = true;
    pData[playerid][pBusy]          = false;
    pData[playerid][pStage]         = 0;
    pData[playerid][pLoginAttempts] = 0;
    pData[playerid][pFirstSpawn]    = true;
    pData[playerid][pSpawned]       = false;

    // مسح البيانات الحساسة من الذاكرة
    pData[playerid][pHash][0]         = '\0';
    pData[playerid][pTempPassword][0] = '\0';
    pData[playerid][pTempEmail][0]    = '\0';

    // تطبيق بيانات اللاعب
    ResetPlayerMoney(playerid);
    GivePlayerMoney(playerid, pData[playerid][pMoney]);
    SetPlayerScore(playerid, pData[playerid][pLevel]);

    SetSpawnInfo(playerid, NO_TEAM, pData[playerid][pSkin],
        pData[playerid][pPosX], pData[playerid][pPosY], pData[playerid][pPosZ], pData[playerid][pPosA],
        0, 0, 0, 0, 0, 0);

    // تحديث آخر دخول وآخر IP
    new ip[16], query[256];
    GetPlayerIp(playerid, ip, sizeof(ip));
    mysql_format(g_SQL, query, sizeof(query),
        "UPDATE `users` SET `last_login` = NOW(), `last_ip` = '%e' WHERE `id` = %d",
        ip, pData[playerid][pID]);
    mysql_tquery(g_SQL, query);

    // رسائل الترحيب
    new msg[200];

    if (pData[playerid][pNewAccount])
    {
        SendClientMessage(playerid, COLOR_SUCCESS, "تم إنشاء حسابك بنجاح! أهلاً بك في WISE-CITY-RP.");
    }
    else
    {
        format(msg, sizeof(msg), "مرحباً بعودتك %s! تم تسجيل دخولك بنجاح.", pData[playerid][pName]);
        SendClientMessage(playerid, COLOR_SUCCESS, msg);
    }

    format(msg, sizeof(msg), "المستوى: %d | الدينار: %d | الذهب: %d | الفصيل: %s (رتبة %d) | الجواز: %s",
        pData[playerid][pLevel], pData[playerid][pMoney], pData[playerid][pMoneyGold],
        g_FactionNames[pData[playerid][pFaction]], pData[playerid][pRank],
        g_PassportNames[pData[playerid][pPassport]]);
    SendClientMessage(playerid, COLOR_INFO, msg);

    // إنهاء وضع المشاهدة => يظهر اللاعب حسب SetSpawnInfo
    Core_OnPlayerLogin(playerid);   // تحميل بيانات كل الأنظمة بعد نجاح الدخول
    TogglePlayerSpectating(playerid, false);
    return 1;
}

/* =====================================================================
    Callbacks : قاعدة البيانات
   ===================================================================== */

public OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle)
{
    // لا نطبع نص الاستعلام حتى لا تظهر بيانات حساسة في السجلات
    printf("[MySQL ERROR] #%d : %s | Callback: %s", errorid, error, callback);
    return 1;
}

// ---------------------------------------------------------------------
// 1) هل الحساب موجود؟
// ---------------------------------------------------------------------
public OnAccountChecked(playerid, session)
{
    if (!IsSessionValid(playerid, session)) return 1;

    pData[playerid][pBusy] = false;
    PrepareAuthScreen(playerid);

    if (cache_num_rows() > 0)
    {
        cache_get_value_name_int(0, "id", pData[playerid][pID]);
        cache_get_value_name(0, "password_hash", pData[playerid][pHash], PASSWORD_HASH_LEN);

        ShowLoginDialog(playerid);
    }
    else
    {
        pData[playerid][pID] = 0;
        ShowRegisterPasswordDialog(playerid);
    }
    return 1;
}

// ---------------------------------------------------------------------
// 2) فحص تكرار البريد الإلكتروني قبل التسجيل
// ---------------------------------------------------------------------
public OnEmailChecked(playerid, session)
{
    if (!IsSessionValid(playerid, session)) return 1;
    if (!pData[playerid][pBusy] || pData[playerid][pStage] != DIALOG_REGISTER_EMAIL) return 1;

    if (cache_num_rows() > 0)
    {
        pData[playerid][pBusy] = false;
        ShowRegisterEmailDialog(playerid, "{FF4C4C}هذا البريد مستخدم في حساب آخر، أدخل بريداً مختلفاً.");
        return 1;
    }

    // البريد متاح => نشفّر كلمة المرور (Thread منفصل)
    bcrypt_hash(playerid, "OnPasswordHashed", pData[playerid][pTempPassword], WC_BCRYPT_COST);
    pData[playerid][pTempPassword][0] = '\0';      // مسح فوري
    return 1;
}

// ---------------------------------------------------------------------
// 3) بعد انتهاء التشفير => إدخال الحساب في قاعدة البيانات
// ---------------------------------------------------------------------
public OnPasswordHashed(playerid)
{
    if (!IsPlayerConnected(playerid)) return 1;
    if (!pData[playerid][pBusy] || pData[playerid][pStage] != DIALOG_REGISTER_EMAIL) return 1;

    new hash[PASSWORD_HASH_LEN];
    bcrypt_get_hash(hash, sizeof(hash));

    if (strlen(hash) < 20)
    {
        KickWithReason(playerid, "حدث خطأ أثناء تشفير كلمة المرور، حاول مجدداً.");
        return 1;
    }

    new ip[16], query[512];
    GetPlayerIp(playerid, ip, sizeof(ip));

    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `users` (`username`, `password_hash`, `email`, `register_ip`, `last_ip`) \
         VALUES ('%e', '%e', '%e', '%e', '%e')",
        pData[playerid][pName], hash, pData[playerid][pTempEmail], ip, ip);

    mysql_tquery(g_SQL, query, "OnPlayerRegistered", "dd", playerid, g_Session[playerid]);

    hash[0] = '\0';
    return 1;
}

// ---------------------------------------------------------------------
// 4) نتيجة إنشاء الحساب
// ---------------------------------------------------------------------
public OnPlayerRegistered(playerid, session)
{
    if (!IsSessionValid(playerid, session)) return 1;

    new id = cache_insert_id();

    if (id <= 0)
    {
        KickWithReason(playerid, "تعذّر إنشاء الحساب، قد يكون الاسم أو البريد مستخدماً. حاول مجدداً.");
        return 1;
    }

    pData[playerid][pID]         = id;
    pData[playerid][pNewAccount] = true;
    pData[playerid][pTempEmail][0] = '\0';

    LoadPlayerData(playerid);
    return 1;
}

// ---------------------------------------------------------------------
// 5) نتيجة التحقق من كلمة المرور (bcrypt)
// ---------------------------------------------------------------------
public OnPasswordVerified(playerid, bool:success)
{
    if (!IsPlayerConnected(playerid)) return 1;
    if (!pData[playerid][pBusy] || pData[playerid][pStage] != DIALOG_LOGIN) return 1;
    if (pData[playerid][pLoggedIn]) return 1;

    if (success)
    {
        LoadPlayerData(playerid);
    }
    else
    {
        HandleFailedLogin(playerid);
    }
    return 1;
}

// ---------------------------------------------------------------------
// 6) تحميل كامل بيانات اللاعب
// ---------------------------------------------------------------------
public OnPlayerDataLoaded(playerid, session)
{
    if (!IsSessionValid(playerid, session)) return 1;

    if (cache_num_rows() < 1)
    {
        KickWithReason(playerid, "تعذّر تحميل بيانات حسابك، تواصل مع الإدارة.");
        return 1;
    }

    cache_get_value_name_int(0, "level",           pData[playerid][pLevel]);
    cache_get_value_name_int(0, "money_dinar",     pData[playerid][pMoney]);
    cache_get_value_name_int(0, "money_gold",      pData[playerid][pMoneyGold]);
    cache_get_value_name_int(0, "passport_status", pData[playerid][pPassport]);
    cache_get_value_name_int(0, "admin_level",     pData[playerid][pAdmin]);
    cache_get_value_name_int(0, "character_skin",  pData[playerid][pSkin]);
    cache_get_value_name_int(0, "spawn_location",  pData[playerid][pSpawnType]);
    cache_get_value_name_int(0, "faction_id",      pData[playerid][pFaction]);
    cache_get_value_name_int(0, "faction_rank",    pData[playerid][pRank]);
    cache_get_value_name_int(0, "faction_warns",   pData[playerid][pWarns]);

    cache_get_value_name_float(0, "pos_x",     pData[playerid][pPosX]);
    cache_get_value_name_float(0, "pos_y",     pData[playerid][pPosY]);
    cache_get_value_name_float(0, "pos_z",     pData[playerid][pPosZ]);
    cache_get_value_name_float(0, "pos_angle", pData[playerid][pPosA]);
    cache_get_value_name_int(0, "interior_id",   pData[playerid][pInterior]);
    cache_get_value_name_int(0, "virtual_world", pData[playerid][pWorld]);
    cache_get_value_name_float(0, "health", pData[playerid][pHealth]);
    cache_get_value_name_float(0, "armour", pData[playerid][pArmour]);

    // ---- التحقق من سلامة القيم المحمّلة ----
    if (pData[playerid][pFaction] < 0 || pData[playerid][pFaction] > 5)         pData[playerid][pFaction] = 0;
    if (pData[playerid][pPassport] < 0 || pData[playerid][pPassport] > 3)       pData[playerid][pPassport] = 0;
    if (pData[playerid][pAdmin] < 0 || pData[playerid][pAdmin] > 5)             pData[playerid][pAdmin] = 0;
    if (pData[playerid][pSpawnType] < 0 || pData[playerid][pSpawnType] > 2)     pData[playerid][pSpawnType] = 0;
    if (pData[playerid][pSkin] < 0 || pData[playerid][pSkin] > 311)             pData[playerid][pSkin] = 0;
    if (pData[playerid][pLevel] < 1)                                            pData[playerid][pLevel] = 1;
    if (pData[playerid][pHealth] <= 0.0 || pData[playerid][pHealth] > 100.0)    pData[playerid][pHealth] = 100.0;
    if (pData[playerid][pArmour] < 0.0  || pData[playerid][pArmour] > 100.0)    pData[playerid][pArmour] = 0.0;

    // ---- تحديد موقع الظهور ----
    switch (pData[playerid][pSpawnType])
    {
        case 0: // آخر موقع محفوظ
        {
            ApplyDefaultSpawnIfEmpty(playerid);
        }
        case 1: // المنزل => نحتاج استعلام العقارات
        {
            new query[256];
            mysql_format(g_SQL, query, sizeof(query),
                "SELECT `entrance_x`, `entrance_y`, `entrance_z` FROM `properties` \
                 WHERE `owner_id` = %d OR `renter_id` = %d ORDER BY `id` ASC LIMIT 1",
                pData[playerid][pID], pData[playerid][pID]);
            mysql_tquery(g_SQL, query, "OnPlayerHomeLoaded", "dd", playerid, g_Session[playerid]);
            return 1;
        }
        case 2: // مقر الفصيل
        {
            new f = pData[playerid][pFaction];

            pData[playerid][pPosX] = g_FactionSpawns[f][0];
            pData[playerid][pPosY] = g_FactionSpawns[f][1];
            pData[playerid][pPosZ] = g_FactionSpawns[f][2];
            pData[playerid][pPosA] = g_FactionSpawns[f][3];
            pData[playerid][pInterior] = 0;
            pData[playerid][pWorld]    = 0;
        }
    }

    FinishLogin(playerid);
    return 1;
}

public OnPlayerHomeLoaded(playerid, session)
{
    if (!IsSessionValid(playerid, session)) return 1;

    if (cache_num_rows() > 0)
    {
        cache_get_value_name_float(0, "entrance_x", pData[playerid][pPosX]);
        cache_get_value_name_float(0, "entrance_y", pData[playerid][pPosY]);
        cache_get_value_name_float(0, "entrance_z", pData[playerid][pPosZ]);
        pData[playerid][pPosA]     = 0.0;
        pData[playerid][pInterior] = 0;
        pData[playerid][pWorld]    = 0;
    }
    else
    {
        SendClientMessage(playerid, COLOR_INFO, "لا تملك منزلاً أو عقاراً مستأجراً، سيتم ظهورك في آخر موقع محفوظ.");
        ApplyDefaultSpawnIfEmpty(playerid);
    }

    FinishLogin(playerid);
    return 1;
}

/* =====================================================================
    Timers
   ===================================================================== */

public OnAuthTimeout(playerid, session)
{
    if (!IsSessionValid(playerid, session)) return 1;

    pData[playerid][pAuthTimer] = 0;

    if (!pData[playerid][pLoggedIn])
    {
        KickWithReason(playerid, "انتهت مهلة تسجيل الدخول / التسجيل، تم طردك.");
    }
    return 1;
}

public DelayedKick(playerid, session)
{
    if (IsSessionValid(playerid, session)) Kick(playerid);
    return 1;
}

public AutoSaveAll()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i)) SavePlayerData(i);
    }
    return 1;
}

/* =====================================================================
    Callbacks : السيرفر
   ===================================================================== */

stock Auth_OnGameModeInit()
{
    SetTimer("AutoSaveAll", AUTOSAVE_INTERVAL_MS, true);
    return 1;
}

stock Auth_OnGameModeExit()
{
    // حفظ متزامن (Blocking) لضمان عدم ضياع أي بيانات عند إغلاق السيرفر
    new query[1024];

    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !pData[i][pLoggedIn]) continue;

        UpdateCachedState(i);
        BuildSaveQuery(i, query, sizeof(query));
        WC_Exec(query);
    }

    return 1;
}

stock Auth_OnPlayerConnect(playerid)
{
    g_Session[playerid]++;
    ResetPlayerData(playerid);

    GetPlayerName(playerid, pData[playerid][pName], MAX_PLAYER_NAME + 1);

    if (REQUIRE_RP_NAME && !IsValidRPName(pData[playerid][pName]))
    {
        KickWithReason(playerid, "الاسم غير مقبول. استخدم الصيغة: Firstname_Lastname (حروف إنجليزية فقط).");
        return 1;
    }

    pData[playerid][pBusy] = true;

    // مهلة المصادقة تبدأ من لحظة الاتصال
    pData[playerid][pAuthTimer] = SetTimerEx("OnAuthTimeout", AUTH_TIMEOUT_MS, false, "dd", playerid, g_Session[playerid]);

    new query[192];
    mysql_format(g_SQL, query, sizeof(query),
        "SELECT `id`, `password_hash` FROM `users` WHERE `username` = '%e' LIMIT 1",
        pData[playerid][pName]);
    mysql_tquery(g_SQL, query, "OnAccountChecked", "dd", playerid, g_Session[playerid]);
    return 1;
}

stock Auth_OnPlayerDisconnect(playerid, reason)
{
    StopAuthTimer(playerid);
    SavePlayerData(playerid);

    g_Session[playerid]++;          // إبطال أي Callback معلّق لهذا اللاعب
    ResetPlayerData(playerid);
    return 1;
}

public OnPlayerRequestClass(playerid, classid)
{
    if (!pData[playerid][pLoggedIn])
    {
        PrepareAuthScreen(playerid);
    }
    return 1;
}

public OnPlayerRequestSpawn(playerid)
{
    if (!pData[playerid][pLoggedIn]) return 0;
    return 1;
}

stock Auth_OnPlayerSpawn(playerid)
{
    if (!pData[playerid][pLoggedIn])
    {
        PrepareAuthScreen(playerid);
        return 1;
    }

    pData[playerid][pSpawned] = true;

    if (pData[playerid][pFirstSpawn])
    {
        // أول ظهور بعد الدخول: نطبّق الحالة المحفوظة
        pData[playerid][pFirstSpawn] = false;

        SetPlayerInterior(playerid, pData[playerid][pInterior]);
        SetPlayerVirtualWorld(playerid, pData[playerid][pWorld]);
        SetPlayerHealth(playerid, pData[playerid][pHealth]);
        SetPlayerArmour(playerid, pData[playerid][pArmour]);
    }
    else
    {
        // ظهور بعد الموت (اربطه لاحقاً بنظام المستشفى)
        SetPlayerInterior(playerid, 0);
        SetPlayerVirtualWorld(playerid, 0);
        SetPlayerHealth(playerid, 100.0);
        SetPlayerArmour(playerid, 0.0);
    }
    return 1;
}

stock Auth_OnPlayerDeath(playerid, killerid, WEAPON:reason)
{
    pData[playerid][pSpawned] = false;
    return 1;
}

stock Auth_OnPlayerText(playerid, text[])
{
    if (!pData[playerid][pLoggedIn])
    {
        SendClientMessage(playerid, COLOR_ERROR, "يجب تسجيل الدخول أولاً.");
        return 0;
    }
    return 1;
}



/* =====================================================================
    OnDialogResponse
   ===================================================================== */
stock Auth_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid < DIALOG_LOGIN || dialogid > DIALOG_REGISTER_EMAIL) return 0;   // ليست حوارات الحسابات

    // ليست من dialogs نظام المصادقة
    if (dialogid < DIALOG_LOGIN || dialogid > DIALOG_REGISTER_EMAIL) return 0;

    // حماية: لاعب مسجّل / عملية جارية / dialog غير الذي أرسلناه فعلاً
    if (pData[playerid][pLoggedIn] || pData[playerid][pBusy] || pData[playerid][pStage] != dialogid)
    {
        return 1;
    }

    if (!response)
    {
        KickWithReason(playerid, "تم إلغاء تسجيل الدخول.");
        return 1;
    }

    switch (dialogid)
    {
        // -------------------------------------------------------------
        case DIALOG_LOGIN:
        {
            new len = strlen(inputtext);

            if (len < 1 || len > MAX_PASSWORD_LEN)
            {
                HandleFailedLogin(playerid);
                return 1;
            }

            pData[playerid][pBusy] = true;
            bcrypt_verify(playerid, "OnPasswordVerified", inputtext, pData[playerid][pHash]);
        }

        // -------------------------------------------------------------
        case DIALOG_REGISTER_PASSWORD:
        {
            new error[128];

            if (!IsPasswordStrong(inputtext, pData[playerid][pName], error))
            {
                ShowRegisterPasswordDialog(playerid, error);
                return 1;
            }

            format(pData[playerid][pTempPassword], MAX_PASSWORD_LEN + 1, "%s", inputtext);
            ShowRegisterConfirmDialog(playerid);
        }

        // -------------------------------------------------------------
        case DIALOG_REGISTER_CONFIRM:
        {
            if (pData[playerid][pTempPassword][0] == '\0')
            {
                ShowRegisterPasswordDialog(playerid, "{FF4C4C}حدث خطأ، أعد إدخال كلمة المرور.");
                return 1;
            }

            if (strcmp(inputtext, pData[playerid][pTempPassword]) != 0)
            {
                pData[playerid][pTempPassword][0] = '\0';
                ShowRegisterPasswordDialog(playerid, "{FF4C4C}كلمتا المرور غير متطابقتين، أعد المحاولة.");
                return 1;
            }

            ShowRegisterEmailDialog(playerid);
        }

        // -------------------------------------------------------------
        case DIALOG_REGISTER_EMAIL:
        {
            if (!IsValidEmail(inputtext))
            {
                ShowRegisterEmailDialog(playerid, "{FF4C4C}البريد الإلكتروني غير صالح، أعد إدخاله.");
                return 1;
            }

            if (pData[playerid][pTempPassword][0] == '\0')
            {
                ShowRegisterPasswordDialog(playerid, "{FF4C4C}حدث خطأ، أعد إدخال كلمة المرور.");
                return 1;
            }

            format(pData[playerid][pTempEmail], MAX_EMAIL_LEN + 1, "%s", inputtext);
            pData[playerid][pBusy] = true;

            new query[256];
            mysql_format(g_SQL, query, sizeof(query),
                "SELECT `id` FROM `users` WHERE `email` = '%e' LIMIT 1",
                pData[playerid][pTempEmail]);
            mysql_tquery(g_SQL, query, "OnEmailChecked", "dd", playerid, g_Session[playerid]);
        }
    }
    return 1;
}
