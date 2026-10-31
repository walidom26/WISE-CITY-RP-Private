/* ============================================================================
    WISE-CITY-RP  |  نظام الإسعاف والمستشفى
    الوحدة: wc_ems.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

#define DIALOG_911               4001
#define DIALOG_911_DESC          4002
#define DIALOG_EMS_PANEL         4003
#define DIALOG_MED_RECORD        4004
#define DIALOG_HOSP_PHARM        4005
#define DIALOG_TREAT_OPTION      4006

/* ============================================================================
 *  WISE-CITY-RP :: EMS & Advanced Hospital System
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
#define MAX_EMS_RANKS        6
#define MAX_EMS_CALLS        50
#define MAX_WOUNDS           5

// Hospital (exterior + interior)
#define HOSP_EXT_X      1177.5328
#define HOSP_EXT_Y      -1323.9876
#define HOSP_EXT_Z      13.9542
#define HOSP_INT        3
#define HOSP_INT_X      390.5577
#define HOSP_INT_Y      173.2288
#define HOSP_INT_Z      1008.3828

// Revival timings
#define BLEED_TICK_MS       5000     // bleed damage every 5s
#define BLEED_DMG_L1        1
#define BLEED_DMG_L2        3
#define BLEED_DMG_L3        6

#define PULSE_CHECK_MS      3000
#define TREAT_ONSITE_TIME   12000    // ms to treat on-site
#define TREAT_ICU_TIME      18000    // ms for ICU procedure

// ---------------------------------------------------------------------------
//  Medical Rank Hierarchy
// ---------------------------------------------------------------------------
enum e_MedRank {
    mrName[64],
    mrMinLevel,
    mrSalary,
    mrCanTreat,       // on-site treatment
    mrCanHospitalize, // ICU
    mrCanPrescribe,   // pharmacy
    mrCanHire,        // hire/fire
    mrCanDispatch     // close calls / manage
};

new const MedRanks[MAX_EMS_RANKS][e_MedRank] = {
    {"مسعف (Paramedic)",             1,  1500, true,  false, false, false, false},
    {"مسعف أول (Senior Paramedic)",  3,  2500, true,  true,  false, false, false},
    {"ممرض (Nurse)",                 5,  3500, true,  true,  true,  false, false},
    {"طبيب (Doctor)",                8,  5000, true,  true,  true,  false, true },
    {"جراح (Surgeon)",               12, 7500, true,  true,  true,  false, true },
    {"رئيس الأطباء (Chief Physician)",15,10000, true,  true,  true,  true,  true }
};

// ---------------------------------------------------------------------------
//  Data structures
// ---------------------------------------------------------------------------
enum e_EmsStaff {
    esID,
    esBadge[10],
    esRank,
    bool:esOnDuty,
    esRevives,
    esDeaths
};

enum e_MedicalRecord {
    mrID,
    mrBlood[4],
    mrAllergies[128],
    mrChronic[128],
    mrWounds,
    mrBleeding,
    mrICUVisits,
    mrOnsiteRevives,
    mrDeaths
};

enum e_EmsCall {
    ecActive,
    ecID,
    ecCallerID,
    ecCallerName[24],
    ecCategory,
    ecDesc[255],
    Float:ecX,
    Float:ecY,
    Float:ecZ,
    ecAssignedTo,
    ecStatus
};

enum e_PlayerMedState {
    Float:pmHealth,
    pmWounds,
    pmBleeding,
    bool:pmInjuryActive,
    bool:pmBeingTreated,
    pmTreatedBy,
    pmStretcherObj,
    pmStretcherTarget,
    bool:pmOnStretcher,
    pmPulseTimer,
    pmBleedTimer
};

// ---------------------------------------------------------------------------
//  Globals
// ---------------------------------------------------------------------------

new EmsStaff[MAX_PLAYERS][e_EmsStaff];
new MedRecord[MAX_PLAYERS][e_MedicalRecord];
new MedState[MAX_PLAYERS][e_PlayerMedState];
new EmsCall[MAX_EMS_CALLS][e_EmsCall];
new g_CallCount = 0;


// Pickups / labels
new g_HospPickups[6];
new Text3D:g_HospLabels[6];

// Predefined interiors (patient ward, OR, pharmacy)
#define INT_WARD     3
#define INT_OR       3
#define INT_PHARMACY 3

// ---------------------------------------------------------------------------
//  Forwards
// ---------------------------------------------------------------------------
forward LoadPlayerMedical(playerid);
forward SaveMedicalRecord(playerid);
forward SaveEmsStaff(playerid);
forward LogEmsAction(playerid, const action[], target, const details[]);
forward BleedTimer(playerid);
forward FinishOnsiteTreat(playerid, targetid);
forward FinishICUTreat(playerid, targetid);
forward ClearCall(callid);

// ===========================================================================
//  GAMEMODE INIT
// ===========================================================================
stock EMS_OnGameModeInit()
{

    print("[WISE-CITY-RP] EMS: Connected to wise_city_db.");
    SetupHospital();
    return 1;
}

stock EMS_OnGameModeExit()
{
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (IsPlayerConnected(p)) {
            SaveMedicalRecord(p);
            if (EmsStaff[p][esID] != 0) SaveEmsStaff(p);
        }
    }
    return 1;
}

/**
 * Builds the luxurious hospital: exterior entrance + interior desks.
 */
