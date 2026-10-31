/* ============================================================================
    WISE-CITY-RP  |  نظام السيارات والمعرض والمزادات
    الوحدة: wc_vehicles.inc  (تُضمَّن من gamemodes/wise_city.pwn - لا تُصرَّف وحدها)
   ============================================================================ */

#define DIALOG_MY_CARS           1100
#define DIALOG_CONFIRM_BUY       1101
#define DIALOG_AUCTIONS_LIST     1102

/*
 * ============================================================================
 *  WISE-CITY-RP | نظام السيارات الشامل وسوق السيارات وقاعة المزادات
 *  ملف: Wise_City_Vehicles_Auction.pwn
 *  الإصدار: 1.0.0
 *  التوافق: open.mp | Pawn 3.10.12+ | MySQL R41-4+
 *  الوصف: نظام ملكية سيارات، معرض سيارات، مزادات علنية مع حفظ آمن
 * ============================================================================
 */

// ============================================================================
// 1. التعريفات والثوابت الأساسية
// ============================================================================

#define SCRIPT_VERSION              "1.0.0"
#define MAX_OWNED_VEHICLES          5       // الحد الأقصى للسيارات المملوكة للاعب
#define MAX_VEHICLE_SLOTS           2000    // الحد الأقصى لخانات السيارات في السيرفر
#define MAX_AUCTION_SLOTS           50      // الحد الأقصى للمزادات المتزامنة
#define AUCTION_TAX_PERCENT         5       // نسبة ضريبة المزاد %
#define FUEL_MAX                    100.0
#define DAMAGE_MAX                  1000.0

// أنواع العملات
#define CURRENCY_DINAR              0
#define CURRENCY_GOLD               1

// حالات المزاد
#define AUCTION_STATUS_CLOSED       0
#define AUCTION_STATUS_OPEN         1

// ألوان الرسائل

// ============================================================================
// 2. التضمينات (Includes)
// ============================================================================


// ============================================================================
// 3. المتغيرات العامة
// ============================================================================

// اتصال قاعدة البيانات

// بيانات السيارات المملوكة
enum E_VEHICLE_DATA
{
    vID,                    // معرف السيارة في قاعدة البيانات
    vOwnerID,               // معرف المالك (0 = للبيع في المعرض)
    vModel,                 // موديل السيارة
    vColor1,
    vColor2,
    Float:vPosX,
    Float:vPosY,
    Float:vPosZ,
    Float:vAngle,
    Float:vFuel,
    Float:vDamage,
    vPlate[16],
    vLocked,                // 0 = مفتوحة، 1 = مقفلة
    vInsurance,             // 0 = لا يوجد، 1 = يوجد
    vSpawnedID,             // معرف السيارة الفعلي في اللعبة (0 = غير مستدعاة)
    vInAuction,             // 0 = لا، 1 = في مزاد
    vPrice,                 // سعر البيع (إذا كانت معروضة)
    vCurrency,              // العملة: دينار أو ذهب
    bool:vForSale           // معروضة للبيع المباشر؟
}
new VehicleData[MAX_VEHICLE_SLOTS][E_VEHICLE_DATA];

// بيانات اللاعب
enum E_VPLAYER
{
    pOwnedVehicles[MAX_OWNED_VEHICLES], // معرفات السيارات المملوكة
    pOwnedCount,
    pAuctionID,
    pAuctionBid,            // قيمة المزايدة المحجوزة
    pAuctionVehicle,
    pInAuctionArea,
    pLastAuctionReceipt
}
new VPlayer[MAX_PLAYERS][E_VPLAYER];

// بيانات المزادات
enum E_AUCTION_DATA
{
    aID,
    aVehicleDBID,           // معرف السيارة في قاعدة البيانات
    aSellerID,              // معرف البائع
    aStartPrice,            // السعر الابتدائي
    aCurrentBid,            // أعلى مزايدة حالياً
    aHighestBidder,         // معرف أعلى مزايد
    aEndTime,               // وقت انتهاء المزاد (timestamp)
    aStatus,                // مفتوح / مغلق
    bool:aActive,           // هل المزاد نشط؟
    aReceiptID[32]          // معرف الإيصال
}
new AuctionData[MAX_AUCTION_SLOTS][E_AUCTION_DATA];
new g_AuctionCount = 0;

// مؤقتات
new g_AuctionTimer;
new g_VehicleSaveTimer;

// ============================================================================
// 4. النماذج (Forward Declarations)
// ============================================================================

forward LoadPlayerVehicles(playerid);
forward SaveVehicle(vehicleDBID);
forward SaveAllVehicles();
forward CreateVehicleFromDB(vehicleDBID);
forward DestroyPlayerVehicle(playerid, slot);
forward SpawnPlayerVehicle(playerid, slot, Float:x, Float:y, Float:z);
forward ProcessAuctionEnd(auctionID);
forward CheckAuctions();
forward OnAuctionTimer();
forward OnVehicleSaveTimer();

// ============================================================================
// 5. دوال قاعدة البيانات (MySQL)
// ============================================================================

