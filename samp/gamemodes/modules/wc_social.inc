/* ============================================================================
    WISE-CITY-RP  |  نظام الحانة والمسرح والفعاليات
    الوحدة: wc_social.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

#define DIALOG_TAVERN_MENU       5001
#define DIALOG_TAVERN_BUY        5002
#define DIALOG_TAVERN_TAB        5003
#define DIALOG_EMOTES            5004
#define DIALOG_THEATER_MENU      5005
#define DIALOG_EVENT_CREATE      5006
#define DIALOG_EVENT_LIST        5007
#define DIALOG_EVENT_DETAIL      5008
#define DIALOG_SOCIAL_PHONE        5009
#define DIALOG_PHONE_EVENTS      5010
#define DIALOG_MINIGAME          5011
#define DIALOG_DICE              5012

/* ============================================================================
 *  WISE-CITY-RP :: Luxury Tavern, Grand Theater & Social Entertainment
 *  Platform : open.mp / SA-MP
 *  Database : wise_city_db (MySQL R41-4)
 *  Author   : WISE-CITY Development Team
 * ============================================================================ */


// ---------------------------------------------------------------------------
//  MySQL
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
#define MAX_MENU_ITEMS      20
#define MAX_ACTIVE_EVENTS   10
#define MAX_EVENT_SEATS     120
#define MAX_TAB_ITEMS       30

// Dialog IDs

// Tavern location
#define TAVERN_EXT_X        2334.4100
#define TAVERN_EXT_Y        -1650.9000
#define TAVERN_EXT_Z        20.6400
#define TAVERN_INT          10
#define TAVERN_INT_X        501.9000
#define TAVERN_INT_Y        -70.0000
#define TAVERN_INT_Z        998.7578

// Theater location
#define THEATER_EXT_X       1481.6400
#define THEATER_EXT_Y       -1772.4000
#define THEATER_EXT_Z       18.7958
#define THEATER_INT         7
#define THEATER_INT_X       316.7730
#define THEATER_INT_Y       1102.9200
#define THEATER_INT_Z       1008.9140

#define STAGE_X             316.7730
#define STAGE_Y             1092.5000
#define STAGE_Z             1008.9140

// VIP costs
#define VIP_COST            50000
#define VIP_DURATION_DAYS   30

// ---------------------------------------------------------------------------
//  Data structures
// ---------------------------------------------------------------------------
enum e_TavernItem {
    tiID,
    tiKey[32],
    tiName[64],
    tiCategory,
    tiPrice,
    tiStock,
    tiAlcohol
};

enum e_TheaterEvent {
    teID,
    teOrganizer,
    teTitle[96],
    teDesc[256],
    teCategory,
    teTicketPrice,
    teMaxSeats,
    teSoldSeats,
    teStartTime,
    teStatus         // 1 scheduled, 2 live, 3 ended, 4 cancelled
};

enum e_PlayerSocial {
    psBroadcastOptIn,
    psVIP,
    psVIPExpires,
    psEmoteFav[32],
    psTabItems[MAX_TAB_ITEMS],
    psTabCount,
    psTabTotal,
    psSeated,
    psLastEmoteTick
};

// ---------------------------------------------------------------------------
//  Globals
// ---------------------------------------------------------------------------
new TavernMenu[MAX_MENU_ITEMS][e_TavernItem];
new g_TavernMenuCount = 0;

new TheaterEvent[MAX_ACTIVE_EVENTS][e_TheaterEvent];
new g_EventCount = 0;

new PlayerSocial[MAX_PLAYERS][e_PlayerSocial];

// Pickups & labels
new g_TavernPickups[5];
new Text3D:g_TavernLabels[5];
new g_TheaterPickups[5];
new Text3D:g_TheaterLabels[5];

// ---------------------------------------------------------------------------
//  Forwards
// ---------------------------------------------------------------------------
forward LoadTavernMenu();
forward LoadActiveEvents();
forward LoadPlayerSocial(playerid);
forward SavePlayerSocial(playerid);
forward BroadcastEventUpdate(eventid);
forward EndTheaterEvent(eventid);
forward PlayerTick(playerid);

// ===========================================================================
//  GAMEMODE INIT
// ===========================================================================
stock Social_OnGameModeInit()
{

    print("[WISE-CITY-RP] Social System: Connected to wise_city_db.");

    SetupTavern();
    SetupTheater();
    LoadTavernMenu();
    LoadActiveEvents();
    return 1;
}

stock Social_OnGameModeExit()
{
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (IsPlayerConnected(p)) SavePlayerSocial(p);
    }
    return 1;
}

/**
 * Builds the luxurious Tavern.
 */