stock SetupHospital()
{
    // ---- Exterior entrance ----
    CreatePickup(1240, 1, HOSP_EXT_X, HOSP_EXT_Y, HOSP_EXT_Z, 0);
    Create3DTextLabel("{FFFFFF}مستشفى {00FF00}WISE-CITY المركزي\n{FFFF00}اضغط ENTER للدخول",
        0xFFFFFFFF, HOSP_EXT_X, HOSP_EXT_Y, HOSP_EXT_Z + 1.2, 25.0, 0, 1);

    // ---- Interior ----
    // [0] Reception
    g_HospPickups[0] = CreatePickup(1247, 1, 390.55, 173.22, 1008.38, -1);
    g_HospLabels[0] = Create3DTextLabel(
        "{FFFFFF}مكتب {00FFFF}الاستقبال\n{FFFFFF}سجّل بياناتك الطبية هنا",
        0xFFFFFFFF, 390.55, 173.22, 1009.38, 15.0, HOSP_INT, 1);

    // [1] Operating Room
    g_HospPickups[1] = CreatePickup(1247, 1, 389.90, 172.60, 1008.38, -1);
    g_HospLabels[1] = Create3DTextLabel(
        "{FFFFFF}غرفة {FF0000}العمليات\n{FFFFFF}إجراء العمليات الجراحية",
        0xFFFFFFFF, 389.90, 172.60, 1009.38, 15.0, HOSP_INT, 1);

    // [2] Patient Ward
    g_HospPickups[2] = CreatePickup(1247, 1, 389.20, 172.00, 1008.38, -1);
    g_HospLabels[2] = Create3DTextLabel(
        "{FFFFFF}عنبر {FFFF00}المرضى\n{FFFFFF}رعاية مركزة (ICU)",
        0xFFFFFFFF, 389.20, 172.00, 1009.38, 15.0, HOSP_INT, 1);

    // [3] Pharmacy
    g_HospPickups[3] = CreatePickup(1247, 1, 388.50, 171.40, 1008.38, -1);
    g_HospLabels[3] = Create3DTextLabel(
        "{FFFFFF}صيدلية {00FF00}المستشفى\n{FFFFFF}شراء الأدوية والمستلزمات",
        0xFFFFFFFF, 388.50, 171.40, 1009.38, 15.0, HOSP_INT, 1);

    // [4] EMS Panel
    g_HospPickups[4] = CreatePickup(1247, 1, 387.80, 170.80, 1008.38, -1);
    g_HospLabels[4] = Create3DTextLabel(
        "{FFFFFF}لوحة {00FFFF}EMS\n{FFFFFF}لطاقم الطوارئ فقط",
        0xFFFFFFFF, 387.80, 170.80, 1009.38, 15.0, HOSP_INT, 1);

    // [5] Exit
    g_HospPickups[5] = CreatePickup(1247, 1, 387.00, 173.50, 1008.38, -1);
    g_HospLabels[5] = Create3DTextLabel("{FFFFFF}الخروج من المستشفى",
        0xFFFFFFFF, 387.00, 173.50, 1009.38, 15.0, HOSP_INT, 1);
}

// ===========================================================================
//  PLAYER CONNECT / LOAD
// ===========================================================================
stock EMS_OnPlayerConnect(playerid)
{
    // Reset in-memory
    EmsStaff[playerid][esID]     = 0;
    EmsStaff[playerid][esRank]   = 0;
    EmsStaff[playerid][esOnDuty] = false;
    EmsStaff[playerid][esRevives] = 0;
    EmsStaff[playerid][esDeaths] = 0;

    MedRecord[playerid][mrWounds]        = 0;
    MedRecord[playerid][mrBleeding]      = 0;
    MedRecord[playerid][mrICUVisits]     = 0;
    MedRecord[playerid][mrOnsiteRevives] = 0;
    MedRecord[playerid][mrDeaths]        = 0;

    MedState[playerid][pmWounds]        = 0;
    MedState[playerid][pmBleeding]      = 0;
    MedState[playerid][pmInjuryActive]  = false;
    MedState[playerid][pmBeingTreated]  = false;
    MedState[playerid][pmTreatedBy]     = INVALID_PLAYER_ID;
    MedState[playerid][pmStretcherObj]  = INVALID_OBJECT_ID;
    MedState[playerid][pmStretcherTarget] = INVALID_PLAYER_ID;
    MedState[playerid][pmOnStretcher]   = false;
    MedState[playerid][pmPulseTimer]    = 0;
    MedState[playerid][pmBleedTimer]    = 0;
    g_PlayerDBID[playerid] = 0;
    return 1;
}

stock EMS_OnPlayerDisconnect(playerid, reason)
{
    if (g_PlayerDBID[playerid] != 0) {
        SaveMedicalRecord(playerid);
        if (EmsStaff[playerid][esID] != 0) SaveEmsStaff(playerid);
    }
    // Destroy stretcher object if any
    if (MedState[playerid][pmStretcherObj] != INVALID_OBJECT_ID)
        DestroyObject(MedState[playerid][pmStretcherObj]);

    // Free any active call assigned to this medic
    for (new i = 0; i < g_CallCount; i++)
        if (EmsCall[i][ecAssignedTo] == playerid)
            EmsCall[i][ecAssignedTo] = 0;

    KillTimerSafe(MedState[playerid][pmBleedTimer]);
    KillTimerSafe(MedState[playerid][pmPulseTimer]);
    return 1;
}

stock KillTimerSafe(&timerid)
{
    if (timerid != 0) { KillTimer(timerid); timerid = 0; }
}

/**
 * Call right after a successful login.
 */
