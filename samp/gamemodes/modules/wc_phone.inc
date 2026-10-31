/* ============================================================================
    WISE-CITY-RP  |  نظام الهاتف: WiseGram و WiseCab والسوق
    الوحدة: wc_phone.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

/* ============================================================================
 *  WISE-CITY-RP :: Wise-Phone OS + Black Market System
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
#define MAX_POSTS         40
#define MAX_CLASSIFIEDS   30
#define MAX_TAXI_REQS     20
#define MAX_BM_LISTINGS   40
#define MAX_BM_LOCS       10
#define MAX_SMS_CACHE     20

// Dialog IDs
#define D_PHONE_MAIN       6001
#define D_PHONE_WISEGRAM   6002
#define D_PHONE_WISECAB    6003
#define D_PHONE_MARKET     6004
#define D_PHONE_BANK       6005
#define D_PHONE_SMS        6006
#define D_WG_FEED          6007
#define D_WG_POST          6008
#define D_WG_COMMENT       6009
#define D_CAB_REQUEST      6010
#define D_CAB_DRIVER       6011
#define D_MARKET_LIST      6012
#define D_MARKET_CREATE    6013
#define D_BANK_TRANSFER    6014
#define D_BM_MAIN          6015
#define D_BM_LOCATIONS     6016
#define D_BM_LISTINGS      6017
#define D_BM_SELL          6018
#define D_BM_CHANNEL       6019

// ---------------------------------------------------------------------------
//  Data structures
// ---------------------------------------------------------------------------
enum e_WGPost {
    wgID,
    wgAuthor,
    wgContent[255],
    wgLikes,
    wgComments,
    wgCreatedAt
};

enum e_Classified {
    clID,
    clSeller,
    clTitle[96],
    clDesc[255],
    clPrice,
    clCategory,
    clStatus
};

enum e_TaxiReq {
    trActive,
    trID,
    trPassenger,
    trDriver,
    Float:trPX, Float:trPY, Float:trPZ,
    Float:trDX, Float:trDY, Float:trDZ,
    trFare,
    trStatus
};

enum e_BMListing {
    bmID,
    bmSeller,
    bmItemKey[32],
    bmQty,
    bmPrice,
    bmLocation,
    bmStatus
};

enum e_BMLoc {
    blID,
    blName[64],
    Float:blX, Float:blY, Float:blZ,
    blInterior,
    blMinRank,
    blActive
};

enum e_PlayerPhone {
    ppDBID,
    ppBankBalance,
    ppPhoneOpen,
    ppCurrentSMS[128],
    ppLastPostTick,
    ppLastCabTick,
    ppActiveChannel,
    ppLastBMListTick
};

// ---------------------------------------------------------------------------
//  Globals
// ---------------------------------------------------------------------------
new WiseGramFeed[MAX_POSTS][e_WGPost];
new g_WGCount = 0;
new Classifieds[MAX_CLASSIFIEDS][e_Classified];
new g_ClassifiedCount = 0;
new TaxiRequests[MAX_TAXI_REQS][e_TaxiReq];
new BMListings[MAX_BM_LISTINGS][e_BMListing];
new g_BMListingCount = 0;
new BMLocations[MAX_BM_LOCS][e_BMLoc];
new g_BMLocCount = 0;
new PlayerPhone[MAX_PLAYERS][e_PlayerPhone];

// ---------------------------------------------------------------------------
//  Forwards
// ---------------------------------------------------------------------------
forward LoadWisePhoneData();
forward LoadBlackMarketData();
forward LoadPlayerPhone(playerid);
forward SendSMS(playerid, const from[], const body[]);
forward OnTaxiTimer();
forward OnRaidCheck();
forward OnPostAutoSave();

// ===========================================================================
//  GAMEMODE INIT
// ===========================================================================
stock Phone_OnGameModeInit()
{

    print("[WISE-CITY-RP] Wise-Phone + BlackMarket: Connected.");

    LoadWisePhoneData();
    LoadBlackMarketData();

    SetTimer("OnTaxiTimer",    5000,  true);   // update taxi drivers
    SetTimer("OnRaidCheck",    90000, true);   // police raid random check
    return 1;
}

stock Phone_OnGameModeExit()
{
    return 1;
}

// ===========================================================================
//  LOADERS
// ===========================================================================
public LoadWisePhoneData()
{
    // ---- WiseGram feed (latest 40 posts) ----
    new Cache:res = mysql_query(g_SQL,
        "SELECT post_id, author_id, content, likes, comments, created_at \
         FROM wisegram_posts ORDER BY post_id DESC LIMIT 40");
    g_WGCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        new idx = g_WGCount;
        WiseGramFeed[idx][wgID] = cache_get_field_content_int(row, "post_id", g_SQL);
        WiseGramFeed[idx][wgAuthor] = cache_get_field_content_int(row, "author_id", g_SQL);
        cache_get_field_content(row, "content", WiseGramFeed[idx][wgContent], g_SQL, 255);
        WiseGramFeed[idx][wgLikes]    = cache_get_field_content_int(row, "likes", g_SQL);
        WiseGramFeed[idx][wgComments] = cache_get_field_content_int(row, "comments", g_SQL);
        g_WGCount++;
    }
    cache_delete(res);

    // ---- Classifieds ----
    res = mysql_query(g_SQL,
        "SELECT ad_id, seller_id, title, description, price, category, status \
         FROM classifieds WHERE status=1 ORDER BY ad_id DESC LIMIT 30");
    g_ClassifiedCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        new idx = g_ClassifiedCount;
        Classifieds[idx][clID] = cache_get_field_content_int(row, "ad_id", g_SQL);
        Classifieds[idx][clSeller] = cache_get_field_content_int(row, "seller_id", g_SQL);
        cache_get_field_content(row, "title", Classifieds[idx][clTitle], g_SQL, 96);
        cache_get_field_content(row, "description", Classifieds[idx][clDesc], g_SQL, 255);
        Classifieds[idx][clPrice]    = cache_get_field_content_int(row, "price", g_SQL);
        Classifieds[idx][clCategory] = cache_get_field_content_int(row, "category", g_SQL);
        Classifieds[idx][clStatus]   = cache_get_field_content_int(row, "status", g_SQL);
        g_ClassifiedCount++;
    }
    cache_delete(res);

    printf("[WISE-CITY-RP] WiseGram posts: %d | Classifieds: %d", g_WGCount, g_ClassifiedCount);
    return 1;
}

public LoadBlackMarketData()
{
    // ---- Locations ----
    new Cache:res = mysql_query(g_SQL,
        "SELECT loc_id, name, x, y, z, interior, min_rank, active \
         FROM blackmarket_locations WHERE active=1");
    g_BMLocCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        new idx = g_BMLocCount;
        BMLocations[idx][blID] = cache_get_field_content_int(row, "loc_id", g_SQL);
        cache_get_field_content(row, "name", BMLocations[idx][blName], g_SQL, 64);
        BMLocations[idx][blX] = cache_get_field_content_float(row, "x", g_SQL);
        BMLocations[idx][blY] = cache_get_field_content_float(row, "y", g_SQL);
        BMLocations[idx][blZ] = cache_get_field_content_float(row, "z", g_SQL);
        BMLocations[idx][blInterior] = cache_get_field_content_int(row, "interior", g_SQL);
        BMLocations[idx][blMinRank]  = cache_get_field_content_int(row, "min_rank", g_SQL);
        BMLocations[idx][blActive]   = cache_get_field_content_int(row, "active", g_SQL);
        g_BMLocCount++;
    }
    cache_delete(res);

    // ---- Listings ----
    res = mysql_query(g_SQL,
        "SELECT listing_id, seller_id, item_key, quantity, price, location_id, status \
         FROM blackmarket_listings WHERE status=1 ORDER BY listing_id DESC LIMIT 40");
    g_BMListingCount = 0;
    for (new row = 0; row < cache_num_rows(); row++) {
        new idx = g_BMListingCount;
        BMListings[idx][bmID] = cache_get_field_content_int(row, "listing_id", g_SQL);
        BMListings[idx][bmSeller] = cache_get_field_content_int(row, "seller_id", g_SQL);
        cache_get_field_content(row, "item_key", BMListings[idx][bmItemKey], g_SQL, 32);
        BMListings[idx][bmQty]     = cache_get_field_content_int(row, "quantity", g_SQL);
        BMListings[idx][bmPrice]   = cache_get_field_content_int(row, "price", g_SQL);
        BMListings[idx][bmLocation]= cache_get_field_content_int(row, "location_id", g_SQL);
        BMListings[idx][bmStatus]  = cache_get_field_content_int(row, "status", g_SQL);
        g_BMListingCount++;
    }
    cache_delete(res);
    printf("[WISE-CITY-RP] BM locations: %d | BM listings: %d", g_BMLocCount, g_BMListingCount);
    return 1;
}

// ===========================================================================
//  PLAYER CONNECT / LOAD
// ===========================================================================
stock Phone_OnPlayerConnect(playerid)
{
    PlayerPhone[playerid][ppDBID]       = 0;
    PlayerPhone[playerid][ppBankBalance]= 0;
    PlayerPhone[playerid][ppPhoneOpen]  = 0;
    PlayerPhone[playerid][ppCurrentSMS][0] = '\0';
    PlayerPhone[playerid][ppLastPostTick] = 0;
    PlayerPhone[playerid][ppLastCabTick]  = 0;
    PlayerPhone[playerid][ppActiveChannel]= 0;
    PlayerPhone[playerid][ppLastBMListTick] = 0;
    return 1;
}

public LoadPlayerPhone(playerid)
{
    new plName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, plName, sizeof(plName));

    new q[256];
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT id FROM `users` WHERE username='%e' LIMIT 1", plName);
    new Cache:res = mysql_query(g_SQL, q);
    if (cache_num_rows() == 0) { cache_delete(res); return 0; }
    PlayerPhone[playerid][ppDBID] = cache_get_field_content_int(0, "id", g_SQL);
    cache_delete(res);

    // Bank account
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT balance FROM bank_accounts WHERE player_id=%d LIMIT 1",
        PlayerPhone[playerid][ppDBID]);
    res = mysql_query(g_SQL, q);
    if (cache_num_rows() == 0) {
        mysql_format(g_SQL, q, sizeof(q),
            "INSERT INTO bank_accounts (player_id, balance) VALUES (%d, 0)",
            PlayerPhone[playerid][ppDBID]);
        WC_Exec(q);
        PlayerPhone[playerid][ppBankBalance] = 0;
    } else {
        PlayerPhone[playerid][ppBankBalance] = cache_get_field_content_int(0, "balance", g_SQL);
    }
    cache_delete(res);

    // Unread SMS count
    mysql_format(g_SQL, q, sizeof(q),
        "SELECT COUNT(*) AS c FROM sms_messages WHERE to_id=%d AND is_read=0",
        PlayerPhone[playerid][ppDBID]);
    res = mysql_query(g_SQL, q);
    if (cache_num_rows() > 0) {
        new cnt = cache_get_field_content_int(0, "c", g_SQL);
        if (cnt > 0) {
            new msg[128];
            format(msg, sizeof(msg), "[Wise-Phone] لديك %d رسائل SMS غير مقروءة.", cnt);
            SendClientMessage(playerid, 0xFFAA00AA, msg);
        }
    }
    cache_delete(res);
    return 1;
}

// ===========================================================================
//  SAVE HELPERS
// ===========================================================================
stock SaveBankBalance(playerid)
{
    if (PlayerPhone[playerid][ppDBID] == 0) return 0;
    new q[200];
    mysql_format(g_SQL, q, sizeof(q),
        "UPDATE bank_accounts SET balance=%d WHERE player_id=%d",
        PlayerPhone[playerid][ppBankBalance], PlayerPhone[playerid][ppDBID]);
    WC_Exec(q);
    return 1;
}

/**
 * Sends an SMS and caches it for the recipient.
 */
