using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Photon.Pun;

namespace UnderdogCity
{
    public class FactionManager : MonoBehaviourPunCallbacks
    {
        public static FactionManager Instance { get; private set; }

        public enum FactionType
        {
            Civilian = 0,         // مواطن مدني
            Police = 1,           // الشرطة
            RepublicanGuard = 2,  // الحرس الجمهوري
            Army = 3,             // الجيش
            EMS = 4,              // الطب والصحة
            ServicesAndMedia = 5  // الخدمات والإعلام
        }

        [System.Serializable]
        public class FactionMember
        {
            public int playerActorNumber;
            public string playerName;
            public FactionType faction;
            public string rank;
            public bool isOffDuty; // نظام الإجازات والخدمة المدنية
            public bool isHandcuffed;
            public float currentJailTimeMinutes;
            public bool isSovereignJail;
        }

        [Header("5️⃣ الفصائل الرسمية الخمس والأمن - وايز سيتي")]
        public FactionType localPlayerFaction = FactionType.Civilian;
        public bool isLocalPlayerOffDuty = false;
        public string localPlayerRank = "مواطن";

        private Dictionary<int, FactionMember> memberRegistry = new Dictionary<int, FactionMember>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity Factions] Faction & Justice System initialized with 5 official factions.");
        }

        public void SetOffDutyStatus(bool offDuty)
        {
            isLocalPlayerOffDuty = offDuty;
            Debug.Log("[WiseCity Factions] Off-Duty status set to: " + (offDuty ? "CIVILIAN ATTIRE" : "ON DUTY UNIFORM"));
        }

        // ① الشرطة: إصدار مخالفة
        public void PoliceIssueTicket(int targetPlayerID, double amount, string reason)
        {
            if (localPlayerFaction != FactionType.Police || isLocalPlayerOffDuty) return;
            photonView.RPC("ReceiveTicketRpc", RpcTarget.All, targetPlayerID, amount, reason);
        }

        // ① الشرطة: الكلبشات والإيداع بالسجن العادي (3 إلى 15 دقيقة)
        public void PoliceJailPlayer(int targetPlayerID, float durationMinutes)
        {
            if (localPlayerFaction != FactionType.Police || isLocalPlayerOffDuty) return;
            durationMinutes = Mathf.Clamp(durationMinutes, 3f, 15f);
            photonView.RPC("SendToJailRpc", RpcTarget.All, targetPlayerID, durationMinutes, false);
        }

        // ② الحرس الجمهوري: الإيداع بالسجن السيادي الخطر (20 إلى 30 دقيقة)
        public void RepublicanGuardSovereignJail(int targetPlayerID, float durationMinutes)
        {
            if (localPlayerFaction != FactionType.RepublicanGuard || isLocalPlayerOffDuty) return;
            durationMinutes = Mathf.Clamp(durationMinutes, 20f, 30f);
            photonView.RPC("SendToJailRpc", RpcTarget.All, targetPlayerID, durationMinutes, true);
        }

        // ④ الطب والصحة: الإنعاش الميداني
        public void EMSRevivePlayer(int targetPlayerID)
        {
            if (localPlayerFaction != FactionType.EMS || isLocalPlayerOffDuty) return;
            photonView.RPC("RevivePlayerRpc", RpcTarget.All, targetPlayerID);
        }

        // ⑤ الخدمات والإعلام: بث الأخبار العاجلة وسحب المركبات
        public void BroadcastNewsNotice(string newsText)
        {
            if (localPlayerFaction != FactionType.ServicesAndMedia || isLocalPlayerOffDuty) return;
            photonView.RPC("ReceiveNewsBroadcastRpc", RpcTarget.All, newsText);
        }

        [PunRPC]
        void SendToJailRpc(int targetPlayerID, float minutes, bool sovereignJail)
        {
            if (PhotonNetwork.LocalPlayer != null && PhotonNetwork.LocalPlayer.ActorNumber == targetPlayerID)
            {
                Debug.Log(string.Format("[WiseCity Justice] Sent to {0} Jail for {1} minutes.", sovereignJail ? "Sovereign" : "Normal", minutes));
            }
        }

        [PunRPC]
        void ReceiveTicketRpc(int targetPlayerID, double amount, string reason)
        {
            if (PhotonNetwork.LocalPlayer != null && PhotonNetwork.LocalPlayer.ActorNumber == targetPlayerID)
            {
                Debug.Log(string.Format("[WiseCity Police] Ticket received: ${0:N0} - Reason: {1}", amount, reason));
            }
        }

        [PunRPC]
        void RevivePlayerRpc(int targetPlayerID)
        {
            if (PhotonNetwork.LocalPlayer != null && PhotonNetwork.LocalPlayer.ActorNumber == targetPlayerID)
            {
                Debug.Log("[WiseCity EMS] You have been revived by paramedics.");
            }
        }

        [PunRPC]
        void ReceiveNewsBroadcastRpc(string text)
        {
            Debug.Log("[WiseCity Media BREAKING NEWS]: " + text);
        }
    }
}