public LoadPlayerMedical(playerid)
{
    new plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));

    new q[300];
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT id FROM `users` WHERE username='%e' LIMIT 1", plName);
    new Cache:res = mysql_query(g_SQL, q);
    if (cache_num_rows() == 0) { cache_delete(res); return 0; }
    g_PlayerDBID[playerid] = cache_get_field_content_int(0, "id", g_SQL);
    cache_delete(res);

    MedRecord[playerid][mrID] = g_PlayerDBID[playerid];

    // ---- Medical record ----
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT blood_type, allergies, chronic, wounds, bleeding_level, icu_visits, \
         onsite_revives, deaths FROM medical_records WHERE player_id=%d LIMIT 1",
        g_PlayerDBID[playerid]);
    res = mysql_query(g_SQL, q);
    if (cache_num_rows() == 0) {
        mysql_format(g_SQL, q, sizeof(q),
            "INSERT INTO medical_records (player_id) VALUES (%d)", g_PlayerDBID[playerid]);
        WC_Exec(q);
    } else {
        cache_get_field_content(0, "blood_type", MedRecord[playerid][mrBlood], g_SQL, 4);
        cache_get_field_content(0, "allergies",  MedRecord[playerid][mrAllergies], g_SQL, 128);
        cache_get_field_content(0, "chronic",    MedRecord[playerid][mrChronic], g_SQL, 128);
        MedRecord[playerid][mrWounds]        = cache_get_field_content_int(0, "wounds", g_SQL);
        MedRecord[playerid][mrBleeding]      = cache_get_field_content_int(0, "bleeding_level", g_SQL);
        MedRecord[playerid][mrICUVisits]     = cache_get_field_content_int(0, "icu_visits", g_SQL);
        MedRecord[playerid][mrOnsiteRevives] = cache_get_field_content_int(0, "onsite_revives", g_SQL);
        MedRecord[playerid][mrDeaths]        = cache_get_field_content_int(0, "deaths", g_SQL);
    }
    cache_delete(res);

    // ---- EMS staff ----
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT badge_number, rank, on_duty, total_revives, total_deaths \
         FROM ems_staff WHERE player_id=%d LIMIT 1",
        g_PlayerDBID[playerid]);
    res = mysql_query(g_SQL, q);
    if (cache_num_rows() > 0) {
        EmsStaff[playerid][esID] = g_PlayerDBID[playerid];
        cache_get_field_content(0, "badge_number", EmsStaff[playerid][esBadge], g_SQL, 10);
        EmsStaff[playerid][esRank]     = cache_get_field_content_int(0, "rank", g_SQL);
        EmsStaff[playerid][esOnDuty]   = bool:cache_get_field_content_int(0, "on_duty", g_SQL);
        EmsStaff[playerid][esRevives]  = cache_get_field_content_int(0, "total_revives", g_SQL);
        EmsStaff[playerid][esDeaths]   = cache_get_field_content_int(0, "total_deaths", g_SQL);
    }
    cache_delete(res);
    return 1;
}

// ===========================================================================
//  SAVE HELPERS
// ===========================================================================
public SaveMedicalRecord(playerid)
{
    if (g_PlayerDBID[playerid] == 0) return 0;
    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE medical_records SET blood_type='%e', allergies='%e', chronic='%e', \
         wounds=%d, bleeding_level=%d, icu_visits=%d, onsite_revives=%d, deaths=%d \
         WHERE player_id=%d",
        MedRecord[playerid][mrBlood],
        MedRecord[playerid][mrAllergies],
        MedRecord[playerid][mrChronic],
        MedRecord[playerid][mrWounds],
        MedRecord[playerid][mrBleeding],
        MedRecord[playerid][mrICUVisits],
        MedRecord[playerid][mrOnsiteRevives],
        MedRecord[playerid][mrDeaths],
        g_PlayerDBID[playerid]);
    WC_Exec(q);
    return 1;
}

public SaveEmsStaff(playerid)
{
    if (EmsStaff[playerid][esID] == 0) return 0;
    new q[300];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE ems_staff SET badge_number='%e', rank=%d, on_duty=%d, \
         total_revives=%d, total_deaths=%d WHERE player_id=%d",
        EmsStaff[playerid][esBadge],
        EmsStaff[playerid][esRank],
        EmsStaff[playerid][esOnDuty],
        EmsStaff[playerid][esRevives],
        EmsStaff[playerid][esDeaths],
        EmsStaff[playerid][esID]);
    WC_Exec(q);
    return 1;
}

public LogEmsAction(playerid, const action[], target, const details[])
{
    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO ems_logs (player_id, action, target_id, details) VALUES (%d, '%e', %d, '%e')",
        g_PlayerDBID[playerid], action, target, details);
    WC_Exec(q);
    return 1;
}

// ===========================================================================
//  PERMISSION HELPERS
// ===========================================================================
stock bool:IsEms(playerid)
{
    return (EmsStaff[playerid][esID] != 0 && EmsStaff[playerid][esRank] >= 1);
}

stock bool:EmsCanTreat(playerid)
{
    return IsEms(playerid) && MedRanks[EmsStaff[playerid][esRank] - 1][mrCanTreat];
}

stock bool:EmsCanHospitalize(playerid)
{
    return IsEms(playerid) && MedRanks[EmsStaff[playerid][esRank] - 1][mrCanHospitalize];
}

stock bool:EmsCanPrescribe(playerid)
{
    return IsEms(playerid) && MedRanks[EmsStaff[playerid][esRank] - 1][mrCanPrescribe];
}

stock bool:EmsCanHire(playerid)
{
    return IsEms(playerid) && MedRanks[EmsStaff[playerid][esRank] - 1][mrCanHire];
}

stock GetEmsRankName(rank, dest[40])
{
    if (rank < 1 || rank > MAX_EMS_RANKS) { dest[0] = '\0'; return 0; }
    format(dest, 40, MedRanks[rank - 1][mrName]);
    return 1;
}

