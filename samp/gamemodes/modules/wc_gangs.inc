/* ============================================================================
    WISE-CITY-RP  |  نظام العصابات والمناطق
    الوحدة: wc_gangs.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

#define DIALOG_GANG_INFO         3101
#define DIALOG_GANG_MEMBERS      3102
#define DIALOG_HQ_MENU           3103
#define DIALOG_ATTACK_TURF       3104
#define DIALOG_VAULT             3105

/* ============================================================================
 *  WISE-CITY-RP :: Gangs, Luxury HQ & Turf War System
 *  Platform : open.mp / SA-MP
 *  Database : wise_city_db (MySQL R41-4)
 *  Author   : WISE-CITY Development Team
 * ============================================================================ */


// ---------------------------------------------------------------------------
//  MySQL
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Core constants
// ---------------------------------------------------------------------------
#define MAX_GANGS           40
#define MAX_TURFS           60
#define MAX_GANG_MEMBERS    30
#define MAX_SMOKE_NADES     5        // max smoke nades player can hold
#define SMOKE_BLIND_SECONDS 5        // قنبلة دخانية – 5 ثوانٍ عمى
#define SMOKE_RADIUS        12.0     // meters
#define WAR_DURATION        600      // 10 minutes
#define WAR_SCORE_INTERVAL  1000     // score every 1s
#define TURF_PLAYTIME_TICK  60000    // 1 minute
#define TURF_PLAYTIME_GOAL  3600     // 60 minutes → 1 free smoke nade

// Dialog IDs

// ---------------------------------------------------------------------------
//  Enums
// ---------------------------------------------------------------------------
enum e_Gang {
    gID,
    gName[32],
    gTag[8],
    gColor,
    gLeader,
    gFunds,
    gLevel,
    gWins,
    gLosses
};

enum e_GangMember {
    gmGangID,
    gmRank           // 1 member, 2 officer, 3 leader
};

enum e_GangHQ {
    hqGangID,
    hqInterior,
    Float:hqExtX,
    Float:hqExtY,
    Float:hqExtZ,
    Float:hqIntX,
    Float:hqIntY,
    Float:hqIntZ,
    hqSafeMoney,
    hqVaultWeapons,
    hqLuxuryLevel
};

enum e_Turf {
    tID,
    tName[48],
    Float:tCX,
    Float:tCY,
    Float:tCZ,
    Float:tRadius,
    tOwnerGang,
    tLocked,
    tZone,           // gangzone handle
    tWarActive,
    tAttackerGang,
    tDefenderGang,
    tAttackerScore,
    tDefenderScore,
    tWarEndTime
};

enum e_PlayerGang {
    pgGangID,
    pgRank,
    pgSmokeNades,
    pgTurfTime,       // accumulated seconds
    bool:pgBlinded
};

// ---------------------------------------------------------------------------
//  Globals
// ---------------------------------------------------------------------------
new GangInfo[MAX_GANGS][e_Gang];
new g_GangCount = 0;

new GangHQ[MAX_GANGS][e_GangHQ];

new TurfInfo[MAX_TURFS][e_Turf];
new g_TurfCount = 0;

new PlayerGang[MAX_PLAYERS][e_PlayerGang];
new Text:g_BlindTD;

// Player DB id cache (resolved from players table)

// ---------------------------------------------------------------------------
//  Forwards
// ---------------------------------------------------------------------------
forward LoadGangsAndTurfs();
forward SaveGang(gangid);
forward SaveTurf(turfid);
forward SavePlayerGang(playerid);
forward TurfWarScoreTimer();
forward TurfPlaytimeTimer();
forward UnblindPlayer(playerid);

// ===========================================================================
//  GAMEMODE INIT
// ===========================================================================
stock Gangs_OnGameModeInit()
{

    print("[WISE-CITY-RP] Connected to wise_city_db.");

    // Fullscreen black TextDraw for smoke-blind effect
    g_BlindTD = TextDrawCreate(0.0, 0.0, " ");
    TextDrawTextSize(g_BlindTD, 640.0, 480.0);
    TextDrawFont(g_BlindTD, 0);
    TextDrawColor(g_BlindTD, 0x000000FF);
    TextDrawBackgroundColor(g_BlindTD, 0x000000FF);
    TextDrawBoxColor(g_BlindTD, 0x000000FF);
    TextDrawUseBox(g_BlindTD, 1);
    TextDrawSetOutline(g_BlindTD, 0);

    LoadGangsAndTurfs();

    // Timers (high performance: global tick, no per-player timers)
    SetTimer("TurfWarScoreTimer",  WAR_SCORE_INTERVAL, true);
    SetTimer("TurfPlaytimeTimer", TURF_PLAYTIME_TICK, true);
    return 1;
}

