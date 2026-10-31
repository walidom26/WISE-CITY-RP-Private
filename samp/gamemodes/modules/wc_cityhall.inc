/* ============================================================================
    WISE-CITY-RP  |  نظام البلدية والوثائق والرخص
    الوحدة: wc_cityhall.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

#define DIALOG_CITYHALL_MAIN     2101
#define DIALOG_CITYHALL_DOCS     2102
#define DIALOG_CITYHALL_JOBS     2103
#define DIALOG_LICENSE_TEST      2104

/* ============================================================================
 *  WISE-CITY-RP :: City Hall (بلدية المدينة) & Civic Jobs System
 *  Platform : open.mp / SA-MP
 *  Database : wise_city_db (MySQL R41-4)
 *  Author   : WISE-CITY Development Team
 * ============================================================================ */


// ---------------------------------------------------------------------------
//  MySQL Configuration
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  System Constants
// ---------------------------------------------------------------------------
#define CITY_HALL_INTERIOR      3           // Interior ID used for City Hall
#define MAX_CIVIC_JOBS          6

// Costs & Requirements
#define COST_ID_CARD            10000       // 10,000 dinars
#define COST_PASSPORT           10000
#define COST_DRIVER_LICENSE     5000
#define REQ_LEVEL_ID            8
#define REQ_LEVEL_PASSPORT      8
#define REQ_LEVEL_LICENSE       5

// Dialogs

// ---------------------------------------------------------------------------
//  City Hall Coordinates (exterior plaza + interior)
// ---------------------------------------------------------------------------
#define CH_EXT_X    1481.2333
#define CH_EXT_Y    -1771.4598
#define CH_EXT_Z    18.7958

#define CH_INT_X    390.5577
#define CH_INT_Y    173.2288
#define CH_INT_Z    1008.3828

// ---------------------------------------------------------------------------
//  Civic Job Definitions
// ---------------------------------------------------------------------------
enum e_CivicJob {
    cjName[64],
    cjPay,               // Pay per delivery/task
    cjRequiredLevel,
    cjVehicleModel
};

new const CivicJobs[MAX_CIVIC_JOBS][e_CivicJob] = {
    {"عامل نظافة (Street Sweeper)",   250,  1, 574},  // Sweeper
    {"سائق توصيل (Delivery Driver)",  400,  3, 414},  // Boxville
    {"سائق باص (Bus Driver)",         600,  5, 431},  // Bus
    {"عامل بناء (Construction)",      500,  4, 486},  // Dozer
    {"سائق أجرة (Taxi Driver)",       350,  2, 420},  // Taxi
    {"حارس أمن (Security Guard)",     700,  6, 0}     // No vehicle
};

// ---------------------------------------------------------------------------
//  Player Document Data
// ---------------------------------------------------------------------------
enum e_PlayerCivic {
    pcID,
    bool:pcHasID,
    bool:pcHasPassport,
    bool:pcHasLicense,
    pcLicensePoints,
    bool:pcLicenseSuspended,
    pcCivicJob,
    pcCivicJobPay
};

new PlayerCivic[MAX_PLAYERS][e_PlayerCivic];
new g_CityHallPickups[5];                 // pickups: ID desk, Passport desk, License desk, Job desk, Exit
new Text3D:g_CityHallLabels[5];

// ---------------------------------------------------------------------------
//  Forward Declarations
// ---------------------------------------------------------------------------
forward LoadPlayerCivicData(playerid);
forward OnCityHallEnter(playerid);
forward OnCityHallExit(playerid);

// ===========================================================================
//  MYSQL CONNECTION & CITY HALL SETUP
// ===========================================================================
stock CityHall_OnGameModeInit()
{

    print("[WISE-CITY-RP] Connected to wise_city_db.");

    SetupCityHall();
    return 1;
}

stock CityHall_OnGameModeExit()
{
    return 1;
}

/**
 * Sets up the City Hall: exterior entrance, interior pickups and 3D labels.
 */