// ===========================================================================
//  911 DISPATCH
// ===========================================================================
CMD:911(playerid, params[])
{
    ShowPlayerDialog(playerid, DIALOG_911, DIALOG_STYLE_LIST,
        "اتصال بالطوارئ (911)",
        "1. حالة طبية طارئة (EMS)\n2. حريق (الإطفاء)\n3. جريمة (الشرطة)",
        "اختيار", "إلغاء");
    return 1;
}

/**
 * Internal helper: create a pending call and notify on-duty EMS.
 */
stock CreateEmergencyCall(playerid, category, const desc[])
{
    if (g_CallCount >= MAX_EMS_CALLS) {
        SendClientMessage(playerid, 0xFF0000AA, "[911] خطوط الطوارئ مشغولة، حاول لاحقاً.");
        return 0;
    }

    new idx = -1;
    for (new i = 0; i < MAX_EMS_CALLS; i++) {
        if (!EmsCall[i][ecActive]) { idx = i; break; }
    }
    if (idx == -1) return 0;

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    EmsCall[idx][ecActive]  = true;
    EmsCall[idx][ecCallerID]= playerid;
    GetPlayerName(playerid, EmsCall[idx][ecCallerName], 24);
    EmsCall[idx][ecCategory]= category;
    format(EmsCall[idx][ecDesc], 255, "%s", desc);
    EmsCall[idx][ecX] = x; EmsCall[idx][ecY] = y; EmsCall[idx][ecZ] = z;
    EmsCall[idx][ecAssignedTo] = 0;
    EmsCall[idx][ecStatus]     = 0;

    // Persist
    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO ems_calls (caller_id, caller_name, category, description, \
         pos_x, pos_y, pos_z) VALUES (%d, '%e', %d, '%e', %f, %f, %f)",
        g_PlayerDBID[playerid], EmsCall[idx][ecCallerName], category, desc, x, y, z);
    WC_Exec(q);
    EmsCall[idx][ecID] = WC_LastInsertID();

    // Notify on-duty EMS
    new msg[256];
    format(msg, sizeof(msg),
        "[911] نداء جديد! %s | الفئة: %d | %s",
        EmsCall[idx][ecCallerName], category, desc);
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (!IsPlayerConnected(p)) continue;
        if (!IsEms(p) || !EmsStaff[p][esOnDuty]) continue;
        SendClientMessage(p, 0xFF4444AA, msg);
        SendClientMessage(p, 0xFFFF00AA, "استخدم /calls لاستعراض النداءات أو /accept [id]");
    }

    SendClientMessage(playerid, 0x00FF00AA, "[911] تم إرسال نداءك إلى الوحدات الطبية.");
    LogEmsAction(playerid, "CALL_911", 0, desc);
    return 1;
}

stock EMS_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (!response) return 1;

    if (dialogid == DIALOG_911) {
        new cat = listitem + 1;
        new desc[128];
        switch (cat) {
            case 1: format(desc, sizeof(desc), "حالة طبية طارئة");
            case 2: format(desc, sizeof(desc), "حريق");
            case 3: format(desc, sizeof(desc), "جريمة");
        }
        CreateEmergencyCall(playerid, cat, desc);
        return 1;
    }

    if (dialogid == DIALOG_EMS_PANEL) {
        switch (listitem) {
            case 0: { // Toggle duty
                if (!IsEms(playerid)) return 1;
                EmsStaff[playerid][esOnDuty] = !EmsStaff[playerid][esOnDuty];
                SaveEmsStaff(playerid);
                SendClientMessage(playerid, 0x00FF00AA,
                    EmsStaff[playerid][esOnDuty] ?
                    "[EMS] أنت الآن في الخدمة." :
                    "[EMS] أنت الآن خارج الخدمة.");
            }
            case 1: { // View calls
                pc_cmd_calls(playerid, "");
            }
            case 2: { // Medical record
                ShowMedicalRecordDialog(playerid, playerid);
            }
        }
        return 1;
    }

    if (dialogid == DIALOG_MED_RECORD) return 1;

    if (dialogid == DIALOG_HOSP_PHARM) {
        if (!EmsCanPrescribe(playerid)) {
            SendClientMessage(playerid, 0xFF0000AA, "[صيدلية] لا تملك صلاحية صرف الأدوية.");
            return 1;
        }
        new item[32];
        switch (listitem) {
            case 0: format(item, sizeof(item), "bandage");
            case 1: format(item, sizeof(item), "medkit");
            case 2: format(item, sizeof(item), "morphine");
            case 3: format(item, sizeof(item), "antidote");
            case 4: format(item, sizeof(item), "defibrillator");
        }
        DispenseItem(playerid, item);
        return 1;
    }

    return 0;
}

CMD:calls(playerid, params[])
{
    if (!IsEms(playerid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] هذا الأمر للأطباء فقط.");
        return 1;
    }
    new str[2048], line[128], found = 0;
    format(str, sizeof(str), "النداءات النشطة\n------------------------\n");
    for (new i = 0; i < MAX_EMS_CALLS; i++) {
        if (!EmsCall[i][ecActive]) continue;
        if (EmsCall[i][ecStatus] == 2) continue;
        found++;
        format(line, sizeof(line), "ID %d | %s | %s | المسعف: %d\n",
            i, EmsCall[i][ecCallerName], EmsCall[i][ecDesc],
            EmsCall[i][ecAssignedTo]);
        strcat(str, line, sizeof(str));
    }
    if (!found) format(str, sizeof(str), "لا توجد نداءات نشطة حالياً.");
    ShowPlayerDialog(playerid, 9999, DIALOG_STYLE_MSGBOX, "لوحة النداءات", str, "حسناً", "");
    return 1;
}