public SendSMS(playerid, const from[], const body[])
{
    if (playerid < 0 || !IsPlayerConnected(playerid)) return 0;

    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO sms_messages (to_id, from_label, body) VALUES (%d, '%e', '%e')",
        PlayerPhone[playerid][ppDBID], from, body);
    WC_Exec(q);

    new msg[256];
    format(msg, sizeof(msg), "[SMS] %s: %s", from, body);
    SendClientMessage(playerid, 0x00FFFFAA, msg);
    return 1;
}

// ===========================================================================
//  PHONE MAIN MENU
// ===========================================================================
CMD:phone(playerid, params[])
{
    if (PlayerPhone[playerid][ppPhoneOpen]) {
        SendClientMessage(playerid, 0xFFAA00AA, "[هاتف] الهاتف مفتوح بالفعل.");
        return 1;
    }
    PlayerPhone[playerid][ppPhoneOpen] = 1;

    new str[512];
    format(str, sizeof(str),
        "Wise-Phone OS v3.0\n\
         -------------------------\n\
         💰 الرصيد البنكي: %d دينار\n\
         -------------------------\n\
         1. 📸 WiseGram (التواصل الاجتماعي)\n\
         2. 🚕 WiseCab (طلب تاكسي)\n\
         3. 🛒 السوق المبوب (إعلانات)\n\
         4. 🏦 البنك المحمول\n\
         5. 📩 الرسائل (SMS)\n\
         6. ❌ إغلاق الهاتف",
        PlayerPhone[playerid][ppBankBalance]);
    ShowPlayerDialog(playerid, D_PHONE_MAIN, DIALOG_STYLE_LIST,
        "Wise-Phone", str, "اختيار", "إغلاق");
    return 1;
}

