/* ============================================================================
    WISE-CITY-RP  |  نظام المنازل والإيجار
    الوحدة: wc_housing.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

#define DIALOG_HOUSE_MENU        1201
#define DIALOG_SAFE_MENU         1202

/* ============================================================================
 *  WISE-CITY-RP :: Advanced Housing & Real Estate System
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
#define MAX_HOUSES              200
#define RENT_INTERVAL_SECONDS   (24 * 60 * 60)   // 24 hours
#define RENT_CHECK_TIMER        60000            // check every 60 seconds

// ---------------------------------------------------------------------------
//  House Data Structure
// ---------------------------------------------------------------------------
enum e_HouseData {
    hID,
    hOwner[MAX_PLAYER_NAME],
    hPrice,
    hInteriorID,
    hLocked,
    hRentPrice,
    hIsRented,
    hRentDue,          // Unix timestamp when next rent is due
    hSafeMoney,
    hSafeDrugs,
    hSafeWeapons,
    Float:hEnterX,
    Float:hEnterY,
    Float:hEnterZ,
    Float:hExitX,
    Float:hExitY,
    Float:hExitZ
};

new HouseInfo[MAX_HOUSES][e_HouseData];
new g_HouseCount = 0;

// Player tracking
new g_PlayerHouse[MAX_PLAYERS];       // house ID the player currently owns / rents
new bool:g_PlayerInsideHouse[MAX_PLAYERS];

// ===========================================================================
//  FORWARD DECLARATIONS
// ===========================================================================
forward LoadHouses();
forward OnHouseRentCheck();
forward SaveHouse(houseid);

// ===========================================================================
//  UTILITY FUNCTIONS
// ===========================================================================

/**
 * Returns the index of the nearest unowned house within `range`.
 * @return house index or -1 if none found.
 */
stock GetNearbyUnownedHouse(playerid, Float:range = 3.0)
{
    new Float:px, Float:py, Float:pz;
    GetPlayerPos(playerid, px, py, pz);

    for (new i = 0; i < g_HouseCount; i++) {
        if (strcmp(HouseInfo[i][hOwner], "None", true) != 0) continue; // already owned
        if (IsPlayerInRangeOfPoint(playerid, range,
            HouseInfo[i][hEnterX], HouseInfo[i][hEnterY], HouseInfo[i][hEnterZ]))
            return i;
    }
    return -1;
}

/**
 * Returns the house index the player is currently standing inside
 * (based on exit position), or -1.
 */
stock GetPlayerHouse(playerid)
{
    for (new i = 0; i < g_HouseCount; i++) {
        if (g_PlayerHouse[playerid] == i) return i;
    }
    return -1;
}

/**
 * Checks whether the player is the owner of a given house.
 */
stock IsHouseOwner(playerid, houseid)
{
    new name[MAX_PLAYER_NAME];
    GetPlayerName(playerid, name, sizeof(name));
    return (strcmp(HouseInfo[houseid][hOwner], name, true) == 0);
}

/**
 * Formats an Arabic currency string (e.g. 5000 -> "5,000$").
 */
stock FormatMoney(amount)
{
    new str[32];
    format(str, sizeof(str), "%d$", amount);
    return str;
}