CMD:accept(playerid, params[])
{
    if (!IsEms(playerid)) { SendClientMessage(playerid, 0xFF0000AA, "[EMS] للأطباء فقط."); return 1; }
    new id;
    if (sscanf(params, "d", id)) {
        SendClientMessage(playerid, 0xFFFF00AA, "استخدم: /accept [call id]");
        return 1;
    }
    if (id < 0 || id >= MAX_EMS_CALLS || !EmsCall[id][ecActive]) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] نداء غير صالح.");
        return 1;
    }
    if (EmsCall[id][ecAssignedTo] != 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] هذا النداء تم استلامه بالفعل.");
        return 1;
    }

    EmsCall[id][ecAssignedTo] = playerid;
    EmsCall[id][ecStatus]     = 1;

    SetPlayerCheckpoint(playerid, EmsCall[id][ecX], EmsCall[id][ecY], EmsCall[id][ecZ], 3.0);
    SendClientMessage(playerid, 0x00FF00AA, "[EMS] تم تعيينك للنداء. اتبع العلامة على الخريطة.");
    LogEmsAction(playerid, "ACCEPT", 0, EmsCall[id][ecDesc]);
    return 1;
}

stock EMS_OnPlayerEnterCheckpoint(playerid)
{
    for (new i = 0; i < MAX_EMS_CALLS; i++) {
        if (!EmsCall[i][ecActive]) continue;
        if (EmsCall[i][ecAssignedTo] != playerid) continue;
        if (IsPlayerInRangeOfPoint(playerid, 3.0, EmsCall[i][ecX], EmsCall[i][ecY], EmsCall[i][ecZ])) {
            DisablePlayerCheckpoint(playerid);
            SendClientMessage(playerid, 0x00FF00AA,
                "[EMS] وصلت إلى موقع الحادث. افحص المريض (/pulse) ثم ابدأ العلاج.");
            break;
        }
    }
    return 1;
}

public ClearCall(callid)
{
    if (callid < 0 || callid >= MAX_EMS_CALLS) return 0;
    EmsCall[callid][ecActive] = false;
    EmsCall[callid][ecAssignedTo] = 0;
    EmsCall[callid][ecStatus] = 2;
    new q[128];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE ems_calls SET status=2 WHERE id=%d", EmsCall[callid][ecID]);
    WC_Exec(q);
    return 1;
}

// ===========================================================================
//  INJURY MECHANICS
// ===========================================================================
/**
 * Applies a wound to a player. Used by combat system.
 */
stock ApplyWound(playerid, amount = 1, bleedLevel = 1)
{
    MedState[playerid][pmWounds] += amount;
    if (MedState[playerid][pmWounds] > MAX_WOUNDS)
        MedState[playerid][pmWounds] = MAX_WOUNDS;

    if (bleedLevel > MedState[playerid][pmBleeding])
        MedState[playerid][pmBleeding] = bleedLevel;

    MedState[playerid][pmInjuryActive] = true;

    if (MedState[playerid][pmBleedTimer] == 0)
        MedState[playerid][pmBleedTimer] = SetTimerEx("BleedTimer", BLEED_TICK_MS, true, "i", playerid);

    SendClientMessage(playerid, 0xFF0000AA, "[إصابة] لقد أصبت بجرح! استخدم /pulse لفحص حالتك.");
    return 1;
}

public BleedTimer(playerid)
{
    if (!IsPlayerConnected(playerid)) {
        MedState[playerid][pmBleedTimer] = 0;
        return 0;
    }
    if (MedState[playerid][pmBleeding] <= 0) {
        KillTimerSafe(MedState[playerid][pmBleedTimer]);
        return 0;
    }

    new dmg = 0;
    switch (MedState[playerid][pmBleeding]) {
        case 1: dmg = BLEED_DMG_L1;
        case 2: dmg = BLEED_DMG_L2;
        case 3: dmg = BLEED_DMG_L3;
    }

    new Float:hp;
    GetPlayerHealth(playerid, hp);
    hp -= float(dmg);
    if (hp < 1.0) hp = 1.0;   // don't hard-kill; we want RP recovery
    SetPlayerHealth(playerid, hp);

    if (hp <= 15.0 && !MedState[playerid][pmBeingTreated]) {
        SendClientMessage(playerid, 0xFF0000AA, "[إصابة] أنت تنزف بشدة! اتصل بالطوارئ فوراً (911).");
    }
    return 1;
}