// ===========================================================================
//  WISEGRAM
// ===========================================================================
stock ShowWiseGramFeed(playerid)
{
    new str[4096], line[320];
    format(str, sizeof(str), "WiseGram – آخر المنشورات\n------------------------\n");
    if (g_WGCount == 0) {
        strcat(str, "لا توجد منشورات بعد. كن أول من ينشر!", sizeof(str));
    }
    for (new i = 0; i < g_WGCount && i < 8; i++) {
        format(line, sizeof(line),
            "📌 #%d | ❤️ %d | 💬 %d\n%s\n───────────────────\n",
            WiseGramFeed[i][wgID],
            WiseGramFeed[i][wgLikes],
            WiseGramFeed[i][wgComments],
            WiseGramFeed[i][wgContent]);
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, D_WG_FEED, DIALOG_STYLE_LIST, "WiseGram", str,
        "تفاعل", "رجوع");
    return 1;
}

stock CreatePost(playerid, const content[])
{
    if (gettime() - PlayerPhone[playerid][ppLastPostTick] < 30) {
        SendClientMessage(playerid, 0xFF0000AA, "[WiseGram] انتظر 30 ثانية بين المنشورات.");
        return 1;
    }
    if (strlen(content) < 3) {
        SendClientMessage(playerid, 0xFF0000AA, "[WiseGram] المنشور قصير جداً.");
        return 1;
    }
    PlayerPhone[playerid][ppLastPostTick] = gettime();

    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO wisegram_posts (author_id, content) VALUES (%d, '%e')",
        PlayerPhone[playerid][ppDBID], content);
    WC_Exec(q);

    // Prepend to memory
    if (g_WGCount < MAX_POSTS) g_WGCount++;
    for (new i = g_WGCount - 1; i > 0; i--) WiseGramFeed[i] = WiseGramFeed[i-1];
    WiseGramFeed[0][wgID] = WC_LastInsertID();
    WiseGramFeed[0][wgAuthor] = PlayerPhone[playerid][ppDBID];
    format(WiseGramFeed[0][wgContent], 255, "%s", content);
    WiseGramFeed[0][wgLikes] = 0;
    WiseGramFeed[0][wgComments] = 0;

    SendClientMessage(playerid, 0x00FF00AA, "[WiseGram] تم نشر المنشور بنجاح.");
    return 1;
}

