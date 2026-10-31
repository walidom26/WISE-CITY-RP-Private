/* ============================================================================
    ██╗    ██╗██╗███████╗███████╗     ██████╗██╗████████╗██╗   ██╗
    ██║    ██║██║██╔════╝██╔════╝    ██╔════╝██║╚══██╔══╝╚██╗ ██╔╝
    ██║ █╗ ██║██║███████╗█████╗      ██║     ██║   ██║    ╚████╔╝
    ██║███╗██║██║╚════██║██╔══╝      ██║     ██║   ██║     ╚██╔╝
    ╚███╔███╔╝██║███████║███████╗    ╚██████╗██║   ██║      ██║
     ╚══╝╚══╝ ╚═╝╚══════╝╚══════╝     ╚═════╝╚═╝   ╚═╝      ╚═╝

    WISE-CITY-RP  |  الملف الرئيسي للسيرفر  (الإصدار 2.0)
    المنصة   : open.mp  |  MySQL R41  |  Pawn.CMD  |  sscanf2  |  streamer  |  samp_bcrypt
    المالك   : walid om

    البنية:
      includes/wc_core.inc     النواة: المكتبات + اتصال MySQL الواحد + طبقة التوافق
      includes/wc_secrets.inc  بيانات قاعدة البيانات (عدّلها قبل التشغيل)
      modules/*.inc            الأنظمة (حسابات، فصائل، إدارة، سيارات، منازل ...)

    كل الأحداث (OnPlayerConnect ... إلخ) تُعرَّف هنا مرة واحدة فقط وتوزّع
    على الأنظمة بترتيب ثابت، فلا توجد Callbacks مكررة ولا تعارض بين الملفات.
   ============================================================================ */

#include "includes/wc_core.inc"

// حوارات الملف الرئيسي (3000 - 3099)
#define DIALOG_SERVER_RULES     3000
#define DIALOG_HELP             3001
#define DIALOG_STATS            3002

// ترتيب التضمين مهم: كل نظام يعتمد على ما قبله (الحسابات أولاً)
#include "modules/wc_auth.inc"
#include "modules/wc_factions.inc"
#include "modules/wc_admin.inc"
#include "modules/wc_vehicles.inc"
#include "modules/wc_housing.inc"
#include "modules/wc_cityhall.inc"
#include "modules/wc_ems.inc"
#include "modules/wc_gangs.inc"
#include "modules/wc_social.inc"
#include "modules/wc_phone.inc"

// ============================================================================
//  بيانات الجلسة
// ============================================================================
new g_Kills[MAX_PLAYERS];
new g_Deaths[MAX_PLAYERS];
new g_LoginTime[MAX_PLAYERS];
new g_PeakPlayers;

forward OnServerTimer();
forward OnAnnounceTimer();

main() {}

// ============================================================================
//  تشغيل السيرفر
// ============================================================================
public OnGameModeInit()
{
    print("================================================================");
    print("   WISE-CITY-RP  |  Arabic Roleplay  |  v" SERVER_VERSION);
    print("================================================================");

    if (!Core_ConnectDatabase())
    {
        print("[WISE-CITY] فشل الاتصال بقاعدة البيانات! راجع includes/wc_secrets.inc");
        SendRconCommand("exit");
        return 1;
    }
    print("[WISE-CITY] تم الاتصال بقاعدة البيانات.");

    CreateCoreTables();

    SetGameModeText("WISE-CITY-RP v" SERVER_VERSION);
    ShowPlayerMarkers(PLAYER_MARKERS_MODE_OFF);
    ShowNameTags(true);
    SetWorldTime(12);
    SetWeather(10);
    AllowInteriorWeapons(true);
    // EnableTirePopping(true);
    EnableStuntBonusForAll(false);
    ManualVehicleEngineAndLights();
    DisableInteriorEnterExits();

    g_ServerStartTime = gettime();

    Auth_OnGameModeInit();
    Fac_OnGameModeInit();
    Admin_OnGameModeInit();
    Veh_OnGameModeInit();
    Housing_OnGameModeInit();
    CityHall_OnGameModeInit();
    EMS_OnGameModeInit();
    Gangs_OnGameModeInit();
    Social_OnGameModeInit();
    Phone_OnGameModeInit();

    SetTimer("OnServerTimer", 1000, true);
    SetTimer("OnAnnounceTimer", 600000, true);

    printf("[WISE-CITY] السيرفر يعمل بنجاح | الإصدار %s", SERVER_VERSION);
    return 1;
}