CMD:pulse(playerid, params[])
{
    new targetid;
    if (sscanf(params, "d", targetid)) {
        // Self pulse if no target
        targetid = playerid;
    }
    if (!IsPlayerConnected(targetid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[فحص] المريض غير متصل.");
        return 1;
    }


    if (playerid != targetid) {
        new Float:px, Float:py, Float:pz;
        GetPlayerPos(targetid, px, py, pz);
        if (!IsPlayerInRangeOfPoint(playerid, 3.5, px, py, pz)) {
            SendClientMessage(playerid, 0xFF0000AA, "[فحص] يجب أن تكون قريباً من المريض.");
            return 1;
        }
    }

    new Float:hp;
    GetPlayerHealth(targetid, hp);

    new status[32];
    if (hp > 75.0)      status = "مستقر";
    else if (hp > 40.0) status = "حرج";
    else if (hp > 15.0) status = "خطر";
    else                status = "يحتضر";

    new str[256];
    format(str, sizeof(str),
        "[فحص] المريض: %s\n\
         النبض: %d نبضة/دقيقة\n\
         مستوى النزيف: %d / 3\n\
         عدد الجروح: %d\n\
         الحالة: %s",
        targetid == playerid ? "أنت" : "المريض",
        60 + floatround(hp / 2.0),
        MedState[targetid][pmBleeding],
        MedState[targetid][pmWounds],
        status);
    SendClientMessage(playerid, 0x00FFFFAA, str);
    return 1;
}

// ===========================================================================
//  TREATMENT WORKFLOW (on-site vs ICU)
// ===========================================================================
CMD:treat(playerid, params[])
{
    if (!EmsCanTreat(playerid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] لا تملك صلاحية العلاج.");
        return 1;
    }
    new targetid;
    if (sscanf(params, "d", targetid)) {
        SendClientMessage(playerid, 0xFFFF00AA, "استخدم: /treat [player id]");
        return 1;
    }
    if (!IsPlayerConnected(targetid) || targetid == playerid) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] مريض غير صالح.");
        return 1;
    }

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(targetid, px, py, pz);
    if (!IsPlayerInRangeOfPoint(playerid, 4.0, px, py, pz)) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] اقترب من المريض أولاً.");
        return 1;
    }
    if (MedState[targetid][pmBeingTreated]) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] المريض قيد العلاج بالفعل.");
        return 1;
    }

    // Choose on-site vs ICU based on severity
    new bool:severe = (MedState[targetid][pmWounds] >= 3 || MedState[targetid][pmBleeding] >= 3);
    new str[256];
    if (severe) {
        format(str, sizeof(str),
            "المريض بحالة خطيرة!\nاختر إجراءً:\n1. علاج ميداني (سريع لكن غير كافٍ)\n2. نقل إلى العناية المركزة (ICU)");
    } else {
        format(str, sizeof(str),
            "المريض بحالة مستقرة نسبياً.\n1. علاج ميداني\n2. نقل إلى ICU");
    }
    MedState[playerid][pmStretcherTarget] = targetid;
    ShowPlayerDialog(playerid, DIALOG_TREAT_OPTION, DIALOG_STYLE_LIST,
        "خيارات العلاج", str, "اختيار", "إلغاء");
    return 1;
}

public OnDialogResponse_Treat(playerid, response, listitem)
{
    if (!response) return 1;
    new targetid = MedState[playerid][pmStretcherTarget];
    if (!IsPlayerConnected(targetid)) return 1;

    if (listitem == 0) {
        // On-site treatment
        MedState[targetid][pmBeingTreated] = true;
        MedState[playerid][pmTreatedBy] = playerid;

        ApplyAnimation(playerid, "MEDIC", "CPR", 4.1, 1, 0, 0, 0, 0, 1);
        ApplyAnimation(targetid, "SWEET", "Sweet_injuredloop", 4.1, 1, 0, 0, 0, 0, 1);

        SendClientMessage(playerid, 0x00FF00AA, "[EMS] جارٍ العلاج الميداني...");
        SendClientMessage(targetid, 0x00FF00AA, "[EMS] يتلقى المسعف علاجك، ابقَ ثابتاً.");
        SetTimerEx("FinishOnsiteTreat", TREAT_ONSITE_TIME, false, "ii", playerid, targetid);
    } else {
        // ICU transfer
        if (!EmsCanHospitalize(playerid)) {
            SendClientMessage(playerid, 0xFF0000AA, "[EMS] صلاحيتك لا تسمح بنقل المريض إلى ICU.");
            return 1;
        }
        MedState[targetid][pmBeingTreated] = true;

        // Attach to stretcher (visual only for now)
        SetPlayerPos(targetid, HOSP_INT_X, HOSP_INT_Y, HOSP_INT_Z);
        SetPlayerInterior(targetid, HOSP_INT);
        SetPlayerPos(playerid, HOSP_INT_X + 1.0, HOSP_INT_Y, HOSP_INT_Z);
        SetPlayerInterior(playerid, HOSP_INT);

        SendClientMessage(playerid, 0x00FF00AA, "[EMS] تم نقل المريض إلى ICU، بدء العلاج المكثف...");
        SendClientMessage(targetid, 0x00FF00AA, "[EMS] أنت في العناية المركزة.");
        SetTimerEx("FinishICUTreat", TREAT_ICU_TIME, false, "ii", playerid, targetid);
    }
    return 1;
}

public FinishOnsiteTreat(playerid, targetid)
{
    if (!IsPlayerConnected(playerid) || !IsPlayerConnected(targetid)) return 0;

    new Float:hp;
    GetPlayerHealth(targetid, hp);
    hp += 40.0;
    if (hp > 100.0) hp = 100.0;
    SetPlayerHealth(targetid, hp);

    MedState[targetid][pmWounds]   = 0;
    MedState[targetid][pmBleeding] = 0;
    MedState[targetid][pmBeingTreated] = false;
    MedState[playerid][pmTreatedBy] = INVALID_PLAYER_ID;

    ClearAnimations(playerid);
    ClearAnimations(targetid);

    EmsStaff[playerid][esRevives]++;
    MedRecord[targetid][mrOnsiteRevives]++;
    SaveEmsStaff(playerid);
    SaveMedicalRecord(targetid);
    LogEmsAction(playerid, "REVIVE_ONSITE", targetid, "on-site treatment");

    SendClientMessage(playerid, 0x00FF00AA, "[EMS] تم العلاج الميداني بنجاح.");
    SendClientMessage(targetid, 0x00FF00AA, "[EMS] استعدت عافيتك بفضل المسعف.");
    return 1;
}

