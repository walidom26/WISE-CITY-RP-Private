/* ============================================================================
    WISE-CITY-RP  |  نظام الفصائل والحرس الجمهوري والسجن
    الوحدة: wc_factions.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

#define DIALOG_ELEVATOR          9100

/*
==============================================================================
    WISE-CITY-RP  |  نظام الفصائل الرئيسي (Factions System)
    الفصيل الأساسي: الحرس الجمهوري (Republican Guard)
    النوع: Filterscript لـ SA-MP 0.3.7 (يمكن تحويله إلى Gamemode بسهولة)

    المتطلبات (Includes):
        - a_samp
        - zcmd
        - sscanf2 (plugin)

    التثبيت:
        1) ضع الملف في /filterscripts وقم بالتجميع (Compile) بواسطة pawncc.
        2) أنشئ المجلد:  scriptfiles/WiseCityFactions
        3) أضف  filterscripts Wise_City_Factions  إلى server.cfg

    ملاحظة: الرسائل داخل اللعبة بالإنجليزية لتجنب مشاكل الترميز، والتعليقات
    بالعربية. يمكنك ترجمة الرسائل إذا كان سيرفرك يدعم ترميز cp1256.
    الإحداثيات (HQ / السجن / المراب) مبنية على إنتيريورات اللعبة الأصلية
    وهي قابلة للتعديل من قسم "الإعدادات" أدناه.
==============================================================================
*/


// ============================================================================
//  الإعدادات العامة
// ============================================================================
#define MAX_RANK                10          // رتبة 10 = القيادة العليا
#define MAX_BARRIERS            60
#define MAX_BARRIERS_PER_PLAYER 8

#define COLOR_RG_CHAT           0x66CC99FF
#define COLOR_FCHAT             0xFFA64DFF


#define PRESSED(%0) (((newkeys & (%0)) == (%0)) && ((oldkeys & (%0)) != (%0)))

// ---- الفصائل ----
enum
{
    FAC_NONE = 0,
    FAC_RG,         // الحرس الجمهوري
    FAC_POLICE,     // الشرطة
    FAC_ARMY,       // الجيش
    FAC_EMS,        // الإسعاف
    FAC_COUNT
}

// ---- صلاحيات الحرس الجمهوري ----
#define RG_RANK_GARAGE          1       // الطابق 0
#define RG_RANK_OPS             3       // الطابق 1
#define RG_RANK_SUMMIT          10      // الطابق 2 (أو رئيس المدينة)
#define RG_RANK_JAIL            4       // أمر /rgjail
#define RG_RANK_RELEASE         6       // أمر /rgrelease
#define RG_RANK_BARRIER         2       // أوامر الحواجز والتفتيش
#define RG_RANK_ARMORY          3

// ---- السجن ----
#define RG_SABOTAGE_JAIL        120     // دقيقة (عقوبة التخريب التلقائية)
#define SABOTAGE_HITS_LIMIT     3       // عدد الإصابات قبل السجن التلقائي

// ============================================================================
//  بيانات ثابتة
// ============================================================================
new const gFactionNames[FAC_COUNT][24] =
{
    "Civilian",
    "Republican Guard",
    "Police",
    "Army",
    "Medics (EMS)"
};

new const gFactionColors[FAC_COUNT] =
{
    0xFFFFFFFF, 0x2E8B57FF, 0x3366FFFF, 0x808000FF, 0xFF6666FF
};

new const gRankNames[FAC_COUNT][MAX_RANK][24] =
{
    // مدني
    {"","","","","","","","","",""},
    // الحرس الجمهوري
    {"Soldier","Corporal","Sergeant","Staff Sergeant","Lieutenant",
     "Captain","Major","Colonel","Brigadier General","Supreme Commander"},
    // الشرطة
    {"Cadet","Officer","Senior Officer","Corporal","Sergeant",
     "Lieutenant","Captain","Commander","Deputy Chief","Chief of Police"},
    // الجيش
    {"Private","Private First Class","Corporal","Sergeant","Lieutenant",
     "Captain","Major","Colonel","General","Chief of Staff"},
    // الإسعاف
    {"Trainee","Paramedic","Senior Paramedic","Specialist","Supervisor",
     "Doctor","Senior Doctor","Chief Doctor","Deputy Director","Director"}
};

// ---- طوابق مقر الحرس الجمهوري: 0 = مراب, 1 = عمليات, 2 = اجتماعات سيادية ----
new const gFloorNames[3][40] =
{
    "Floor 0 - Garage (Heavy Vehicles)",
    "Floor 1 - Operations, Offices & Armory",
    "Floor 2 - Sovereign Meeting Hall"
};

// موقع الوصول عند كل طابق (x, y, z, angle) - وهو نفسه موقع المصعد
new const Float:gFloorPos[3][4] =
{
    {1568.65, -1690.00,   5.89, 180.0},     // المراب (تحت الأرض)
    { 246.78,    63.90, 1003.64,   0.0},    // العمليات
    { 288.74,   167.60, 1007.17,   0.0}     // القاعة السيادية
};
new const gFloorInt[3]   = {0, 6, 3};
new const gFloorWorld[3] = {0, 5101, 5102};

// المدخل الرئيسي للمقر (خارجي) ومخرج الطابق 1
new const Float:gHQEntrance[4] = {1554.80, -1675.60, 16.19, 90.0};
new const Float:gHQExit[3]     = { 246.40,    62.30, 1003.64};

// مخزن الأسلحة (الطابق 1)
new const Float:gArmoryPos[3]  = { 253.10,    77.00, 1003.64};

// موقع ظهور المركبات في المراب
new const Float:gGarageVehPos[4] = {1585.00, -1690.00, 5.89, 270.0};

// السجن الجمهوري المشدد
new const Float:gJailPos[4]    = { 197.60,   175.40, 1003.02, 0.0};
#define JAIL_INTERIOR   3
#define JAIL_WORLD      5200
new const Float:gReleasePos[4] = {1582.00, -1637.00, 13.55, 0.0};

// مركبات المراب: موديل، أقل رتبة، الاسم
new const gGarageModels[5]    = {470, 427, 433, 432, 548};
new const gGarageMinRank[5]   = {1,   2,   3,   8,   9};
new const gGarageNames[5][20] = {"Patriot","Enforcer","Barracks","Rhino (Tank)","Cargobob"};

// موديلات الحواجز
new const gBarrierModels[3]   = {981, 1422, 1459};
new const gBarrierNames[3][24]= {"Helix Barrier","Concrete Barrier","Road Block"};