stock Gangs_OnGameModeExit()
{
    for (new i = 0; i < g_GangCount; i++) SaveGang(i);
    for (new i = 0; i < g_TurfCount; i++) SaveTurf(i);
    return 1;
}

// ===========================================================================
//  MYSQL LOADING
// ===========================================================================
public LoadGangsAndTurfs()
{
    // ---------- GANGS ----------
    new Cache:res = mysql_query(g_SQL,
        "SELECT id, name, tag, color, leader_id, funds, level, wins, losses FROM gangs");
    g_GangCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        if (g_GangCount >= MAX_GANGS) break;
        new idx = g_GangCount;
        GangInfo[idx][gID] = cache_get_field_content_int(row, "id", g_SQL);
        cache_get_field_content(row, "name", GangInfo[idx][gName], g_SQL, 32);
        cache_get_field_content(row, "tag",  GangInfo[idx][gTag],  g_SQL, 8);
        GangInfo[idx][gColor]  = cache_get_field_content_int(row, "color", g_SQL);
        GangInfo[idx][gLeader] = cache_get_field_content_int(row, "leader_id", g_SQL);
        GangInfo[idx][gFunds]  = cache_get_field_content_int(row, "funds", g_SQL);
        GangInfo[idx][gLevel]  = cache_get_field_content_int(row, "level", g_SQL);
        GangInfo[idx][gWins]   = cache_get_field_content_int(row, "wins", g_SQL);
        GangInfo[idx][gLosses] = cache_get_field_content_int(row, "losses", g_SQL);
        g_GangCount++;
    }
    cache_delete(res);

    // ---------- HQs ----------
    res = mysql_query(g_SQL,
        "SELECT gang_id, interior_id, ext_x, ext_y, ext_z, int_x, int_y, int_z, \
         safe_money, vault_weapons, luxury_level FROM gang_hqs");
    for (new row = 0; row < cache_num_rows(); row++) {
        new gid = cache_get_field_content_int(row, "gang_id", g_SQL);
        new idx = GetGangIndexByID(gid);
        if (idx == -1) continue;
        GangHQ[idx][hqGangID]      = gid;
        GangHQ[idx][hqInterior]    = cache_get_field_content_int(row, "interior_id", g_SQL);
        GangHQ[idx][hqExtX]        = cache_get_field_content_float(row, "ext_x", g_SQL);
        GangHQ[idx][hqExtY]        = cache_get_field_content_float(row, "ext_y", g_SQL);
        GangHQ[idx][hqExtZ]        = cache_get_field_content_float(row, "ext_z", g_SQL);
        GangHQ[idx][hqIntX]        = cache_get_field_content_float(row, "int_x", g_SQL);
        GangHQ[idx][hqIntY]        = cache_get_field_content_float(row, "int_y", g_SQL);
        GangHQ[idx][hqIntZ]        = cache_get_field_content_float(row, "int_z", g_SQL);
        GangHQ[idx][hqSafeMoney]   = cache_get_field_content_int(row, "safe_money", g_SQL);
        GangHQ[idx][hqVaultWeapons]= cache_get_field_content_int(row, "vault_weapons", g_SQL);
        GangHQ[idx][hqLuxuryLevel] = cache_get_field_content_int(row, "luxury_level", g_SQL);
    }
    cache_delete(res);

    // ---------- TURFS ----------
    res = mysql_query(g_SQL,
        "SELECT id, name, cx, cy, cz, radius, owner_gang, locked FROM turfs");
    g_TurfCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        if (g_TurfCount >= MAX_TURFS) break;
        new idx = g_TurfCount;
        TurfInfo[idx][tID]        = cache_get_field_content_int(row, "id", g_SQL);
        cache_get_field_content(row, "name", TurfInfo[idx][tName], g_SQL, 48);
        TurfInfo[idx][tCX]        = cache_get_field_content_float(row, "cx", g_SQL);
        TurfInfo[idx][tCY]        = cache_get_field_content_float(row, "cy", g_SQL);
        TurfInfo[idx][tCZ]        = cache_get_field_content_float(row, "cz", g_SQL);
        TurfInfo[idx][tRadius]    = cache_get_field_content_float(row, "radius", g_SQL);
        TurfInfo[idx][tOwnerGang] = cache_get_field_content_int(row, "owner_gang", g_SQL);
        TurfInfo[idx][tLocked]    = cache_get_field_content_int(row, "locked", g_SQL);

        // GangZone
        TurfInfo[idx][tZone] = GangZoneCreate(
            TurfInfo[idx][tCX] - TurfInfo[idx][tRadius], TurfInfo[idx][tCY] - TurfInfo[idx][tRadius],
            TurfInfo[idx][tCX] + TurfInfo[idx][tRadius], TurfInfo[idx][tCY] + TurfInfo[idx][tRadius]);
        ApplyTurfColor(idx);
        g_TurfCount++;
    }
    cache_delete(res);
    printf("[WISE-CITY-RP] Loaded %d gangs, %d turfs.", g_GangCount, g_TurfCount);
    return 1;
}