stock SetupCityHall()
{
    // -------- Exterior entrance pickup --------
    CreatePickup(19133, 1, CH_EXT_X, CH_EXT_Y, CH_EXT_Z, 0);
    Create3DTextLabel("{FFFFFF}بلدية {FFD700}WISE-CITY\n{FFFFFF}اضغط {FFFF00}ENTER {FFFFFF}للدخول",
        0xFFFFFFFF, CH_EXT_X, CH_EXT_Y, CH_EXT_Z + 1.0, 25.0, 0, 1);

    // -------- Interior desktops --------
    // [0] ID Card desk
    g_CityHallPickups[0] = CreatePickup(19133, 1, 391.1747, 173.8008, 1008.3828, -1);
    g_CityHallLabels[0] = Create3DTextLabel(
        "{FFFFFF}مكتب {FFD700}البطاقة الوطنية\n{FFFFFF}سعر: {00FF00}10,000 دينار\n{FFFFFF}الشرط: {FFFF00}مستوى 8",
        0xFFFFFFFF, 391.1747, 173.8008, 1009.3828, 15.0, CITY_HALL_INTERIOR, 1);

    // [1] Passport desk
    g_CityHallPickups[1] = CreatePickup(19133, 1, 390.5577, 173.2288, 1008.3828, -1);
    g_CityHallLabels[1] = Create3DTextLabel(
        "{FFFFFF}مكتب {FFD700}جواز السفر\n{FFFFFF}سعر: {00FF00}10,000 دينار\n{FFFFFF}الشرط: {FFFF00}مستوى 8",
        0xFFFFFFFF, 390.5577, 173.2288, 1009.3828, 15.0, CITY_HALL_INTERIOR, 1);

    // [2] Driver License desk
    g_CityHallPickups[2] = CreatePickup(19133, 1, 389.9000, 172.6000, 1008.3828, -1);
    g_CityHallLabels[2] = Create3DTextLabel(
        "{FFFFFF}مكتب {FFD700}رخصة القيادة\n{FFFFFF}سعر: {00FF00}5,000 دينار\n{FFFFFF}يتطلب اختباراً عملياً",
        0xFFFFFFFF, 389.9000, 172.6000, 1009.3828, 15.0, CITY_HALL_INTERIOR, 1);

    // [3] Civic Jobs desk
    g_CityHallPickups[3] = CreatePickup(19133, 1, 389.2000, 172.0000, 1008.3828, -1);
    g_CityHallLabels[3] = Create3DTextLabel(
        "{FFFFFF}مكتب {FFD700}الوظائف المدنية\n{FFFFFF}اختر وظيفتك المناسبة",
        0xFFFFFFFF, 389.2000, 172.0000, 1009.3828, 15.0, CITY_HALL_INTERIOR, 1);

    // [4] Exit pickup
    g_CityHallPickups[4] = CreatePickup(19133, 1, 387.5000, 173.5000, 1008.3828, -1);
    g_CityHallLabels[4] = Create3DTextLabel(
        "{FFFFFF}مخرج البلدية",
        0xFFFFFFFF, 387.5000, 173.5000, 1009.3828, 15.0, CITY_HALL_INTERIOR, 1);
}

// ===========================================================================
//  PLAYER CONNECT / LOAD
// ===========================================================================
stock CityHall_OnPlayerConnect(playerid)
{
    // Reset in memory
    PlayerCivic[playerid][pcHasID]         = false;
    PlayerCivic[playerid][pcHasPassport]   = false;
    PlayerCivic[playerid][pcHasLicense]    = false;
    PlayerCivic[playerid][pcLicensePoints] = 12;
    PlayerCivic[playerid][pcLicenseSuspended] = false;
    PlayerCivic[playerid][pcCivicJob]      = 0;
    PlayerCivic[playerid][pcCivicJobPay]   = 0;
    return 1;
}

/**
 * Loads the player's civic documents from MySQL once they've logged in.
 * Call this from your auth system after the player is authenticated.
 */