// ============================================================================
//  متغيرات اللاعبين
// ============================================================================
new fcFaction[MAX_PLAYERS];
new fcRank[MAX_PLAYERS];
new pJailTime[MAX_PLAYERS];             // بالدقائق
new pPresident[MAX_PLAYERS];            // رئيس المدينة (0/1)
new pSiren[MAX_PLAYERS];
new pSirenVeh[MAX_PLAYERS];
new pSirenObj[MAX_PLAYERS][2];
new pSpawnedVeh[MAX_PLAYERS];
new pSabotageHits[MAX_PLAYERS];
new pArmoryTick[MAX_PLAYERS];

// ---- مركبات الفصائل ----
new gVehFaction[MAX_VEHICLES];
new gVehMinRank[MAX_VEHICLES];

// ---- الحواجز ----
new gBarrierObj[MAX_BARRIERS];
new gBarrierOwner[MAX_BARRIERS];        // playerid (-1 = فارغ)
new Float:gBarrierPos[MAX_BARRIERS][3];

// ============================================================================
//  دوال مساعدة
// ============================================================================
#define IsStaff(%0) IsPlayerAdmin(%0)

forward SirenTick();
forward JailTick();

stock GetRankName(factionid, rank, dest[], len)
{
    if (factionid <= FAC_NONE || factionid >= FAC_COUNT || rank < 1 || rank > MAX_RANK)
    {
        format(dest, len, "None");
        return 0;
    }
    format(dest, len, "%s", gRankNames[factionid][rank - 1]);
    return 1;
}

stock IsSecurityFaction(factionid)
{
    return (factionid >= FAC_RG && factionid <= FAC_EMS);
}

stock SendFactionMessage(factionid, color, const msg[])
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i) && fcFaction[i] == factionid) SendClientMessage(i, color, msg);
    }
    return 1;
}

stock UpdatePlayerColor(playerid)
{
    if (fcFaction[playerid] > FAC_NONE) SetPlayerColor(playerid, gFactionColors[fcFaction[playerid]]);
    else SetPlayerColor(playerid, COLOR_WHITE);
    return 1;
}

stock Fac_ResetPlayerData(playerid)
{
    fcFaction[playerid] = FAC_NONE;
    fcRank[playerid] = 0;
    pJailTime[playerid] = 0;
    pPresident[playerid] = 0;
    pSiren[playerid] = 0;
    pSirenVeh[playerid] = 0;
    pSirenObj[playerid][0] = INVALID_OBJECT_ID;
    pSirenObj[playerid][1] = INVALID_OBJECT_ID;
    pSpawnedVeh[playerid] = 0;
    pSabotageHits[playerid] = 0;
    pArmoryTick[playerid] = 0;
    return 1;
}

// ترقيم الفصائل: users.faction_id (0 مدني،1 شرطة،2 جيش،3 حرس جمهوري،4 إسعاف،5 عصابات)
new const g_AuthToFac[6] = {FAC_NONE, FAC_POLICE, FAC_ARMY, FAC_RG, FAC_EMS, FAC_NONE};
new const g_FacToAuth[FAC_COUNT] = {0, 3, 1, 2, 4};

// ---- الحفظ والتحميل ----
stock Fac_SyncToAuth(playerid)
{
    if (!pData[playerid][pLoggedIn]) return 0;
    if (!(pData[playerid][pFaction] == 5 && fcFaction[playerid] == FAC_NONE))
    {
        pData[playerid][pFaction] = g_FacToAuth[fcFaction[playerid]];
        pData[playerid][pRank] = fcRank[playerid];
    }
    return 1;
}

stock Fac_SavePlayerData(playerid)
{
    if (!pData[playerid][pLoggedIn] || pData[playerid][pID] == 0) return 0;
    new q[200];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO `faction_players` (`player_id`, `jail_minutes`, `is_president`) VALUES (%d, %d, %d) ON DUPLICATE KEY UPDATE `jail_minutes` = %d, `is_president` = %d",
        pData[playerid][pID], pJailTime[playerid], pPresident[playerid] ? 1 : 0,
        pJailTime[playerid], pPresident[playerid] ? 1 : 0);
    mysql_tquery(g_SQL, q);
    return 1;
}

stock Fac_LoadPlayerData(playerid)
{
    // الفصيل والرتبة يأتيان من جدول users عبر pData (مع تحويل الترقيم)
    new af = pData[playerid][pFaction];
    fcFaction[playerid] = (af >= 0 && af < 6) ? g_AuthToFac[af] : FAC_NONE;
    fcRank[playerid] = pData[playerid][pRank];
    if (fcFaction[playerid] == FAC_NONE) fcRank[playerid] = 0;
    else if (fcRank[playerid] < 1) fcRank[playerid] = 1;
    else if (fcRank[playerid] > MAX_RANK) fcRank[playerid] = MAX_RANK;
    // السجن ورئاسة المدينة من جدول faction_players
    new q[160];
    mysql_format(g_SQL, q, sizeof(q), "SELECT `jail_minutes`, `is_president` FROM `faction_players` WHERE `player_id` = %d LIMIT 1", pData[playerid][pID]);
    new Cache:res = mysql_query(g_SQL, q);
    if (cache_num_rows() > 0)
    {
        pJailTime[playerid]  = cache_get_field_content_int(0, "jail_minutes", g_SQL);
        pPresident[playerid] = cache_get_field_content_int(0, "is_president", g_SQL);
    }
    cache_delete(res);
    UpdatePlayerColor(playerid);
    return 1;
}

// ============================================================================
//  نظام المقر والطوابق
// ============================================================================
stock bool:CanAccessFloor(playerid, floor)
{
    if (IsStaff(playerid) || pPresident[playerid]) return true;
    if (fcFaction[playerid] != FAC_RG) return false;
    switch (floor)
    {
        case 0: return (fcRank[playerid] >= RG_RANK_GARAGE);
        case 1: return (fcRank[playerid] >= RG_RANK_OPS);
        case 2: return (fcRank[playerid] >= RG_RANK_SUMMIT);
    }
    return false;
}

// يرجع رقم الطابق الذي يقف اللاعب عند مصعده أو -1
stock GetElevatorFloor(playerid)
{
    for (new i = 0; i < 3; i++)
    {
        if (GetPlayerInterior(playerid) == gFloorInt[i] &&
            GetPlayerVirtualWorld(playerid) == gFloorWorld[i] &&
            IsPlayerInRangeOfPoint(playerid, 4.0, gFloorPos[i][0], gFloorPos[i][1], gFloorPos[i][2]))
            return i;
    }
    return -1;
}