// ===========================================================================
//  SAVE HELPERS
// ===========================================================================
public SaveGang(gangid)
{
    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE gangs SET name='%e', tag='%e', color=%d, leader_id=%d, funds=%d, \
         level=%d, wins=%d, losses=%d WHERE id=%d",
        GangInfo[gangid][gName], GangInfo[gangid][gTag], GangInfo[gangid][gColor],
        GangInfo[gangid][gLeader], GangInfo[gangid][gFunds], GangInfo[gangid][gLevel],
        GangInfo[gangid][gWins], GangInfo[gangid][gLosses], GangInfo[gangid][gID]);
    WC_Exec(q);
    return 1;
}

public SaveTurf(turfid)
{
    new q[256];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE turfs SET owner_gang=%d, locked=%d WHERE id=%d",
        TurfInfo[turfid][tOwnerGang], TurfInfo[turfid][tLocked], TurfInfo[turfid][tID]);
    WC_Exec(q);
    return 1;
}

public SavePlayerGang(playerid)
{
    if (g_PlayerDBID[playerid] == 0) return 0;
    new q[256];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE smoke_grenades SET count=%d, turf_playtime=%d WHERE player_id=%d",
        PlayerGang[playerid][pgSmokeNades],
        PlayerGang[playerid][pgTurfTime],
        g_PlayerDBID[playerid]);
    WC_Exec(q);
    return 1;
}

// ===========================================================================
//  UTILITY
// ===========================================================================
stock GetGangIndexByID(gid)
{
    for (new i = 0; i < g_GangCount; i++)
        if (GangInfo[i][gID] == gid) return i;
    return -1;
}

stock GetPlayerGangIndex(playerid)
{
    if (PlayerGang[playerid][pgGangID] == 0) return -1;
    return GetGangIndexByID(PlayerGang[playerid][pgGangID]);
}

stock ApplyTurfColor(turfid)
{
    new color = 0x80808060; // neutral grey
    new gidx = GetGangIndexByID(TurfInfo[turfid][tOwnerGang]);
    if (gidx != -1) color = GangInfo[gidx][gColor] | 0x60;
    GangZoneShowForAll(TurfInfo[turfid][tZone], color);
    return 1;
}

stock IsPlayerInTurf(playerid, turfid)
{
    return IsPlayerInRangeOfPoint(playerid, TurfInfo[turfid][tRadius],
        TurfInfo[turfid][tCX], TurfInfo[turfid][tCY], TurfInfo[turfid][tCZ]);
}

stock GetPlayerTurf(playerid)
{
    for (new i = 0; i < g_TurfCount; i++)
        if (IsPlayerInTurf(playerid, i)) return i;
    return -1;
}

stock NotifyGang(gangid, const msg[])
{
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (!IsPlayerConnected(p)) continue;
        if (PlayerGang[p][pgGangID] == gangid)
            SendClientMessage(p, 0xFFAA00AA, msg);
    }
    return 1;
}