public LoadPlayerCivicData(playerid)
{
    new query[256], plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));

    // Resolve player DB id (adjust to your players table)
    mysql_format(g_SQL, query, sizeof(query),
        "SELECT id FROM `users` WHERE username='%e' LIMIT 1", plName);
    new Cache:res = mysql_query(g_SQL, query);
    if (cache_num_rows() == 0) { cache_delete(res); return 0; }
    PlayerCivic[playerid][pcID] = cache_get_field_content_int(0, "id", g_SQL);
    cache_delete(res);

    mysql_format(g_SQL, query, sizeof(query),
        "SELECT has_id_card, has_passport, has_driver_license, license_points, \
         license_suspended, civic_job, civic_job_pay FROM civic_documents WHERE player_id=%d LIMIT 1",
        PlayerCivic[playerid][pcID]);
    res = mysql_query(g_SQL, query);

    if (cache_num_rows() == 0) {
        // Create default row
        mysql_format(g_SQL, query, sizeof(query),
            "INSERT INTO civic_documents (player_id) VALUES (%d)",
            PlayerCivic[playerid][pcID]);
        WC_Exec(query);
    } else {
        PlayerCivic[playerid][pcHasID]            = bool:cache_get_field_content_int(0, "has_id_card", g_SQL);
        PlayerCivic[playerid][pcHasPassport]      = bool:cache_get_field_content_int(0, "has_passport", g_SQL);
        PlayerCivic[playerid][pcHasLicense]       = bool:cache_get_field_content_int(0, "has_driver_license", g_SQL);
        PlayerCivic[playerid][pcLicensePoints]    = cache_get_field_content_int(0, "license_points", g_SQL);
        PlayerCivic[playerid][pcLicenseSuspended] = bool:cache_get_field_content_int(0, "license_suspended", g_SQL);
        PlayerCivic[playerid][pcCivicJob]         = cache_get_field_content_int(0, "civic_job", g_SQL);
        PlayerCivic[playerid][pcCivicJobPay]      = cache_get_field_content_int(0, "civic_job_pay", g_SQL);
    }
    cache_delete(res);
    return 1;
}

/**
 * Saves civic documents back to MySQL.
 */
stock SavePlayerCivic(playerid)
{
    if (PlayerCivic[playerid][pcID] == 0) return 0;
    new query[400];
    mysql_format(g_SQL, query, sizeof(query),
        "UPDATE civic_documents SET has_id_card=%d, has_passport=%d, has_driver_license=%d, \
         license_points=%d, license_suspended=%d, civic_job=%d, civic_job_pay=%d WHERE player_id=%d",
        PlayerCivic[playerid][pcHasID],
        PlayerCivic[playerid][pcHasPassport],
        PlayerCivic[playerid][pcHasLicense],
        PlayerCivic[playerid][pcLicensePoints],
        PlayerCivic[playerid][pcLicenseSuspended],
        PlayerCivic[playerid][pcCivicJob],
        PlayerCivic[playerid][pcCivicJobPay],
        PlayerCivic[playerid][pcID]);
    WC_Exec(query);
    return 1;
}

/**
 * Logs a City Hall transaction.
 */
stock LogCityHallAction(playerid, const action[], amount)
{
    new query[256];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO cityhall_log (player_id, action, amount) VALUES (%d, '%e', %d)",
        PlayerCivic[playerid][pcID], action, amount);
    WC_Exec(query);
}

// ===========================================================================
//  PICKUP HANDLING – INTERIOR & EXTERIOR
// ===========================================================================
stock CityHall_OnPlayerPickUpPickup(playerid, pickupid)
{
    // ---- Exterior City Hall entrance ----
    if (IsPlayerInRangeOfPoint(playerid, 2.5, CH_EXT_X, CH_EXT_Y, CH_EXT_Z)) {
        OnCityHallEnter(playerid);
        return 1;
    }

    // ---- Interior desks ----
    if (pickupid == g_CityHallPickups[0]) { ShowDocsDialog(playerid); return 1; }
    if (pickupid == g_CityHallPickups[1]) { ShowDocsDialog(playerid); return 1; }
    if (pickupid == g_CityHallPickups[2]) { ShowLicenseDialog(playerid); return 1; }
    if (pickupid == g_CityHallPickups[3]) { ShowJobsDialog(playerid); return 1; }
    if (pickupid == g_CityHallPickups[4]) { OnCityHallExit(playerid); return 1; }

    return 1;
}