stock TeleportToFloor(playerid, floor)
{
    SetPlayerInterior(playerid, gFloorInt[floor]);
    SetPlayerVirtualWorld(playerid, gFloorWorld[floor]);
    SetPlayerPos(playerid, gFloorPos[floor][0], gFloorPos[floor][1], gFloorPos[floor][2]);
    SetPlayerFacingAngle(playerid, gFloorPos[floor][3]);
    SetCameraBehindPlayer(playerid);
    return 1;
}

stock CreateHQObjects()
{
    // المدخل الرئيسي
    CreatePickup(1318, 1, gHQEntrance[0], gHQEntrance[1], gHQEntrance[2], 0);
    Create3DTextLabel("{2E8B57}Republican Guard HQ\n{FFFFFF}Press {FFD700}F{FFFFFF} to enter", COLOR_WHITE,
        gHQEntrance[0], gHQEntrance[1], gHQEntrance[2] + 0.5, 15.0, 0, 0);

    // المخرج من الطابق 1
    CreatePickup(1318, 1, gHQExit[0], gHQExit[1], gHQExit[2], gFloorWorld[1]);
    Create3DTextLabel("{FFFFFF}Exit - Press {FFD700}F", COLOR_WHITE,
        gHQExit[0], gHQExit[1], gHQExit[2] + 0.5, 10.0, gFloorWorld[1], 0);

    // المصاعد
    for (new i = 0; i < 3; i++)
    {
        new text[80];
        CreatePickup(1239, 1, gFloorPos[i][0], gFloorPos[i][1], gFloorPos[i][2], gFloorWorld[i]);
        format(text, sizeof(text), "{2E8B57}Elevator\n{FFFFFF}Use {FFD700}/lift");
        Create3DTextLabel(text, COLOR_WHITE, gFloorPos[i][0], gFloorPos[i][1], gFloorPos[i][2] + 0.5, 10.0, gFloorWorld[i], 0);
    }

    // مخزن الأسلحة
    CreatePickup(1239, 1, gArmoryPos[0], gArmoryPos[1], gArmoryPos[2], gFloorWorld[1]);
    Create3DTextLabel("{FF4444}Armory\n{FFFFFF}Use {FFD700}/rg_armory", COLOR_WHITE,
        gArmoryPos[0], gArmoryPos[1], gArmoryPos[2] + 0.5, 10.0, gFloorWorld[1], 0);

    // المراب
    Create3DTextLabel("{2E8B57}Heavy Vehicle Garage\n{FFFFFF}Use {FFD700}/rgveh", COLOR_WHITE,
        gGarageVehPos[0], gGarageVehPos[1], gGarageVehPos[2] + 1.0, 20.0, 0, 0);
    return 1;
}

// ============================================================================
//  السجن الجمهوري المشدد
// ============================================================================
stock JailPlayer(playerid, minutes, const reason[], const by[])
{
    if (!IsPlayerConnected(playerid) || minutes < 1) return 0;

    if (pSiren[playerid]) StopSiren(playerid);
    if (IsPlayerInAnyVehicle(playerid)) RemovePlayerFromVehicle(playerid);

    pJailTime[playerid] = minutes;
    ResetPlayerWeapons(playerid);
    SetPlayerHealth(playerid, 100.0);
    SetPlayerInterior(playerid, JAIL_INTERIOR);
    SetPlayerVirtualWorld(playerid, JAIL_WORLD);
    SetPlayerPos(playerid, gJailPos[0], gJailPos[1], gJailPos[2]);
    SetPlayerFacingAngle(playerid, gJailPos[3]);
    Fac_SavePlayerData(playerid);

    new msg[144];
    format(msg, sizeof(msg), "[RG PRISON] You have been jailed for %d minute(s). Reason: %s (by %s)", minutes, reason, by);
    SendClientMessage(playerid, COLOR_RED, msg);
    GameTextForPlayer(playerid, "~r~REPUBLICAN PRISON", 4000, 3);

    format(msg, sizeof(msg), "[RG PRISON] %s was jailed for %d min. Reason: %s", "Prisoner", minutes, reason);
    SendFactionMessage(FAC_RG, COLOR_YELLOW, msg);
    return 1;
}

stock ReleasePlayer(playerid)
{
    pJailTime[playerid] = 0;
    pSabotageHits[playerid] = 0;
    SetPlayerInterior(playerid, 0);
    SetPlayerVirtualWorld(playerid, 0);
    SetPlayerPos(playerid, gReleasePos[0], gReleasePos[1], gReleasePos[2]);
    SetPlayerFacingAngle(playerid, gReleasePos[3]);
    Fac_SavePlayerData(playerid);
    SendClientMessage(playerid, COLOR_GREEN, "[RG PRISON] You have been released. Behave yourself.");
    return 1;
}

public JailTick()
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || pJailTime[i] <= 0) continue;
        pJailTime[i]--;
        if (pJailTime[i] <= 0) ReleasePlayer(i);
        else
        {
            new msg[64];
            format(msg, sizeof(msg), "[RG PRISON] Time remaining: %d minute(s).", pJailTime[i]);
            SendClientMessage(i, COLOR_GREY, msg);
            Fac_SavePlayerData(i);
        }
    }
    return 1;
}

// ============================================================================
//  الفلاشر التكتيكي
// ============================================================================
stock StopSiren(playerid)
{
    for (new k = 0; k < 2; k++)
    {
        if (pSirenObj[playerid][k] != INVALID_OBJECT_ID)
        {
            DestroyObject(pSirenObj[playerid][k]);
            pSirenObj[playerid][k] = INVALID_OBJECT_ID;
        }
    }
    if (pSirenVeh[playerid] != 0)
    {
        new e, l, a, d, b, bo, o;
        GetVehicleParamsEx(pSirenVeh[playerid], e, l, a, d, b, bo, o);
        SetVehicleParamsEx(pSirenVeh[playerid], e, 0, a, d, b, bo, o);
    }
    pSiren[playerid] = 0;
    pSirenVeh[playerid] = 0;
    return 1;
}