stock CountGangInTurf(gangid, turfid)
{
    new c = 0;
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (!IsPlayerConnected(p)) continue;
        if (PlayerGang[p][pgGangID] != gangid) continue;
        if (IsPlayerInTurf(p, turfid)) c++;
    }
    return c;
}

// ===========================================================================
//  PLAYER CONNECT / DISCONNECT
// ===========================================================================
stock Gangs_OnPlayerConnect(playerid)
{
    PlayerGang[playerid][pgGangID]     = 0;
    PlayerGang[playerid][pgRank]       = 0;
    PlayerGang[playerid][pgSmokeNades] = 0;
    PlayerGang[playerid][pgTurfTime]   = 0;
    PlayerGang[playerid][pgBlinded]    = false;
    g_PlayerDBID[playerid] = 0;
    return 1;
}

stock Gangs_OnPlayerDisconnect(playerid, reason)
{
    SavePlayerGang(playerid);
    if (PlayerGang[playerid][pgBlinded]) {
        TextDrawHideForPlayer(playerid, g_BlindTD);
        SetPlayerDrunkLevel(playerid, 0);
    }
    PlayerGang[playerid][pgGangID] = 0;
    return 1;
}

/**
 * Call after successful login. Loads gang + smoke inventory.
 */
stock LoadPlayerGangData(playerid)
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

    // Gang membership
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT gang_id, rank FROM gang_members WHERE player_id=%d LIMIT 1",
        g_PlayerDBID[playerid]);
    res = mysql_query(g_SQL, q);
    if (cache_num_rows() > 0) {
        PlayerGang[playerid][pgGangID] = cache_get_field_content_int(0, "gang_id", g_SQL);
        PlayerGang[playerid][pgRank]   = cache_get_field_content_int(0, "rank", g_SQL);
    }
    cache_delete(res);

    // Smoke inventory
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT count, turf_playtime FROM smoke_grenades WHERE player_id=%d LIMIT 1",
        g_PlayerDBID[playerid]);
    res = mysql_query(g_SQL, q);
    if (cache_num_rows() > 0) {
        PlayerGang[playerid][pgSmokeNades] = cache_get_field_content_int(0, "count", g_SQL);
        PlayerGang[playerid][pgTurfTime]   = cache_get_field_content_int(0, "turf_playtime", g_SQL);
    } else {
        mysql_format(g_SQL, q, sizeof(q),
            "INSERT INTO smoke_grenades (player_id) VALUES (%d)", g_PlayerDBID[playerid]);
        WC_Exec(q);
    }
    cache_delete(res);

    // Send a welcome reminder
    if (PlayerGang[playerid][pgGangID] != 0) {
        SendClientMessage(playerid, 0xFFD700AA,
            "[عصابة] تم استرجاع بيانات عصابتك ورصيد القنابل الدخانية.");
    }
    return 1;
}

// ===========================================================================
//  GANG COMMANDS
// ===========================================================================
CMD:gang(playerid, params[])
{
    new gidx = GetPlayerGangIndex(playerid);
    if (gidx == -1) {
        SendClientMessage(playerid, 0xFF0000AA, "[عصابة] أنت لست عضواً في أي عصابة.");
        return 1;
    }
    new str[512];
    format(str, sizeof(str),
        "معلومات العصابة\n\
         -------------------------\n\
         الاسم: %s [%s]\n\
         المستوى: %d\n\
         القائد: ID %d\n\
         الأموال: %d دينار\n\
         الانتصارات: %d | الخسائر: %d\n\
         رتبتك: %d",
        GangInfo[gidx][gName], GangInfo[gidx][gTag],
        GangInfo[gidx][gLevel], GangInfo[gidx][gLeader],
        GangInfo[gidx][gFunds], GangInfo[gidx][gWins], GangInfo[gidx][gLosses],
        PlayerGang[playerid][pgRank]);
    ShowPlayerDialog(playerid, DIALOG_GANG_INFO, DIALOG_STYLE_MSGBOX,
        "عصابة WISE-CITY", str, "حسناً", "");
    return 1;
}

