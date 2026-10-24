using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Photon.Pun;

namespace UnderdogCity
{
    public class AdminAndAntiCheatManager : MonoBehaviourPunCallbacks
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

        // 1. وضع الاختفاء (Vanish Mode)
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

        // 2. المراقبة الحية (Live Spectate)
        public void SpectatePlayer(int targetPlayerID)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            isSpectating = true;
            spectateTargetPlayerID = targetPlayerID;
            Debug.Log("[WiseCity Admin] Spectating player ID: " + targetPlayerID);
        }

        // 3. تجميد اللاعب (Freeze Player)
        public void FreezePlayer(int targetPlayerID, bool freeze)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            photonView.RPC("AdminFreezeRpc", RpcTarget.All, targetPlayerID, freeze);
        }

        // 4. استدعاء اللاعب للتحقيق (Summon Player)
        public void SummonPlayerToAdmin(int targetPlayerID)
        {
            if (localAdminRank < AdminRank.Moderator) return;
            Vector3 adminPos = transform.position + transform.forward * 2f;
            photonView.RPC("AdminSummonRpc", RpcTarget.All, targetPlayerID, adminPos);
        }

        // 5. حظر الأجهزة والآي بي التلقائي (HWID / IP Ban)
        public void BanHardwareAndIP(int targetPlayerID, string hwid, string ip, string banReason)
        {
            if (localAdminRank < AdminRank.GameAdmin) return;

            bannedHWIDs.Add(hwid);
            bannedIPs.Add(ip);

            photonView.RPC("AdminBanRpc", RpcTarget.All, targetPlayerID, banReason);
            Debug.Log(string.Format("[WiseCity AntiCheat] Banned Player ID: {0} | HWID: {1} | IP: {2} | Reason: {3}", targetPlayerID, hwid, ip, banReason));
        }

        [PunRPC]
        void AdminFreezeRpc(int targetPlayerID, bool freeze)
        {
            if (PhotonNetwork.LocalPlayer != null && PhotonNetwork.LocalPlayer.ActorNumber == targetPlayerID)
            {
                Controller ctrl = GetComponent<Controller>();
                if (ctrl != null) ctrl.enabled = !freeze;
                Debug.Log("[WiseCity Admin] You have been " + (freeze ? "FROZEN" : "UNFROZEN") + " by an admin.");
            }
        }

        [PunRPC]
        void AdminSummonRpc(int targetPlayerID, Vector3 destination)
        {
            if (PhotonNetwork.LocalPlayer != null && PhotonNetwork.LocalPlayer.ActorNumber == targetPlayerID)
            {
                transform.position = destination;
                Debug.Log("[WiseCity Admin] You have been summoned by an admin for investigation.");
            }
        }

        [PunRPC]
        void AdminBanRpc(int targetPlayerID, string reason)
        {
            if (PhotonNetwork.LocalPlayer != null && PhotonNetwork.LocalPlayer.ActorNumber == targetPlayerID)
            {
                Debug.LogError("[WiseCity AntiCheat] BANNED FROM SERVER. Reason: " + reason);
                PhotonNetwork.Disconnect();
            }
        }
    }
}
