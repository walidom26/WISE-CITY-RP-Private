/* ============================================================================
    WISE-CITY-RP  |  النواة المشتركة (Core)
    ----------------------------------------------------------------------------
    - مكتبات المشروع الأساسية (تُضمَّن مرة واحدة فقط هنا)
    - اتصال MySQL واحد مشترك (g_SQL) لكل الأنظمة
    - طبقة توافق لدوال MySQL القديمة (R39) فوق R41 حتى لا تتعطل الوحدات
    - ألوان ودوال عامة مشتركة
   ============================================================================ */

#if defined _WC_CORE_INC
    #endinput
#endif
#define _WC_CORE_INC

#include <open.mp>
#include <a_mysql>
#include <sscanf2>
#include <Pawn.CMD>
#include <streamer>
#include <samp_bcrypt>

#include "wc_secrets.inc"

/* ---------------------------- هوية السيرفر ---------------------------- */
#define SERVER_NAME             "WISE-CITY-RP"
#define SERVER_VERSION          "2.0.0"
#define SERVER_OWNER            "walid om"

/* ------------------------------ الألوان ------------------------------- */
#define COLOR_WHITE             0xFFFFFFFF
#define COLOR_RED               0xFF4444FF
#define COLOR_GREEN             0x33CC66FF
#define COLOR_YELLOW            0xFFD700FF
#define COLOR_GREY              0xAAAAAAFF
#define COLOR_ORANGE            0xFFA500FF
#define COLOR_CYAN              0x00FFFFFF

/* ------------------------- بيانات مشتركة بين الأنظمة ------------------------- */
new MySQL:g_SQL;                         // الاتصال الوحيد بقاعدة البيانات
new g_PlayerDBID[MAX_PLAYERS];           // users.id للاعب (يُملأ عند تسجيل الدخول)

new g_WC_LastInsert;
new g_WC_LastAffected;

// دوال تُعرَّف في wise_city.pwn
forward Core_SyncPlayer(playerid);
forward Core_OnPlayerLogin(playerid);

/* ------------- طبقة التوافق: دوال R39 القديمة فوق a_mysql R41 ------------- */
stock cache_get_field_content_int(row, const field[], MySQL:handle = MySQL:0)
{
    #pragma unused handle
    new value = 0;
    cache_get_value_name_int(row, field, value);
    return value;
}

stock Float:cache_get_field_content_float(row, const field[], MySQL:handle = MySQL:0)
{
    #pragma unused handle
    new Float:value = 0.0;
    cache_get_value_name_float(row, field, value);
    return value;
}

stock cache_get_field_content(row, const field[], dest[], MySQL:handle = MySQL:0, size = sizeof(dest))
{
    #pragma unused handle
    cache_get_value_name(row, field, dest, size);
    return 1;
}

// تنفيذ استعلام متزامن وحفظ آخر ID / عدد الصفوف المتأثرة (بديل mysql_insert_id)
stock WC_Exec(const query[])
{
    new Cache:c = mysql_query(g_SQL, query);
    g_WC_LastInsert   = cache_insert_id();
    g_WC_LastAffected = cache_affected_rows();
    cache_delete(c);
    return 1;
}
stock WC_LastInsertID() return g_WC_LastInsert;
stock WC_LastAffected() return g_WC_LastAffected;

stock Core_ConnectDatabase()
{
    mysql_log(ERROR | WARNING);

    new MySQLOpt:options = mysql_init_options();
    mysql_set_option(options, AUTO_RECONNECT, true);

    g_SQL = mysql_connect(WC_DB_HOST, WC_DB_USER, WC_DB_PASS, WC_DB_NAME, options);
    if (g_SQL == MYSQL_INVALID_HANDLE || mysql_errno(g_SQL) != 0) return 0;

    mysql_set_charset("utf8mb4", g_SQL);
    return 1;
}

/* ------------------------------ دوال عامة ------------------------------ */
stock PlayerName(playerid)
{
    static name[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, name, sizeof(name));
    return name;
}

// الدينار = رصيد اللاعب داخل اللعبة (يُحفظ تلقائياً في users.money_dinar)
stock SetPlayerDinar(playerid, amount)
{
    ResetPlayerMoney(playerid);
    GivePlayerMoney(playerid, amount);
    return 1;
}