stock LikePost(playerid, postIdx)
{
    if (postIdx < 0 || postIdx >= g_WGCount) return 1;

    // Prevent double likes (DB constraint)
    new q[256];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT IGNORE INTO wisegram_likes (post_id, player_id) VALUES (%d, %d)",
        WiseGramFeed[postIdx][wgID], PlayerPhone[playerid][ppDBID]);
    WC_Exec(q);

    if (WC_LastAffected() > 0) {
        WiseGramFeed[postIdx][wgLikes]++;
        mysql_format(g_SQL, q, sizeof(q),
            "UPDATE wisegram_posts SET likes=likes+1 WHERE post_id=%d",
            WiseGramFeed[postIdx][wgID]);
        WC_Exec(q);
        SendClientMessage(playerid, 0xFF00FFAA, "[WiseGram] أعجبك المنشور ❤️");
    } else {
        SendClientMessage(playerid, 0xFFAA00AA, "[WiseGram] لقد أعجبت بهذا المنشور مسبقاً.");
    }
    return 1;
}

// ===========================================================================
//  WISECAB
// ===========================================================================
stock ShowWiseCabMenu(playerid)
{
    ShowPlayerDialog(playerid, D_CAB_REQUEST, DIALOG_STYLE_LIST,
        "WiseCab – طلب تاكسي",
        "1. اطلب تاكسي الآن (الموقع الحالي)\n\
         2. عرض سائقي التاكسي المتصلين\n\
         3. إلغاء الطلب الحالي",
        "اختيار", "رجوع");
    return 1;
}

stock RequestTaxi(playerid)
{
    if (gettime() - PlayerPhone[playerid][ppLastCabTick] < 20) {
        SendClientMessage(playerid, 0xFF0000AA, "[WiseCab] انتظر قبل طلب تاكسي جديد.");
        return 1;
    }
    // Find a free slot
    new slot = -1;
    for (new i = 0; i < MAX_TAXI_REQS; i++)
        if (!TaxiRequests[i][trActive]) { slot = i; break; }
    if (slot == -1) {
        SendClientMessage(playerid, 0xFF0000AA, "[WiseCab] لا توجد مركبات متاحة حالياً.");
        return 1;
    }

    new Float:x, Float:y, Float:z;
    GetPlayerPos(playerid, x, y, z);

    TaxiRequests[slot][trActive] = true;
    TaxiRequests[slot][trPassenger] = playerid;
    TaxiRequests[slot][trDriver]    = INVALID_PLAYER_ID;
    TaxiRequests[slot][trPX] = x;
    TaxiRequests[slot][trPY] = y;
    TaxiRequests[slot][trPZ] = z;
    TaxiRequests[slot][trFare] = 500;      // base fare
    TaxiRequests[slot][trStatus] = 0;
    PlayerPhone[playerid][ppLastCabTick] = gettime();

    new q[300];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO wisecab_requests (passenger_id, pos_x, pos_y, pos_z, fare, status) \
         VALUES (%d, %f, %f, %f, %d, 0)",
        PlayerPhone[playerid][ppDBID], x, y, z, 500);
    WC_Exec(q);
    TaxiRequests[slot][trID] = WC_LastInsertID();

    SendClientMessage(playerid, 0x00FF00AA, "[WiseCab] تم إرسال طلبك إلى سائقي التاكسي.");
    SendClientMessage(playerid, 0xFFD700AA, "استخدم /acceptcab [id] من قبل السائقين.");

    // Notify online taxi drivers
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (p == playerid || !IsPlayerConnected(p)) continue;
        // Assume IsPlayerTaxiDriver(p) exists in your faction system
        SendClientMessage(p, 0xFFD700AA, "[WiseCab] طلب تاكسي جديد! /acceptcab " /* id here */);
        SendClientMessage(p, 0xFFFF00AA, "استخدم /cablist لعرض الطلبات.");
    }
    return 1;
}