// --------------------------------------------------
// الاتصال بقاعدة البيانات
// --------------------------------------------------
stock Veh_OnGameModeInit()
{
    // إعداد اتصال MySQL


    print("[WISE-CITY] تم الاتصال بقاعدة البيانات بنجاح.");

    // إنشاء الجداول إذا لم تكن موجودة
    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `vehicles` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `owner_id` INT(11) NOT NULL DEFAULT 0, \
        `model` INT(11) NOT NULL, \
        `color1` INT(11) NOT NULL DEFAULT 1, \
        `color2` INT(11) NOT NULL DEFAULT 1, \
        `pos_x` FLOAT NOT NULL DEFAULT 0.0, \
        `pos_y` FLOAT NOT NULL DEFAULT 0.0, \
        `pos_z` FLOAT NOT NULL DEFAULT 0.0, \
        `angle` FLOAT NOT NULL DEFAULT 0.0, \
        `fuel` FLOAT NOT NULL DEFAULT 100.0, \
        `damage` FLOAT NOT NULL DEFAULT 0.0, \
        `plate` VARCHAR(16) NOT NULL DEFAULT 'WISE-CITY', \
        `locked` TINYINT(1) NOT NULL DEFAULT 0, \
        `insurance` TINYINT(1) NOT NULL DEFAULT 0, \
        `for_sale` TINYINT(1) NOT NULL DEFAULT 0, \
        `price` INT(11) NOT NULL DEFAULT 0, \
        `currency` TINYINT(1) NOT NULL DEFAULT 0, \
        `in_auction` TINYINT(1) NOT NULL DEFAULT 0, \
        PRIMARY KEY (`id`), \
        INDEX `idx_owner` (`owner_id`), \
        INDEX `idx_for_sale` (`for_sale`), \
        INDEX `idx_auction` (`in_auction`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `auctions` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `vehicle_id` INT(11) NOT NULL, \
        `seller_id` INT(11) NOT NULL, \
        `start_price` INT(11) NOT NULL DEFAULT 0, \
        `current_bid` INT(11) NOT NULL DEFAULT 0, \
        `highest_bidder` INT(11) NOT NULL DEFAULT 0, \
        `end_time` INT(11) NOT NULL DEFAULT 0, \
        `status` TINYINT(1) NOT NULL DEFAULT 1, \
        `receipt_id` VARCHAR(32) NOT NULL DEFAULT '', \
        PRIMARY KEY (`id`), \
        INDEX `idx_status` (`status`), \
        INDEX `idx_vehicle` (`vehicle_id`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    mysql_tquery(g_SQL,
        "CREATE TABLE IF NOT EXISTS `auction_bids` ( \
        `id` INT(11) NOT NULL AUTO_INCREMENT, \
        `auction_id` INT(11) NOT NULL, \
        `bidder_id` INT(11) NOT NULL, \
        `amount` INT(11) NOT NULL, \
        `bid_time` INT(11) NOT NULL, \
        PRIMARY KEY (`id`), \
        INDEX `idx_auction` (`auction_id`), \
        INDEX `idx_bidder` (`bidder_id`) \
        ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;");

    // تحميل جميع السيارات من قاعدة البيانات
    mysql_tquery(g_SQL, "SELECT * FROM `vehicles`", "LoadAllVehicles", "");

    // بدء المؤقتات
    g_AuctionTimer = SetTimer("OnAuctionTimer", 1000, true);       // كل ثانية للتحقق من المزادات
    g_VehicleSaveTimer = SetTimer("OnVehicleSaveTimer", 300000, true); // كل 5 دقائق حفظ تلقائي

    print("[WISE-CITY] تم تحميل نظام السيارات والمزادات بنجاح.");
    return 1;
}

// --------------------------------------------------
// تحميل جميع السيارات من قاعدة البيانات
// --------------------------------------------------
forward LoadAllVehicles();
public LoadAllVehicles()
{
    new rows = cache_num_rows();
    new count = 0;

    for (new i = 0; i < rows; i++)
    {
        if (count >= MAX_VEHICLE_SLOTS) break;

        VehicleData[count][vID]          = cache_get_field_content_int(i, "id", g_SQL);
        VehicleData[count][vOwnerID]     = cache_get_field_content_int(i, "owner_id", g_SQL);
        VehicleData[count][vModel]       = cache_get_field_content_int(i, "model", g_SQL);
        VehicleData[count][vColor1]      = cache_get_field_content_int(i, "color1", g_SQL);
        VehicleData[count][vColor2]      = cache_get_field_content_int(i, "color2", g_SQL);
        VehicleData[count][vPosX]        = cache_get_field_content_float(i, "pos_x", g_SQL);
        VehicleData[count][vPosY]        = cache_get_field_content_float(i, "pos_y", g_SQL);
        VehicleData[count][vPosZ]        = cache_get_field_content_float(i, "pos_z", g_SQL);
        VehicleData[count][vAngle]       = cache_get_field_content_float(i, "angle", g_SQL);
        VehicleData[count][vFuel]        = cache_get_field_content_float(i, "fuel", g_SQL);
        VehicleData[count][vDamage]      = cache_get_field_content_float(i, "damage", g_SQL);
        cache_get_field_content(i, "plate", VehicleData[count][vPlate], g_SQL, 16);
        VehicleData[count][vLocked]      = cache_get_field_content_int(i, "locked", g_SQL);
        VehicleData[count][vInsurance]   = cache_get_field_content_int(i, "insurance", g_SQL);
        VehicleData[count][vForSale]     = bool:cache_get_field_content_int(i, "for_sale", g_SQL);
        VehicleData[count][vPrice]       = cache_get_field_content_int(i, "price", g_SQL);
        VehicleData[count][vCurrency]    = cache_get_field_content_int(i, "currency", g_SQL);
        VehicleData[count][vInAuction]   = cache_get_field_content_int(i, "in_auction", g_SQL);
        VehicleData[count][vSpawnedID]   = 0;

        count++;
    }

    printf("[WISE-CITY] تم تحميل %d سيارة من قاعدة البيانات.", count);

    // تحميل المزادات النشطة
    mysql_tquery(g_SQL, "SELECT * FROM `auctions` WHERE `status` = 1", "LoadActiveAuctions", "");
    return 1;
}

// --------------------------------------------------
// تحميل المزادات النشطة
// --------------------------------------------------
forward LoadActiveAuctions();
public LoadActiveAuctions()
{
    new rows = cache_num_rows();
    g_AuctionCount = 0;

    for (new i = 0; i < rows; i++)
    {
        if (g_AuctionCount >= MAX_AUCTION_SLOTS) break;

        AuctionData[g_AuctionCount][aID]            = cache_get_field_content_int(i, "id", g_SQL);
        AuctionData[g_AuctionCount][aVehicleDBID]   = cache_get_field_content_int(i, "vehicle_id", g_SQL);
        AuctionData[g_AuctionCount][aSellerID]      = cache_get_field_content_int(i, "seller_id", g_SQL);
        AuctionData[g_AuctionCount][aStartPrice]    = cache_get_field_content_int(i, "start_price", g_SQL);
        AuctionData[g_AuctionCount][aCurrentBid]    = cache_get_field_content_int(i, "current_bid", g_SQL);
        AuctionData[g_AuctionCount][aHighestBidder] = cache_get_field_content_int(i, "highest_bidder", g_SQL);
        AuctionData[g_AuctionCount][aEndTime]       = cache_get_field_content_int(i, "end_time", g_SQL);
        AuctionData[g_AuctionCount][aStatus]        = cache_get_field_content_int(i, "status", g_SQL);
        cache_get_field_content(i, "receipt_id", AuctionData[g_AuctionCount][aReceiptID], g_SQL, 32);
        AuctionData[g_AuctionCount][aActive]        = true;

        g_AuctionCount++;
    }

    printf("[WISE-CITY] تم تحميل %d مزاد نشط.", g_AuctionCount);
    return 1;
}

// --------------------------------------------------
// حفظ سيارة محددة
// --------------------------------------------------
public SaveVehicle(vehicleDBID)
{
    new query[1024];
    new idx = GetVehicleIndexByDBID(vehicleDBID);

    if (idx == -1) return 0;

    // إذا كانت السيارة مستدعاة، تحديث إحداثياتها
    if (VehicleData[idx][vSpawnedID] != 0)
    {
        new Float:x, Float:y, Float:z, Float:a;
        GetVehiclePos(VehicleData[idx][vSpawnedID], x, y, z);
        GetVehicleZAngle(VehicleData[idx][vSpawnedID], a);
        VehicleData[idx][vPosX] = x;
        VehicleData[idx][vPosY] = y;
        VehicleData[idx][vPosZ] = z;
        VehicleData[idx][vAngle] = a;

        // حفظ الوقود والضرر إذا كان النظام يدعمها
        // VehicleData[idx][vFuel] = GetVehicleFuel(VehicleData[idx][vSpawnedID]);
        // VehicleData[idx][vDamage] = GetVehicleDamage(VehicleData[idx][vSpawnedID]);
    }

    mysql_format(g_SQL, query, sizeof(query),
        "UPDATE `vehicles` SET \
        `owner_id` = %d, \
        `model` = %d, \
        `color1` = %d, \
        `color2` = %d, \
        `pos_x` = %f, \
        `pos_y` = %f, \
        `pos_z` = %f, \
        `angle` = %f, \
        `fuel` = %f, \
        `damage` = %f, \
        `plate` = '%e', \
        `locked` = %d, \
        `insurance` = %d, \
        `for_sale` = %d, \
        `price` = %d, \
        `currency` = %d, \
        `in_auction` = %d \
        WHERE `id` = %d",
        VehicleData[idx][vOwnerID],
        VehicleData[idx][vModel],
        VehicleData[idx][vColor1],
        VehicleData[idx][vColor2],
        VehicleData[idx][vPosX],
        VehicleData[idx][vPosY],
        VehicleData[idx][vPosZ],
        VehicleData[idx][vAngle],
        VehicleData[idx][vFuel],
        VehicleData[idx][vDamage],
        VehicleData[idx][vPlate],
        VehicleData[idx][vLocked],
        VehicleData[idx][vInsurance],
        VehicleData[idx][vForSale] ? 1 : 0,
        VehicleData[idx][vPrice],
        VehicleData[idx][vCurrency],
        VehicleData[idx][vInAuction],
        vehicleDBID
    );

    mysql_tquery(g_SQL, query, "", "");
    return 1;
}

// --------------------------------------------------
// حفظ جميع السيارات (يُستدعى بشكل دوري)
// --------------------------------------------------
public SaveAllVehicles()
{
    for (new i = 0; i < MAX_VEHICLE_SLOTS; i++)
    {
        if (VehicleData[i][vID] != 0)
        {
            SaveVehicle(VehicleData[i][vID]);
        }
    }
    print("[WISE-CITY] تم حفظ جميع السيارات تلقائياً.");
    return 1;
}

// ============================================================================
// 6. دوال مساعدة
// ============================================================================

// --------------------------------------------------
// الحصول على فهرس السيارة بواسطة معرف قاعدة البيانات
// --------------------------------------------------
stock GetVehicleIndexByDBID(dbID)
{
    for (new i = 0; i < MAX_VEHICLE_SLOTS; i++)
    {
        if (VehicleData[i][vID] == dbID) return i;
    }
    return -1;
}

// --------------------------------------------------
// الحصول على فهرس سيارة فارغة
// --------------------------------------------------
stock GetFreeVehicleSlot()
{
    for (new i = 0; i < MAX_VEHICLE_SLOTS; i++)
    {
        if (VehicleData[i][vID] == 0) return i;
    }
    return -1;
}

// --------------------------------------------------
// الحصول على فهرس لاعب بواسطة معرف قاعدة البيانات
// --------------------------------------------------
stock GetPlayerIndexByDBID(dbID)
{
    for (new i = 0; i < MAX_PLAYERS; i++)
    {
        if (IsPlayerConnected(i) && pData[i][pID] == dbID) return i;
    }
    return -1;
}

// --------------------------------------------------
// إرسال رسالة خاصة
// --------------------------------------------------
stock SendMessage(playerid, color, const msg[])
{
    SendClientMessage(playerid, color, msg);
    return 1;
}

// --------------------------------------------------
// إنشاء سيارة في اللعبة من بيانات قاعدة البيانات
// --------------------------------------------------
public CreateVehicleFromDB(vehicleDBID)
{
    new idx = GetVehicleIndexByDBID(vehicleDBID);
    if (idx == -1) return 0;

    if (VehicleData[idx][vSpawnedID] != 0)
    {
        DestroyVehicle(VehicleData[idx][vSpawnedID]);
        VehicleData[idx][vSpawnedID] = 0;
    }

    new vehicleid = CreateVehicle(
        VehicleData[idx][vModel],
        VehicleData[idx][vPosX],
        VehicleData[idx][vPosY],
        VehicleData[idx][vPosZ],
        VehicleData[idx][vAngle],
        VehicleData[idx][vColor1],
        VehicleData[idx][vColor2],
        -1, // respawn delay
        VehicleData[idx][vLocked] ? 1 : 0
    );

    if (vehicleid != INVALID_VEHICLE_ID)
    {
        VehicleData[idx][vSpawnedID] = vehicleid;
        SetVehicleNumberPlate(vehicleid, VehicleData[idx][vPlate]);
        ChangeVehicleColor(vehicleid, VehicleData[idx][vColor1], VehicleData[idx][vColor2]);
    }

    return 1;
}

// ============================================================================
// 7. نظام الملكية والمرائب (Garages & Ownership)
// ============================================================================

// --------------------------------------------------
// تحميل بيانات اللاعب
// --------------------------------------------------






// --------------------------------------------------
// تحميل سيارات اللاعب
// --------------------------------------------------
public LoadPlayerVehicles(playerid)
{
    new query[256];
    mysql_format(g_SQL, query, sizeof(query),
        "SELECT `id` FROM `vehicles` WHERE `owner_id` = %d",
        pData[playerid][pID]
    );
    mysql_tquery(g_SQL, query, "OnPlayerVehiclesLoaded", "d", playerid);
    return 1;
}

forward OnPlayerVehiclesLoaded(playerid);
public OnPlayerVehiclesLoaded(playerid)
{
    new rows = cache_num_rows();
    VPlayer[playerid][pOwnedCount] = 0;

    for (new i = 0; i < rows; i++)
    {
        if (VPlayer[playerid][pOwnedCount] >= MAX_OWNED_VEHICLES) break;

        new dbID = cache_get_field_content_int(i, "id", g_SQL);
        new idx = GetVehicleIndexByDBID(dbID);

        if (idx != -1)
        {
            VPlayer[playerid][pOwnedVehicles][VPlayer[playerid][pOwnedCount]] = dbID;
            VPlayer[playerid][pOwnedCount]++;
        }
    }

    printf("[WISE-CITY] اللاعب %s لديه %d سيارة.", pData[playerid][pName], VPlayer[playerid][pOwnedCount]);
    return 1;
}

// --------------------------------------------------
// حفظ بيانات اللاعب
// --------------------------------------------------


// --------------------------------------------------
// استدعاء السيارة من المرآب
// --------------------------------------------------
CMD:spawncar(playerid, params[])
{
    if (isnull(params))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /spawncar [رقم السيارة في القائمة]");

    new slot = strval(params);
    if (slot < 1 || slot > VPlayer[playerid][pOwnedCount])
        return SendMessage(playerid, COLOR_RED, "[خطأ] رقم سيارة غير صالح.");

    new dbID = VPlayer[playerid][pOwnedVehicles][slot - 1];
    new idx = GetVehicleIndexByDBID(dbID);

    if (idx == -1)
        return SendMessage(playerid, COLOR_RED, "[خطأ] السيارة غير موجودة في قاعدة البيانات.");

    if (VehicleData[idx][vSpawnedID] != 0)
        return SendMessage(playerid, COLOR_YELLOW, "[تنبيه] السيارة مستدعاة بالفعل.");

    // استدعاء السيارة أمام اللاعب
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(playerid, x, y, z);
    GetPlayerFacingAngle(playerid, a);

    x += 3.0 * floatsin(-a, degrees);
    y += 3.0 * floatcos(-a, degrees);

    new vehicleid = CreateVehicle(
        VehicleData[idx][vModel],
        x, y, z, a,
        VehicleData[idx][vColor1],
        VehicleData[idx][vColor2],
        -1,
        VehicleData[idx][vLocked] ? 1 : 0
    );

    if (vehicleid == INVALID_VEHICLE_ID)
        return SendMessage(playerid, COLOR_RED, "[خطأ] فشل استدعاء السيارة. حاول مرة أخرى.");

    VehicleData[idx][vSpawnedID] = vehicleid;
    SetVehicleNumberPlate(vehicleid, VehicleData[idx][vPlate]);

    SendMessage(playerid, COLOR_GREEN, "[WISE-CITY] تم استدعاء سيارتك بنجاح. استخدم /parkcar لإعادتها.");
    return 1;
}

// --------------------------------------------------
// إعادة السيارة إلى المرآب
// --------------------------------------------------
CMD:parkcar(playerid, params[])
{
    // البحث عن سيارة اللاعب المستدعاة حالياً
    for (new i = 0; i < VPlayer[playerid][pOwnedCount]; i++)
    {
        new dbID = VPlayer[playerid][pOwnedVehicles][i];
        new idx = GetVehicleIndexByDBID(dbID);

        if (idx != -1 && VehicleData[idx][vSpawnedID] != 0)
        {
            // التحقق من أن اللاعب قريب من السيارة
            if (IsPlayerInVehicle(playerid, VehicleData[idx][vSpawnedID]))
            {
                // حفظ الموقع والوقود والضرر قبل التدمير
                new Float:x, Float:y, Float:z, Float:a;
                GetVehiclePos(VehicleData[idx][vSpawnedID], x, y, z);
                GetVehicleZAngle(VehicleData[idx][vSpawnedID], a);

                VehicleData[idx][vPosX] = x;
                VehicleData[idx][vPosY] = y;
                VehicleData[idx][vPosZ] = z;
                VehicleData[idx][vAngle] = a;

                DestroyVehicle(VehicleData[idx][vSpawnedID]);
                VehicleData[idx][vSpawnedID] = 0;

                SaveVehicle(dbID);
                SendMessage(playerid, COLOR_GREEN, "[WISE-CITY] تم إعادة سيارتك إلى المرآب بنجاح.");
                return 1;
            }
        }
    }

    SendMessage(playerid, COLOR_RED, "[خطأ] أنت لست داخل سيارة مملوكة لك.");
    return 1;
}

// --------------------------------------------------
// عرض قائمة سيارات اللاعب
// --------------------------------------------------
CMD:mycars(playerid, params[])
{
    if (VPlayer[playerid][pOwnedCount] == 0)
        return SendMessage(playerid, COLOR_YELLOW, "[WISE-CITY] ليس لديك أي سيارات. زر معرض السيارات لشراء واحدة.");

    new string[512];
    strcat(string, "{00FFFF}===== سياراتك =====\n");
    strcat(string, "{FFFFFF}الرقم | الموديل | اللوحة | الحالة\n");

    for (new i = 0; i < VPlayer[playerid][pOwnedCount]; i++)
    {
        new dbID = VPlayer[playerid][pOwnedVehicles][i];
        new idx = GetVehicleIndexByDBID(dbID);

        if (idx != -1)
        {
            new status[32];
            if (VehicleData[idx][vSpawnedID] != 0)
                format(status, sizeof(status), "{00FF00}مستدعاة");
            else
                format(status, sizeof(status), "{FFA500}في المرآب");

            new line[128];
            format(line, sizeof(line), "%d | %d | %s | %s\n", i + 1, VehicleData[idx][vModel], VehicleData[idx][vPlate], status);
            strcat(string, line);
        }
    }

    ShowPlayerDialog(playerid, DIALOG_MY_CARS, DIALOG_STYLE_MSGBOX, "سياراتك", string, "حسناً", "");
    return 1;
}

// ============================================================================
// 8. سوق السيارات المعروضة (Car Dealership)
// ============================================================================

// --------------------------------------------------
// عرض السيارة للبيع المباشر
// --------------------------------------------------
CMD:sellcar(playerid, params[])
{
    new targetid, price, currency;
    if (sscanf(params, "uii", targetid, price, currency))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /sellcar [اللاعب] [السعر] [0=دينار / 1=ذهب]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (price <= 0)
        return SendMessage(playerid, COLOR_RED, "[خطأ] السعر يجب أن يكون أكبر من صفر.");

    if (currency < 0 || currency > 1)
        return SendMessage(playerid, COLOR_RED, "[خطأ] العملة غير صالحة. استخدم 0 للدينار أو 1 للذهب.");

    // البحث عن سيارة اللاعب المستدعاة
    new found = 0;
    for (new i = 0; i < VPlayer[playerid][pOwnedCount]; i++)
    {
        new dbID = VPlayer[playerid][pOwnedVehicles][i];
        new idx = GetVehicleIndexByDBID(dbID);

        if (idx != -1 && VehicleData[idx][vSpawnedID] != 0)
        {
            if (IsPlayerInVehicle(playerid, VehicleData[idx][vSpawnedID]))
            {
                // فتح حوار التأكيد للطرفين
                new string[256];
                format(string, sizeof(string),
                    "اللاعب %s يريد بيعك سيارته (موديل %d) بسعر %d %s.\n\nهل توافق على الشراء؟",
                    pData[playerid][pName],
                    VehicleData[idx][vModel],
                    price,
                    currency == 0 ? "دينار" : "ذهب"
                );

                ShowPlayerDialog(targetid, DIALOG_CONFIRM_BUY, DIALOG_STYLE_MSGBOX, "تأكيد الشراء", string, "أوافق", "أرفض");

                // تخزين مؤقت لبيانات البيع
                VPlayer[targetid][pAuctionVehicle] = dbID;
                VPlayer[targetid][pAuctionBid] = price;

                found = 1;
                break;
            }
        }
    }

    if (!found)
        return SendMessage(playerid, COLOR_RED, "[خطأ] يجب أن تكون داخل سيارة مملوكة لك لبيعها.");

    return 1;
}

// --------------------------------------------------
// معالجة تأكيد الشراء (من حوار DIALOG_CONFIRM_BUY)
// --------------------------------------------------
forward OnPlayerDialogBuyResponse(playerid, response);
public OnPlayerDialogBuyResponse(playerid, response)
{
    if (response)
    {
        new dbID = VPlayer[playerid][pAuctionVehicle];
        new idx = GetVehicleIndexByDBID(dbID);
        new price = VPlayer[playerid][pAuctionBid];

        if (idx == -1)
        {
            SendMessage(playerid, COLOR_RED, "[خطأ] السيارة غير موجودة.");
            return 1;
        }

        new sellerid = GetPlayerIndexByDBID(VehicleData[idx][vOwnerID]);
        if (sellerid == -1)
        {
            SendMessage(playerid, COLOR_RED, "[خطأ] البائع غير متصل.");
            return 1;
        }

        // التحقق من الرصيد
        if (VehicleData[idx][vCurrency] == 0) // دينار
        {
            if (GetPlayerMoney(playerid) < price)
                return SendMessage(playerid, COLOR_RED, "[خطأ] رصيدك من الدينار غير كافٍ.");

            // خصم من المشتري
            GivePlayerMoney(playerid, -(price));
            // إضافة للبائع
            GivePlayerMoney(sellerid, price);
        }
        else // ذهب
        {
            if (pData[playerid][pMoneyGold] < price)
                return SendMessage(playerid, COLOR_RED, "[خطأ] رصيدك من الذهب غير كافٍ.");

            pData[playerid][pMoneyGold] -= price;
            pData[sellerid][pMoneyGold] += price;
        }

        // نقل الملكية
        new oldOwner = VehicleData[idx][vOwnerID];
        VehicleData[idx][vOwnerID] = pData[playerid][pID];

        // إزالة السيارة من قائمة البائع
        for (new i = 0; i < VPlayer[sellerid][pOwnedCount]; i++)
        {
            if (VPlayer[sellerid][pOwnedVehicles][i] == dbID)
            {
                // إزاحة العناصر
                for (new j = i; j < VPlayer[sellerid][pOwnedCount] - 1; j++)
                {
                    VPlayer[sellerid][pOwnedVehicles][j] = VPlayer[sellerid][pOwnedVehicles][j + 1];
                }
                VPlayer[sellerid][pOwnedCount]--;
                break;
            }
        }

        // إضافة السيارة لقائمة المشتري
        if (VPlayer[playerid][pOwnedCount] < MAX_OWNED_VEHICLES)
        {
            VPlayer[playerid][pOwnedVehicles][VPlayer[playerid][pOwnedCount]] = dbID;
            VPlayer[playerid][pOwnedCount]++;
        }

        // حفظ التغييرات في قاعدة البيانات
        SaveVehicle(dbID);
        Veh_SavePlayerVehicles(sellerid);
        Veh_SavePlayerVehicles(playerid);

        // إرسال رسائل التأكيد
        new string[256];
        format(string, sizeof(string), "[WISE-CITY] تم شراء السيارة (موديل %d) بنجاح من %s بمبلغ %d %s.",
            VehicleData[idx][vModel], pData[sellerid][pName], price,
            VehicleData[idx][vCurrency] == 0 ? "دينار" : "ذهب");
        SendMessage(playerid, COLOR_GREEN, string);

        format(string, sizeof(string), "[WISE-CITY] تم بيع سيارتك (موديل %d) إلى %s بمبلغ %d %s.",
            VehicleData[idx][vModel], pData[playerid][pName], price,
            VehicleData[idx][vCurrency] == 0 ? "دينار" : "ذهب");
        SendMessage(sellerid, COLOR_GREEN, string);

        // إعادة تعيين المتغيرات المؤقتة
        VPlayer[playerid][pAuctionVehicle] = 0;
        VPlayer[playerid][pAuctionBid] = 0;
    }
    else
    {
        SendMessage(playerid, COLOR_YELLOW, "[WISE-CITY] تم رفض عملية الشراء.");
    }

    return 1;
}

// --------------------------------------------------
// إضافة سيارة جديدة إلى قاعدة البيانات (للمعرض)
// --------------------------------------------------
stock CreateVehicleDB(model, color1, color2, Float:x, Float:y, Float:z, Float:a, const plate[], price, currency)
{
    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `vehicles` (`owner_id`, `model`, `color1`, `color2`, `pos_x`, `pos_y`, `pos_z`, `angle`, `plate`, `price`, `currency`, `for_sale`) \
        VALUES (0, %d, %d, %d, %f, %f, %f, %f, '%e', %d, %d, 1)",
        model, color1, color2, x, y, z, a, plate, price, currency
    );
    mysql_tquery(g_SQL, query, "OnVehicleCreated", "");
    return 1;
}

forward OnVehicleCreated();
public OnVehicleCreated()
{
    new dbID = cache_insert_id();
    new idx = GetFreeVehicleSlot();

    if (idx != -1)
    {
        VehicleData[idx][vID]        = dbID;
        VehicleData[idx][vOwnerID]   = 0;
        VehicleData[idx][vModel]     = 411; // سيارة افتراضية
        VehicleData[idx][vColor1]    = 1;
        VehicleData[idx][vColor2]    = 1;
        VehicleData[idx][vPosX]      = 0.0;
        VehicleData[idx][vPosY]      = 0.0;
        VehicleData[idx][vPosZ]      = 0.0;
        VehicleData[idx][vAngle]     = 0.0;
        VehicleData[idx][vFuel]      = FUEL_MAX;
        VehicleData[idx][vDamage]    = 0.0;
        format(VehicleData[idx][vPlate], 16, "WISE-CITY");
        VehicleData[idx][vLocked]    = 0;
        VehicleData[idx][vForSale]   = true;
        VehicleData[idx][vSpawnedID] = 0;
    }

    printf("[WISE-CITY] تم إنشاء سيارة جديدة في قاعدة البيانات (ID: %d).", dbID);
    return 1;
}

// ============================================================================
// 9. نظام المزادات العلنية (Auction System)
// ============================================================================

// --------------------------------------------------
// إنشاء مزاد جديد
// --------------------------------------------------
CMD:create_auction(playerid, params[])
{
    new startPrice, duration;
    if (sscanf(params, "ii", startPrice, duration))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /create_auction [السعر الابتدائي] [المدة بالدقائق]");

    if (startPrice <= 0)
        return SendMessage(playerid, COLOR_RED, "[خطأ] السعر الابتدائي يجب أن يكون أكبر من صفر.");

    if (duration < 1 || duration > 60)
        return SendMessage(playerid, COLOR_RED, "[خطأ] المدة يجب أن تكون بين 1 و 60 دقيقة.");

    // البحث عن سيارة اللاعب
    new found = 0;
    for (new i = 0; i < VPlayer[playerid][pOwnedCount]; i++)
    {
        new dbID = VPlayer[playerid][pOwnedVehicles][i];
        new idx = GetVehicleIndexByDBID(dbID);

        if (idx != -1 && VehicleData[idx][vSpawnedID] != 0)
        {
            if (IsPlayerInVehicle(playerid, VehicleData[idx][vSpawnedID]))
            {
                // التحقق من أن السيارة ليست في مزاد آخر
                if (VehicleData[idx][vInAuction] == 1)
                    return SendMessage(playerid, COLOR_RED, "[خطأ] هذه السيارة معروضة في مزاد آخر بالفعل.");

                // إنشاء المزاد
                if (g_AuctionCount >= MAX_AUCTION_SLOTS)
                    return SendMessage(playerid, COLOR_RED, "[خطأ] لا توجد خانات مزاد متاحة حالياً.");

                new auctionID = g_AuctionCount;
                new endTime = gettime() + (duration * 60);

                AuctionData[auctionID][aID]            = auctionID + 1;
                AuctionData[auctionID][aVehicleDBID]   = dbID;
                AuctionData[auctionID][aSellerID]      = pData[playerid][pID];
                AuctionData[auctionID][aStartPrice]    = startPrice;
                AuctionData[auctionID][aCurrentBid]    = 0;
                AuctionData[auctionID][aHighestBidder] = 0;
                AuctionData[auctionID][aEndTime]       = endTime;
                AuctionData[auctionID][aStatus]        = AUCTION_STATUS_OPEN;
                AuctionData[auctionID][aActive]        = true;
                AuctionData[auctionID][aReceiptID][0]  = EOS;

                // تحديث بيانات السيارة
                VehicleData[idx][vInAuction] = 1;
                VehicleData[idx][vForSale] = false;

                // حفظ المزاد في قاعدة البيانات
                new query[512];
                mysql_format(g_SQL, query, sizeof(query),
                    "INSERT INTO `auctions` (`vehicle_id`, `seller_id`, `start_price`, `end_time`, `status`) \
                    VALUES (%d, %d, %d, %d, %d)",
                    dbID, pData[playerid][pID], startPrice, endTime, AUCTION_STATUS_OPEN
                );
                mysql_tquery(g_SQL, query, "OnAuctionCreated", "d", auctionID);

                g_AuctionCount++;

                // إرسال رسائل الإشعار
                new string[256];
                format(string, sizeof(string),
                    "[WISE-CITY] تم إنشاء مزاد جديد! السيارة: موديل %d | السعر الابتدائي: %d دينار | المدة: %d دقيقة",
                    VehicleData[idx][vModel], startPrice, duration);
                SendClientMessageToAll(COLOR_CYAN, string);

                format(string, sizeof(string),
                    "[WISE-CITY] استخدم /bid [المبلغ] للمزايدة. أعلى مزايد يفوز بالسيارة عند انتهاء الوقت.");
                SendClientMessageToAll(COLOR_YELLOW, string);

                found = 1;
                break;
            }
        }
    }

    if (!found)
        return SendMessage(playerid, COLOR_RED, "[خطأ] يجب أن تكون داخل سيارة مملوكة لك لإنشاء مزاد عليها.");

    return 1;
}

forward OnAuctionCreated(auctionID);
public OnAuctionCreated(auctionID)
{
    new dbAuctionID = cache_insert_id();
    AuctionData[auctionID][aID] = dbAuctionID;
    printf("[WISE-CITY] تم إنشاء مزاد جديد (ID: %d).", dbAuctionID);
    return 1;
}

// --------------------------------------------------
// المزايدة على المزاد
// --------------------------------------------------
CMD:bid(playerid, params[])
{
    new amount;
    if (sscanf(params, "i", amount))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /bid [المبلغ]");

    if (amount <= 0)
        return SendMessage(playerid, COLOR_RED, "[خطأ] يجب أن يكون المبلغ أكبر من صفر.");

    // البحث عن مزاد نشط
    new auctionID = -1;
    for (new i = 0; i < g_AuctionCount; i++)
    {
        if (AuctionData[i][aActive] && AuctionData[i][aStatus] == AUCTION_STATUS_OPEN)
        {
            auctionID = i;
            break;
        }
    }

    if (auctionID == -1)
        return SendMessage(playerid, COLOR_RED, "[خطأ] لا يوجد مزاد نشط حالياً.");

    if (amount <= AuctionData[auctionID][aCurrentBid])
        return SendMessage(playerid, COLOR_RED, "[خطأ] يجب أن تكون مزايدتك أعلى من أعلى مزايدة حالية.");

    if (amount < AuctionData[auctionID][aStartPrice])
        return SendMessage(playerid, COLOR_RED, "[خطأ] يجب أن تكون مزايدتك أعلى من السعر الابتدائي.");

    // التحقق من الرصيد
    if (GetPlayerMoney(playerid) < amount)
        return SendMessage(playerid, COLOR_RED, "[خطأ] رصيدك غير كافٍ للمزايدة بهذا المبلغ.");

    // رد المبلغ المحجوز للمزايد السابق
    if (AuctionData[auctionID][aHighestBidder] != 0)
    {
        new prevBidder = GetPlayerIndexByDBID(AuctionData[auctionID][aHighestBidder]);
        if (prevBidder != -1)
        {
            GivePlayerMoney(prevBidder, AuctionData[auctionID][aCurrentBid]);
            VPlayer[prevBidder][pAuctionBid] = 0;
            VPlayer[prevBidder][pAuctionVehicle] = 0;

            new string[128];
            format(string, sizeof(string), "[WISE-CITY] تم رد مبلغ %d دينار إلى حسابك لأن مزايدتك تجاوزت.", AuctionData[auctionID][aCurrentBid]);
            SendMessage(prevBidder, COLOR_YELLOW, string);
        }
    }

    // حجز المبلغ من المزايد الجديد
    GivePlayerMoney(playerid, -(amount));

    // تحديث بيانات المزاد
    AuctionData[auctionID][aCurrentBid]    = amount;
    AuctionData[auctionID][aHighestBidder] = pData[playerid][pID];

    VPlayer[playerid][pAuctionBid] = amount;
    VPlayer[playerid][pAuctionVehicle] = AuctionData[auctionID][aVehicleDBID];

    // حفظ المزايدة في قاعدة البيانات
    new query[256];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `auction_bids` (`auction_id`, `bidder_id`, `amount`, `bid_time`) \
        VALUES (%d, %d, %d, %d)",
        AuctionData[auctionID][aID], pData[playerid][pID], amount, gettime()
    );
    mysql_tquery(g_SQL, query, "", "");

    // تحديث المزاد
    mysql_format(g_SQL, query, sizeof(query),
        "UPDATE `auctions` SET `current_bid` = %d, `highest_bidder` = %d WHERE `id` = %d",
        amount, pData[playerid][pID], AuctionData[auctionID][aID]
    );
    mysql_tquery(g_SQL, query, "", "");

    // إشعار جميع اللاعبين
    new string[256];
    format(string, sizeof(string), "[المزاد] %s قدم أعلى مزايدة: %d دينار! استخدم /bid للمزايدة.",
        pData[playerid][pName], amount);
    SendClientMessageToAll(COLOR_ORANGE, string);

    return 1;
}