stock SetupTavern()
{
    // ---- Exterior ----
    CreatePickup(1318, 1, TAVERN_EXT_X, TAVERN_EXT_Y, TAVERN_EXT_Z, 0);
    Create3DTextLabel("{FFFFFF}حانة {FF00FF}WISE-CITY الفاخرة\n{FFFF00}اضغط ENTER للدخول",
        0xFFFFFFFF, TAVERN_EXT_X, TAVERN_EXT_Y, TAVERN_EXT_Z + 1.2, 25.0, 0, 1);

    // Luxurious exterior decorations
    CreateObject(3749, TAVERN_EXT_X, TAVERN_EXT_Y - 3.0, TAVERN_EXT_Z + 6.0, 0.0, 0.0, 0.0);   // arch
    CreateObject(3934, TAVERN_EXT_X + 5.0, TAVERN_EXT_Y + 3.0, TAVERN_EXT_Z, 0.0, 0.0, 0.0);  // neon
    CreateObject(3934, TAVERN_EXT_X - 5.0, TAVERN_EXT_Y + 3.0, TAVERN_EXT_Z, 0.0, 0.0, 0.0);  // neon

    // ---- Interior pickups ----
    // [0] Bar counter
    g_TavernPickups[0] = CreatePickup(1544, 1, 501.9000, -70.0000, 998.7578, -1);
    g_TavernLabels[0] = Create3DTextLabel(
        "{FFFFFF}طاولة {FFAA00}البار\n{FFFFFF}اطلب مشروبك المفضل",
        0xFFFFFFFF, 501.9000, -70.0000, 999.7578, 15.0, TAVERN_INT, 1);

    // [1] Social table 1 (card games)
    g_TavernPickups[1] = CreatePickup(2116, 1, 500.0000, -73.0000, 998.7578, -1);
    g_TavernLabels[1] = Create3DTextLabel(
        "{FFFFFF}طاولة {00FF00}الألعاب\n{FFFFFF}النرد، الورق، والرهانات",
        0xFFFFFFFF, 500.0000, -73.0000, 999.7578, 15.0, TAVERN_INT, 1);

    // [2] VIP lounge
    g_TavernPickups[2] = CreatePickup(19134, 1, 505.0000, -73.0000, 998.7578, -1);
    g_TavernLabels[2] = Create3DTextLabel(
        "{FFFFFF}صالة {FFD700}VIP\n{FFFFFF}لأعضاء VIP فقط",
        0xFFFFFFFF, 505.0000, -73.0000, 999.7578, 15.0, TAVERN_INT, 1);

    // [3] Bar tab / pay bill
    g_TavernPickups[3] = CreatePickup(1274, 1, 503.0000, -68.0000, 998.7578, -1);
    g_TavernLabels[3] = Create3DTextLabel(
        "{FFFFFF}دفع الفاتورة\n{FFFFFF}سدد حسابك الحالي",
        0xFFFFFFFF, 503.0000, -68.0000, 999.7578, 15.0, TAVERN_INT, 1);

    // [4] Exit
    g_TavernPickups[4] = CreatePickup(19198, 1, 498.0000, -70.0000, 998.7578, -1);
    g_TavernLabels[4] = Create3DTextLabel("{FFFFFF}الخروج من الحانة",
        0xFFFFFFFF, 498.0000, -70.0000, 999.7578, 15.0, TAVERN_INT, 1);
}

/**
 * Builds the Grand Theater.
 */
stock SetupTheater()
{
    // ---- Exterior ----
    CreatePickup(19198, 1, THEATER_EXT_X, THEATER_EXT_Y, THEATER_EXT_Z, 0);
    Create3DTextLabel("{FFFFFF}المسرح {FFD700}الكبير\n{FFFF00}اضغط ENTER للدخول",
        0xFFFFFFFF, THEATER_EXT_X, THEATER_EXT_Y, THEATER_EXT_Z + 1.5, 30.0, 0, 1);

    // Majestic exterior (columns + lights)
    for (new i = 0; i < 4; i++) {
        CreateObject(3384, THEATER_EXT_X - 6.0 + (i * 4.0), THEATER_EXT_Y - 4.0,
            THEATER_EXT_Z + 3.0, 0.0, 0.0, 0.0);
    }
    CreateObject(3526, THEATER_EXT_X, THEATER_EXT_Y + 3.0, THEATER_EXT_Z + 1.0, 0.0, 0.0, 0.0);

    // ---- Interior pickups ----
    // [0] Stage manager (organizer)
    g_TheaterPickups[0] = CreatePickup(1275, 1, STAGE_X, STAGE_Y, STAGE_Z, -1);
    g_TheaterLabels[0] = Create3DTextLabel(
        "{FFFFFF}منصة {FF0000}المسرح\n{FFFFFF}للمنظمين: إنشاء وإدارة الأحداث",
        0xFFFFFFFF, STAGE_X, STAGE_Y, STAGE_Z + 1.5, 20.0, THEATER_INT, 1);

    // [1] Audience seating (view)
    g_TheaterPickups[1] = CreatePickup(1239, 1, 316.7730, 1112.0000, 1008.9140, -1);
    g_TheaterLabels[1] = Create3DTextLabel(
        "{FFFFFF}مقاعد {00FFFF}الجمهور\n{FFFFFF}اجلس وحضر العرض",
        0xFFFFFFFF, 316.7730, 1112.0000, 1009.9140, 20.0, THEATER_INT, 1);

    // [2] Ticket booth
    g_TheaterPickups[2] = CreatePickup(1210, 1, 320.0000, 1105.0000, 1008.9140, -1);
    g_TheaterLabels[2] = Create3DTextLabel(
        "{FFFFFF}شباك {FFD700}التذاكر\n{FFFFFF}احجز مقعدك",
        0xFFFFFFFF, 320.0000, 1105.0000, 1009.9140, 15.0, THEATER_INT, 1);

    // [3] Event board
    g_TheaterPickups[3] = CreatePickup(19197, 1, 313.0000, 1105.0000, 1008.9140, -1);
    g_TheaterLabels[3] = Create3DTextLabel(
        "{FFFFFF}لوحة {FFFF00}الأحداث\n{FFFFFF}شاهد الفعاليات القادمة",
        0xFFFFFFFF, 313.0000, 1105.0000, 1009.9140, 15.0, THEATER_INT, 1);

    // [4] Exit
    g_TheaterPickups[4] = CreatePickup(19198, 1, 311.0000, 1114.0000, 1008.9140, -1);
    g_TheaterLabels[4] = Create3DTextLabel("{FFFFFF}الخروج من المسرح",
        0xFFFFFFFF, 311.0000, 1114.0000, 1009.9140, 15.0, THEATER_INT, 1);
}