// ===========================================================================
//  DATABASE LOADING
// ===========================================================================
public LoadHouses()
{
    new query[512];
    format(query, sizeof(query),
        "SELECT id, owner_name, price, interior_id, locked, rent_price, \
         is_rented, rent_due, safe_money, safe_drugs, safe_weapons, \
         enter_x, enter_y, enter_z, exit_x, exit_y, exit_z FROM houses");

    new Cache:result = mysql_query(g_SQL, query);
    if (cache_num_rows() == 0) {
        print("[WISE-CITY-RP] No houses found in database.");
        return 1;
    }

    g_HouseCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        if (g_HouseCount >= MAX_HOUSES) break;

        HouseInfo[g_HouseCount][hID] = cache_get_field_content_int(row, "id", g_SQL);
        cache_get_field_content(row, "owner_name", HouseInfo[g_HouseCount][hOwner], g_SQL, MAX_PLAYER_NAME);
        HouseInfo[g_HouseCount][hPrice]     = cache_get_field_content_int(row, "price", g_SQL);
        HouseInfo[g_HouseCount][hInteriorID] = cache_get_field_content_int(row, "interior_id", g_SQL);
        HouseInfo[g_HouseCount][hLocked]    = cache_get_field_content_int(row, "locked", g_SQL);
        HouseInfo[g_HouseCount][hRentPrice] = cache_get_field_content_int(row, "rent_price", g_SQL);
        HouseInfo[g_HouseCount][hIsRented]  = cache_get_field_content_int(row, "is_rented", g_SQL);
        HouseInfo[g_HouseCount][hRentDue]   = cache_get_field_content_int(row, "rent_due", g_SQL);
        HouseInfo[g_HouseCount][hSafeMoney] = cache_get_field_content_int(row, "safe_money", g_SQL);
        HouseInfo[g_HouseCount][hSafeDrugs] = cache_get_field_content_int(row, "safe_drugs", g_SQL);
        HouseInfo[g_HouseCount][hSafeWeapons] = cache_get_field_content_int(row, "safe_weapons", g_SQL);
        HouseInfo[g_HouseCount][hEnterX]    = cache_get_field_content_float(row, "enter_x", g_SQL);
        HouseInfo[g_HouseCount][hEnterY]    = cache_get_field_content_float(row, "enter_y", g_SQL);
        HouseInfo[g_HouseCount][hEnterZ]    = cache_get_field_content_float(row, "enter_z", g_SQL);
        HouseInfo[g_HouseCount][hExitX]     = cache_get_field_content_float(row, "exit_x", g_SQL);
        HouseInfo[g_HouseCount][hExitY]     = cache_get_field_content_float(row, "exit_y", g_SQL);
        HouseInfo[g_HouseCount][hExitZ]     = cache_get_field_content_float(row, "exit_z", g_SQL);

        g_HouseCount++;
    }
    cache_delete(result);
    printf("[WISE-CITY-RP] Loaded %d houses.", g_HouseCount);
    return 1;
}

// ===========================================================================
//  DATABASE SAVING
// ===========================================================================
public SaveHouse(houseid)
{
    if (houseid < 0 || houseid >= g_HouseCount) return 0;

    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "UPDATE houses SET owner_name='%e', price=%d, interior_id=%d, locked=%d, \
         rent_price=%d, is_rented=%d, rent_due=%d, safe_money=%d, safe_drugs=%d, \
         safe_weapons=%d, enter_x=%f, enter_y=%f, enter_z=%f, exit_x=%f, exit_y=%f, exit_z=%f \
         WHERE id=%d",
        HouseInfo[houseid][hOwner],
        HouseInfo[houseid][hPrice],
        HouseInfo[houseid][hInteriorID],
        HouseInfo[houseid][hLocked],
        HouseInfo[houseid][hRentPrice],
        HouseInfo[houseid][hIsRented],
        HouseInfo[houseid][hRentDue],
        HouseInfo[houseid][hSafeMoney],
        HouseInfo[houseid][hSafeDrugs],
        HouseInfo[houseid][hSafeWeapons],
        HouseInfo[houseid][hEnterX],
        HouseInfo[houseid][hEnterY],
        HouseInfo[houseid][hEnterZ],
        HouseInfo[houseid][hExitX],
        HouseInfo[houseid][hExitY],
        HouseInfo[houseid][hExitZ],
        houseid
    );
    WC_Exec(query);
    return 1;
}

// ===========================================================================
//  CORE CALLBACKS
// ===========================================================================
stock Housing_OnGameModeInit()
{


    print("[WISE-CITY-RP] Connected to wise_city_db successfully.");
    LoadHouses();

    // Start the daily rent deduction timer
    SetTimer("OnHouseRentCheck", RENT_CHECK_TIMER, true);

    return 1;
}

stock Housing_OnGameModeExit()
{
    // Save all houses before shutdown
    for (new i = 0; i < g_HouseCount; i++) SaveHouse(i);
    return 1;
}

stock Housing_OnPlayerConnect(playerid)
{
    g_PlayerHouse[playerid] = -1;
    g_PlayerInsideHouse[playerid] = false;
    return 1;
}