stock StartSiren(playerid, vehicleid)
{
    pSiren[playerid] = 1;
    pSirenVeh[playerid] = vehicleid;
    pSirenObj[playerid][0] = CreateObject(18646, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);   // أحمر
    pSirenObj[playerid][1] = CreateObject(18647, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);   // أزرق
    AttachObjectToVehicle(pSirenObj[playerid][0], vehicleid, -0.40, 0.0, 0.95, 0.0, 0.0, 0.0);
    AttachObjectToVehicle(pSirenObj[playerid][1], vehicleid,  0.40, 0.0, 0.95, 0.0, 0.0, 0.0);
    return 1;
}

public SirenTick()
{
    static phase = 0;
    phase = !phase;
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || !pSiren[i]) continue;
        new vid = GetPlayerVehicleID(i);
        if (vid == 0 || vid != pSirenVeh[i] || GetPlayerState(i) != PLAYER_STATE_DRIVER)
        {
            StopSiren(i);
            continue;
        }
        new e, l, a, d, b, bo, o;
        GetVehicleParamsEx(vid, e, l, a, d, b, bo, o);
        SetVehicleParamsEx(vid, e, phase, a, d, b, bo, o);
    }
    return 1;
}

// ============================================================================
//  الحواجز العسكرية
// ============================================================================
stock CountPlayerBarriers(playerid)
{
    new c = 0;
    for (new i = 0; i < MAX_BARRIERS; i++) if (gBarrierOwner[i] == playerid) c++;
    return c;
}

stock RemoveBarrier(idx)
{
    if (idx < 0 || idx >= MAX_BARRIERS || gBarrierOwner[idx] == -1) return 0;
    DestroyObject(gBarrierObj[idx]);
    gBarrierObj[idx] = INVALID_OBJECT_ID;
    gBarrierOwner[idx] = -1;
    return 1;
}

stock ClearPlayerBarriers(playerid)
{
    for (new i = 0; i < MAX_BARRIERS; i++) if (gBarrierOwner[i] == playerid) RemoveBarrier(i);
    return 1;
}

stock IsNearAnyBarrier(playerid, Float:range)
{
    for (new i = 0; i < MAX_BARRIERS; i++)
    {
        if (gBarrierOwner[i] == -1) continue;
        if (IsPlayerInRangeOfPoint(playerid, range, gBarrierPos[i][0], gBarrierPos[i][1], gBarrierPos[i][2])) return 1;
    }
    return 0;
}

// ============================================================================
//  مركبات الفصائل
// ============================================================================
stock DestroyPlayerGarageVehicle(playerid)
{
    new vid = pSpawnedVeh[playerid];
    if (vid != 0)
    {
        gVehFaction[vid] = FAC_NONE;
        gVehMinRank[vid] = 0;
        DestroyVehicle(vid);
        pSpawnedVeh[playerid] = 0;
    }
    return 1;
}

stock CanUseVehicle(playerid, vehicleid)
{
    if (gVehFaction[vehicleid] == FAC_NONE) return 1;
    if (IsStaff(playerid)) return 1;
    if (fcFaction[playerid] != gVehFaction[vehicleid]) return 0;
    if (fcRank[playerid] < gVehMinRank[vehicleid]) return 0;
    return 1;
}

// ============================================================================
//  Callbacks
// ============================================================================
stock Fac_OnGameModeInit()
{
    for (new v = 0; v < MAX_VEHICLES; v++) { gVehFaction[v] = FAC_NONE; gVehMinRank[v] = 0; }
    for (new b = 0; b < MAX_BARRIERS; b++) { gBarrierOwner[b] = -1; gBarrierObj[b] = INVALID_OBJECT_ID; }
    for (new p = 0; p < MAX_PLAYERS; p++) Fac_ResetPlayerData(p);
    CreateHQObjects();
    SetTimer("SirenTick", 250, true);
    SetTimer("JailTick", 60000, true);
    mysql_tquery(g_SQL, "CREATE TABLE IF NOT EXISTS `faction_players` (`player_id` INT NOT NULL, `jail_minutes` INT NOT NULL DEFAULT 0, `is_president` TINYINT(1) NOT NULL DEFAULT 0, PRIMARY KEY (`player_id`)) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
    print("[WISE-CITY] نظام الفصائل جاهز.");
    return 1;
}

stock Fac_OnGameModeExit()
{
    for (new p = 0; p < MAX_PLAYERS; p++)
    {
        if (!IsPlayerConnected(p)) continue;
        if (pSiren[p]) StopSiren(p);
        Fac_SavePlayerData(p);
    }
    for (new b = 0; b < MAX_BARRIERS; b++) RemoveBarrier(b);
    return 1;
}

stock Fac_OnPlayerConnect(playerid)
{
    Fac_ResetPlayerData(playerid);
    return 1;
}

stock Fac_OnPlayerDisconnect(playerid, reason)
{
    if (pSiren[playerid]) StopSiren(playerid);
    ClearPlayerBarriers(playerid);
    DestroyPlayerGarageVehicle(playerid);
    Fac_SyncToAuth(playerid);   // مزامنة قبل الحفظ المركزي
    Fac_SavePlayerData(playerid);
    return 1;
}

stock Fac_OnPlayerSpawn(playerid)
{
    UpdatePlayerColor(playerid);
    if (pJailTime[playerid] > 0)
    {
        SetPlayerInterior(playerid, JAIL_INTERIOR);
        SetPlayerVirtualWorld(playerid, JAIL_WORLD);
        SetPlayerPos(playerid, gJailPos[0], gJailPos[1], gJailPos[2]);
        SetPlayerFacingAngle(playerid, gJailPos[3]);
        new msg[80];
        format(msg, sizeof(msg), "[RG PRISON] You still have %d minute(s) to serve.", pJailTime[playerid]);
        SendClientMessage(playerid, COLOR_RED, msg);
    }
    return 1;
}

// منع استخدام الأوامر أثناء السجن
stock Fac_OnPlayerCommandReceived(playerid, const cmdtext[])
{
    if (pJailTime[playerid] > 0 && !IsStaff(playerid))
    {
        SendClientMessage(playerid, COLOR_RED, "[RG PRISON] You cannot use commands while in prison.");
        return 0;
    }
    return 1;
}

// الدخول/الخروج من المقر بمفتاح F
stock Fac_OnPlayerKeyStateChange(playerid, newkeys, oldkeys)
{
    if (PRESSED(KEY_SECONDARY_ATTACK))
    {
        if (IsPlayerInAnyVehicle(playerid)) return 1;

        // الدخول من المدخل الرئيسي
        if (GetPlayerInterior(playerid) == 0 && GetPlayerVirtualWorld(playerid) == 0 &&
            IsPlayerInRangeOfPoint(playerid, 2.5, gHQEntrance[0], gHQEntrance[1], gHQEntrance[2]))
        {
            if (!CanAccessFloor(playerid, 1))
                return SendClientMessage(playerid, COLOR_RED, "[RG HQ] Access denied. Authorized personnel only.");
            TeleportToFloor(playerid, 1);
            return 1;
        }
        // الخروج من الطابق 1
        if (GetPlayerVirtualWorld(playerid) == gFloorWorld[1] &&
            IsPlayerInRangeOfPoint(playerid, 2.5, gHQExit[0], gHQExit[1], gHQExit[2]))
        {
            SetPlayerInterior(playerid, 0);
            SetPlayerVirtualWorld(playerid, 0);
            SetPlayerPos(playerid, gHQEntrance[0], gHQEntrance[1], gHQEntrance[2]);
            SetPlayerFacingAngle(playerid, gHQEntrance[3]);
            return 1;
        }
    }
    return 1;
}

stock Fac_OnPlayerEnterVehicle(playerid, vehicleid, ispassenger)
{
    if (!CanUseVehicle(playerid, vehicleid))
    {
        ClearAnimations(playerid);
        new msg[96];
        format(msg, sizeof(msg), "[VEHICLE] Restricted to %s (rank %d+).", gFactionNames[gVehFaction[vehicleid]], gVehMinRank[vehicleid]);
        SendClientMessage(playerid, COLOR_RED, msg);
    }
    return 1;
}

stock Fac_OnPlayerStateChange(playerid, newstate, oldstate)
{
    if (newstate == PLAYER_STATE_DRIVER || newstate == PLAYER_STATE_PASSENGER)
    {
        new vid = GetPlayerVehicleID(playerid);
        if (!CanUseVehicle(playerid, vid))
        {
            RemovePlayerFromVehicle(playerid);
            return 1;
        }
    }
    if (oldstate == PLAYER_STATE_DRIVER && pSiren[playerid]) StopSiren(playerid);
    return 1;
}

stock Fac_OnVehicleDeath(vehicleid, killerid)
{
    gVehFaction[vehicleid] = FAC_NONE;
    gVehMinRank[vehicleid] = 0;
    return 1;
}

// التخريب: إطلاق النار على مركبات الحرس من غير الأعضاء = سجن 120 دقيقة تلقائياً
stock Fac_OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, Float:fX, Float:fY, Float:fZ)
{
    if (hittype == BULLET_HIT_TYPE_VEHICLE && hitid > 0 && hitid < MAX_VEHICLES)
    {
        if (gVehFaction[hitid] == FAC_RG && fcFaction[playerid] != FAC_RG &&
            pJailTime[playerid] <= 0 && !IsStaff(playerid))
        {
            pSabotageHits[playerid]++;
            if (pSabotageHits[playerid] >= SABOTAGE_HITS_LIMIT)
            {
                pSabotageHits[playerid] = 0;
                JailPlayer(playerid, RG_SABOTAGE_JAIL, "Sabotage of Republican Guard property", "Auto-System");
            }
            else SendClientMessage(playerid, COLOR_RED, "[WARNING] Damaging Republican Guard vehicles is sabotage! (120 min prison)");
        }
    }
    return 1;
}