// ===========================================================================
//  LOADERS
// ===========================================================================
public LoadTavernMenu()
{
    new Cache:res = mysql_query(g_SQL,
        "SELECT item_id, item_key, item_name, category, price, stock, alcohol \
         FROM tavern_inventory WHERE stock > 0 ORDER BY category, price");
    g_TavernMenuCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        if (g_TavernMenuCount >= MAX_MENU_ITEMS) break;
        new idx = g_TavernMenuCount;
        TavernMenu[idx][tiID] = cache_get_field_content_int(row, "item_id", g_SQL);
        cache_get_field_content(row, "item_key",  TavernMenu[idx][tiKey],  g_SQL, 32);
        cache_get_field_content(row, "item_name", TavernMenu[idx][tiName], g_SQL, 64);
        TavernMenu[idx][tiCategory] = cache_get_field_content_int(row, "category", g_SQL);
        TavernMenu[idx][tiPrice]    = cache_get_field_content_int(row, "price", g_SQL);
        TavernMenu[idx][tiStock]    = cache_get_field_content_int(row, "stock", g_SQL);
        TavernMenu[idx][tiAlcohol]  = cache_get_field_content_int(row, "alcohol", g_SQL);
        g_TavernMenuCount++;
    }
    cache_delete(res);
    printf("[WISE-CITY-RP] Loaded %d tavern items.", g_TavernMenuCount);
    return 1;
}

public LoadActiveEvents()
{
    new Cache:res = mysql_query(g_SQL,
        "SELECT event_id, organizer_id, title, description, category, ticket_price, \
         max_seats, sold_seats, start_time, status FROM theater_events \
         WHERE status IN (1, 2) ORDER BY start_time ASC LIMIT 10");
    g_EventCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        if (g_EventCount >= MAX_ACTIVE_EVENTS) break;
        new idx = g_EventCount;
        TheaterEvent[idx][teID] = cache_get_field_content_int(row, "event_id", g_SQL);
        TheaterEvent[idx][teOrganizer] = cache_get_field_content_int(row, "organizer_id", g_SQL);
        cache_get_field_content(row, "title", TheaterEvent[idx][teTitle], g_SQL, 96);
        cache_get_field_content(row, "description", TheaterEvent[idx][teDesc], g_SQL, 256);
        TheaterEvent[idx][teCategory]    = cache_get_field_content_int(row, "category", g_SQL);
        TheaterEvent[idx][teTicketPrice] = cache_get_field_content_int(row, "ticket_price", g_SQL);
        TheaterEvent[idx][teMaxSeats]    = cache_get_field_content_int(row, "max_seats", g_SQL);
        TheaterEvent[idx][teSoldSeats]   = cache_get_field_content_int(row, "sold_seats", g_SQL);
        TheaterEvent[idx][teStartTime]   = cache_get_field_content_int(row, "start_time", g_SQL);
        TheaterEvent[idx][teStatus]      = cache_get_field_content_int(row, "status", g_SQL);
        g_EventCount++;
    }
    cache_delete(res);
    return 1;
}

public LoadPlayerSocial(playerid)
{
    new plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));

    new q[256];
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT id FROM `users` WHERE username='%e' LIMIT 1", plName);
    new Cache:res = mysql_query(g_SQL, q);
    if (cache_num_rows() == 0) { cache_delete(res); return 0; }
    g_PlayerDBID[playerid] = cache_get_field_content_int(0, "id", g_SQL);
    cache_delete(res);

    mysql_format(g_SQL, q, sizeof(q),
        "SELECT broadcast_optin, vip_member, vip_expires, emote_fav \
         FROM social_settings WHERE player_id=%d LIMIT 1",
        g_PlayerDBID[playerid]);
    res = mysql_query(g_SQL, q);
    if (cache_num_rows() == 0) {
        mysql_format(g_SQL, q, sizeof(q),
            "INSERT INTO social_settings (player_id) VALUES (%d)", g_PlayerDBID[playerid]);
        WC_Exec(q);
        PlayerSocial[playerid][psBroadcastOptIn] = 1;
        format(PlayerSocial[playerid][psEmoteFav], 32, "wave");
    } else {
        PlayerSocial[playerid][psBroadcastOptIn] = cache_get_field_content_int(0, "broadcast_optin", g_SQL);
        PlayerSocial[playerid][psVIP]            = cache_get_field_content_int(0, "vip_member", g_SQL);
        PlayerSocial[playerid][psVIPExpires]     = cache_get_field_content_int(0, "vip_expires", g_SQL);
        cache_get_field_content(0, "emote_fav", PlayerSocial[playerid][psEmoteFav], g_SQL, 32);
    }
    cache_delete(res);

    PlayerSocial[playerid][psTabCount] = 0;
    PlayerSocial[playerid][psTabTotal] = 0;
    PlayerSocial[playerid][psSeated]   = 0;
    PlayerSocial[playerid][psLastEmoteTick] = 0;

    // Expire VIP if needed
    if (PlayerSocial[playerid][psVIP] && gettime() > PlayerSocial[playerid][psVIPExpires]) {
        PlayerSocial[playerid][psVIP] = 0;
        SavePlayerSocial(playerid);
    }
    return 1;
}

public SavePlayerSocial(playerid)
{
    if (g_PlayerDBID[playerid] == 0) return 0;
    new q[300];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE social_settings SET broadcast_optin=%d, vip_member=%d, vip_expires=%d, \
         emote_fav='%e' WHERE player_id=%d",
        PlayerSocial[playerid][psBroadcastOptIn],
        PlayerSocial[playerid][psVIP],
        PlayerSocial[playerid][psVIPExpires],
        PlayerSocial[playerid][psEmoteFav],
        g_PlayerDBID[playerid]);
    WC_Exec(q);
    return 1;
}

// ===========================================================================
//  PLAYER CONNECT / DISCONNECT
// ===========================================================================
stock Social_OnPlayerConnect(playerid)
{
    PlayerSocial[playerid][psBroadcastOptIn] = 1;
    PlayerSocial[playerid][psVIP]            = 0;
    PlayerSocial[playerid][psVIPExpires]     = 0;
    PlayerSocial[playerid][psTabCount]       = 0;
    PlayerSocial[playerid][psTabTotal]       = 0;
    PlayerSocial[playerid][psSeated]         = 0;
    PlayerSocial[playerid][psLastEmoteTick]  = 0;
    format(PlayerSocial[playerid][psEmoteFav], 32, "wave");
    g_PlayerDBID[playerid] = 0;
    return 1;
}