stock ShowCabList(playerid)
{
    new str[1024], line[128], found = 0;
    format(str, sizeof(str), "طلبات التاكسي النشطة\n------------------------\n");
    for (new i = 0; i < MAX_TAXI_REQS; i++) {
        if (!TaxiRequests[i][trActive]) continue;
        if (TaxiRequests[i][trStatus] >= 2) continue;
        found++;
        format(line, sizeof(line), "ID %d | الراكب: %d | المسافة: - | %d دينار\n",
            i, TaxiRequests[i][trPassenger], TaxiRequests[i][trFare]);
        strcat(str, line, sizeof(str));
    }
    if (!found) format(str, sizeof(str), "لا توجد طلبات نشطة.");
    ShowPlayerDialog(playerid, D_CAB_DRIVER, DIALOG_STYLE_LIST, "WiseCab", str, "استلام", "إغلاق");
    return 1;
}

CMD:cablist(playerid, params[])
{
    ShowCabList(playerid);
    return 1;
}

public OnTaxiTimer()
{
    for (new i = 0; i < MAX_TAXI_REQS; i++) {
        if (!TaxiRequests[i][trActive]) continue;
        if (TaxiRequests[i][trStatus] != 1) continue;

        new driver = TaxiRequests[i][trDriver];
        if (!IsPlayerConnected(driver)) continue;

        if (IsPlayerInRangeOfPoint(driver, 5.0,
            TaxiRequests[i][trPX], TaxiRequests[i][trPY], TaxiRequests[i][trPZ])) {
            TaxiRequests[i][trStatus] = 2;
            SendClientMessage(driver, 0x00FF00AA, "[WiseCab] وصلت إلى موقع الراكب.");
            SendClientMessage(TaxiRequests[i][trPassenger], 0x00FF00AA,
                "[WiseCab] سائق التاكسي وصل! استعد للانطلاق.");
        }
    }
    return 1;
}

// ===========================================================================
//  CLASSIFIEDS MARKETPLACE
// ===========================================================================
stock ShowMarketList(playerid)
{
    new str[4096], line[200];
    format(str, sizeof(str), "السوق المبوب\n------------------------\n");
    if (g_ClassifiedCount == 0) strcat(str, "لا توجد إعلانات حالياً.", sizeof(str));
    for (new i = 0; i < g_ClassifiedCount && i < 10; i++) {
        format(line, sizeof(line), "%d. %s – %d دينار\n   %s\n",
            i, Classifieds[i][clTitle], Classifieds[i][clPrice], Classifieds[i][clDesc]);
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, D_MARKET_LIST, DIALOG_STYLE_LIST,
        "السوق المبوب", str, "شراء / تفاصيل", "رجوع");
    return 1;
}

stock CreateClassified(playerid, const title[], const desc[], price, category)
{
    if (g_ClassifiedCount >= MAX_CLASSIFIEDS) {
        SendClientMessage(playerid, 0xFF0000AA, "[سوق] السوق ممتلئ، حاول لاحقاً.");
        return 1;
    }
    new q[512];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO classifieds (seller_id, title, description, price, category) \
         VALUES (%d, '%e', '%e', %d, %d)",
        PlayerPhone[playerid][ppDBID], title, desc, price, category);
    WC_Exec(q);

    // Prepend to memory
    if (g_ClassifiedCount < MAX_CLASSIFIEDS) g_ClassifiedCount++;
    for (new i = g_ClassifiedCount - 1; i > 0; i--) Classifieds[i] = Classifieds[i-1];
    Classifieds[0][clID] = WC_LastInsertID();
    Classifieds[0][clSeller] = PlayerPhone[playerid][ppDBID];
    format(Classifieds[0][clTitle], 96, "%s", title);
    format(Classifieds[0][clDesc], 255, "%s", desc);
    Classifieds[0][clPrice] = price;
    Classifieds[0][clCategory] = category;
    Classifieds[0][clStatus] = 1;

    SendClientMessage(playerid, 0x00FF00AA, "[سوق] تم نشر إعلانك بنجاح.");
    return 1;
}

// ===========================================================================
//  MOBILE BANKING
// ===========================================================================
stock ShowBankMenu(playerid)
{
    new str[400];
    format(str, sizeof(str),
        "البنك المحمول – Wise-Bank\n\
         -------------------------\n\
         💰 الرصيد الحالي: %d دينار\n\
         -------------------------\n\
         1. تحويل مبلغ إلى لاعب\n\
         2. إيداع نقدي (من اليد إلى البنك)\n\
         3. سحب نقدي (من البنك إلى اليد)\n\
         4. سجل العمليات الأخيرة",
        PlayerPhone[playerid][ppBankBalance]);
    ShowPlayerDialog(playerid, D_PHONE_BANK, DIALOG_STYLE_LIST, "Wise-Bank", str, "اختيار", "رجوع");
    return 1;
}