// --------------------------------------------------
// مؤقت التحقق من المزادات (يُستدعى كل ثانية)
// --------------------------------------------------
public OnAuctionTimer()
{
    for (new i = 0; i < g_AuctionCount; i++)
    {
        if (!AuctionData[i][aActive] || AuctionData[i][aStatus] != AUCTION_STATUS_OPEN) continue;

        if (gettime() >= AuctionData[i][aEndTime])
        {
            ProcessAuctionEnd(i);
        }
        else
        {
            // إشعار بالوقت المتبقي كل 10 ثوانٍ
            new remaining = AuctionData[i][aEndTime] - gettime();
            if (remaining % 10 == 0 && remaining > 0)
            {
                new string[128];
                format(string, sizeof(string), "[المزاد] الوقت المتبقي: %d ثانية. أعلى مزايدة: %d دينار.",
                    remaining, AuctionData[i][aCurrentBid]);
                SendClientMessageToAll(COLOR_YELLOW, string);
            }
        }
    }
    return 1;
}

// --------------------------------------------------
// معالجة انتهاء المزاد
// --------------------------------------------------
public ProcessAuctionEnd(auctionID)
{
    if (!AuctionData[auctionID][aActive]) return 0;

    AuctionData[auctionID][aActive] = false;
    AuctionData[auctionID][aStatus] = AUCTION_STATUS_CLOSED;

    new dbID = AuctionData[auctionID][aVehicleDBID];
    new idx = GetVehicleIndexByDBID(dbID);

    if (idx == -1)
    {
        printf("[WISE-CITY] خطأ: السيارة %d غير موجودة عند انتهاء المزاد.", dbID);
        return 0;
    }

    // إذا لم يزايد أحد
    if (AuctionData[auctionID][aHighestBidder] == 0)
    {
        VehicleData[idx][vInAuction] = 0;
        SaveVehicle(dbID);

        new string[128];
        format(string, sizeof(string), "[المزاد] انتهى المزاد على السيارة (موديل %d) دون أي مزايدات.", VehicleData[idx][vModel]);
        SendClientMessageToAll(COLOR_YELLOW, string);

        // إغلاق المزاد في قاعدة البيانات
        new query[256];
        mysql_format(g_SQL, query, sizeof(query),
            "UPDATE `auctions` SET `status` = %d WHERE `id` = %d",
            AUCTION_STATUS_CLOSED, AuctionData[auctionID][aID]
        );
        mysql_tquery(g_SQL, query, "", "");
        return 1;
    }

    // هناك فائز
    new winnerID = AuctionData[auctionID][aHighestBidder];
    new winner = GetPlayerIndexByDBID(winnerID);
    new seller = GetPlayerIndexByDBID(AuctionData[auctionID][aSellerID]);

    if (winner == -1)
    {
        printf("[WISE-CITY] الفائز بالمزاد %d غير متصل. سيتم إلغاء المزاد.", AuctionData[auctionID][aID]);
        VehicleData[idx][vInAuction] = 0;
        SaveVehicle(dbID);

        // إعادة المبلغ المحجوز إذا كان الفائز غير متصل
        // (سيتم معالجتها عند دخوله)
        return 1;
    }

    // حساب العمولة
    new commission = (AuctionData[auctionID][aCurrentBid] * AUCTION_TAX_PERCENT) / 100;
    new sellerAmount = AuctionData[auctionID][aCurrentBid] - commission;

    // نقل الملكية
    new oldOwnerID = VehicleData[idx][vOwnerID];
    VehicleData[idx][vOwnerID] = winnerID;
    VehicleData[idx][vInAuction] = 0;

    // إزالة السيارة من قائمة البائع القديم
    if (seller != -1)
    {
        for (new i = 0; i < VPlayer[seller][pOwnedCount]; i++)
        {
            if (VPlayer[seller][pOwnedVehicles][i] == dbID)
            {
                for (new j = i; j < VPlayer[seller][pOwnedCount] - 1; j++)
                {
                    VPlayer[seller][pOwnedVehicles][j] = VPlayer[seller][pOwnedVehicles][j + 1];
                }
                VPlayer[seller][pOwnedCount]--;
                break;
            }
        }

        // إضافة المبلغ للبائع (بعد الخصم)
        GivePlayerMoney(seller, sellerAmount);
    }

    // إضافة السيارة لقائمة الفائز
    if (VPlayer[winner][pOwnedCount] < MAX_OWNED_VEHICLES)
    {
        VPlayer[winner][pOwnedVehicles][VPlayer[winner][pOwnedCount]] = dbID;
        VPlayer[winner][pOwnedCount]++;
    }

    // إعادة المبلغ المحجوز المتبقي للفائز (إذا كان قد دفع أكثر من المطلوب)
    // الفائز دفع already amount عند المزايدة، لذلك لا نعيد له شيء

    // حفظ التغييرات
    SaveVehicle(dbID);
    if (seller != -1) Veh_SavePlayerVehicles(seller);
    Veh_SavePlayerVehicles(winner);

    // إنشاء إيصال مالي
    new receiptID[32];
    format(receiptID, sizeof(receiptID), "AUC-%d-%d", AuctionData[auctionID][aID], gettime());
    format(AuctionData[auctionID][aReceiptID], 32, "%s", receiptID);

    // تحديث قاعدة البيانات
    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "UPDATE `auctions` SET `status` = %d, `receipt_id` = '%e' WHERE `id` = %d",
        AUCTION_STATUS_CLOSED, receiptID, AuctionData[auctionID][aID]
    );
    mysql_tquery(g_SQL, query, "", "");

    // إرسال الإشعارات
    new string[512];
    format(string, sizeof(string),
        "[المزاد] انتهى المزاد! الفائز: %s | المبلغ: %d دينار | السيارة: موديل %d",
        pData[winner][pName], AuctionData[auctionID][aCurrentBid], VehicleData[idx][vModel]);
    SendClientMessageToAll(COLOR_GREEN, string);

    format(string, sizeof(string),
        "[الإيصال المالي] رقم: %s | السيارة: %d | المبلغ المدفوع: %d دينار | العمولة: %d دينار",
        receiptID, VehicleData[idx][vModel], AuctionData[auctionID][aCurrentBid], commission);
    SendMessage(winner, COLOR_CYAN, string);

    if (seller != -1)
    {
        format(string, sizeof(string),
            "[الإيصال المالي] رقم: %s | تم بيع سيارتك (موديل %d) بمبلغ %d دينار | صافي المبلغ بعد العمولة: %d دينار",
            receiptID, VehicleData[idx][vModel], AuctionData[auctionID][aCurrentBid], sellerAmount);
        SendMessage(seller, COLOR_CYAN, string);
    }

    // إعادة تعيين بيانات المزاد
    AuctionData[auctionID][aActive] = false;
    g_AuctionCount--;

    return 1;
}

