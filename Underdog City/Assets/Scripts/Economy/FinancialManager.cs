using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class FinancialManager : MonoBehaviour
    {
        public static FinancialManager Instance { get; private set; }

        [Header("💳 النظام المالي المتقدم - Wise City Economy")]
        public double dailyTaxAmount = 50.0;
        public double currentLoanBalance = 0.0;
        public double interestRate = 0.05; // 5% فائدة يومية

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            InvokeRepeating("DeductDailyTaxes", 60f, 300f);
        }

        public void DepositToBank(double amount)
        {
            if (amount <= 0 || HUDController.Instance == null) return;

            if (HUDController.Instance.cash >= amount)
            {
                HUDController.Instance.cash -= amount;
                HUDController.Instance.bank += amount;
                HUDController.Instance.UpdatePlayerInfoUI();
                Debug.Log("[WiseCity Bank] Deposited $" + amount + " to bank account.");
            }
        }

        public void WithdrawFromBank(double amount)
        {
            if (amount <= 0 || HUDController.Instance == null) return;

            if (HUDController.Instance.bank >= amount)
            {
                HUDController.Instance.bank -= amount;
                HUDController.Instance.cash += amount;
                HUDController.Instance.UpdatePlayerInfoUI();
                Debug.Log("[WiseCity Bank] Withdrew $" + amount + " from bank account.");
            }
        }

        public void ApplyForLoan(double loanAmount)
        {
            if (currentLoanBalance > 0 || loanAmount <= 0) return;

            currentLoanBalance = loanAmount * (1.0 + interestRate);
            if (HUDController.Instance != null)
            {
                HUDController.Instance.bank += loanAmount;
                HUDController.Instance.UpdatePlayerInfoUI();
            }
            Debug.Log("[WiseCity Bank] Loan Approved: $" + loanAmount + " | Total Repayable: $" + currentLoanBalance);
        }

        public void RepayLoan(double amount)
        {
            if (currentLoanBalance <= 0 || amount <= 0 || HUDController.Instance == null) return;

            if (HUDController.Instance.bank >= amount)
            {
                HUDController.Instance.bank -= amount;
                currentLoanBalance -= amount;
                if (currentLoanBalance < 0) currentLoanBalance = 0;
                HUDController.Instance.UpdatePlayerInfoUI();
                Debug.Log("[WiseCity Bank] Repaid $" + amount + " towards loan. Remaining: $" + currentLoanBalance);
            }
        }

        void DeductDailyTaxes()
        {
            if (HUDController.Instance != null)
            {
                if (HUDController.Instance.bank >= dailyTaxAmount)
                {
                    HUDController.Instance.bank -= dailyTaxAmount;
                    HUDController.Instance.UpdatePlayerInfoUI();
                    Debug.Log("[WiseCity Taxes] Deducted daily taxes: $" + dailyTaxAmount);
                }
            }
        }
    }
}