stock Social_OnPlayerDisconnect(playerid, reason)
{
    if (g_PlayerDBID[playerid] != 0) SavePlayerSocial(playerid);
    // Auto-charge remaining tab
    if (PlayerSocial[playerid][psTabTotal] > 0) {
        GivePlayerMoney(playerid, -PlayerSocial[playerid][psTabTotal]);
    }
    return 1;
}

// ===========================================================================
//  TAVERN SYSTEM
// ===========================================================================
stock Social_OnPlayerPickUpPickup(playerid, pickupid)
{
    // ---------- Tavern exterior ----------
    if (IsPlayerInRangeOfPoint(playerid, 2.5, TAVERN_EXT_X, TAVERN_EXT_Y, TAVERN_EXT_Z)) {
        SetPlayerInterior(playerid, TAVERN_INT);
        SetPlayerPos(playerid, TAVERN_INT_X, TAVERN_INT_Y, TAVERN_INT_Z);
        SendClientMessage(playerid, 0xFF00FFAA, "[حانة] مرحباً بك في حانة WISE-CITY الفاخرة.");
        return 1;
    }
    // ---------- Theater exterior ----------
    if (IsPlayerInRangeOfPoint(playerid, 2.5, THEATER_EXT_X, THEATER_EXT_Y, THEATER_EXT_Z)) {
        SetPlayerInterior(playerid, THEATER_INT);
        SetPlayerPos(playerid, THEATER_INT_X, THEATER_INT_Y, THEATER_INT_Z);
        SendClientMessage(playerid, 0xFFD700AA, "[مسرح] مرحباً بك في المسرح الكبير.");
        return 1;
    }

    // ---------- Tavern interior ----------
    if (pickupid == g_TavernPickups[0]) { ShowTavernMenu(playerid); return 1; }
    if (pickupid == g_TavernPickups[1]) { ShowMiniGameMenu(playerid); return 1; }
    if (pickupid == g_TavernPickups[2]) { HandleVIPLounge(playerid); return 1; }
    if (pickupid == g_TavernPickups[3]) { ShowTabDialog(playerid); return 1; }
    if (pickupid == g_TavernPickups[4]) {
        SetPlayerInterior(playerid, 0);
        SetPlayerPos(playerid, TAVERN_EXT_X + 1.5, TAVERN_EXT_Y, TAVERN_EXT_Z);
        SendClientMessage(playerid, 0xFF00FFAA, "[حانة] وداعاً، نتمنى لك ليلة سعيدة.");
        return 1;
    }

    // ---------- Theater interior ----------
    if (pickupid == g_TheaterPickups[0]) { ShowOrganizerMenu(playerid); return 1; }
    if (pickupid == g_TheaterPickups[1]) { SitInAudience(playerid); return 1; }
    if (pickupid == g_TheaterPickups[2]) { ShowTicketBooth(playerid); return 1; }
    if (pickupid == g_TheaterPickups[3]) { ShowEventBoard(playerid); return 1; }
    if (pickupid == g_TheaterPickups[4]) {
        SetPlayerInterior(playerid, 0);
        SetPlayerPos(playerid, THEATER_EXT_X + 1.5, THEATER_EXT_Y, THEATER_EXT_Z);
        SendClientMessage(playerid, 0xFFD700AA, "[مسرح] وداعاً، نتمنى لك وقتاً ممتعاً.");
        return 1;
    }
    return 1;
}

stock ShowTavernMenu(playerid)
{
    new str[2048], line[160], catHeader[3][32] = {
        "{FFAA00}المشروبات", "{00FF00}المأكولات", "{FFD700}الفاخر"
    };
    new lastCat = 0;
    format(str, sizeof(str), "قائمة الحانة\n------------------------\n");
    for (new i = 0; i < g_TavernMenuCount; i++) {
        if (TavernMenu[i][tiCategory] != lastCat) {
            lastCat = TavernMenu[i][tiCategory];
            format(line, sizeof(line), "\n%s:\n", catHeader[lastCat - 1]);
            strcat(str, line, sizeof(str));
        }
        format(line, sizeof(line), "%s – %d دينار (المخزون: %d)\n",
            TavernMenu[i][tiName], TavernMenu[i][tiPrice], TavernMenu[i][tiStock]);
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, DIALOG_TAVERN_MENU, DIALOG_STYLE_LIST,
        "طاولة البار", str, "اطلب", "إغلاق");
    return 1;
}

stock ShowTabDialog(playerid)
{
    if (PlayerSocial[playerid][psTabCount] == 0) {
        SendClientMessage(playerid, 0xFFAA00AA, "[حانة] ليس لديك فاتورة مفتوحة.");
        return 1;
    }
    new str[256];
    format(str, sizeof(str),
        "فاتورتك الحالية\n------------------------\nعدد العناصر: %d\nالإجمالي: %d دينار",
        PlayerSocial[playerid][psTabCount], PlayerSocial[playerid][psTabTotal]);
    ShowPlayerDialog(playerid, DIALOG_TAVERN_TAB, DIALOG_STYLE_MSGBOX,
        "الفاتورة", str, "دفع", "إلغاء");
    return 1;
}

stock HandleVIPLounge(playerid)
{
    if (PlayerSocial[playerid][psVIP]) {
        SendClientMessage(playerid, 0xFFD700AA, "[VIP] مرحباً بك في صالة VIP.");
        SetPlayerPos(playerid, 505.5, -73.5, 998.7578);
        return 1;
    }
    new str[256];
    format(str, sizeof(str),
        "صالة VIP\n------------------------\n\
         تصبح عضواً مميزاً لمدة 30 يوماً!\n\
         • خصم 20%% على المشروبات\n\
         • مسارات VIP في المسرح\n\
         • شارة VIP ذهبية\n\
         التكلفة: %d دينار",
        VIP_COST);
    ShowPlayerDialog(playerid, 9999, DIALOG_STYLE_MSGBOX,
        "عضوية VIP", str, "اشتراك", "إلغاء");
    return 1;
}