/**
 * Teleports the player into the City Hall interior.
 */
public OnCityHallEnter(playerid)
{
    SetPlayerInterior(playerid, CITY_HALL_INTERIOR);
    SetPlayerPos(playerid, CH_INT_X, CH_INT_Y, CH_INT_Z);
    SendClientMessage(playerid, 0xFFD700AA, "[البلدية] مرحباً بك في بلدية WISE-CITY. تفضل بالدخول.");
    return 1;
}

/**
 * Teleports the player back outside.
 */
public OnCityHallExit(playerid)
{
    SetPlayerInterior(playerid, 0);
    SetPlayerPos(playerid, CH_EXT_X + 1.5, CH_EXT_Y, CH_EXT_Z);
    SendClientMessage(playerid, 0xFFD700AA, "[البلدية] شكراً لزيارتك. نتمنى لك يوماً سعيداً.");
    return 1;
}

// ===========================================================================
//  MAIN CITY HALL DIALOGS
// ===========================================================================
stock ShowCityHallMenu(playerid)
{
    new str[512];
    format(str, sizeof(str),
        "خدمات البلدية\n\
         ----------------------------\n\
         1. إصدار بطاقة وطنية (10,000 دينار)\n\
         2. إصدار جواز سفر (10,000 دينار)\n\
         3. استخراج رخصة قيادة (5,000 دينار)\n\
         4. التقدم لوظيفة مدنية\n\
         5. عرض وثائقي\n\
         6. مغادرة البلدية");
    ShowPlayerDialog(playerid, DIALOG_CITYHALL_MAIN, DIALOG_STYLE_LIST,
        "بلدية WISE-CITY", str, "اختيار", "إغلاق");
    return 1;
}

stock ShowDocsDialog(playerid)
{
    new str[512];
    format(str, sizeof(str),
        "المستندات الرسمية\n\
         ----------------------------\n\
         1. شراء بطاقة وطنية\n\
         2. شراء جواز سفر");
    ShowPlayerDialog(playerid, DIALOG_CITYHALL_DOCS, DIALOG_STYLE_LIST,
        "مكتب المستندات", str, "اختيار", "رجوع");
    return 1;
}

stock ShowLicenseDialog(playerid)
{
    if (PlayerCivic[playerid][pcHasLicense]) {
        SendClientMessage(playerid, 0xFFAA00AA, "[البلدية] أنت تملك رخصة قيادة بالفعل.");
        return 1;
    }
    if (PlayerCivic[playerid][pcLicenseSuspended]) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] رخصتك موقوفة حالياً، لا يمكنك إصدار رخصة جديدة.");
        return 1;
    }
    ShowPlayerDialog(playerid, DIALOG_LICENSE_TEST, DIALOG_STYLE_MSGBOX,
        "اختبار القيادة",
        "هل أنت مستعد للاختبار العملي لرخصة القيادة؟\n\
         سعر الرخصة: 5,000 دينار\n\
         يجب أن تكون في مركبة قيادة في منطقة الاختبار.",
        "ابدأ", "إلغاء");
    return 1;
}

stock ShowJobsDialog(playerid)
{
    new str[1024], line[128];
    format(str, sizeof(str), "الوظائف المدنية المتاحة\n---------------------------\n");
    for (new i = 0; i < MAX_CIVIC_JOBS; i++) {
        format(line, sizeof(line), "%d. %s\n   الراتب: %d دينار | المستوى: %d\n",
            i + 1, CivicJobs[i][cjName], CivicJobs[i][cjPay], CivicJobs[i][cjRequiredLevel]);
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, DIALOG_CITYHALL_JOBS, DIALOG_STYLE_LIST,
        "مكتب الوظائف المدنية", str, "اختيار", "إغلاق");
    return 1;
}