public OnGameModeExit()
{
    // عكس ترتيب التشغيل. الحسابات آخراً لأنها تحفظ اللاعبين.
    Phone_OnGameModeExit();
    Social_OnGameModeExit();
    Gangs_OnGameModeExit();
    EMS_OnGameModeExit();
    CityHall_OnGameModeExit();
    Housing_OnGameModeExit();
    Veh_OnGameModeExit();
    Admin_OnGameModeExit();
    Fac_OnGameModeExit();
    Auth_OnGameModeExit();

    mysql_close(g_SQL);
    print("[WISE-CITY] تم إيقاف السيرفر وحفظ كل البيانات.");
    return 1;
}

stock CreateCoreTables()
{
    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `server_logs` ( \
         `id` INT NOT NULL AUTO_INCREMENT, `log_type` VARCHAR(32) NOT NULL, \
         `player_id` INT NOT NULL DEFAULT -1, `player_name` VARCHAR(24) NOT NULL DEFAULT '', \
         `log_message` TEXT NOT NULL, `log_time` INT NOT NULL, \
         PRIMARY KEY (`id`), INDEX `idx_type` (`log_type`), INDEX `idx_time` (`log_time`) \
         ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `chat_logs` ( \
         `id` INT NOT NULL AUTO_INCREMENT, `player_id` INT NOT NULL, \
         `player_name` VARCHAR(24) NOT NULL, `chat_type` TINYINT NOT NULL DEFAULT 0, \
         `message` TEXT NOT NULL, `logged_at` INT NOT NULL, \
         PRIMARY KEY (`id`), INDEX `idx_player` (`player_id`), INDEX `idx_time` (`logged_at`) \
         ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
    return 1;
}

// ============================================================================
//  السجلات
// ============================================================================
stock LogServerEvent(const type[], playerid, const message[])
{
    new q[512];
    new pNameStr[MAX_PLAYER_NAME];
    new pDBID = -1;
    if (playerid != INVALID_PLAYER_ID) {
        format(pNameStr, sizeof(pNameStr), "%s", pData[playerid][pName]);
        pDBID = pData[playerid][pID];
    } else {
        format(pNameStr, sizeof(pNameStr), "SYSTEM");
    }
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO `server_logs` (`log_type`, `player_id`, `player_name`, `log_message`, `log_time`) VALUES ('%e', %d, '%e', '%e', %d)",
        type, pDBID, pNameStr, message, gettime());
    mysql_tquery(g_SQL, q);
    return 1;
}

stock LogChatMessage(playerid, const message[])
{
    new q[512];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO `chat_logs` (`player_id`, `player_name`, `chat_type`, `message`, `logged_at`) VALUES (%d, '%e', 0, '%e', %d)",
        pData[playerid][pID], pData[playerid][pName], message, gettime());
    mysql_tquery(g_SQL, q);
    return 1;
}

// ============================================================================
//  ربط الأنظمة: بعد تسجيل الدخول + المزامنة قبل الحفظ
// ============================================================================
public Core_OnPlayerLogin(playerid)
{
    g_PlayerDBID[playerid] = pData[playerid][pID];
    g_LoginTime[playerid]  = gettime();
    g_Kills[playerid]      = 0;
    g_Deaths[playerid]     = 0;

    LoadAdminData(playerid);
    Fac_LoadPlayerData(playerid);
    LoadPlayerVehicles(playerid);
    LoadPlayerMedical(playerid);
    LoadPlayerGangData(playerid);
    LoadPlayerSocial(playerid);
    LoadPlayerPhone(playerid);
    LoadPlayerCivicData(playerid);

    ShowWelcome(playerid);
    LogServerEvent("LOGIN", playerid, "تسجيل دخول ناجح");
    return 1;
}

public Core_SyncPlayer(playerid)
{
    Fac_SyncToAuth(playerid);
    return 1;
}