CMD:gangleave(playerid, params[])
{
    if (PlayerGang[playerid][pgGangID] == 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[عصابة] أنت لست عضواً في أي عصابة.");
        return 1;
    }
    new q[200];
    mysql_format(g_SQL, q, sizeof(q),
        "DELETE FROM gang_members WHERE player_id=%d", g_PlayerDBID[playerid]);
    WC_Exec(q);
    PlayerGang[playerid][pgGangID] = 0;
    PlayerGang[playerid][pgRank]   = 0;
    SendClientMessage(playerid, 0xFFAA00AA, "[عصابة] لقد غادرت العصابة.");
    return 1;
}

// ===========================================================================
//  LUXURY HQ
// ===========================================================================
CMD:gethq(playerid, params[])
{
    new gidx = GetPlayerGangIndex(playerid);
    if (gidx == -1) { SendClientMessage(playerid, 0xFF0000AA, "[عصابة] لا عصابة."); return 1; }
    if (GangHQ[gidx][hqExtX] == 0.0) {
        SendClientMessage(playerid, 0xFF0000AA, "[مقر] لا يوجد مقر لعصابتك بعد.");
        return 1;
    }
    SetPlayerInterior(playerid, 0);
    SetPlayerPos(playerid, GangHQ[gidx][hqExtX], GangHQ[gidx][hqExtY], GangHQ[gidx][hqExtZ]);
    SendClientMessage(playerid, 0xFFD700AA, "[مقر] تم نقلك إلى المقر الفخم لعصابتك.");
    return 1;
}

CMD:hq(playerid, params[])
{
    new gidx = GetPlayerGangIndex(playerid);
    if (gidx == -1) { SendClientMessage(playerid, 0xFF0000AA, "[مقر] لا عصابة."); return 1; }

    if (!IsPlayerInRangeOfPoint(playerid, 5.0,
        GangHQ[gidx][hqExtX], GangHQ[gidx][hqExtY], GangHQ[gidx][hqExtZ])) {
        SendClientMessage(playerid, 0xFF0000AA, "[مقر] يجب أن تكون عند مدخل المقر.");
        return 1;
    }

    SetPlayerInterior(playerid, GangHQ[gidx][hqInterior]);
    SetPlayerPos(playerid, GangHQ[gidx][hqIntX], GangHQ[gidx][hqIntY], GangHQ[gidx][hqIntZ]);

    new str[256];
    format(str, sizeof(str),
        "المقر الفخم – %s\n\
         -------------------------\n\
         1. الخزنة (الخزنة الحديدية)\n\
         2. مخزن الأسلحة\n\
         3. الخروج من المقر\n\
         4. ترقية الفخامة (المستوى %d)",
        GangInfo[gidx][gName], GangHQ[gidx][hqLuxuryLevel]);
    ShowPlayerDialog(playerid, DIALOG_HQ_MENU, DIALOG_STYLE_LIST,
        "المقر الفخم", str, "اختيار", "إغلاق");
    return 1;
}

CMD:hqexit(playerid, params[])
{
    new gidx = GetPlayerGangIndex(playerid);
    if (gidx == -1) return 1;
    SetPlayerInterior(playerid, 0);
    SetPlayerPos(playerid, GangHQ[gidx][hqExtX], GangHQ[gidx][hqExtY] + 1.0, GangHQ[gidx][hqExtZ]);
    SendClientMessage(playerid, 0xFFAA00AA, "[مقر] لقد خرجت من المقر.");
    return 1;
}

// ===========================================================================
//  TURF WAR
// ===========================================================================
CMD:turfs(playerid, params[])
{
    new str[2048], line[128];
    format(str, sizeof(str), "المناطق ومالكيها\n------------------------\n");
    for (new i = 0; i < g_TurfCount; i++) {
        new ownerName[32] = "محايدة";
        new gidx = GetGangIndexByID(TurfInfo[i][tOwnerGang]);
        if (gidx != -1) format(ownerName, sizeof(ownerName), GangInfo[gidx][gName]);
        format(line, sizeof(line), "%d. %s | المالك: %s | الحالة: %s\n",
            i, TurfInfo[i][tName], ownerName,
            TurfInfo[i][tWarActive] ? "{FF0000}حرب" : "{00FF00}سلمية");
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, DIALOG_ATTACK_TURF, DIALOG_STYLE_LIST,
        "المناطق", str, "مهاجمة", "إغلاق");
    return 1;
}