// ===========================================================================
//  DIALOG RESPONSES
// ===========================================================================
stock CityHall_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (!response) return 1;

    // ---------------- MAIN MENU ----------------
    if (dialogid == DIALOG_CITYHALL_MAIN) {
        switch (listitem) {
            case 0: { // Buy ID card
                if (PlayerCivic[playerid][pcHasID]) {
                    SendClientMessage(playerid, 0xFFAA00AA, "[البلدية] أنت تملك بطاقة وطنية بالفعل.");
                } else if (GetPlayerScore(playerid) < REQ_LEVEL_ID) {
                    SendClientMessage(playerid, 0xFF0000AA, "[البلدية] يجب أن تكون في المستوى 8 على الأقل.");
                } else if (GetPlayerMoney(playerid) < COST_ID_CARD) {
                    SendClientMessage(playerid, 0xFF0000AA, "[البلدية] لا تملك المال الكافي (10,000 دينار).");
                } else {
                    GivePlayerMoney(playerid, -COST_ID_CARD);
                    PlayerCivic[playerid][pcHasID] = true;
                    new q[200]; mysql_format(g_SQL, q, sizeof(q),
                        "UPDATE civic_documents SET has_id_card=1, id_issued_at=NOW() WHERE player_id=%d",
                        PlayerCivic[playerid][pcID]);
                    WC_Exec(q);
                    LogCityHallAction(playerid, "BUY_ID", COST_ID_CARD);
                    SendClientMessage(playerid, 0x00FF00AA, "[البلدية] تم إصدار بطاقتك الوطنية بنجاح.");
                }
            }
            case 1: { // Buy Passport
                if (PlayerCivic[playerid][pcHasPassport]) {
                    SendClientMessage(playerid, 0xFFAA00AA, "[البلدية] أنت تملك جواز سفر بالفعل.");
                } else if (GetPlayerScore(playerid) < REQ_LEVEL_PASSPORT) {
                    SendClientMessage(playerid, 0xFF0000AA, "[البلدية] يجب أن تكون في المستوى 8 على الأقل.");
                } else if (GetPlayerMoney(playerid) < COST_PASSPORT) {
                    SendClientMessage(playerid, 0xFF0000AA, "[البلدية] لا تملك المال الكافي (10,000 دينار).");
                } else {
                    GivePlayerMoney(playerid, -COST_PASSPORT);
                    PlayerCivic[playerid][pcHasPassport] = true;
                    new q[200]; mysql_format(g_SQL, q, sizeof(q),
                        "UPDATE civic_documents SET has_passport=1, passport_issued_at=NOW() WHERE player_id=%d",
                        PlayerCivic[playerid][pcID]);
                    WC_Exec(q);
                    LogCityHallAction(playerid, "BUY_PASSPORT", COST_PASSPORT);
                    SendClientMessage(playerid, 0x00FF00AA, "[البلدية] تم إصدار جواز سفرك بنجاح.");
                }
            }
            case 2: ShowLicenseDialog(playerid);
            case 3: ShowJobsDialog(playerid);
            case 4: { // Show documents
                new str[256];
                format(str, sizeof(str),
                    "البطاقة الوطنية: %s\nجواز السفر: %s\nرخصة القيادة: %s\nنقاط الرخصة: %d",
                    PlayerCivic[playerid][pcHasID] ? "{00FF00}نعم" : "{FF0000}لا",
                    PlayerCivic[playerid][pcHasPassport] ? "{00FF00}نعم" : "{FF0000}لا",
                    PlayerCivic[playerid][pcHasLicense] ? "{00FF00}نعم" : "{FF0000}لا",
                    PlayerCivic[playerid][pcLicensePoints]);
                ShowPlayerDialog(playerid, 9999, DIALOG_STYLE_MSGBOX,
                    "وثائقي", str, "حسناً", "");
            }
            case 5: OnCityHallExit(playerid);
        }
        return 1;
    }

    // ---------------- DOCS DIALOG ----------------
    if (dialogid == DIALOG_CITYHALL_DOCS) {
        if (listitem == 0) {
            // shortcut: simulate choosing ID card in main menu
            new fake = 0;
            #pragma unused fake
        }
        return 1;
    }

    // ---------------- DRIVER LICENSE TEST ----------------
    if (dialogid == DIALOG_LICENSE_TEST) {
        if (GetPlayerScore(playerid) < REQ_LEVEL_LICENSE) {
            SendClientMessage(playerid, 0xFF0000AA, "[البلدية] تحتاج المستوى 5 على الأقل.");
            return 1;
        }
        if (GetPlayerMoney(playerid) < COST_DRIVER_LICENSE) {
            SendClientMessage(playerid, 0xFF0000AA, "[البلدية] لا تملك المال الكافي (5,000 دينار).");
            return 1;
        }
        // Simple pass check – in a real RP server you'd launch a race/checkpoint route.
        new success = random(100) < 80;   // 80% pass rate
        if (success) {
            GivePlayerMoney(playerid, -COST_DRIVER_LICENSE);
            PlayerCivic[playerid][pcHasLicense] = true;
            PlayerCivic[playerid][pcLicensePoints] = 12;
            new q[200]; mysql_format(g_SQL, q, sizeof(q),
                "UPDATE civic_documents SET has_driver_license=1, license_points=12, \
                 license_issued_at=NOW() WHERE player_id=%d",
                PlayerCivic[playerid][pcID]);
            WC_Exec(q);
            LogCityHallAction(playerid, "GET_LICENSE", COST_DRIVER_LICENSE);
            SendClientMessage(playerid, 0x00FF00AA, "[البلدية] مبروك! لقد نجحت في اختبار القيادة وحصلت على رخصتك.");
        } else {
            SendClientMessage(playerid, 0xFF0000AA, "[البلدية] للأسف رسبت في الاختبار. حاول مرة أخرى لاحقاً.");
        }
        return 1;
    }

    // ---------------- CIVIC JOBS ----------------
    if (dialogid == DIALOG_CITYHALL_JOBS) {
        if (listitem < 0 || listitem >= MAX_CIVIC_JOBS) return 1;
        if (GetPlayerScore(playerid) < CivicJobs[listitem][cjRequiredLevel]) {
            new str[128];
            format(str, sizeof(str),
                "[البلدية] هذه الوظيفة تتطلب المستوى %d على الأقل.",
                CivicJobs[listitem][cjRequiredLevel]);
            SendClientMessage(playerid, 0xFF0000AA, str);
            return 1;
        }

        PlayerCivic[playerid][pcCivicJob]    = listitem + 1;
        PlayerCivic[playerid][pcCivicJobPay] = CivicJobs[listitem][cjPay];

        new q[256]; mysql_format(g_SQL, q, sizeof(q),
            "UPDATE civic_documents SET civic_job=%d, civic_job_pay=%d, \
             civic_job_started=NOW() WHERE player_id=%d",
            listitem + 1, CivicJobs[listitem][cjPay], PlayerCivic[playerid][pcID]);
        WC_Exec(q);

        LogCityHallAction(playerid, "JOB_PICK", CivicJobs[listitem][cjPay]);

        new msg[160];
        format(msg, sizeof(msg),
            "[البلدية] تم تعيينك في وظيفة: %s براتب %d دينار للمهمة.",
            CivicJobs[listitem][cjName], CivicJobs[listitem][cjPay]);
        SendClientMessage(playerid, 0x00FF00AA, msg);
        return 1;
    }

    return 0;
}

