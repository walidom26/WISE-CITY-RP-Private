using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class FactionManager : MonoBehaviour
    {
        public static FactionManager Instance { get; private set; }

        public enum FactionType
        {
            Civilian = 0,
            Police = 1,
            RepublicanGuard = 2,
            Military = 3,
            EMS = 4,
            MediaService = 5
        }

        [Header("🛡️ نظام الفصائل الخمس - وايز سيتي")]
        public FactionType currentFaction = FactionType.Civilian;
        public int factionRank = 1;
        public bool isOffDuty = true;
        public bool isSwatCertified = false; // SWAT sub-team INSIDE police

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void SetOffDutyStatus(bool offDuty)
        {
            isOffDuty = offDuty;
            Debug.Log("[WiseCity Faction] Off-Duty Status: " + (offDuty ? "ON (Civilian)" : "OFF (Active Duty)"));
        }

        public void CertifySwat(bool certified)
        {
            if (currentFaction != FactionType.Police) return;
            isSwatCertified = certified;
            Debug.Log("[WiseCity Police] SWAT Certification updated: " + certified);
        }

        public void IssueCitation(int targetPlayerID, double fineAmount, string reason)
        {
            if (currentFaction != FactionType.Police && currentFaction != FactionType.RepublicanGuard) return;
            PoliceCitationRpc(targetPlayerID, fineAmount, reason);
        }

        public void SendToJail(int targetPlayerID, int durationMinutes, bool isSovereignJail)
        {
            if (currentFaction != FactionType.Police && currentFaction != FactionType.RepublicanGuard) return;
            int clampedMinutes = isSovereignJail ? Mathf.Clamp(durationMinutes, 20, 30) : Mathf.Clamp(durationMinutes, 3, 15);
            PoliceJailRpc(targetPlayerID, clampedMinutes, isSovereignJail);
        }

        public void RevivePlayer(int targetPlayerID)
        {
            if (currentFaction != FactionType.EMS) return;
            EMSReviveRpc(targetPlayerID);
        }

        void PoliceCitationRpc(int targetPlayerID, double fineAmount, string reason)
        {
            Debug.Log(string.Format("[WiseCity Police] Issued Citation to Player ID {0} for ${1} | Reason: {2}", targetPlayerID, fineAmount, reason));
        }

        void PoliceJailRpc(int targetPlayerID, int durationMinutes, bool isSovereignJail)
        {
            Debug.Log(string.Format("[WiseCity Justice] Sent Player ID {0} to {1} Jail for {2} minutes.", targetPlayerID, isSovereignJail ? "Sovereign" : "Standard", durationMinutes));
        }

        void EMSReviveRpc(int targetPlayerID)
        {
            Debug.Log("[WiseCity EMS] Player ID " + targetPlayerID + " Revived!");
        }
    }
}