public FinishICUTreat(playerid, targetid)
{
    if (!IsPlayerConnected(playerid) || !IsPlayerConnected(targetid)) return 0;

    SetPlayerHealth(targetid, 100.0);
    MedState[targetid][pmWounds]   = 0;
    MedState[targetid][pmBleeding] = 0;
    MedState[targetid][pmBeingTreated] = false;
    MedState[playerid][pmTreatedBy] = INVALID_PLAYER_ID;

    MedRecord[targetid][mrICUVisits]++;
    MedRecord[targetid][mrBleeding] = 0;
    SaveMedicalRecord(targetid);

    EmsStaff[playerid][esRevives]++;
    SaveEmsStaff(playerid);
    LogEmsAction(playerid, "REVIVE_ICU", targetid, "ICU treatment");

    SendClientMessage(playerid, 0x00FF00AA, "[EMS] تم إنهاء علاج ICU.");
    SendClientMessage(targetid, 0x00FF00AA, "[EMS] خرجت من العناية المركزة بصحة كاملة.");
    return 1;
}

// ===========================================================================
//  STRETCHERS (نقالات)
// ===========================================================================
CMD:stretcher(playerid, params[])
{
    if (!IsEms(playerid)) { SendClientMessage(playerid, 0xFF0000AA, "[نقالة] للأطباء فقط."); return 1; }
    new targetid;
    if (sscanf(params, "d", targetid)) {
        SendClientMessage(playerid, 0xFFFF00AA, "استخدم: /stretcher [player id]");
        return 1;
    }
    if (!IsPlayerConnected(targetid)) return 1;

    new Float:px, Float:py, Float:pz;
    GetPlayerPos(targetid, px, py, pz);
    if (!IsPlayerInRangeOfPoint(playerid, 4.0, px, py, pz)) {
        SendClientMessage(playerid, 0xFF0000AA, "[نقالة] اقترب من المريض.");
        return 1;
    }

    MedState[targetid][pmStretcherObj] = CreateObject(1997, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    AttachObjectToPlayer(MedState[targetid][pmStretcherObj], targetid, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0);
    MedState[targetid][pmOnStretcher] = true;
    SetPlayerSpecialAction(targetid, SPECIAL_ACTION_CARRY);

    SendClientMessage(playerid, 0x00FF00AA, "[نقالة] تم وضع المريض على النقالة، انقله إلى المستشفى.");
    SendClientMessage(targetid, 0x00FF00AA, "[نقالة] تم وضعك على النقالة، ابقَ ثابتاً.");
    return 1;
}

CMD:remove_stretcher(playerid, params[])
{
    if (!IsEms(playerid)) return 1;
    new targetid;
    if (sscanf(params, "d", targetid)) {
        SendClientMessage(playerid, 0xFFFF00AA, "استخدم: /remove_stretcher [player id]");
        return 1;
    }
    if (!IsPlayerConnected(targetid)) return 1;
    if (MedState[targetid][pmStretcherObj] != INVALID_OBJECT_ID) {
        DestroyObject(MedState[targetid][pmStretcherObj]);
        MedState[targetid][pmStretcherObj] = INVALID_OBJECT_ID;
    }
    MedState[targetid][pmOnStretcher] = false;
    SetPlayerSpecialAction(targetid, SPECIAL_ACTION_NONE);
    SendClientMessage(playerid, 0x00FF00AA, "[نقالة] تم إزالة المريض من النقالة.");
    return 1;
}

// ===========================================================================
//  HOSPITAL INTERACTIONS (Pickups)
// ===========================================================================
stock EMS_OnPlayerPickUpPickup(playerid, pickupid)
{
    // Exterior entrance
    if (IsPlayerInRangeOfPoint(playerid, 2.5, HOSP_EXT_X, HOSP_EXT_Y, HOSP_EXT_Z)) {
        SetPlayerInterior(playerid, HOSP_INT);
        SetPlayerPos(playerid, HOSP_INT_X, HOSP_INT_Y, HOSP_INT_Z);
        SendClientMessage(playerid, 0x00FF00AA, "[مستشفى] مرحباً بك في مستشفى WISE-CITY المركزي.");
        return 1;
    }

    if (pickupid == g_HospPickups[0]) { // Reception
        ShowMedicalRecordDialog(playerid, playerid);
        return 1;
    }
    if (pickupid == g_HospPickups[1]) { // OR
        SendClientMessage(playerid, 0xFFFF00AA, "[مستشفى] غرفة العمليات – للطاقم الطبي فقط.");
        return 1;
    }
    if (pickupid == g_HospPickups[2]) { // Ward
        SendClientMessage(playerid, 0xFFFF00AA, "[مستشفى] عنبر المرضى – رعاية مركزة.");
        return 1;
    }
    if (pickupid == g_HospPickups[3]) { // Pharmacy
        ShowPharmacyDialog(playerid);
        return 1;
    }
    if (pickupid == g_HospPickups[4]) { // EMS panel
        if (!IsEms(playerid)) {
            SendClientMessage(playerid, 0xFF0000AA, "[EMS] هذه اللوحة للطاقم الطبي فقط.");
            return 1;
        }
        ShowPlayerDialog(playerid, DIALOG_EMS_PANEL, DIALOG_STYLE_LIST,
            "لوحة EMS",
            "1. تبديل الحالة (On/Off Duty)\n2. عرض النداءات النشطة\n3. عرض سجلي الطبي",
            "اختيار", "إغلاق");
        return 1;
    }
    if (pickupid == g_HospPickups[5]) { // Exit
        SetPlayerInterior(playerid, 0);
        SetPlayerPos(playerid, HOSP_EXT_X + 1.5, HOSP_EXT_Y, HOSP_EXT_Z);
        SendClientMessage(playerid, 0x00FF00AA, "[مستشفى] وداعاً، نتمنى لك الشفاء العاجل.");
        return 1;
    }
    return 1;
}

stock ShowMedicalRecordDialog(playerid, targetid)
{
    new str[512];
    format(str, sizeof(str),
        "السجل الطبي\n\
         -------------------------\n\
         فصيلة الدم: %s\n\
         الحساسية: %s\n\
         الأمراض المزمنة: %s\n\
         عدد الجروح النشطة: %d\n\
         مستوى النزيف: %d/3\n\
         زيارات ICU: %d\n\
         عمليات إنعاش ميداني: %d\n\
         الوفيات: %d",
        MedRecord[targetid][mrBlood],
        MedRecord[targetid][mrAllergies],
        MedRecord[targetid][mrChronic],
        MedState[targetid][pmWounds],
        MedState[targetid][pmBleeding],
        MedRecord[targetid][mrICUVisits],
        MedRecord[targetid][mrOnsiteRevives],
        MedRecord[targetid][mrDeaths]);
    ShowPlayerDialog(playerid, DIALOG_MED_RECORD, DIALOG_STYLE_MSGBOX,
        "السجل الطبي", str, "حسناً", "");
    return 1;
}

stock ShowPharmacyDialog(playerid)
{
    new str[512];
    format(str, sizeof(str),
        "الصيدلية المركزية\n\
         -------------------------\n\
         1. ضمادة (bandage)      – 200 دينار\n\
         2. حقيبة إسعاف (medkit) – 800 دينار\n\
         3. مورفين (morphine)    – 1500 دينار\n\
         4. مضاد السموم (antidote) – 2500 دينار\n\
         5. جهاز صدمات (defibrillator) – 5000 دينار");
    ShowPlayerDialog(playerid, DIALOG_HOSP_PHARM, DIALOG_STYLE_LIST,
        "صيدلية المستشفى", str, "شراء", "إغلاق");
    return 1;
}

stock DispenseItem(playerid, const item[])
{
    // Simplified: deduct money, decrement inventory, notify
    new price = 0;
    if (strcmp(item, "bandage") == 0) price = 200;
    else if (strcmp(item, "medkit") == 0) price = 800;
    else if (strcmp(item, "morphine") == 0) price = 1500;
    else if (strcmp(item, "antidote") == 0) price = 2500;
    else if (strcmp(item, "defibrillator") == 0) price = 5000;

    if (GetPlayerMoney(playerid) < price) {
        SendClientMessage(playerid, 0xFF0000AA, "[صيدلية] أموالك غير كافية.");
        return 1;
    }

    // Check stock
    new q[256];
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT quantity FROM hospital_inventory WHERE item='%e' LIMIT 1", item);
    new Cache:res = mysql_query(g_SQL, q);
    if (cache_num_rows() == 0 || cache_get_field_content_int(0, "quantity", g_SQL) <= 0) {
        cache_delete(res);
        SendClientMessage(playerid, 0xFF0000AA, "[صيدلية] هذا المنتج غير متوفر حالياً.");
        return 1;
    }
    cache_delete(res);

    GivePlayerMoney(playerid, -price);
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE hospital_inventory SET quantity = quantity - 1 WHERE item='%e'", item);
    WC_Exec(q);

    new msg[128];
    format(msg, sizeof(msg), "[صيدلية] اشتريت %s مقابل %d دينار.", item, price);
    SendClientMessage(playerid, 0x00FF00AA, msg);
    LogEmsAction(playerid, "PHARMACY_BUY", 0, item);
    return 1;
}

// ===========================================================================
//  EMS HIRING (Admin/Chief command)
// ===========================================================================
CMD:emshire(playerid, params[])
{
    if (!EmsCanHire(playerid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] فقط رئيس الأطباء يمكنه التوظيف.");
        return 1;
    }
    new targetid, rank;
    if (sscanf(params, "dd", targetid, rank)) {
        SendClientMessage(playerid, 0xFFFF00AA, "استخدم: /emshire [player id] [rank 1-6]");
        return 1;
    }
    if (!IsPlayerConnected(targetid)) return 1;
    if (rank < 1 || rank > MAX_EMS_RANKS) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] رتبة غير صالحة (1-6).");
        return 1;
    }

    new badge[10];
    format(badge, sizeof(badge), "EMS%03d", random(999));

    EmsStaff[targetid][esID]     = g_PlayerDBID[targetid];
    format(EmsStaff[targetid][esBadge], 10, badge);
    EmsStaff[targetid][esRank]   = rank;
    EmsStaff[targetid][esOnDuty] = false;

    new q[300];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO ems_staff (player_id, badge_number, rank) VALUES (%d, '%e', %d) \
         ON DUPLICATE KEY UPDATE badge_number='%e', rank=%d",
        g_PlayerDBID[targetid], badge, rank, badge, rank);
    WC_Exec(q);

    new rankName[40];
    GetEmsRankName(rank, rankName);
    new msg[160];
    format(msg, sizeof(msg), "[EMS] تم توظيف اللاعب برتبة: %s (شارة %s).", rankName, badge);
    SendClientMessage(playerid, 0x00FF00AA, msg);
    SendClientMessage(targetid, 0x00FF00AA, msg);
    LogEmsAction(playerid, "HIRE", targetid, rankName);
    return 1;
}

CMD:emspanel(playerid, params[])
{
    if (!IsEms(playerid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[EMS] هذا الأمر للطاقم الطبي.");
        return 1;
    }
    ShowPlayerDialog(playerid, DIALOG_EMS_PANEL, DIALOG_STYLE_LIST,
        "لوحة EMS",
        "1. تبديل الحالة (On/Off Duty)\n2. عرض النداءات النشطة\n3. عرض سجلي الطبي",
        "اختيار", "إغلاق");
    return 1;
}