stock ShowWelcome(playerid)
{
    new msg[160];
    SendClientMessage(playerid, COLOR_WHITE, " ");
    SendClientMessage(playerid, 0xFFD700FF, "{FFD700}▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬  W I S E   C I T Y   R P  ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬");
    format(msg, sizeof(msg), "{FFFFFF}أهلاً بك يا {FFD700}%s{FFFFFF} في {FFD700}%s {FFFFFF}| الإصدار {FFD700}%s", pData[playerid][pName], SERVER_NAME, SERVER_VERSION);
    SendClientMessage(playerid, COLOR_WHITE, msg);

    if (pData[playerid][pNewAccount])
    {
        SendClientMessage(playerid, COLOR_WHITE, "{00E676}• هذه أول مرة لك: اقرأ القوانين بـ {FFD700}/rules {00E676}وتعرّف على الأوامر بـ {FFD700}/help");
    }
    else
    {
        SendClientMessage(playerid, COLOR_WHITE, "{00E676}• نتمنى لك جلسة ممتعة. اكتب {FFD700}/help {00E676}لقائمة الأوامر.");
    }
    SendClientMessage(playerid, 0xFFD700FF, "{FFD700}▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬");
    GameTextForPlayer(playerid, "~y~WISE ~w~CITY ~y~RP", 4000, 1);
    return 1;
}

// ============================================================================
//  أحداث اللاعب (موزّع مركزي)
// ============================================================================
public OnPlayerConnect(playerid)
{
    Auth_OnPlayerConnect(playerid);
    Fac_OnPlayerConnect(playerid);
    Admin_OnPlayerConnect(playerid);
    Veh_OnPlayerConnect(playerid);
    Housing_OnPlayerConnect(playerid);
    CityHall_OnPlayerConnect(playerid);
    EMS_OnPlayerConnect(playerid);
    Gangs_OnPlayerConnect(playerid);
    Social_OnPlayerConnect(playerid);
    Phone_OnPlayerConnect(playerid);

    g_Kills[playerid] = 0;
    g_Deaths[playerid] = 0;
    g_LoginTime[playerid] = 0;

    new count = 0;
    for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i)) count++;
    if (count > g_PeakPlayers) g_PeakPlayers = count;
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    // 1) الأنظمة تنظّف وتُرجع ما حجزته (مثل مبلغ المزاد)
    Veh_OnPlayerDisconnect(playerid, reason);
    Fac_OnPlayerDisconnect(playerid, reason);
    Admin_OnPlayerDisconnect(playerid, reason);
    Housing_OnPlayerDisconnect(playerid, reason);
    EMS_OnPlayerDisconnect(playerid, reason);
    Gangs_OnPlayerDisconnect(playerid, reason);
    Social_OnPlayerDisconnect(playerid, reason);

    if (pData[playerid][pLoggedIn]) LogServerEvent("LOGOUT", playerid, "خروج من السيرفر");

    // 2) الحسابات آخراً: تحفظ الصف الكامل ثم تصفّر بيانات اللاعب
    Auth_OnPlayerDisconnect(playerid, reason);
    g_PlayerDBID[playerid] = 0;
    return 1;
}

public OnPlayerSpawn(playerid)
{
    Auth_OnPlayerSpawn(playerid);
    Fac_OnPlayerSpawn(playerid);
    Veh_OnPlayerSpawn(playerid);
    return 1;
}

public OnPlayerDeath(playerid, killerid, WEAPON:reason)
{
    Auth_OnPlayerDeath(playerid, killerid, reason);
    g_Deaths[playerid]++;
    if (killerid != INVALID_PLAYER_ID) g_Kills[killerid]++;
    return 1;
}