stock Fac_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid == DIALOG_ELEVATOR)
    {
        if (!response) return 1;
        if (listitem < 0 || listitem > 2) return 1;
        if (GetElevatorFloor(playerid) == -1)
            return SendClientMessage(playerid, COLOR_RED, "[ELEVATOR] You must stand at an elevator.");
        if (IsPlayerInAnyVehicle(playerid))
            return SendClientMessage(playerid, COLOR_RED, "[ELEVATOR] Leave your vehicle first.");
        if (!CanAccessFloor(playerid, listitem))
        {
            new msg[96];
            format(msg, sizeof(msg), "[ELEVATOR] Access denied to %s.", gFloorNames[listitem]);
            return SendClientMessage(playerid, COLOR_RED, msg);
        }
        TeleportToFloor(playerid, listitem);
        return 1;
    }
    return 0;
}

// ============================================================================
//  الأوامر: المصعد وتصاريح الطوابق
// ============================================================================
CMD:lift(playerid, params[])
{
    if (GetElevatorFloor(playerid) == -1)
        return SendClientMessage(playerid, COLOR_RED, "[ELEVATOR] You are not near an elevator.");
    new list[256];
    for (new i = 0; i < 3; i++)
    {
        new line[80];
        format(line, sizeof(line), "%s%s\n", CanAccessFloor(playerid, i) ? "{33CC66}" : "{FF4444}", gFloorNames[i]);
        strcat(list, line);
    }
    ShowPlayerDialog(playerid, DIALOG_ELEVATOR, DIALOG_STYLE_LIST, "Republican Guard HQ - Elevator", list, "Go", "Cancel");
    return 1;
}

// ---- المراب: مركبات وآليات ثقيلة ----
CMD:rgveh(playerid, params[])
{
    if (fcFaction[playerid] != FAC_RG && !IsStaff(playerid))
        return SendClientMessage(playerid, COLOR_RED, "[RG] Republican Guard only.");
    if (GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != 0 ||
        !IsPlayerInRangeOfPoint(playerid, 40.0, gFloorPos[0][0], gFloorPos[0][1], gFloorPos[0][2]))
        return SendClientMessage(playerid, COLOR_RED, "[RG] You must be inside the garage (Floor 0).");

    new idx;
    if (sscanf(params, "d", idx) || idx < 1 || idx > 5)
    {
        SendClientMessage(playerid, COLOR_YELLOW, "Usage: /rgveh [1-5]");
        for (new i = 0; i < 5; i++)
        {
            new msg[80];
            format(msg, sizeof(msg), "  %d - %s (Rank %d+)", i + 1, gGarageNames[i], gGarageMinRank[i]);
            SendClientMessage(playerid, COLOR_GREY, msg);
        }
        return 1;
    }
    idx--;
    if (fcRank[playerid] < gGarageMinRank[idx] && !IsStaff(playerid))
        return SendClientMessage(playerid, COLOR_RED, "[RG] Your rank is too low for this vehicle.");

    DestroyPlayerGarageVehicle(playerid);
    new vid = CreateVehicle(gGarageModels[idx], gGarageVehPos[0], gGarageVehPos[1], gGarageVehPos[2], gGarageVehPos[3], 86, 86, -1);
    if (vid == INVALID_VEHICLE_ID) return SendClientMessage(playerid, COLOR_RED, "[RG] Could not create vehicle.");

    pSpawnedVeh[playerid] = vid;
    gVehFaction[vid] = FAC_RG;
    gVehMinRank[vid] = gGarageMinRank[idx];
    PutPlayerInVehicle(playerid, vid, 0);

    new msg[80];
    format(msg, sizeof(msg), "[RG] %s deployed from the garage.", gGarageNames[idx]);
    SendClientMessage(playerid, COLOR_GREEN, msg);
    return 1;
}