CMD:attack(playerid, params[])
{
    if (PlayerGang[playerid][pgGangID] == 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[حرب] يجب أن تكون في عصابة.");
        return 1;
    }
    if (PlayerGang[playerid][pgRank] < 2) {
        SendClientMessage(playerid, 0xFF0000AA, "[حرب] فقط القائد أو الضابط يمكنه شن الهجوم.");
        return 1;
    }

    new turfid;
    if (sscanf(params, "d", turfid)) {
        SendClientMessage(playerid, 0xFFAA00AA, "استخدم: /attack [رقم المنطقة]");
        return 1;
    }
    if (turfid < 0 || turfid >= g_TurfCount) {
        SendClientMessage(playerid, 0xFF0000AA, "[حرب] منطقة غير صحيحة.");
        return 1;
    }
    if (TurfInfo[turfid][tWarActive]) {
        SendClientMessage(playerid, 0xFF0000AA, "[حرب] هناك حرب جارية على هذه المنطقة.");
        return 1;
    }
    if (TurfInfo[turfid][tOwnerGang] == PlayerGang[playerid][pgGangID]) {
        SendClientMessage(playerid, 0xFF0000AA, "[حرب] لا يمكنك مهاجمة منطقتك.");
        return 1;
    }

    StartTurfWar(turfid, PlayerGang[playerid][pgGangID]);
    return 1;
}

stock StartTurfWar(turfid, attackerGang)
{
    new defenderGang = TurfInfo[turfid][tOwnerGang];

    TurfInfo[turfid][tWarActive]     = 1;
    TurfInfo[turfid][tAttackerGang]  = attackerGang;
    TurfInfo[turfid][tDefenderGang]  = defenderGang;
    TurfInfo[turfid][tAttackerScore] = 0;
    TurfInfo[turfid][tDefenderScore] = 0;
    TurfInfo[turfid][tWarEndTime]    = gettime() + WAR_DURATION;

    // Persist war
    new q[300];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO turf_wars (turf_id, attacker_gang, defender_gang, start_time, end_time) \
         VALUES (%d, %d, %d, %d, %d)",
        TurfInfo[turfid][tID], attackerGang, defenderGang,
        gettime(), TurfInfo[turfid][tWarEndTime]);
    WC_Exec(q);

    new msg[160];
    format(msg, sizeof(msg),
        "[حرب] بدأت حرب على المنطقة '%s'! المدة: 10 دقائق.", TurfInfo[turfid][tName]);
    SendClientMessageToAll(0xFF0000AA, msg);

    NotifyGang(attackerGang, "[حرب] أنتم المهاجمون! توجهوا للمنطقة واقتلوا الأعداء.");
    if (defenderGang != 0)
        NotifyGang(defenderGang, "[حرب] أنتم المدافعون! دافعوا عن منطقتكم.");
    return 1;
}

public TurfWarScoreTimer()
{
    new now = gettime();
    for (new i = 0; i < g_TurfCount; i++) {
        if (!TurfInfo[i][tWarActive]) continue;

        // Score based on presence in zone
        new aCount = CountGangInTurf(TurfInfo[i][tAttackerGang], i);
        new dCount = CountGangInTurf(TurfInfo[i][tDefenderGang], i);

        if (aCount > dCount) TurfInfo[i][tAttackerScore] += (aCount - dCount);
        else if (dCount > aCount) TurfInfo[i][tDefenderScore] += (dCount - aCount);

        // End war?
        if (now >= TurfInfo[i][tWarEndTime]) {
            EndTurfWar(i);
        }
    }
    return 1;
}

