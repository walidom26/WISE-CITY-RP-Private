using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class AdminAndAntiCheatManager : MonoBehaviour
    {
        public static AdminAndAntiCheatManager Instance { get; private set; }

        public enum AdminRank
        {
            None = 0,
            Moderator = 1,
            GameAdmin = 2,
            HeadAdmin = 3,
            ServerOwner = 4 // mr walid
        }

        [Header("9️⃣ أدوات المشرفين وحماية السيرفر - وايز سيتي")]
        public AdminRank localAdminRank = AdminRank.ServerOwner;
        public bool isVanishActive = false;
        public bool isSpectating = false;
        public int spectateTargetPlayerID = 0;

        private HashSet<string> bannedHWIDs = new HashSet<string>();
        private HashSet<string> bannedIPs = new HashSet<string>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity Admin & AntiCheat] Admin Panel and HWID/IP Anti-Cheat initialized.");
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

            Debug.Log("[WiseCity Admin] Vanish mode: " + (isVanishActive ? "ENABLED" : "DISABLED"));
        }

        public void SpectatePlayer(int targetPlayerID)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            isSpectating = true;
            spectateTargetPlayerID = targetPlayerID;
            Debug.Log("[WiseCity Admin] Spectating player ID: " + targetPlayerID);
        }

        public void FreezePlayer(int targetPlayerID, bool freeze)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            AdminFreezeRpc(targetPlayerID, freeze);
        }

        public void SummonPlayerToAdmin(int targetPlayerID)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            Vector3 adminPos = transform.position + transform.forward * 2f;
            AdminSummonRpc(targetPlayerID, adminPos);
        }

        public void BanHardwareAndIP(int targetPlayerID, string hwid, string ip, string banReason)
        {
            if (localAdminRank < AdminRank.GameAdmin) return;

            bannedHWIDs.Add(hwid);
            bannedIPs.Add(ip);

            AdminBanRpc(targetPlayerID, banReason);
            Debug.Log(string.Format("[WiseCity AntiCheat] Banned Player ID: {0} | HWID: {1} | IP: {2} | Reason: {3}", targetPlayerID, hwid, ip, banReason));
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