// ---- مخزن الأسلحة (الطابق 1) ----
CMD:rg_armory(playerid, params[])
{
    if (fcFaction[playerid] != FAC_RG || fcRank[playerid] < RG_RANK_ARMORY)
        return SendClientMessage(playerid, COLOR_RED, "[RG] Armory access: Republican Guard rank 3+.");
    if (GetPlayerVirtualWorld(playerid) != gFloorWorld[1] ||
        !IsPlayerInRangeOfPoint(playerid, 4.0, gArmoryPos[0], gArmoryPos[1], gArmoryPos[2]))
        return SendClientMessage(playerid, COLOR_RED, "[RG] You must be at the armory (Floor 1).");
    if (gettime() < pArmoryTick[playerid])
        return SendClientMessage(playerid, COLOR_RED, "[RG] Armory cooldown active. Try again shortly.");

    pArmoryTick[playerid] = gettime() + 300;
    new rank = fcRank[playerid];
    SetPlayerArmour(playerid, 100.0);
    SetPlayerHealth(playerid, 100.0);
    GivePlayerWeapon(playerid, 24, 70);                 // Deagle
    GivePlayerWeapon(playerid, 29, 200);                // MP5
    if (rank >= 5) GivePlayerWeapon(playerid, 31, 250); // M4
    if (rank >= 7) GivePlayerWeapon(playerid, 34, 40);  // Sniper
    if (rank >= 9) GivePlayerWeapon(playerid, 16, 5);   // قنابل
    SendClientMessage(playerid, COLOR_GREEN, "[RG] Equipment issued according to your rank.");
    return 1;
}

// ============================================================================
//  الأوامر الميدانية للحرس الجمهوري
// ============================================================================
// ---- الفلاشر التكتيكي ----
CMD:rg_siren(playerid, params[])
{
    if (fcFaction[playerid] != FAC_RG)
        return SendClientMessage(playerid, COLOR_RED, "[RG] Republican Guard only.");
    if (GetPlayerState(playerid) != PLAYER_STATE_DRIVER)
        return SendClientMessage(playerid, COLOR_RED, "[RG] You must be driving a vehicle.");

    if (pSiren[playerid])
    {
        StopSiren(playerid);
        return SendClientMessage(playerid, COLOR_YELLOW, "[RG] Tactical flasher: OFF.");
    }
    StartSiren(playerid, GetPlayerVehicleID(playerid));
    SendClientMessage(playerid, COLOR_GREEN, "[RG] Tactical flasher: ON.");
    return 1;
}

// ---- تفتيش المباشر بالحواجز ----
CMD:military_check(playerid, params[])
{
    if (fcFaction[playerid] != FAC_RG || fcRank[playerid] < RG_RANK_BARRIER)
        return SendClientMessage(playerid, COLOR_RED, "[RG] Republican Guard rank 2+ only.");

    new target;
    if (sscanf(params, "u", target))
        return SendClientMessage(playerid, COLOR_YELLOW, "Usage: /military_check [playerid/name]");
    if (!IsPlayerConnected(target) || target == playerid)
        return SendClientMessage(playerid, COLOR_RED, "[RG] Invalid target.");
    if (!IsNearAnyBarrier(playerid, 15.0))
        return SendClientMessage(playerid, COLOR_RED, "[RG] You must be at a deployed checkpoint barrier (/deploy_barrier).");

    new Float:tx, Float:ty, Float:tz;
    GetPlayerPos(target, tx, ty, tz);
    if (!IsPlayerInRangeOfPoint(playerid, 6.0, tx, ty, tz))
        return SendClientMessage(playerid, COLOR_RED, "[RG] The target is too far from you.");

    new tname[MAX_PLAYER_NAME], msg[144];
    GetPlayerName(target, tname, sizeof(tname));
    format(msg, sizeof(msg), "===== CHECKPOINT INSPECTION: %s (ID %d) =====", tname, target);
    SendClientMessage(playerid, COLOR_RG_CHAT, msg);

    new count = 0, flagged = 0;
    for (new slot = 0; slot < 13; slot++)
    {
        new weapon, ammo;
        GetPlayerWeaponData(target, slot, weapon, ammo);
        if (weapon <= 0) continue;
        new wname[32];
        GetWeaponName(weapon, wname, sizeof(wname));
        new bool:illegal = (weapon == 16 || weapon == 18 || weapon == 35 || weapon == 36 || weapon == 38 || weapon == 39);
        if (illegal) flagged++;
        format(msg, sizeof(msg), "  Weapon: %s (%d ammo) %s", wname, ammo, illegal ? "{FF4444}[ILLEGAL]" : "");
        SendClientMessage(playerid, COLOR_WHITE, msg);
        count++;
    }
    if (count == 0) SendClientMessage(playerid, COLOR_WHITE, "  No weapons found.");

    format(msg, sizeof(msg), "  Money: $%d | Faction: %s | Illegal items: %d", GetPlayerMoney(target), gFactionNames[fcFaction[target]], flagged);
    SendClientMessage(playerid, COLOR_WHITE, msg);

    if (IsPlayerInAnyVehicle(target))
    {
        format(msg, sizeof(msg), "  Vehicle model: %d", GetVehicleModel(GetPlayerVehicleID(target)));
        SendClientMessage(playerid, COLOR_WHITE, msg);
    }

    new pname[MAX_PLAYER_NAME];
    GetPlayerName(playerid, pname, sizeof(pname));
    format(msg, sizeof(msg), "[RG CHECKPOINT] You have been searched by Republican Guard officer %s.", pname);
    SendClientMessage(target, COLOR_YELLOW, msg);
    return 1;
}