public OnPlayerText(playerid, text[])
{
    if (!Auth_OnPlayerText(playerid, text)) return 0;     // لم يسجّل الدخول
    if (!Admin_OnPlayerText(playerid, text)) return 0;    // ميوت أو شات الإدارة @
    if (strlen(text) < 1) return 0;

    // شات محلي بنطاق 30 متر داخل نفس العالم الافتراضي
    new msg[256], Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    format(msg, sizeof(msg), "%s يقول: %s", pData[playerid][pName], text);

    new world = GetPlayerVirtualWorld(playerid);
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !pData[i][pLoggedIn]) continue;
        if (GetPlayerVirtualWorld(i) != world) continue;
        if (GetPlayerDistanceFromPoint(i, x, y, z) <= 30.0) SendClientMessage(i, COLOR_WHITE, msg);
    }
    LogChatMessage(playerid, text);
    return 0;   // منع الشات الافتراضي
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (Auth_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (!pData[playerid][pLoggedIn]) return 1;    // أي حوار قبل الدخول يُتجاهل (حماية)

    if (Fac_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Admin_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Veh_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Housing_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (CityHall_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (EMS_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Gangs_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Social_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;
    if (Phone_OnDialogResponse(playerid, dialogid, response, listitem, inputtext)) return 1;

    if (dialogid == DIALOG_SERVER_RULES || dialogid == DIALOG_HELP || dialogid == DIALOG_STATS) return 1;
    return 0;
}

public OnPlayerPickUpPickup(playerid, pickupid)
{
    CityHall_OnPlayerPickUpPickup(playerid, pickupid);
    EMS_OnPlayerPickUpPickup(playerid, pickupid);
    Social_OnPlayerPickUpPickup(playerid, pickupid);
    return 1;
}

public OnPlayerEnterCheckpoint(playerid)
{
    EMS_OnPlayerEnterCheckpoint(playerid);
    return 1;
}

public OnPlayerEnterVehicle(playerid, vehicleid, ispassenger)
{
    Fac_OnPlayerEnterVehicle(playerid, vehicleid, ispassenger);
    return 1;
}

public OnPlayerStateChange(playerid, PLAYER_STATE:newstate, PLAYER_STATE:oldstate)
{
    Fac_OnPlayerStateChange(playerid, newstate, oldstate);
    return 1;
}

public OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys)
{
    Fac_OnPlayerKeyStateChange(playerid, newkeys, oldkeys);
    return 1;
}

public OnVehicleDeath(vehicleid, killerid)
{
    Fac_OnVehicleDeath(vehicleid, killerid);
    return 1;
}

public OnPlayerWeaponShot(playerid, WEAPON:weaponid, BULLET_HIT_TYPE:hittype, hitid, Float:fX, Float:fY, Float:fZ)
{
    return Fac_OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, fX, fY, fZ);
}

// ============================================================================
//  بوابة الأوامر (Pawn.CMD)
// ============================================================================
public OnPlayerCommandReceived(playerid, cmd[], params[], flags)
{
    if (!pData[playerid][pLoggedIn])
    {
        SendClientMessage(playerid, 0xFF4C4CFF, "يجب تسجيل الدخول أولاً.");
        return 0;
    }
    if (!Fac_OnPlayerCommandReceived(playerid, cmd)) return 0;   // السجن يمنع الأوامر
    return 1;
}

public OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags)
{
    if (result == -1)
    {
        SendClientMessage(playerid, 0xFF4C4CFF, "[WISE-CITY] هذا الأمر غير موجود. اكتب /help لقائمة الأوامر.");
        return 0;
    }
    return 1;
}

// ============================================================================
//  الأوامر العامة
// ============================================================================
CMD:help(playerid, params[])
{
    ShowPlayerDialog(playerid, DIALOG_HELP, DIALOG_STYLE_MSGBOX, "{FFD700}دليل الأوامر - WISE-CITY",
        "{FFD700}عام\n{FFFFFF}/stats  /rules  /uptime  /report\n\n\
         {FFD700}الاقتصاد والممتلكات\n{FFFFFF}/spawncar  /buyhouse  /sellhouse  /rentapartment  /safe  /enter  /exit\n\n\
         {FFD700}الخدمات\n{FFFFFF}/phone  /911  /cityhall\n\n\
         {FFD700}الفصائل والعصابات\n{FFFFFF}/factions  /gang\n\n\
         {AAAAAA}بعض الأوامر تحتاج رتبة أو وظيفة معينة.",
        "إغلاق", "");
    return 1;
}

