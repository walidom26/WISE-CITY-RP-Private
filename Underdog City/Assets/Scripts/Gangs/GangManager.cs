using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class GangManager : MonoBehaviour
    {
        public static GangManager Instance { get; private set; }

        public enum GangType
        {
            None = 0,
            Vipers = 1,
            Shadows = 2,
            Cartel = 3
        }

        [Header("🔫 العصابات وحروب النفوذ - Wise City Gangs")]
        public GangType currentGang = GangType.None;
        public int gangRank = 1;
        public double launderedMoneyPool = 0.0;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void StartBankHeist(string bankName)
        {
            if (currentGang == GangType.None) return;
            Debug.Log("[WiseCity Gangs] Bank Heist Started at " + bankName + " by Gang " + currentGang);
        }

        public void LaunderMoney(double dirtyAmount)
        {
            double cleanAmount = dirtyAmount * 0.8;
            launderedMoneyPool += cleanAmount;
            Debug.Log(string.Format("[WiseCity Gangs] Laundered ${0} dirty cash -> ${1} clean cash.", dirtyAmount, cleanAmount));
        }
    }
}