// --------------------------------------------------
// عرض المزادات النشطة
// --------------------------------------------------
CMD:auctions(playerid, params[])
{
    new count = 0;
    for (new i = 0; i < g_AuctionCount; i++)
    {
        if (AuctionData[i][aActive] && AuctionData[i][aStatus] == AUCTION_STATUS_OPEN)
            count++;
    }

    if (count == 0)
        return SendMessage(playerid, COLOR_YELLOW, "[WISE-CITY] لا توجد مزادات نشطة حالياً.");

    new string[1024];
    strcat(string, "{00FFFF}===== المزادات النشطة =====\n");

    for (new i = 0; i < g_AuctionCount; i++)
    {
        if (AuctionData[i][aActive] && AuctionData[i][aStatus] == AUCTION_STATUS_OPEN)
        {
            new idx = GetVehicleIndexByDBID(AuctionData[i][aVehicleDBID]);
            new remaining = AuctionData[i][aEndTime] - gettime();

            new line[256];
            format(line, sizeof(line),
                "{FFFFFF}مزاد #%d | سيارة: موديل %d | أعلى مزايدة: {FFFF00}%d دينار {FFFFFF}| الوقت المتبقي: {FFA500}%d ثانية\n",
                AuctionData[i][aID],
                idx != -1 ? VehicleData[idx][vModel] : 0,
                AuctionData[i][aCurrentBid],
                remaining
            );
            strcat(string, line);
        }
    }

    ShowPlayerDialog(playerid, DIALOG_AUCTIONS_LIST, DIALOG_STYLE_MSGBOX, "المزادات النشطة", string, "حسناً", "");
    return 1;
}