// ===========================================================================
//  THEATER SYSTEM
// ===========================================================================
stock ShowOrganizerMenu(playerid)
{
    // Any player can organize if they can afford it; OR a city admin
    new str[512];
    format(str, sizeof(str),
        "لوحة المنظمين\n\
         -------------------------\n\
         1. إنشاء حدث جديد\n\
         2. إعلان انطلاق حدث\n\
         3. إعلان نهاية حدث\n\
         4. قائمة الفعاليات");
    ShowPlayerDialog(playerid, DIALOG_THEATER_MENU, DIALOG_STYLE_LIST,
        "منصة المسرح", str, "اختيار", "إغلاق");
    return 1;
}

stock ShowTicketBooth(playerid)
{
    if (g_EventCount == 0) {
        SendClientMessage(playerid, 0xFFAA00AA, "[مسرح] لا توجد فعاليات قادمة.");
        return 1;
    }
    new str[2048], line[128];
    format(str, sizeof(str), "الفعاليات القادمة\n------------------------\n");
    for (new i = 0; i < g_EventCount; i++) {
        if (TheaterEvent[i][teStatus] != 1) continue;
        format(line, sizeof(line), "%d. %s – %d دينار (%d/%d مقعد)\n",
            i, TheaterEvent[i][teTitle], TheaterEvent[i][teTicketPrice],
            TheaterEvent[i][teSoldSeats], TheaterEvent[i][teMaxSeats]);
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, DIALOG_EVENT_LIST, DIALOG_STYLE_LIST,
        "شباك التذاكر", str, "حجز", "إغلاق");
    return 1;
}

stock ShowEventBoard(playerid)
{
    if (g_EventCount == 0) {
        SendClientMessage(playerid, 0xFFAA00AA, "[مسرح] لا توجد فعاليات قادمة.");
        return 1;
    }
    new str[2048], line[160];
    format(str, sizeof(str), "لوحة الأحداث\n------------------------\n");
    for (new i = 0; i < g_EventCount; i++) {
        format(line, sizeof(line), "%d. %s – %s\n", i, TheaterEvent[i][teTitle],
            TheaterEvent[i][teStatus] == 2 ? "{FF0000}مباشر" : "{00FF00}قادم");
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, DIALOG_EVENT_DETAIL, DIALOG_STYLE_LIST,
        "لوحة الأحداث", str, "تفاصيل", "إغلاق");
    return 1;
}

stock SitInAudience(playerid)
{
    if (g_EventCount == 0) {
        SendClientMessage(playerid, 0xFFAA00AA, "[مسرح] لا يوجد عرض حالياً.");
        return 1;
    }
    // Find live event
    new liveIdx = -1;
    for (new i = 0; i < g_EventCount; i++)
        if (TheaterEvent[i][teStatus] == 2) { liveIdx = i; break; }

    if (liveIdx == -1) {
        SendClientMessage(playerid, 0xFFAA00AA, "[مسرح] لا يوجد عرض مباشر حالياً، انتظر انطلاق الفعالية.");
        return 1;
    }

    // Place player in a seat
    new seat = random(60);
    SetPlayerPos(playerid, 316.7730 + (seat % 10) * 1.2, 1112.0 + (seat / 10) * 1.0, 1008.9140);
    SetPlayerFacingAngle(playerid, 180.0);
    ApplyAnimation(playerid, "ped", "SEAT_down", 4.1, 0, 0, 0, 0, 0, 1);
    PlayerSocial[playerid][psSeated] = 1;
    SendClientMessage(playerid, 0x00FF00AA, "[مسرح] جلست في مقعد الجمهور. استمتع بالعرض.");
    return 1;
}

// ===========================================================================
//  EVENT CREATION & MANAGEMENT
// ===========================================================================
stock CreateTheaterEvent(playerid, const title[], const desc[], category, ticketPrice, maxSeats, delayMinutes)
{
    if (g_EventCount >= MAX_ACTIVE_EVENTS) {
        SendClientMessage(playerid, 0xFF0000AA, "[مسرح] لا يمكن إنشاء المزيد من الأحداث حالياً.");
        return 0;
    }

    new startTime = gettime() + (delayMinutes * 60);
    new q[512];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO theater_events (organizer_id, title, description, category, \
         ticket_price, max_seats, start_time, status) \
         VALUES (%d, '%e', '%e', %d, %d, %d, %d, 1)",
        g_PlayerDBID[playerid], title, desc, category, ticketPrice, maxSeats, startTime);
    WC_Exec(q);

    // Add to memory
    new idx = g_EventCount;
    TheaterEvent[idx][teID] = WC_LastInsertID();
    TheaterEvent[idx][teOrganizer] = g_PlayerDBID[playerid];
    format(TheaterEvent[idx][teTitle], 96, "%s", title);
    format(TheaterEvent[idx][teDesc], 256, "%s", desc);
    TheaterEvent[idx][teCategory]    = category;
    TheaterEvent[idx][teTicketPrice] = ticketPrice;
    TheaterEvent[idx][teMaxSeats]    = maxSeats;
    TheaterEvent[idx][teSoldSeats]   = 0;
    TheaterEvent[idx][teStartTime]   = startTime;
    TheaterEvent[idx][teStatus]      = 1;
    g_EventCount++;

    SendClientMessage(playerid, 0x00FF00AA, "[مسرح] تم إنشاء الحدث بنجاح! سيبدأ بعد بضع دقائق.");
    BroadcastEventUpdate(idx);
    return 1;
}

public BroadcastEventUpdate(eventid)
{
    if (eventid < 0 || eventid >= g_EventCount) return 0;
    new str[256];
    format(str, sizeof(str),
        "[مسرح] فعالية جديدة: %s | التذكرة: %d دينار | المقاعد: %d",
        TheaterEvent[eventid][teTitle],
        TheaterEvent[eventid][teTicketPrice],
        TheaterEvent[eventid][teMaxSeats]);

    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (!IsPlayerConnected(p)) continue;
        if (!PlayerSocial[p][psBroadcastOptIn]) continue;
        SendClientMessage(p, 0xFFD700AA, str);
    }
    return 1;
}