// ===========================================================================
//  PLAYER COMMANDS (shortcuts)
// ===========================================================================
CMD:cityhall(playerid, params[])
{
    if (!IsPlayerInRangeOfPoint(playerid, 30.0, CH_EXT_X, CH_EXT_Y, CH_EXT_Z) &&
        GetPlayerInterior(playerid) != CITY_HALL_INTERIOR) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] يجب أن تكون قريباً من البلدية لاستخدام هذا الأمر.");
        return 1;
    }
    ShowCityHallMenu(playerid);
    return 1;
}

CMD:buyid(playerid, params[])
{
    if (GetPlayerInterior(playerid) != CITY_HALL_INTERIOR) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] يجب أن تكون داخل البلدية لإصدار البطاقة.");
        return 1;
    }
    if (PlayerCivic[playerid][pcHasID]) {
        SendClientMessage(playerid, 0xFFAA00AA, "[البلدية] أنت تملك بطاقة وطنية بالفعل.");
        return 1;
    }
    if (GetPlayerScore(playerid) < REQ_LEVEL_ID) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] تحتاج المستوى 8 على الأقل.");
        return 1;
    }
    if (GetPlayerMoney(playerid) < COST_ID_CARD) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] تحتاج 10,000 دينار.");
        return 1;
    }
    GivePlayerMoney(playerid, -COST_ID_CARD);
    PlayerCivic[playerid][pcHasID] = true;
    new q[200]; mysql_format(g_SQL, q, sizeof(q),
        "UPDATE civic_documents SET has_id_card=1, id_issued_at=NOW() WHERE player_id=%d",
        PlayerCivic[playerid][pcID]);
    WC_Exec(q);
    LogCityHallAction(playerid, "BUY_ID", COST_ID_CARD);
    SendClientMessage(playerid, 0x00FF00AA, "[البلدية] تم إصدار بطاقتك الوطنية بنجاح.");
    return 1;
}