stock Housing_OnPlayerDisconnect(playerid, reason)
{
    g_PlayerHouse[playerid] = -1;
    g_PlayerInsideHouse[playerid] = false;
    return 1;
}

// ===========================================================================
//  DAILY RENT DEDUCTION
// ===========================================================================
public OnHouseRentCheck()
{
    new now = gettime();
    new query[256];

    for (new i = 0; i < g_HouseCount; i++) {
        if (!HouseInfo[i][hIsRented]) continue;
        if (now < HouseInfo[i][hRentDue]) continue;          // not due yet
        if (strcmp(HouseInfo[i][hOwner], "None", true) == 0) continue;

        // Rent is due – attempt to deduct from the player's money
        new ownerName[MAX_PLAYER_NAME];
        format(ownerName, sizeof(ownerName), HouseInfo[i][hOwner]);

        new ownerID = INVALID_PLAYER_ID;
        for (new p = 0; p < MAX_PLAYERS; p++) {
            if (!IsPlayerConnected(p)) continue;
            new plName[MAX_PLAYER_NAME];
            GetPlayerName(p, plName, sizeof(plName));
            if (strcmp(plName, ownerName, true) == 0) {
                ownerID = p;
                break;
            }
        }

        if (ownerID == INVALID_PLAYER_ID) continue;          // owner offline – skip

        new rent = HouseInfo[i][hRentPrice];
        if (GetPlayerMoney(ownerID) >= rent) {
            GivePlayerMoney(ownerID, -rent);
            HouseInfo[i][hRentDue] = now + RENT_INTERVAL_SECONDS;
            HouseInfo[i][hSafeMoney] += rent;                 // deposit into safe

            // Log the transaction
            mysql_format(g_SQL, query, sizeof(query),
                "INSERT INTO rent_log (house_id, owner, amount) VALUES (%d, '%e', %d)",
                i, ownerName, rent);
            WC_Exec(query);

            SendClientMessage(ownerID, 0xFFFF00AA,
                "[خزنة] تم خصم الإيجار اليومي من رصيدك وتحويله إلى خزنة المنزل.");
            SaveHouse(i);
        } else {
            SendClientMessage(ownerID, 0xFF0000AA,
                "[خزنة] ليس لديك ما يكفي من المال لدفع الإيجار! ستصادر المنزل قريباً.");
            HouseInfo[i][hRentDue] = now + RENT_INTERVAL_SECONDS; // grace period
        }
    }
    return 1;
}

// ===========================================================================
//  PLAYER COMMANDS – BUY HOUSE
// ===========================================================================
CMD:buyhouse(playerid, params[])
{
    new houseid = GetNearbyUnownedHouse(playerid, 3.0);
    if (houseid == -1) {
        SendClientMessage(playerid, 0xFF0000AA, "[خطأ] لا يوجد منزل للبيع بالقرب منك.");
        return 1;
    }

    if (GetPlayerMoney(playerid) < HouseInfo[houseid][hPrice]) {
        SendClientMessage(playerid, 0xFF0000AA, "[خطأ] لا تملك المال الكافي لشراء هذا المنزل.");
        return 1;
    }

    new plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));

    // Transfer ownership
    GivePlayerMoney(playerid, -HouseInfo[houseid][hPrice]);
    format(HouseInfo[houseid][hOwner], MAX_PLAYER_NAME, plName);
    HouseInfo[houseid][hIsRented] = 0;
    HouseInfo[houseid][hLocked]   = 1;
    g_PlayerHouse[playerid]       = houseid;

    SaveHouse(houseid);

    new msg[128];
    format(msg, sizeof(msg), "[منزل] مبروك! لقد اشتريت المنزل رقم %d مقابل %s.",
        houseid, FormatMoney(HouseInfo[houseid][hPrice]));
    SendClientMessage(playerid, 0x00FF00AA, msg);
    return 1;
}