public EndTheaterEvent(eventid)
{
    if (eventid < 0 || eventid >= g_EventCount) return 0;
    TheaterEvent[eventid][teStatus] = 3;

    new q[128];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE theater_events SET status=3 WHERE event_id=%d",
        TheaterEvent[eventid][teID]);
    WC_Exec(q);

    new str[160];
    format(str, sizeof(str), "[مسرح] انتهت فعالية '%s'. شكراً للحضور!",
        TheaterEvent[eventid][teTitle]);
    SendClientMessageToAll(0xFFD700AA, str);
    return 1;
}

// ===========================================================================
//  MINI-GAMES
// ===========================================================================
stock ShowMiniGameMenu(playerid)
{
    ShowPlayerDialog(playerid, DIALOG_MINIGAME, DIALOG_STYLE_LIST,
        "طاولة الألعاب",
        "1. لعبة النرد (رهان)\n\
         2. رمي العملة\n\
         3. لعبة الورق (سريع)\n\
         4. مصافحة (تفاعل اجتماعي)",
        "اختيار", "إغلاق");
    return 1;
}

stock PlayDiceGame(playerid, bet)
{
    if (GetPlayerMoney(playerid) < bet) {
        SendClientMessage(playerid, 0xFF0000AA, "[لعبة] لا تملك المال الكافي.");
        return 1;
    }
    new roll = random(6) + 1;
    new win = (roll >= 4);
    if (win) {
        GivePlayerMoney(playerid, bet);
        new str[128];
        format(str, sizeof(str), "[نرد] رميت %d – فزت! (+%d دينار)", roll, bet);
        SendClientMessage(playerid, 0x00FF00AA, str);
        // Notify nearby
        new Float:x, Float:y, Float:z;
        GetPlayerPos(playerid, x, y, z);
        for (new p = 0; p < MAX_PLAYERS; p++) {
            if (p == playerid || !IsPlayerConnected(p)) continue;
            if (IsPlayerInRangeOfPoint(p, 15.0, x, y, z)) {
                SendClientMessage(p, 0xFFFF00AA, str);
            }
        }
    } else {
        GivePlayerMoney(playerid, -bet);
        new str[128];
        format(str, sizeof(str), "[نرد] رميت %d – خسرت. (-%d دينار)", roll, bet);
        SendClientMessage(playerid, 0xFF0000AA, str);
    }
    return 1;
}

stock PlayCoinToss(playerid, bet)
{
    if (GetPlayerMoney(playerid) < bet) {
        SendClientMessage(playerid, 0xFF0000AA, "[لعبة] لا تملك المال الكافي.");
        return 1;
    }
    new head = random(2);
    if (head) {
        GivePlayerMoney(playerid, bet);
        SendClientMessage(playerid, 0x00FF00AA, "[عملة] ظهر! فزت.");
    } else {
        GivePlayerMoney(playerid, -bet);
        SendClientMessage(playerid, 0xFF0000AA, "[عملة] كتابة! خسرت.");
    }
    return 1;
}

// ===========================================================================
//  EMOTES
// ===========================================================================
stock ShowEmoteDialog(playerid)
{
    ShowPlayerDialog(playerid, DIALOG_EMOTES, DIALOG_STYLE_LIST,
        "التفاعلات الاجتماعية",
        "1. تحية (wave)\n\
         2. رقص (dance)\n\
         3. جلوس (sit)\n\
         4. ضحك (laugh)\n\
         5. حزن (cry)\n\
         6. مصافحة (handshake)\n\
         7. توقف (stop)",
        "اختيار", "إغلاق");
    return 1;
}

stock ApplyEmote(playerid, emote)
{
    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    switch (emote) {
        case 1: { ApplyAnimation(playerid, "ped", "wave", 4.1, 0, 0, 0, 0, 0, 1); SendClientMessage(playerid, 0xFFFF00AA, "[تفاعل] لوّحت بيدك."); }
        case 2: { ApplyAnimation(playerid, "dancing", "dance_loop", 4.1, 1, 0, 0, 0, 0, 1); SendClientMessage(playerid, 0xFFFF00AA, "[تفاعل] بدأت بالرقص."); }
        case 3: { ApplyAnimation(playerid, "ped", "SEAT_down", 4.1, 0, 0, 0, 0, 0, 1); SendClientMessage(playerid, 0xFFFF00AA, "[تفاعل] جلست."); }
        case 4: { ApplyAnimation(playerid, "ped", "laugh", 4.1, 0, 0, 0, 0, 0, 1); SendClientMessage(playerid, 0xFFFF00AA, "[تفاعل] ضحكت."); }
        case 5: { ApplyAnimation(playerid, "ped", "cry", 4.1, 1, 0, 0, 0, 0, 1); SendClientMessage(playerid, 0xFFFF00AA, "[تفاعل] بكيت."); }
        case 6: {
            ApplyAnimation(playerid, "mp_ped_interaction", "handshake_guy_a", 4.1, 0, 0, 0, 0, 0, 1);
            SendClientMessage(playerid, 0xFFFF00AA, "[تفاعل] صافحت شخصاً.");
        }
        case 7: { ClearAnimations(playerid); SendClientMessage(playerid, 0xFFFF00AA, "[تفاعل] توقفت."); }
    }

    // Notify nearby players
    new msg[128];
    new plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));
    format(msg, sizeof(msg), "[تفاعل] %s قام بتفاعل اجتماعي.", plName);
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (p == playerid || !IsPlayerConnected(p)) continue;
        if (IsPlayerInRangeOfPoint(p, 20.0, x, y, z)) {
            SendClientMessage(p, 0xFFDD00AA, msg);
        }
    }
    return 1;
}