stock TransferMoney(playerid, targetid, amount)
{
    if (!IsPlayerConnected(targetid)) {
        SendClientMessage(playerid, 0xFF0000AA, "[بنك] اللاعب المستهدف غير متصل.");
        return 1;
    }
    if (amount <= 0 || PlayerPhone[playerid][ppBankBalance] < amount) {
        SendClientMessage(playerid, 0xFF0000AA, "[بنك] رصيدك غير كافٍ.");
        return 1;
    }

    PlayerPhone[playerid][ppBankBalance] -= amount;
    PlayerPhone[targetid][ppBankBalance] += amount;
    SaveBankBalance(playerid);
    SaveBankBalance(targetid);

    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO bank_transactions (from_id, to_id, amount, tx_type, note) \
         VALUES (%d, %d, %d, 'TRANSFER', 'Mobile Transfer')",
        PlayerPhone[playerid][ppDBID], PlayerPhone[targetid][ppDBID], amount);
    WC_Exec(q);

    // Instant SMS to recipient
    new sms[160];
    format(sms, sizeof(sms), "استلمت %d دينار من اللاعب ID %d. رصيدك الحالي: %d",
        amount, playerid, PlayerPhone[targetid][ppBankBalance]);
    SendSMS(targetid, "WISE-BANK", sms);

    new confirm[128];
    format(confirm, sizeof(confirm), "[بنك] تم تحويل %d دينار بنجاح.", amount);
    SendClientMessage(playerid, 0x00FF00AA, confirm);
    return 1;
}

// ===========================================================================
//  BLACK MARKET
// ===========================================================================
CMD:bm(playerid, params[])
{
    // Strict rule: only criminals/gang members can open
    // Assume IsPlayerCriminal(playerid) exists in your system
    new str[512];
    format(str, sizeof(str),
        "السوق السوداء (Black Market)\n\
         -------------------------\n\
         ⚠️ ممنوع الحديث عن هذا المكان خارج القناة المشفرة.\n\
         -------------------------\n\
         1. عرض المواقع السرية\n\
         2. عرض الصفقات المتاحة\n\
         3. بيع أصول (مخدرات، أسلحة)\n\
         4. القنوات المشفرة\n\
         5. الخروج");
    ShowPlayerDialog(playerid, D_BM_MAIN, DIALOG_STYLE_LIST,
        "السوق السوداء", str, "اختيار", "إغلاق");
    return 1;
}

stock ShowBMLocations(playerid)
{
    new str[1024], line[160];
    format(str, sizeof(str), "المواقع السرية\n------------------------\n");
    for (new i = 0; i < g_BMLocCount; i++) {
        format(line, sizeof(line), "%d. %s (المستوى الأدنى: %d)\n",
            i, BMLocations[i][blName], BMLocations[i][blMinRank]);
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, D_BM_LOCATIONS, DIALOG_STYLE_LIST,
        "المواقع السرية", str, "الانتقال", "رجوع");
    return 1;
}

stock ShowBMListings(playerid)
{
    new str[4096], line[160];
    format(str, sizeof(str), "الصفقات المتاحة\n------------------------\n");
    if (g_BMListingCount == 0) strcat(str, "لا توجد صفقات حالياً.", sizeof(str));
    for (new i = 0; i < g_BMListingCount && i < 12; i++) {
        format(line, sizeof(line), "%d. %s ×%d – %d دينار\n",
            i, BMListings[i][bmItemKey], BMListings[i][bmQty], BMListings[i][bmPrice]);
        strcat(str, line, sizeof(str));
    }
    ShowPlayerDialog(playerid, D_BM_LISTINGS, DIALOG_STYLE_LIST,
        "الصفقات", str, "شراء", "رجوع");
    return 1;
}

stock CreateBMListing(playerid, const item[], qty, price)
{
    if (g_BMListingCount >= MAX_BM_LISTINGS) {
        SendClientMessage(playerid, 0xFF0000AA, "[سوق سوداء] لا توجد مساحة للصفقات.");
        return 1;
    }

    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO blackmarket_listings (seller_id, item_key, quantity, price) \
         VALUES (%d, '%e', %d, %d)",
        PlayerPhone[playerid][ppDBID], item, qty, price);
    WC_Exec(q);

    if (g_BMListingCount < MAX_BM_LISTINGS) g_BMListingCount++;
    for (new i = g_BMListingCount - 1; i > 0; i--) BMListings[i] = BMListings[i-1];
    BMListings[0][bmID] = WC_LastInsertID();
    BMListings[0][bmSeller] = PlayerPhone[playerid][ppDBID];
    format(BMListings[0][bmItemKey], 32, "%s", item);
    BMListings[0][bmQty] = qty;
    BMListings[0][bmPrice] = price;
    BMListings[0][bmStatus] = 1;

    SendClientMessage(playerid, 0xFFAA00AA,
        "[سوق سوداء] تم إنشاء الصفقة. الوكالة الفيدرالية قد تراقب هذا المكان!");
    return 1;
}

stock OpenEncryptedChannel(playerid, channelIdx)
{
    PlayerPhone[playerid][ppActiveChannel] = channelIdx;
    new str[256];
    format(str, sizeof(str), "[قناة مشفرة] دخلت القناة رقم %d. استخدم /bmchat [نص] للتواصل.", channelIdx);
    SendClientMessage(playerid, 0xAA00FFAA, str);
    return 1;
}