// ============================================================================
// 10. مؤقتات الحفظ التلقائي
// ============================================================================

public OnVehicleSaveTimer()
{
    SaveAllVehicles();
    return 1;
}

// ============================================================================
// 11. أحداث اللاعب
// ============================================================================

stock Veh_OnPlayerConnect(playerid)
{
    VPlayer[playerid][pOwnedCount] = 0;
    VPlayer[playerid][pAuctionBid] = 0;
    VPlayer[playerid][pAuctionVehicle] = 0;
    VPlayer[playerid][pInAuctionArea] = 0;
    for (new i = 0; i < MAX_OWNED_VEHICLES; i++) VPlayer[playerid][pOwnedVehicles][i] = 0;
    return 1;
}

stock Veh_OnPlayerDisconnect(playerid, reason)
{
    // إعادة المبلغ المحجوز في المزاد قبل الحفظ المركزي
    if (VPlayer[playerid][pAuctionBid] > 0 && pData[playerid][pLoggedIn])
    {
        GivePlayerMoney(playerid, VPlayer[playerid][pAuctionBid]);
    }
    VPlayer[playerid][pAuctionBid] = 0;
    Veh_SavePlayerVehicles(playerid);
    VPlayer[playerid][pOwnedCount] = 0;
    return 1;
}

