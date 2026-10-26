using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class AdminAndAntiCheatManager : MonoBehaviour
    {
        public static AdminAndAntiCheatManager Instance { get; private set; }

        public const string OWNER_ACCOUNT_ID = "mr_walid_owner_id";

        public enum AdminRank
        {
            None = 0,
            Moderator = 1,
            GameAdmin = 2,
            HeadAdmin = 3,
            ServerOwner = 4 // walid om
        }

        [Header("9️⃣ أدوات المشرفين وحماية السيرفر - وايز سيتي")]
        public string boundOwnerAccountID = OWNER_ACCOUNT_ID;
        public AdminRank localAdminRank = AdminRank.ServerOwner;
        public bool isVanishActive = false;
        public bool isSpectating = false;
        public int spectateTargetPlayerID = 0;

        private HashSet<string> bannedHWIDs = new HashSet<string>();
        private HashSet<string> bannedIPs = new HashSet<string>();
        private List<string> immutableAuditLog = new List<string>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity Admin & AntiCheat] Admin Panel, Owner Binding (" + boundOwnerAccountID + ") and HWID/IP Anti-Cheat initialized.");
            LogAuditAction("SYSTEM_INIT", "Admin Panel Initialized", "SUCCESS");
        }

        public void LogAuditAction(string action, string details, string result)
        {
            string logEntry = string.Format("[{0}] Admin: {1} (Rank {2}) | Action: {3} | Details: {4} | Result: {5}", System.DateTime.UtcNow.ToString("s"), boundOwnerAccountID, localAdminRank, action, details, result);
            immutableAuditLog.Add(logEntry);
            Debug.Log("[AUDIT LOG] " + logEntry);
        }

        public void ToggleVanishMode()
        {
            if (localAdminRank < AdminRank.Moderator) return;
            isVanishActive = !isVanishActive;

            Renderer[] renderers = GetComponentsInChildren<Renderer>();
            foreach (var r in renderers)
            {
                if (r != null) r.enabled = !isVanishActive;
            }

            LogAuditAction("TOGGLE_VANISH", "Vanish mode state: " + isVanishActive, "SUCCESS");
        }

        public void SpectatePlayer(int targetPlayerID)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            isSpectating = true;
            spectateTargetPlayerID = targetPlayerID;
            LogAuditAction("SPECTATE_PLAYER", "Spectating Target ID: " + targetPlayerID, "SUCCESS");
        }

        public void FreezePlayer(int targetPlayerID, bool freeze)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            AdminFreezeRpc(targetPlayerID, freeze);
            LogAuditAction("FREEZE_PLAYER", string.Format("Target ID {0} Freeze: {1}", targetPlayerID, freeze), "SUCCESS");
        }

        public void SummonPlayerToAdmin(int targetPlayerID)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            Vector3 adminPos = transform.position + transform.forward * 2f;
            AdminSummonRpc(targetPlayerID, adminPos);
            LogAuditAction("SUMMON_PLAYER", "Summoned Target ID: " + targetPlayerID, "SUCCESS");
        }

        public void BanHardwareAndIP(int targetPlayerID, string hwid, string ip, string banReason)
        {
            if (localAdminRank < AdminRank.GameAdmin) return;

            bannedHWIDs.Add(hwid);
            bannedIPs.Add(ip);

            AdminBanRpc(targetPlayerID, banReason);
            LogAuditAction("BAN_HWID_IP", string.Format("Banned Target ID {0} | HWID: {1} | IP: {2} | Reason: {3}", targetPlayerID, hwid, ip, banReason), "SUCCESS");
        }

        void AdminFreezeRpc(int targetPlayerID, bool freeze)
        {
            PlayerMechanics mechanics = Object.FindObjectOfType<PlayerMechanics>();
            if (mechanics != null) mechanics.enabled = !freeze;
            Debug.Log("[WiseCity Admin] Player ID " + targetPlayerID + (freeze ? " FROZEN" : " UNFROZEN"));
        }

        void AdminSummonRpc(int targetPlayerID, Vector3 destination)
        {
            PlayerMechanics mechanics = Object.FindObjectOfType<PlayerMechanics>();
            if (mechanics != null) mechanics.transform.position = destination;
            Debug.Log("[WiseCity Admin] Player ID " + targetPlayerID + " summoned to investigation point.");
        }

        void AdminBanRpc(int targetPlayerID, string reason)
        {
            Debug.LogError("[WiseCity AntiCheat] Player ID " + targetPlayerID + " BANNED FROM SERVER. Reason: " + reason);
        }
    }
}