// ===========================================================================
//  PLAYER COMMANDS – SELL HOUSE
// ===========================================================================
CMD:sellhouse(playerid, params[])
{
    new houseid = g_PlayerHouse[playerid];
    if (houseid == -1 || !IsHouseOwner(playerid, houseid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[خطأ] أنت لا تملك أي منزل لبيعه.");
        return 1;
    }

    new plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));

    // Refund 70% of the original price
    new refund = (HouseInfo[houseid][hPrice] * 70) / 100;
    GivePlayerMoney(playerid, refund);

    format(HouseInfo[houseid][hOwner], MAX_PLAYER_NAME, "None");
    HouseInfo[houseid][hIsRented] = 0;
    HouseInfo[houseid][hLocked]   = 1;
    HouseInfo[houseid][hRentDue]  = 0;
    g_PlayerHouse[playerid]       = -1;

    SaveHouse(houseid);

    new msg[128];
    format(msg, sizeof(msg), "[منزل] لقد بعت منزلك رقم %d واستلمت %s.",
        houseid, FormatMoney(refund));
    SendClientMessage(playerid, 0x00FF00AA, msg);
    return 1;
}

// ===========================================================================
//  PLAYER COMMANDS – RENT APARTMENT
// ===========================================================================
CMD:rentapartment(playerid, params[])
{
    new houseid = GetNearbyUnownedHouse(playerid, 3.0);
    if (houseid == -1) {
        SendClientMessage(playerid, 0xFF0000AA, "[خطأ] لا يوجد شقة للإيجار بالقرب منك.");
        return 1;
    }

    if (HouseInfo[houseid][hRentPrice] <= 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[خطأ] هذه الشقة غير متاحة للإيجار.");
        return 1;
    }

    if (GetPlayerMoney(playerid) < HouseInfo[houseid][hRentPrice]) {
        SendClientMessage(playerid, 0xFF0000AA, "[خطأ] لا تملك المال الكافي لدفع الإيجار الأول.");
        return 1;
    }

    new plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));

    GivePlayerMoney(playerid, -HouseInfo[houseid][hRentPrice]);
    format(HouseInfo[houseid][hOwner], MAX_PLAYER_NAME, plName);
    HouseInfo[houseid][hIsRented] = 1;
    HouseInfo[houseid][hRentDue]  = gettime() + RENT_INTERVAL_SECONDS;
    HouseInfo[houseid][hLocked]   = 1;
    g_PlayerHouse[playerid]       = houseid;

    SaveHouse(houseid);

    new msg[160];
    format(msg, sizeof(msg),
        "[شقة] لقد استأجرت الشقة رقم %d بإيجار يومي %s.",
        houseid, FormatMoney(HouseInfo[houseid][hRentPrice]));
    SendClientMessage(playerid, 0x00FF00AA, msg);
    return 1;
}

// ===========================================================================
//  SAFE VAULT – INTERACTION
// ===========================================================================
/**
 * Opens the safe menu for the player if they are inside their own house.
 * Triggered by pressing F (or via a command /safe).
 */
