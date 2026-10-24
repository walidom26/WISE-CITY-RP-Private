using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Photon.Pun;

namespace UnderdogCity
{
    public class GangManager : MonoBehaviourPunCallbacks
    {
        public static GangManager Instance { get; private set; }

        [System.Serializable]
        public class GangTerritory
        {
            public string territoryID;
            public string areaName;
            public string controllingGangName;
            public double hourlyTaxRevenue;
            public float captureProgressPercent; // 0 to 100%
            public bool isUnderContest;
        }

        [System.Serializable]
        public class BankHeistPlan
        {
            public string bankLocationName;
            public bool vaultDrilled;
            public bool alarmTriggered;
            public double totalLootAmount;
            public float timeRemainingSeconds;
        }

        [Header("6️⃣ العصابات والاقتصاد السفلي - وايز سيتي")]
        public string playerGangName = "بدون عصابة";
        public bool isGangLeader = false;
        public List<GangTerritory> cityTerritories = new List<GangTerritory>();
        public BankHeistPlan activeHeist;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            InitializeTerritories();
            Debug.Log("[WiseCity Gangs] Gangs, Turf Wars, Bank Heist & Money Laundering System initialized.");
        }

        private void InitializeTerritories()
        {
            cityTerritories.Add(new GangTerritory { territoryID = "T1", areaName = "حي المرفأ - Harbor District", controllingGangName = "الثعالب", hourlyTaxRevenue = 5000, captureProgressPercent = 100f });
            cityTerritories.Add(new GangTerritory { territoryID = "T2", areaName = "المنطقة الصناعية - Industrial Zone", controllingGangName = "غير محتلة", hourlyTaxRevenue = 8000, captureProgressPercent = 0f });
            cityTerritories.Add(new GangTerritory { territoryID = "T3", areaName = "وسط المدينة القديمة - Old Town", controllingGangName = "غير محتلة", hourlyTaxRevenue = 12000, captureProgressPercent = 0f });
        }

        public void StartTurfWarCapture(string territoryID)
        {
            if (string.IsNullOrEmpty(playerGangName) || playerGangName == "بدون عصابة") return;
            photonView.RPC("BroadcastTurfWarStatusRpc", RpcTarget.All, territoryID, playerGangName, true);
        }

        public void PlanBankHeist(string bankName, double lootValue)
        {
            activeHeist = new BankHeistPlan
            {
                bankLocationName = bankName,
                vaultDrilled = false,
                alarmTriggered = false,
                totalLootAmount = lootValue,
                timeRemainingSeconds = 600f
            };
            Debug.Log(string.Format("[WiseCity Bank Heist] Planned heist at {0} for ${1:N0}", bankName, lootValue));
        }

        public void MoneyLaunder(double dirtyMoneyAmount, float feePercent)
        {
            double cleanCash = dirtyMoneyAmount * (1f - (feePercent / 100f));
            if (HUDController.Instance != null)
            {
                HUDController.Instance.cash += cleanCash;
                HUDController.Instance.UpdatePlayerInfoUI();
            }
            Debug.Log(string.Format("[WiseCity Money Laundering] Laundered ${0:N0} dirty cash into ${1:N0} clean cash (Fee: {2}%).", dirtyMoneyAmount, cleanCash, feePercent));
        }

        [PunRPC]
        void BroadcastTurfWarStatusRpc(string territoryID, string gangName, bool contested)
        {
            GangTerritory t = cityTerritories.Find(x => x.territoryID == territoryID);
            if (t != null)
            {
                t.isUnderContest = contested;
                t.controllingGangName = gangName;
                Debug.Log(string.Format("[WiseCity Turf War] Territory {0} ({1}) is now under attack by {2}!", t.territoryID, t.areaName, gangName));
            }
        }
    }
}