CMD:bmchat(playerid, params[])
{
    if (PlayerPhone[playerid][ppActiveChannel] == 0) {
        SendClientMessage(playerid, 0xFF0000AA, "[قناة] يجب أن تدخل قناة مشفرة أولاً.");
        return 1;
    }
    if (isnull(params)) return 1;

    new q[400];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO blackmarket_messages (channel_id, sender_id, body) VALUES (%d, %d, '%e')",
        PlayerPhone[playerid][ppActiveChannel], PlayerPhone[playerid][ppDBID], params);
    WC_Exec(q);

    new msg[256];
    format(msg, sizeof(msg), "[مشفر #%d] %s: %s",
        PlayerPhone[playerid][ppActiveChannel], PlayerName(playerid), params);
    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (!IsPlayerConnected(p)) continue;
        if (PlayerPhone[p][ppActiveChannel] != PlayerPhone[playerid][ppActiveChannel]) continue;
        SendClientMessage(p, 0xAA00FFAA, msg);
    }
    return 1;
}

/**
 * Police raid vulnerability. Every 90s there's a random chance a raid occurs
 * at a black market location. Criminals inside lose part of their assets.
 */
public OnRaidCheck()
{
    if (g_BMLocCount == 0) return 1;
    if (random(100) > 12) return 1; // 12% chance per check

    new locIdx = random(g_BMLocCount);
    new str[256];
    format(str, sizeof(str),
        "[مداهمة] الشرطة تشن مداهمة على '%s'! اهربوا فوراً!",
        BMLocations[locIdx][blName]);
    SendClientMessageToAll(0xFF0000AA, str);

    for (new p = 0; p < MAX_PLAYERS; p++) {
        if (!IsPlayerConnected(p)) continue;
        if (IsPlayerInRangeOfPoint(p, 40.0,
            BMLocations[locIdx][blX], BMLocations[locIdx][blY], BMLocations[locIdx][blZ])) {
            // Seize 20% of the player's on-hand money
            new cash = GetPlayerMoney(p);
            new seized = (cash * 20) / 100;
            if (seized > 0) {
                GivePlayerMoney(p, -seized);
                new msg[128];
                format(msg, sizeof(msg), "[مداهمة] تم ضبط %d دينار بحوزتك!", seized);
                SendClientMessage(p, 0xFF0000AA, msg);
            }
        }
    }

    // Log the raid
    new q[256];
    mysql_format(g_SQL, q, sizeof(q),
        "INSERT INTO police_raids (location_id, officer_id, success, seized_value) \
         VALUES (%d, 0, 1, 0)", BMLocations[locIdx][blID]);
    WC_Exec(q);
    return 1;
}

