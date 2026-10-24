using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class FinancialManager : MonoBehaviour
    {
        public static FinancialManager Instance { get; private set; }

        [System.Serializable]
        public class P2PListing
        {
            public string listingID;
            public string sellerName;
            public string itemType; // Vehicle or Property
            public string itemName;
            public double askingPrice;
            public bool isSold;
        }

        [Header("7️⃣ & 8️⃣ النظام المالي المتقدم والـ P2P Market - وايز سيتي")]
        public double dailyTaxAmount = 250;
        public double currentLoanBalance = 0;
        public float loanInterestRatePercent = 5.0f;
        public List<P2PListing> p2pMarketplace = new List<P2PListing>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity Finance] Advanced Financial & ATM System initialized.");
            InvokeRepeating("DeductDailyTaxes", 1200f, 1200f); // خصم الضرائب كل 20 دقيقة
        }

        public void DepositToATM(double amount)
        {
            if (HUDController.Instance != null && HUDController.Instance.cash >= amount)
            {
                HUDController.Instance.cash -= amount;
                HUDController.Instance.bank += amount;
                HUDController.Instance.UpdatePlayerInfoUI();
                Debug.Log(string.Format("[WiseCity ATM] Deposited ${0:N0} to bank account.", amount));
            }
        }

        public void WithdrawFromATM(double amount)
        {
            if (HUDController.Instance != null && HUDController.Instance.bank >= amount)
            {
                HUDController.Instance.bank -= amount;
                HUDController.Instance.cash += amount;
                HUDController.Instance.UpdatePlayerInfoUI();
                Debug.Log(string.Format("[WiseCity ATM] Withdrew ${0:N0} from bank account.", amount));
            }
        }

        public void ApplyBankLoan(double amount)
        {
            currentLoanBalance += amount * (1f + (loanInterestRatePercent / 100f));
            if (HUDController.Instance != null)
            {
                HUDController.Instance.bank += amount;
                HUDController.Instance.UpdatePlayerInfoUI();
            }
            Debug.Log(string.Format("[WiseCity Bank Loan] Approved loan of ${0:N0}. Total repayment owed: ${1:N0}", amount, currentLoanBalance));
        }

        public void CreateP2PListing(string seller, string type, string name, double price)
        {
            string id = "P2P-" + Random.Range(1000, 9999);
            p2pMarketplace.Add(new P2PListing
            {
                listingID = id,
                sellerName = seller,
                itemType = type,
                itemName = name,
                askingPrice = price,
                isSold = false
            });
            Debug.Log(string.Format("[WiseCity P2P Market] Item listed: {0} ({1}) for ${2:N0}", name, type, price));
        }

        private void DeductDailyTaxes()
        {
            if (HUDController.Instance != null)
            {
                if (HUDController.Instance.bank >= dailyTaxAmount)
                {
                    HUDController.Instance.bank -= dailyTaxAmount;
                }
                else if (HUDController.Instance.cash >= dailyTaxAmount)
                {
                    HUDController.Instance.cash -= dailyTaxAmount;
                }
                HUDController.Instance.UpdatePlayerInfoUI();
                Debug.Log(string.Format("[WiseCity Taxes] Daily tax of ${0:N0} deducted.", dailyTaxAmount));
            }
        }
    }
}