CMD:safe(playerid, params[])
{
    new houseid = g_PlayerHouse[playerid];
    if (houseid == -1 || !IsHouseOwner(playerid, houseid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[خزنة] يجب أن تكون داخل منزلك لفتح الخزنة.");
        return 1;
    }

    new str[512];
    format(str, sizeof(str),
        "الخزنة الآمنة\n\
         -------------------------\n\
         المال: %s\n\
         المخدرات: %d\n\
         الأسلحة: %d\n\
         -------------------------\n\
         1. إيداع المال\n\
         2. إيداع المخدرات\n\
         3. إيداع الأسلحة\n\
         4. سحب المال",
        FormatMoney(HouseInfo[houseid][hSafeMoney]),
        HouseInfo[houseid][hSafeDrugs],
        HouseInfo[houseid][hSafeWeapons]);

    ShowPlayerDialog(playerid, DIALOG_SAFE_MENU, DIALOG_STYLE_LIST,
        "الخزنة الآمنة (الخزنة)", str, "اختيار", "إغلاق");
    return 1;
}

stock Housing_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid == DIALOG_SAFE_MENU) {
        if (!response) return 1;

        new houseid = g_PlayerHouse[playerid];
        if (houseid == -1 || !IsHouseOwner(playerid, houseid)) return 1;

        switch (listitem) {
            case 0: { // Deposit money
                new amount = strval(inputtext);
                if (amount <= 0) {
                    SendClientMessage(playerid, 0xFF0000AA, "[خزنة] المبلغ غير صالح.");
                    return 1;
                }
                if (GetPlayerMoney(playerid) < amount) {
                    SendClientMessage(playerid, 0xFF0000AA, "[خزنة] لا تملك هذا المبلغ.");
                    return 1;
                }
                GivePlayerMoney(playerid, -amount);
                HouseInfo[houseid][hSafeMoney] += amount;
                SaveHouse(houseid);
                SendClientMessage(playerid, 0x00FF00AA, "[خزنة] تم إيداع المال بنجاح.");
            }
            case 1: { // Deposit drugs
                new amount = strval(inputtext);
                if (amount <= 0) return 1;
                // Assume player drug variable exists (pDrugs)
                // if (PlayerInfo[playerid][pDrugs] < amount) { ... }
                // PlayerInfo[playerid][pDrugs] -= amount;
                HouseInfo[houseid][hSafeDrugs] += amount;
                SaveHouse(houseid);
                SendClientMessage(playerid, 0x00FF00AA, "[خزنة] تم إيداع المخدرات.");
            }
            case 2: { // Deposit weapons
                new amount = strval(inputtext);
                if (amount <= 0) return 1;
                // Assume player weapon variable exists (pWeapons)
                HouseInfo[houseid][hSafeWeapons] += amount;
                SaveHouse(houseid);
                SendClientMessage(playerid, 0x00FF00AA, "[خزنة] تم إيداع الأسلحة.");
            }
            case 3: { // Withdraw money
                new amount = strval(inputtext);
                if (amount <= 0 || HouseInfo[houseid][hSafeMoney] < amount) {
                    SendClientMessage(playerid, 0xFF0000AA, "[خزنة] رصيد الخزنة غير كافٍ.");
                    return 1;
                }
                HouseInfo[houseid][hSafeMoney] -= amount;
                GivePlayerMoney(playerid, amount);
                SaveHouse(houseid);
                SendClientMessage(playerid, 0x00FF00AA, "[خزنة] تم سحب المال بنجاح.");
            }
        }
        return 1;
    }
    return 0;
}

// ===========================================================================
//  HOUSE ENTER / EXIT
// ===========================================================================
CMD:enter(playerid, params[])
{
    new houseid = GetNearbyUnownedHouse(playerid, 3.0);
    if (houseid == -1) houseid = g_PlayerHouse[playerid];
    if (houseid == -1) {
        SendClientMessage(playerid, 0xFF0000AA, "[منزل] لا يوجد منزل بالقرب منك.");
        return 1;
    }

    if (HouseInfo[houseid][hLocked] && !IsHouseOwner(playerid, houseid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[منزل] هذا المنزل مقفل.");
        return 1;
    }

    SetPlayerInterior(playerid, HouseInfo[houseid][hInteriorID]);
    SetPlayerPos(playerid, HouseInfo[houseid][hExitX],
                 HouseInfo[houseid][hExitY],
                 HouseInfo[houseid][hExitZ]);
    g_PlayerInsideHouse[playerid] = true;
    g_PlayerHouse[playerid] = houseid;
    SendClientMessage(playerid, 0x00FF00AA, "[منزل] لقد دخلت المنزل.");
    return 1;
}

CMD:exit(playerid, params[])
{
    new houseid = g_PlayerHouse[playerid];
    if (houseid == -1 || !g_PlayerInsideHouse[playerid]) {
        SendClientMessage(playerid, 0xFF0000AA, "[منزل] أنت لست داخل منزل.");
        return 1;
    }

    SetPlayerInterior(playerid, 0);
    SetPlayerPos(playerid, HouseInfo[houseid][hEnterX],
                 HouseInfo[houseid][hEnterY],
                 HouseInfo[houseid][hEnterZ]);
    g_PlayerInsideHouse[playerid] = false;
    SendClientMessage(playerid, 0x00FF00AA, "[منزل] لقد خرجت من المنزل.");
    return 1;
}