CMD:getpassport(playerid, params[])
{
    if (GetPlayerInterior(playerid) != CITY_HALL_INTERIOR) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] يجب أن تكون داخل البلدية لإصدار الجواز.");
        return 1;
    }
    if (PlayerCivic[playerid][pcHasPassport]) {
        SendClientMessage(playerid, 0xFFAA00AA, "[البلدية] أنت تملك جواز سفر بالفعل.");
        return 1;
    }
    if (GetPlayerScore(playerid) < REQ_LEVEL_PASSPORT) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] تحتاج المستوى 8 على الأقل.");
        return 1;
    }
    if (GetPlayerMoney(playerid) < COST_PASSPORT) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] تحتاج 10,000 دينار.");
        return 1;
    }
    GivePlayerMoney(playerid, -COST_PASSPORT);
    PlayerCivic[playerid][pcHasPassport] = true;
    new q[200]; mysql_format(g_SQL, q, sizeof(q),
        "UPDATE civic_documents SET has_passport=1, passport_issued_at=NOW() WHERE player_id=%d",
        PlayerCivic[playerid][pcID]);
    WC_Exec(q);
    LogCityHallAction(playerid, "BUY_PASSPORT", COST_PASSPORT);
    SendClientMessage(playerid, 0x00FF00AA, "[البلدية] تم إصدار جواز سفرك بنجاح.");
    return 1;
}

CMD:getlicense(playerid, params[])
{
    if (GetPlayerInterior(playerid) != CITY_HALL_INTERIOR) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] يجب أن تكون داخل البلدية.");
        return 1;
    }
    ShowLicenseDialog(playerid);
    return 1;
}

CMD:quitjob(playerid, params[])
{
    if (PlayerCivic[playerid][pcCivicJob] == 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[البلدية] أنت لا تعمل في أي وظيفة مدنية.");
        return 1;
    }
    PlayerCivic[playerid][pcCivicJob]    = 0;
    PlayerCivic[playerid][pcCivicJobPay] = 0;
    new q[200]; mysql_format(g_SQL, q, sizeof(q),
        "UPDATE civic_documents SET civic_job=0, civic_job_pay=0 WHERE player_id=%d",
        PlayerCivic[playerid][pcID]);
    WC_Exec(q);
    SendClientMessage(playerid, 0xFFAA00AA, "[البلدية] لقد استقلت من وظيفتك المدنية.");
    return 1;
}

CMD:myid(playerid, params[])
{
    new plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));

    new str[400];
    format(str, sizeof(str),
        "بطاقة WISE-CITY الوطنية\n\
         ----------------------------\n\
         الاسم: %s\n\
         رقم الهوية: %d\n\
         الحالة: %s\n\
         جواز السفر: %s\n\
         رخصة القيادة: %s",
        plName,
        PlayerCivic[playerid][pcID],
        PlayerCivic[playerid][pcHasID] ? "مواطن موثّق" : "غير موثّق",
        PlayerCivic[playerid][pcHasPassport] ? "نعم" : "لا",
        PlayerCivic[playerid][pcHasLicense] ? "نعم" : "لا");

    ShowPlayerDialog(playerid, 9998, DIALOG_STYLE_MSGBOX, "بطاقتي", str, "حسناً", "");
    return 1;
}