// ===========================================================================
//  PHONE SYSTEM (broadcast + events)
// ===========================================================================
CMD:socialphone(playerid, params[])
{
    ShowPlayerDialog(playerid, DIALOG_SOCIAL_PHONE, DIALOG_STYLE_LIST,
        "الهاتف الذكي",
        "1. الفعاليات القادمة\n\
         2. حجز تذكرة\n\
         3. الإعدادات الاجتماعية\n\
         4. الإعلانات العامة",
        "اختيار", "إغلاق");
    return 1;
}

// ===========================================================================
//  DIALOG HANDLER
// ===========================================================================
stock Social_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (!response) return 1;

    // ---------------- TAVERN MENU ----------------
    if (dialogid == DIALOG_TAVERN_MENU) {
        if (listitem < 0 || listitem >= g_TavernMenuCount) return 1;
        new price = TavernMenu[listitem][tiPrice];

        // VIP discount
        if (PlayerSocial[playerid][psVIP]) price = (price * 80) / 100;

        if (TavernMenu[listitem][tiStock] <= 0) {
            SendClientMessage(playerid, 0xFF0000AA, "[حانة] هذا المنتج غير متوفر.");
            return 1;
        }
        if (PlayerSocial[playerid][psTabCount] >= MAX_TAB_ITEMS) {
            SendClientMessage(playerid, 0xFF0000AA, "[حانة] فاتورتك ممتلئة، سددها أولاً.");
            return 1;
        }

        // Add to tab
        new slot = PlayerSocial[playerid][psTabCount];
        PlayerSocial[playerid][psTabItems][slot] = listitem;
        PlayerSocial[playerid][psTabCount]++;
        PlayerSocial[playerid][psTabTotal] += price;

        // Decrement stock in DB
        new q[200];
        mysql_format(g_SQL, q, sizeof(q),
            "UPDATE tavern_inventory SET stock=stock-1 WHERE item_id=%d", TavernMenu[listitem][tiID]);
        WC_Exec(q);
        TavernMenu[listitem][tiStock]--;

        // Alcohol effect (simple)
        if (TavernMenu[listitem][tiAlcohol] > 0) {
            new drunk = GetPlayerDrunkLevel(playerid);
            SetPlayerDrunkLevel(playerid, drunk + (TavernMenu[listitem][tiAlcohol] * 1500));
        }

        new str[160];
        format(str, sizeof(str), "[حانة] أضفت %s إلى فاتورتك (%d دينار).",
            TavernMenu[listitem][tiName], price);
        SendClientMessage(playerid, 0xFF00FFAA, str);
        return 1;
    }

    // ---------------- TAB PAY ----------------
    if (dialogid == DIALOG_TAVERN_TAB) {
        new total = PlayerSocial[playerid][psTabTotal];
        if (GetPlayerMoney(playerid) < total) {
            SendClientMessage(playerid, 0xFF0000AA, "[حانة] لا تملك المال الكافي لسداد الفاتورة.");
            return 1;
        }
        GivePlayerMoney(playerid, -total);
        PlayerSocial[playerid][psTabCount] = 0;
        PlayerSocial[playerid][psTabTotal] = 0;
        SendClientMessage(playerid, 0x00FF00AA, "[حانة] تم سداد فاتورتك بنجاح.");
        return 1;
    }

    // ---------------- EMOTES ----------------
    if (dialogid == DIALOG_EMOTES) {
        ApplyEmote(playerid, listitem + 1);
        return 1;
    }

    // ---------------- MINI-GAMES ----------------
    if (dialogid == DIALOG_MINIGAME) {
        switch (listitem) {
            case 0: { // Dice
                ShowPlayerDialog(playerid, DIALOG_DICE, DIALOG_STYLE_INPUT,
                    "لعبة النرد", "أدخل الرهان (100 – 10000 دينار):", "لعب", "إلغاء");
            }
            case 1: { // Coin
                new bet = 500;
                PlayCoinToss(playerid, bet);
            }
            case 2: { // Card game (simplified dice)
                PlayDiceGame(playerid, 1000);
            }
            case 3: { // Handshake
                ApplyEmote(playerid, 6);
            }
        }
        return 1;
    }

    if (dialogid == DIALOG_DICE) {
        new bet = strval(inputtext);
        if (bet < 100 || bet > 10000) {
            SendClientMessage(playerid, 0xFF0000AA, "[لعبة] الرهان بين 100 و 10,000 دينار.");
            return 1;
        }
        PlayDiceGame(playerid, bet);
        return 1;
    }

    // ---------------- THEATER: ORGANIZER MENU ----------------
    if (dialogid == DIALOG_THEATER_MENU) {
        switch (listitem) {
            case 0: { // Create event
                ShowPlayerDialog(playerid, DIALOG_EVENT_CREATE, DIALOG_STYLE_INPUT,
                    "إنشاء فعالية",
                    "أدخل عنوان الفعالية:",
                    "التالي", "إلغاء");
            }
            case 1: { // Start live
                new idx = FindEventByOrganizer(playerid);
                if (idx == -1) { SendClientMessage(playerid, 0xFF0000AA, "[مسرح] لا تملك فعالية مفتوحة."); return 1; }
                TheaterEvent[idx][teStatus] = 2;
                new q[128];
                mysql_format(g_SQL, q, sizeof(q),
                    "UPDATE theater_events SET status=2 WHERE event_id=%d", TheaterEvent[idx][teID]);
                WC_Exec(q);
                new msg[160];
                format(msg, sizeof(msg), "[مسرح] انطلقت فعالية '%s' مباشرة! تفضلوا بالحضور.",
                    TheaterEvent[idx][teTitle]);
                SendClientMessageToAll(0xFFD700AA, msg);
            }
            case 2: { // End event
                new idx = FindEventByOrganizer(playerid);
                if (idx == -1) return 1;
                EndTheaterEvent(idx);
            }
            case 3: ShowEventBoard(playerid);
        }
        return 1;
    }

    if (dialogid == DIALOG_EVENT_CREATE) {
        // Create with default parameters
        CreateTheaterEvent(playerid, inputtext, "فعالية من تنظيم WISE-CITY", 1, 500, 60, 5);
        return 1;
    }

    // ---------------- THEATER: AUDIENCE ----------------
    if (dialogid == DIALOG_EVENT_LIST) {
        if (listitem < 0 || listitem >= g_EventCount) return 1;
        new idx = listitem;
        new price = TheaterEvent[idx][teTicketPrice];
        if (GetPlayerMoney(playerid) < price) {
            SendClientMessage(playerid, 0xFF0000AA, "[مسرح] لا تملك المال الكافي للتذكرة.");
            return 1;
        }
        if (TheaterEvent[idx][teSoldSeats] >= TheaterEvent[idx][teMaxSeats]) {
            SendClientMessage(playerid, 0xFF0000AA, "[مسرح] المقاعد نفدت.");
            return 1;
        }
        GivePlayerMoney(playerid, -price);
        TheaterEvent[idx][teSoldSeats]++;

        new q[256];
        mysql_format(g_SQL, q, sizeof(q),
            "UPDATE theater_events SET sold_seats=sold_seats+1 WHERE event_id=%d",
            TheaterEvent[idx][teID]);
        WC_Exec(q);

        mysql_format(g_SQL, q, sizeof(q),
            "INSERT INTO theater_reservations (event_id, player_id, seats, paid) \
             VALUES (%d, %d, 1, %d)",
            TheaterEvent[idx][teID], g_PlayerDBID[playerid], price);
        WC_Exec(q);

        SendClientMessage(playerid, 0x00FF00AA, "[مسرح] تم حجز تذكرتك بنجاح.");
        return 1;
    }

    if (dialogid == DIALOG_EVENT_DETAIL) {
        if (listitem < 0 || listitem >= g_EventCount) return 1;
        new str[512];
        format(str, sizeof(str),
            "%s\n------------------------\n\
             %s\n\
             السعر: %d دينار\n\
             المقاعد: %d / %d\n\
             الحالة: %s",
            TheaterEvent[listitem][teTitle],
            TheaterEvent[listitem][teDesc],
            TheaterEvent[listitem][teTicketPrice],
            TheaterEvent[listitem][teSoldSeats],
            TheaterEvent[listitem][teMaxSeats],
            TheaterEvent[listitem][teStatus] == 2 ? "مباشر" : "قادم");
        ShowPlayerDialog(playerid, DIALOG_EVENT_DETAIL + 100, DIALOG_STYLE_MSGBOX,
            "تفاصيل الفعالية", str, "حسناً", "");
        return 1;
    }

    // ---------------- PHONE ----------------
    if (dialogid == DIALOG_SOCIAL_PHONE) {
        switch (listitem) {
            case 0, 1: ShowTicketBooth(playerid);
            case 2: {
                PlayerSocial[playerid][psBroadcastOptIn] = !PlayerSocial[playerid][psBroadcastOptIn];
                SavePlayerSocial(playerid);
                SendClientMessage(playerid, 0x00FF00AA,
                    PlayerSocial[playerid][psBroadcastOptIn] ?
                    "[هاتف] تم تفعيل إشعارات الفعاليات." :
                    "[هاتف] تم إيقاف إشعارات الفعاليات.");
            }
            case 3: {
                SendClientMessage(playerid, 0xFFD700AA,
                    "[هاتف] لعرض الفعاليات العامة، توجه إلى المسرح الكبير.");
            }
        }
        return 1;
    }

    return 0;
}