stock Veh_OnPlayerSpawn(playerid)
{
    // إعادة تعيين حالة المزاد
    VPlayer[playerid][pInAuctionArea] = 0;
    return 1;
}

// ============================================================================
// 12. أوامر إضافية
// ============================================================================

CMD:givecar(playerid, params[])
{
    if (!IsPlayerAdmin(playerid, 5))
        return SendMessage(playerid, COLOR_RED, "[خطأ] هذا الأمر للمشرفين فقط.");

    new targetid, model, price, currency;
    if (sscanf(params, "uiii", targetid, model, price, currency))
        return SendMessage(playerid, COLOR_YELLOW, "[الاستخدام] /givecar [اللاعب] [الموديل] [السعر] [العملة]");

    if (!IsPlayerConnected(targetid))
        return SendMessage(playerid, COLOR_RED, "[خطأ] اللاعب غير متصل.");

    if (model < 400 || model > 611)
        return SendMessage(playerid, COLOR_RED, "[خطأ] موديل السيارة غير صالح.");

    // إنشاء السيارة في قاعدة البيانات
    new Float:x, Float:y, Float:z, Float:a;
    GetPlayerPos(targetid, x, y, z);
    GetPlayerFacingAngle(targetid, a);

    new plate[16];
    format(plate, sizeof(plate), "WISE-%d", random(9999));

    new query[512];
    mysql_format(g_SQL, query, sizeof(query),
        "INSERT INTO `vehicles` (`owner_id`, `model`, `color1`, `color2`, `pos_x`, `pos_y`, `pos_z`, `angle`, `plate`, `price`, `currency`) \
        VALUES (%d, %d, 1, 1, %f, %f, %f, %f, '%e', %d, %d)",
        pData[targetid][pID], model, x, y, z, a, plate, price, currency
    );
    mysql_tquery(g_SQL, query, "OnGiveCarCreated", "d", targetid);

    SendMessage(playerid, COLOR_GREEN, "[WISE-CITY] تم إعطاء السيارة للاعب بنجاح.");
    return 1;
}