// ---- وضع حواجز عسكرية ----
CMD:deploy_barrier(playerid, params[])
{
    if (fcFaction[playerid] != FAC_RG || fcRank[playerid] < RG_RANK_BARRIER)
        return SendClientMessage(playerid, COLOR_RED, "[RG] Republican Guard rank 2+ only.");
    if (IsPlayerInAnyVehicle(playerid))
        return SendClientMessage(playerid, COLOR_RED, "[RG] Leave your vehicle first.");

    new type;
    if (sscanf(params, "d", type) || type < 1 || type > 3)
    {
        SendClientMessage(playerid, COLOR_YELLOW, "Usage: /deploy_barrier [1-3]");
        SendClientMessage(playerid, COLOR_GREY, "  1 - Helix Barrier | 2 - Concrete Barrier | 3 - Road Block");
        SendClientMessage(playerid, COLOR_GREY, "Related: /remove_barrier (nearest) | /clear_barriers (all yours)");
        return 1;
    }
    if (CountPlayerBarriers(playerid) >= MAX_BARRIERS_PER_PLAYER)
        return SendClientMessage(playerid, COLOR_RED, "[RG] You reached your barrier limit. Use /remove_barrier.");

    new slot = -1;
    for (new i = 0; i < MAX_BARRIERS; i++) if (gBarrierOwner[i] == -1) { slot = i; break; }
    if (slot == -1) return SendClientMessage(playerid, COLOR_RED, "[RG] Server barrier limit reached.");

    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);
    x += (2.0 * floatsin(-a, degrees));
    y += (2.0 * floatcos(-a, degrees));

    gBarrierObj[slot] = CreateObject(gBarrierModels[type - 1], x, y, z - 0.9, 0.0, 0.0, a + 90.0);
    gBarrierOwner[slot] = playerid;
    gBarrierPos[slot][0] = x;
    gBarrierPos[slot][1] = y;
    gBarrierPos[slot][2] = z;

    new msg[64];
    format(msg, sizeof(msg), "[RG] %s deployed.", gBarrierNames[type - 1]);
    SendClientMessage(playerid, COLOR_GREEN, msg);
    return 1;
}

CMD:remove_barrier(playerid, params[])
{
    if (fcFaction[playerid] != FAC_RG)
        return SendClientMessage(playerid, COLOR_RED, "[RG] Republican Guard only.");
    new best = -1;
    new Float:bestDist = 8.0;
    for (new i = 0; i < MAX_BARRIERS; i++)
    {
        if (gBarrierOwner[i] == -1) continue;
        new Float:d = GetPlayerDistanceFromPoint(playerid, gBarrierPos[i][0], gBarrierPos[i][1], gBarrierPos[i][2]);
        if (d < bestDist) { bestDist = d; best = i; }
    }
    if (best == -1) return SendClientMessage(playerid, COLOR_RED, "[RG] No barrier nearby.");
    RemoveBarrier(best);
    SendClientMessage(playerid, COLOR_GREEN, "[RG] Barrier removed.");
    return 1;
}

CMD:clear_barriers(playerid, params[])
{
    if (fcFaction[playerid] != FAC_RG)
        return SendClientMessage(playerid, COLOR_RED, "[RG] Republican Guard only.");
    ClearPlayerBarriers(playerid);
    SendClientMessage(playerid, COLOR_GREEN, "[RG] All of your barriers were removed.");
    return 1;
}

// ---- شات الفصيل الداخلي ----
CMD:r(playerid, params[])
{
    if (fcFaction[playerid] == FAC_NONE)
        return SendClientMessage(playerid, COLOR_RED, "[FACTION] You are not in a faction.");
    if (isnull(params))
        return SendClientMessage(playerid, COLOR_YELLOW, "Usage: /r [message]");

    new name[MAX_PLAYER_NAME], rn[24], msg[144];
    GetPlayerName(playerid, name, sizeof(name));
    GetRankName(fcFaction[playerid], fcRank[playerid], rn, sizeof(rn));
    format(msg, sizeof(msg), "** [%s] %s %s: %s", gFactionNames[fcFaction[playerid]], rn, name, params);
    SendFactionMessage(fcFaction[playerid], COLOR_RG_CHAT, msg);
    return 1;
}

// ---- شات التنسيق بين الفصائل الأمنية ----
CMD:f(playerid, params[])
{
    if (!IsSecurityFaction(fcFaction[playerid]))
        return SendClientMessage(playerid, COLOR_RED, "[FACTION] Security factions only.");
    if (isnull(params))
        return SendClientMessage(playerid, COLOR_YELLOW, "Usage: /f [message]");

    new name[MAX_PLAYER_NAME], rn[24], msg[144];
    GetPlayerName(playerid, name, sizeof(name));
    GetRankName(fcFaction[playerid], fcRank[playerid], rn, sizeof(rn));
    format(msg, sizeof(msg), "(( [JOINT] %s | %s %s: %s ))", gFactionNames[fcFaction[playerid]], rn, name, params);
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i) && IsSecurityFaction(fcFaction[i])) SendClientMessage(i, COLOR_FCHAT, msg);
    }
    return 1;
}

// ---- أوامر السجن الجمهوري ----
// /rgjail [id] [minutes | sabotage]
CMD:rgjail(playerid, params[])
{
    if ((fcFaction[playerid] != FAC_RG || fcRank[playerid] < RG_RANK_JAIL) && !IsStaff(playerid))
        return SendClientMessage(playerid, COLOR_RED, "[RG] Republican Guard rank 4+ only.");

    new target, type[16];
    if (sscanf(params, "us[16]", target, type))
        return SendClientMessage(playerid, COLOR_YELLOW, "Usage: /rgjail [id] [minutes | sabotage]  (sabotage = 120 minutes automatically)");
    if (!IsPlayerConnected(target) || target == playerid)
        return SendClientMessage(playerid, COLOR_RED, "[RG] Invalid target.");

    new minutes, reason[48];
    if (!strcmp(type, "sabotage", true))
    {
        minutes = RG_SABOTAGE_JAIL;
        format(reason, sizeof(reason), "Sabotage");
    }
    else
    {
        minutes = strval(type);
        if (minutes < 1 || minutes > 300)
            return SendClientMessage(playerid, COLOR_RED, "[RG] Minutes must be between 1 and 300.");
        format(reason, sizeof(reason), "Republican Guard order");
    }

    new by[MAX_PLAYER_NAME];
    GetPlayerName(playerid, by, sizeof(by));
    JailPlayer(target, minutes, reason, by);
    return 1;
}