stock EndTurfWar(turfid)
{
    new aGang = TurfInfo[turfid][tAttackerGang];
    new dGang = TurfInfo[turfid][tDefenderGang];
    new winner = 0;

    if (TurfInfo[turfid][tAttackerScore] > TurfInfo[turfid][tDefenderScore]) winner = aGang;
    else if (TurfInfo[turfid][tDefenderScore] > TurfInfo[turfid][tAttackerScore]) winner = dGang;

    // Apply ownership
    if (winner != 0) {
        TurfInfo[turfid][tOwnerGang] = winner;
        new wIdx = GetGangIndexByID(winner);
        if (wIdx != -1) {
            GangInfo[wIdx][gWins]++;
            GangInfo[wIdx][gFunds] += 50000;
            SaveGang(wIdx);
        }
        new lIdx = GetGangIndexByID(winner == aGang ? dGang : aGang);
        if (lIdx != -1) { GangInfo[lIdx][gLosses]++; SaveGang(lIdx); }
    }

    // Log
    new q[300];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO turf_war_log (turf_id, attacker, defender, winner, score_a, score_d) \
         VALUES (%d, %d, %d, %d, %d, %d)",
        TurfInfo[turfid][tID], aGang, dGang, winner,
        TurfInfo[turfid][tAttackerScore], TurfInfo[turfid][tDefenderScore]);
    WC_Exec(q);

    // Reset
    TurfInfo[turfid][tWarActive] = 0;
    ApplyTurfColor(turfid);
    SaveTurf(turfid);

    new msg[160];
    if (winner == 0) format(msg, sizeof(msg), "[حرب] انتهت الحرب على '%s' بالتعادل.", TurfInfo[turfid][tName]);
    else {
        new wIdx = GetGangIndexByID(winner);
        new gangName[64];
        if (wIdx != -1) format(gangName, sizeof(gangName), "%s", GangInfo[wIdx][gName]);
        else format(gangName, sizeof(gangName), "?");
        format(msg, sizeof(msg), "[حرب] فازت عصابة %s بالمنطقة '%s'!",
            gangName, TurfInfo[turfid][tName]);
    }
    SendClientMessageToAll(0xFFD700AA, msg);
    return 1;
}

// ===========================================================================
//  SMOKE GRENADE SYSTEM
// ===========================================================================
CMD:smoke(playerid, params[])
{
    if (PlayerGang[playerid][pgGangID] == 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[قنبلة] يجب أن تكون في عصابة.");
        return 1;
    }
    if (PlayerGang[playerid][pgSmokeNades] <= 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[قنبلة] لا تملك قنابل دخانية.");
        return 1;
    }

    // Rule: only usable inside an active turf war
    new turfid = GetPlayerTurf(playerid);
    if (turfid == -1 || !TurfInfo[turfid][tWarActive]) {
        SendClientMessage(playerid, 0xFF0000AA,
            "[قنبلة] يمكن استخدام القنابل الدخانية داخل حرب المناطق فقط!");
        return 1;
    }

    // Throw
    PlayerGang[playerid][pgSmokeNades]--;
    SavePlayerGang(playerid);

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);
    new Float:fx = x + 1.5 * floatsin(-GetPlayerFacingAngle(playerid), degrees);
    new Float:fy = y + 1.5 * floatcos(-GetPlayerFacingAngle(playerid), degrees);

    // Visual smoke (no damage — explosion type 4)
    CreateExplosion(fx, fy, z, 4, 5.0);

    // Blind every member of the opposing gangs
    new throwerGang = PlayerGang[playerid][pgGangID];
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (!IsPlayerConnected(p) || p == playerid) continue;
        if (PlayerGang[p][pgGangID] == throwerGang) continue; // allies are safe
        if (!IsPlayerInRangeOfPoint(p, SMOKE_RADIUS, fx, fy, z)) continue;

        BlindPlayer(p);
    }

    new msg[128];
    format(msg, sizeof(msg), "[قنبلة] ألقيت قنبلة دخانية! المتبقي: %d",
        PlayerGang[playerid][pgSmokeNades]);
    SendClientMessage(playerid, 0xFFAA00AA, msg);
    return 1;
}

stock BlindPlayer(playerid)
{
    if (PlayerGang[playerid][pgBlinded]) return 1;
    PlayerGang[playerid][pgBlinded] = true;

    TextDrawShowForPlayer(playerid, g_BlindTD);
    SetPlayerDrunkLevel(playerid, 50000);
    SendClientMessage(playerid, 0xFF0000AA, "[قنبلة] لقد أصبت بالعمى لمدة 5 ثوانٍ!");
    SetTimerEx("UnblindPlayer", SMOKE_BLIND_SECONDS * 1000, false, "i", playerid);
    return 1;
}