forward OnGiveCarCreated(targetid);
public OnGiveCarCreated(targetid)
{
    new dbID = cache_insert_id();
    new idx = GetFreeVehicleSlot();

    if (idx != -1)
    {
        VehicleData[idx][vID]        = dbID;
        VehicleData[idx][vOwnerID]   = pData[targetid][pID];
        VehicleData[idx][vModel]     = 411;
        VehicleData[idx][vColor1]    = 1;
        VehicleData[idx][vColor2]    = 1;
        VehicleData[idx][vPosX]      = 0.0;
        VehicleData[idx][vPosY]      = 0.0;
        VehicleData[idx][vPosZ]      = 0.0;
        VehicleData[idx][vAngle]     = 0.0;
        VehicleData[idx][vFuel]      = FUEL_MAX;
        VehicleData[idx][vDamage]    = 0.0;
        format(VehicleData[idx][vPlate], 16, "WISE-0000");
        VehicleData[idx][vLocked]    = 0;
        VehicleData[idx][vForSale]   = false;
        VehicleData[idx][vSpawnedID] = 0;
    }

    // إضافة السيارة لقائمة اللاعب
    if (VPlayer[targetid][pOwnedCount] < MAX_OWNED_VEHICLES)
    {
        VPlayer[targetid][pOwnedVehicles][VPlayer[targetid][pOwnedCount]] = dbID;
        VPlayer[targetid][pOwnedCount]++;

        SendMessage(targetid, COLOR_GREEN, "[WISE-CITY] تم إضافة سيارة جديدة إلى ممتلكاتك. استخدم /mycars لعرضها.");
    }

    return 1;
}

// ============================================================================
// 13. الحوارات (Dialogs)
// ============================================================================


stock Veh_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid == DIALOG_CONFIRM_BUY)
    {
        OnPlayerDialogBuyResponse(playerid, response);
        return 1;
    }
    return 0;
}

// ============================================================================
// 14. نهاية السكريبت
// ============================================================================

stock Veh_OnGameModeExit()
{
    SaveAllVehicles();
    KillTimer(g_AuctionTimer);
    KillTimer(g_VehicleSaveTimer);
    print("[WISE-CITY] تم حفظ المركبات وإيقاف مؤقتات المزادات.");
    return 1;
}

/*
 * ============================================================================
 *  WISE-CITY-RP | نهاية الملف
 *  Wise_City_Vehicles_Auction.pwn
 * ============================================================================
 */

stock Veh_SavePlayerVehicles(playerid)
{
    for (new i = 0; i < VPlayer[playerid][pOwnedCount]; i++)
        SaveVehicle(VPlayer[playerid][pOwnedVehicles][i]);
    return 1;
}