CMD:rgrelease(playerid, params[])
{
    if ((fcFaction[playerid] != FAC_RG || fcRank[playerid] < RG_RANK_RELEASE) && !IsStaff(playerid))
        return SendClientMessage(playerid, COLOR_RED, "[RG] Republican Guard rank 6+ only.");
    new target;
    if (sscanf(params, "u", target))
        return SendClientMessage(playerid, COLOR_YELLOW, "Usage: /rgrelease [id]");
    if (!IsPlayerConnected(target) || pJailTime[target] <= 0)
        return SendClientMessage(playerid, COLOR_RED, "[RG] That player is not in prison.");
    ReleasePlayer(target);
    SendClientMessage(playerid, COLOR_GREEN, "[RG] Prisoner released.");
    return 1;
}

// ---- معلومات ----
CMD:fmembers(playerid, params[])
{
    if (fcFaction[playerid] == FAC_NONE)
        return SendClientMessage(playerid, COLOR_RED, "[FACTION] You are not in a faction.");
    new msg[144], name[MAX_PLAYER_NAME], rn[24];
    format(msg, sizeof(msg), "===== Online members: %s =====", gFactionNames[fcFaction[playerid]]);
    SendClientMessage(playerid, COLOR_YELLOW, msg);
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (!IsPlayerConnected(i) || fcFaction[i] != fcFaction[playerid]) continue;
        GetPlayerName(i, name, sizeof(name));
        GetRankName(fcFaction[i], fcRank[i], rn, sizeof(rn));
        format(msg, sizeof(msg), "  [%d] %s - %s (Rank %d)", i, name, rn, fcRank[i]);
        SendClientMessage(playerid, COLOR_WHITE, msg);
    }
    return 1;
}

CMD:rghelp(playerid, params[])
{
    SendClientMessage(playerid, COLOR_YELLOW, "===== Republican Guard Commands =====");
    SendClientMessage(playerid, COLOR_WHITE, "/lift  /rgveh  /rg_armory  /rg_siren  /military_check");
    SendClientMessage(playerid, COLOR_WHITE, "/deploy_barrier  /remove_barrier  /clear_barriers");
    SendClientMessage(playerid, COLOR_WHITE, "/rgjail  /rgrelease  /r  /f  /fmembers");
    return 1;
}

// ============================================================================
//  أوامر الإدارة (RCON Admin) - عدّل IsStaff حسب نظام الإدارة لديك
// ============================================================================
// /setfaction [id] [0-4]
CMD:setfaction(playerid, params[])
{
    if (!IsStaff(playerid)) return SendClientMessage(playerid, COLOR_RED, "[ADMIN] You are not authorized.");

    new target, fac;
    if (sscanf(params, "ud", target, fac) || fac < 0 || fac >= FAC_COUNT)
    {
        SendClientMessage(playerid, COLOR_YELLOW, "Usage: /setfaction [id] [0-4]");
        SendClientMessage(playerid, COLOR_GREY, "0 Civilian | 1 Republican Guard | 2 Police | 3 Army | 4 EMS");
        return 1;
    }
    if (!IsPlayerConnected(target)) return SendClientMessage(playerid, COLOR_RED, "[ADMIN] Player not connected.");

    if (pSiren[target]) StopSiren(target);
    if (fcFaction[target] == FAC_RG && fac != FAC_RG)
    {
        ClearPlayerBarriers(target);
        DestroyPlayerGarageVehicle(target);
    }
    fcFaction[target] = fac;
    fcRank[target] = (fac == FAC_NONE) ? 0 : 1;
    UpdatePlayerColor(target);
    Fac_SavePlayerData(target);

    new tname[MAX_PLAYER_NAME], aname[MAX_PLAYER_NAME], msg[144];
    GetPlayerName(target, tname, sizeof(tname));
    GetPlayerName(playerid, aname, sizeof(aname));
    format(msg, sizeof(msg), "[ADMIN] %s set your faction to %s.", aname, gFactionNames[fac]);
    SendClientMessage(target, COLOR_GREEN, msg);
    format(msg, sizeof(msg), "[ADMIN] You set %s's faction to %s.", tname, gFactionNames[fac]);
    SendClientMessage(playerid, COLOR_GREEN, msg);
    return 1;
}

// /setrank [id] [1-10]
CMD:setrank(playerid, params[])
{
    if (!IsStaff(playerid)) return SendClientMessage(playerid, COLOR_RED, "[ADMIN] You are not authorized.");

    new target, rank;
    if (sscanf(params, "ud", target, rank) || rank < 1 || rank > MAX_RANK)
        return SendClientMessage(playerid, COLOR_YELLOW, "Usage: /setrank [id] [1-10]");
    if (!IsPlayerConnected(target)) return SendClientMessage(playerid, COLOR_RED, "[ADMIN] Player not connected.");
    if (fcFaction[target] == FAC_NONE) return SendClientMessage(playerid, COLOR_RED, "[ADMIN] That player has no faction. Use /setfaction first.");

    fcRank[target] = rank;
    Fac_SavePlayerData(target);

    new rn[24], tname[MAX_PLAYER_NAME], msg[144];
    GetRankName(fcFaction[target], rank, rn, sizeof(rn));
    GetPlayerName(target, tname, sizeof(tname));
    format(msg, sizeof(msg), "[FACTION] Your rank is now %d - %s.", rank, rn);
    SendClientMessage(target, COLOR_GREEN, msg);
    format(msg, sizeof(msg), "[ADMIN] %s is now rank %d - %s.", tname, rank, rn);
    SendClientMessage(playerid, COLOR_GREEN, msg);
    return 1;
}

// /setpresident [id] [0/1]  - منح صلاحية رئيس المدينة (الطابق 2)
CMD:setpresident(playerid, params[])
{
    if (!IsStaff(playerid)) return SendClientMessage(playerid, COLOR_RED, "[ADMIN] You are not authorized.");
    new target, pstate;
    if (sscanf(params, "ud", target, pstate) || pstate < 0 || pstate > 1)
        return SendClientMessage(playerid, COLOR_YELLOW, "Usage: /setpresident [id] [0/1]");
    if (!IsPlayerConnected(target)) return SendClientMessage(playerid, COLOR_RED, "[ADMIN] Player not connected.");

    pPresident[target] = pstate;
    Fac_SavePlayerData(target);
    SendClientMessage(target, COLOR_GREEN, pstate ? "[CITY] You are now the City President." : "[CITY] Your presidential authority was revoked.");
    SendClientMessage(playerid, COLOR_GREEN, "[ADMIN] Done.");
    return 1;
}