public UnblindPlayer(playerid)
{
    if (!IsPlayerConnected(playerid)) return 0;
    PlayerGang[playerid][pgBlinded] = false;
    TextDrawHideForPlayer(playerid, g_BlindTD);
    SetPlayerDrunkLevel(playerid, 0);
    SendClientMessage(playerid, 0x00FF00AA, "[قنبلة] استعدت رؤيتك.");
    return 1;
}

CMD:smokes(playerid, params[])
{
    new str[128];
    format(str, sizeof(str),
        "[قنبلة] رصيد القنابل الدخانية: %d / %d | وقت اللعب في المناطق: %d دقيقة",
        PlayerGang[playerid][pgSmokeNades], MAX_SMOKE_NADES,
        PlayerGang[playerid][pgTurfTime] / 60);
    SendClientMessage(playerid, 0xFFD700AA, str);
    return 1;
}

// ===========================================================================
//  TURF PLAYTIME REWARD
// ===========================================================================
public TurfPlaytimeTimer()
{
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (!IsPlayerConnected(p)) continue;
        if (PlayerGang[p][pgGangID] == 0) continue;

        new turfid = GetPlayerTurf(p);
        if (turfid == -1) continue;

        // Player is inside a turf – accumulate
        PlayerGang[p][pgTurfTime] += TURF_PLAYTIME_TICK / 1000; // +60s

        if (PlayerGang[p][pgTurfTime] >= TURF_PLAYTIME_GOAL) {
            PlayerGang[p][pgTurfTime] = 0;
            if (PlayerGang[p][pgSmokeNades] < MAX_SMOKE_NADES)
                PlayerGang[p][pgSmokeNades]++;

            SendClientMessage(p, 0x00FF00AA,
                "[مكافأة] أمضيت ساعة كاملة داخل المناطق! حصلت على قنبلة دخانية مجانية.");
            SavePlayerGang(p);
        }
    }
    return 1;
}

// ===========================================================================
//  HQ MENU HANDLER
// ===========================================================================
stock Gangs_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (!response) return 1;

    if (dialogid == DIALOG_HQ_MENU) {
        new gidx = GetPlayerGangIndex(playerid);
        if (gidx == -1) return 1;

        switch (listitem) {
            case 0: { // Safe vault
                new str[256];
                format(str, sizeof(str),
                    "الخزنة الحديدية\n------------------------\n\
                     الأموال: %d دينار\n\
                     الأسلحة: %d",
                    GangHQ[gidx][hqSafeMoney], GangHQ[gidx][hqVaultWeapons]);
                ShowPlayerDialog(playerid, DIALOG_VAULT, DIALOG_STYLE_MSGBOX,
                    "الخزنة", str, "حسناً", "");
            }
            case 1: { // Weapon vault
                SendClientMessage(playerid, 0xFFAA00AA,
                    "[مقر] مخزن الأسلحة يحتوي على أسلحة العصابة.");
            }
            case 2: pc_cmd_hqexit(playerid, ""); // exit
            case 3: { // Upgrade luxury level
                if (GangHQ[gidx][hqLuxuryLevel] >= 5) {
                    SendClientMessage(playerid, 0xFFAA00AA, "[مقر] المقر بأعلى مستوى فخامة بالفعل.");
                    return 1;
                }
                new cost = GangHQ[gidx][hqLuxuryLevel] * 100000;
                if (GangInfo[gidx][gFunds] < cost) {
                    SendClientMessage(playerid, 0xFF0000AA, "[مقر] أموال العصابة غير كافية للترقية.");
                    return 1;
                }
                GangInfo[gidx][gFunds] -= cost;
                GangHQ[gidx][hqLuxuryLevel]++;
                SaveGang(gidx);
                SendClientMessage(playerid, 0x00FF00AA, "[مقر] تم ترقية فخامة المقر!");
            }
        }
        return 1;
    }

    if (dialogid == DIALOG_ATTACK_TURF) {
        if (listitem < 0 || listitem >= g_TurfCount) return 1;
        new params[8];
        format(params, sizeof(params), "%d", listitem);
        pc_cmd_attack(playerid, params);
        return 1;
    }

    return 0;
}