stock FindEventByOrganizer(playerid)
{
    for (new i = 0; i < g_EventCount; i++) {
        if (TheaterEvent[i][teOrganizer] == g_PlayerDBID[playerid] &&
            TheaterEvent[i][teStatus] != 3 && TheaterEvent[i][teStatus] != 4)
            return i;
    }
    return -1;
}

// ===========================================================================
//  COMMANDS
// ===========================================================================
CMD:tavern(playerid, params[])
{
    if (GetPlayerInterior(playerid) != TAVERN_INT) {
        SendClientMessage(playerid, 0xFF0000AA, "[حانة] يجب أن تكون داخل الحانة.");
        return 1;
    }
    ShowTavernMenu(playerid);
    return 1;
}

CMD:emote(playerid, params[])
{
    ShowEmoteDialog(playerid);
    return 1;
}

CMD:theater(playerid, params[])
{
    if (GetPlayerInterior(playerid) != THEATER_INT) {
        SendClientMessage(playerid, 0xFF0000AA, "[مسرح] يجب أن تكون داخل المسرح.");
        return 1;
    }
    ShowOrganizerMenu(playerid);
    return 1;
}

CMD:event(playerid, params[])
{
    ShowTicketBooth(playerid);
    return 1;
}

CMD:vip(playerid, params[])
{
    if (PlayerSocial[playerid][psVIP]) {
        SendClientMessage(playerid, 0xFFD700AA, "[VIP] أنت عضو مميز بالفعل.");
        return 1;
    }
    if (GetPlayerMoney(playerid) < VIP_COST) {
        SendClientMessage(playerid, 0xFF0000AA, "[VIP] تحتاج 50,000 دينار.");
        return 1;
    }
    GivePlayerMoney(playerid, -VIP_COST);
    PlayerSocial[playerid][psVIP] = 1;
    PlayerSocial[playerid][psVIPExpires] = gettime() + (VIP_DURATION_DAYS * 86400);
    SavePlayerSocial(playerid);
    SendClientMessage(playerid, 0xFFD700AA, "[VIP] مبروك! أصبحت عضواً مميزاً لمدة 30 يوماً.");
    return 1;
}

CMD:broadcast(playerid, params[])
{
    if (PlayerSocial[playerid][psVIP] == 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[بث] البث العام للأعضاء VIP فقط.");
        return 1;
    }
    if (isnull(params)) {
        SendClientMessage(playerid, 0xFFFF00AA, "استخدم: /broadcast [الرسالة]");
        return 1;
    }
    new msg[256];
    format(msg, sizeof(msg), "[بث عام] %s: %s", PlayerName(playerid), params);
    SendClientMessageToAll(0xFFD700AA, msg);
    return 1;
}