CMD:rules(playerid, params[])
{
    ShowPlayerDialog(playerid, DIALOG_SERVER_RULES, DIALOG_STYLE_MSGBOX, "{FFD700}قوانين WISE-CITY-RP",
        "{FFFFFF}1. {FFD700}احترم جميع اللاعبين والإدارة.\n\
         {FFFFFF}2. {FFD700}يمنع الغش والهاكات بكل أنواعها.\n\
         {FFFFFF}3. {FFD700}يمنع DM و RK و VDM.\n\
         {FFFFFF}4. {FFD700}يمنع Metagaming و Powergaming.\n\
         {FFFFFF}5. {FFD700}التزم بقوانين الـ Roleplay في كل الأوقات.\n\
         {FFFFFF}6. {FFD700}يمنع الإعلان عن سيرفرات أخرى.\n\
         {FFFFFF}7. {FFD700}أبلغ عن الثغرات ولا تستغلها.\n\
         {FFFFFF}8. {FFD700}نفّذ تعليمات الإدارة فوراً.\n\n\
         {00E676}المخالفة قد تصل إلى الحظر الدائم.",
        "موافق", "");
    return 1;
}

CMD:stats(playerid, params[])
{
    new str[700], played = gettime() - g_LoginTime[playerid];
    new fid = pData[playerid][pFaction];
    format(str, sizeof(str),
        "{FFD700}الاسم: {FFFFFF}%s\n{FFD700}المستوى: {FFFFFF}%d\n{FFD700}الدينار: {FFFFFF}%d\n{FFD700}الذهب: {FFFFFF}%d\n\n\
         {FFD700}الفصيل: {FFFFFF}%s (رتبة %d)\n{FFD700}الجواز: {FFFFFF}%s\n{FFD700}السيارات: {FFFFFF}%d\n\n\
         {FFD700}قتل/موت (الجلسة): {FFFFFF}%d / %d\n{FFD700}وقت الجلسة: {FFFFFF}%d ساعة %d دقيقة",
        pData[playerid][pName], pData[playerid][pLevel], GetPlayerMoney(playerid), pData[playerid][pMoneyGold],
        g_FactionNames[(fid >= 0 && fid < 6) ? fid : 0], pData[playerid][pRank],
        g_PassportNames[pData[playerid][pPassport]], VPlayer[playerid][pOwnedCount],
        g_Kills[playerid], g_Deaths[playerid], played / 3600, (played % 3600) / 60);
    ShowPlayerDialog(playerid, DIALOG_STATS, DIALOG_STYLE_MSGBOX, "{FFD700}إحصائياتك", str, "إغلاق", "");
    return 1;
}

CMD:uptime(playerid, params[])
{
    new up = gettime() - g_ServerStartTime, msg[128], online = 0;
    for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i)) online++;
    format(msg, sizeof(msg), "{FFD700}[WISE-CITY] {FFFFFF}مدة التشغيل: %d ساعة %d دقيقة | المتصلون: %d | الذروة: %d",
        up / 3600, (up % 3600) / 60, online, g_PeakPlayers);
    SendClientMessage(playerid, COLOR_WHITE, msg);
    return 1;
}

// ============================================================================
//  مؤقتات
// ============================================================================
public OnServerTimer()
{
    return 1;
}

public OnAnnounceTimer()
{
    static const tips[][] =
    {
        "{FFD700}[تلميح] {FFFFFF}اقرأ القوانين بـ /rules لتجنب العقوبات.",
        "{FFD700}[تلميح] {FFFFFF}تحتاج مساعدة؟ استخدم /report وستتواصل معك الإدارة.",
        "{FFD700}[تلميح] {FFFFFF}تابع مزادات السيارات الرسمية من معرض السيارات.",
        "{FFD700}[تلميح] {FFFFFF}اكتب /help لرؤية كل الأوامر المتاحة."
    };
    static idx;
    SendClientMessageToAll(COLOR_WHITE, tips[idx]);
    idx = (idx + 1) % sizeof(tips);
    return 1;
}

public OnRconLoginAttempt(ip[], password[], success)
{
    if (!success) printf("[WISE-CITY] محاولة RCON فاشلة من %s", ip);
    return 1;
}