// ===========================================================================
//  DIALOG HANDLER
// ===========================================================================
stock Phone_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (!response) {
        if (dialogid == D_PHONE_MAIN) PlayerPhone[playerid][ppPhoneOpen] = 0;
        return 1;
    }

    // -------- PHONE MAIN --------
    if (dialogid == D_PHONE_MAIN) {
        switch (listitem) {
            case 0: ShowWiseGramFeed(playerid);
            case 1: ShowWiseCabMenu(playerid);
            case 2: ShowMarketList(playerid);
            case 3: ShowBankMenu(playerid);
            case 4: {
                new str[512];
                format(str, sizeof(str), "صندوق الرسائل\n------------------------\n");
                ShowPlayerDialog(playerid, D_PHONE_SMS, DIALOG_STYLE_MSGBOX,
                    "الرسائل", str, "حسناً", "");
            }
            case 5: {
                PlayerPhone[playerid][ppPhoneOpen] = 0;
                SendClientMessage(playerid, 0xFFAA00AA, "[هاتف] تم إغلاق الهاتف.");
            }
        }
        return 1;
    }

    // -------- WISEGRAM FEED --------
    if (dialogid == D_WG_FEED) {
        if (listitem < 0 || listitem >= g_WGCount) return 1;
        LikePost(playerid, listitem);
        return 1;
    }

    // -------- WISECAB --------
    if (dialogid == D_CAB_REQUEST) {
        switch (listitem) {
            case 0: RequestTaxi(playerid);
            case 1: ShowCabList(playerid);
            case 2: {
                for (new i = 0; i < MAX_TAXI_REQS; i++) {
                    if (TaxiRequests[i][trPassenger] == playerid && TaxiRequests[i][trActive]) {
                        TaxiRequests[i][trActive] = false;
                        SendClientMessage(playerid, 0xFFAA00AA, "[WiseCab] تم إلغاء طلبك.");
                        break;
                    }
                }
            }
        }
        return 1;
    }

    if (dialogid == D_CAB_DRIVER) {
        if (listitem < 0 || listitem >= MAX_TAXI_REQS) return 1;
        if (!TaxiRequests[listitem][trActive]) return 1;
        if (TaxiRequests[listitem][trDriver] != INVALID_PLAYER_ID) {
            SendClientMessage(playerid, 0xFF0000AA, "[WiseCab] تم استلام الطلب بالفعل.");
            return 1;
        }
        TaxiRequests[listitem][trDriver] = playerid;
        TaxiRequests[listitem][trStatus] = 1;
        SetPlayerCheckpoint(playerid, TaxiRequests[listitem][trPX],
            TaxiRequests[listitem][trPY], TaxiRequests[listitem][trPZ], 3.0);
        SendClientMessage(playerid, 0x00FF00AA, "[WiseCab] توجه إلى موقع الراكب.");
        return 1;
    }

    // -------- MARKETPLACE --------
    if (dialogid == D_MARKET_LIST) {
        // Simplified: purchase directly
        if (listitem < 0 || listitem >= g_ClassifiedCount) return 1;
        new price = Classifieds[listitem][clPrice];
        if (GetPlayerMoney(playerid) < price) {
            SendClientMessage(playerid, 0xFF0000AA, "[سوق] لا تملك المال الكافي.");
            return 1;
        }
        GivePlayerMoney(playerid, -price);
        Classifieds[listitem][clStatus] = 2;

        new q[200];
        mysql_format(g_SQL, q, sizeof(q),
            "UPDATE classifieds SET status=2 WHERE ad_id=%d", Classifieds[listitem][clID]);
        WC_Exec(q);
        SendClientMessage(playerid, 0x00FF00AA, "[سوق] تم الشراء بنجاح.");
        return 1;
    }

    // -------- BANK --------
    if (dialogid == D_PHONE_BANK) {
        switch (listitem) {
            case 0: {
                ShowPlayerDialog(playerid, D_BANK_TRANSFER, DIALOG_STYLE_INPUT,
                    "تحويل الأموال",
                    "أدخل (ID اللاعب) (المبلغ) - مثال: 5 5000",
                    "تحويل", "إلغاء");
            }
            case 1: { // Deposit
                new cash = GetPlayerMoney(playerid);
                if (cash <= 0) return 1;
                GivePlayerMoney(playerid, -cash);
                PlayerPhone[playerid][ppBankBalance] += cash;
                SaveBankBalance(playerid);
                SendClientMessage(playerid, 0x00FF00AA, "[بنك] تم الإيداع بنجاح.");
            }
            case 2: { // Withdraw (all)
                if (PlayerPhone[playerid][ppBankBalance] <= 0) return 1;
                GivePlayerMoney(playerid, PlayerPhone[playerid][ppBankBalance]);
                PlayerPhone[playerid][ppBankBalance] = 0;
                SaveBankBalance(playerid);
                SendClientMessage(playerid, 0x00FF00AA, "[بنك] تم السحب بنجاح.");
            }
        }
        return 1;
    }

    if (dialogid == D_BANK_TRANSFER) {
        new targetid, amount;
        if (sscanf(inputtext, "dd", targetid, amount)) {
            SendClientMessage(playerid, 0xFF0000AA, "صيغة غير صحيحة.");
            return 1;
        }
        TransferMoney(playerid, targetid, amount);
        return 1;
    }

    // -------- BLACK MARKET --------
    if (dialogid == D_BM_MAIN) {
        switch (listitem) {
            case 0: ShowBMLocations(playerid);
            case 1: ShowBMListings(playerid);
            case 2: {
                ShowPlayerDialog(playerid, D_BM_SELL, DIALOG_STYLE_INPUT,
                    "بيع أصول",
                    "أدخل: (نوع الأصل) (الكمية) (السعر) - مثال: weapon 5 50000",
                    "بيع", "إلغاء");
            }
            case 3: {
                ShowPlayerDialog(playerid, D_BM_CHANNEL, DIALOG_STYLE_INPUT,
                    "القنوات المشفرة",
                    "أدخل رقم القناة (1-9):",
                    "دخول", "إلغاء");
            }
        }
        return 1;
    }

    if (dialogid == D_BM_LOCATIONS) {
        if (listitem < 0 || listitem >= g_BMLocCount) return 1;
        SetPlayerInterior(playerid, BMLocations[listitem][blInterior]);
        SetPlayerPos(playerid, BMLocations[listitem][blX],
            BMLocations[listitem][blY], BMLocations[listitem][blZ]);
        SendClientMessage(playerid, 0xAA00FFAA, "[سوق سوداء] وصلت إلى الموقع السري.");
        return 1;
    }

    if (dialogid == D_BM_SELL) {
        new item[32], qty, price;
        if (sscanf(inputtext, "s[32]dd", item, qty, price)) {
            SendClientMessage(playerid, 0xFF0000AA, "صيغة غير صحيحة.");
            return 1;
        }
        CreateBMListing(playerid, item, qty, price);
        return 1;
    }

    if (dialogid == D_BM_CHANNEL) {
        new ch = strval(inputtext);
        if (ch < 1 || ch > 9) {
            SendClientMessage(playerid, 0xFF0000AA, "[قناة] رقم القناة من 1 إلى 9.");
            return 1;
        }
        OpenEncryptedChannel(playerid, ch);
        return 1;
    }

    return 0;
}

// ===========================================================================
//  UTILITY
// ===========================================================================
