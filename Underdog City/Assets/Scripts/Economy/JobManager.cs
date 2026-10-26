using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class JobManager : MonoBehaviour
    {
        public static JobManager Instance { get; private set; }

        public enum JobType
        {
            Unemployed = 0,
            TaxiDriver = 1,
            BusDriver = 2,
            HeavyTrucker = 3,
            DeliveryDriver = 4,
            Mechanic = 5,
            Cleaner = 6,
            ConstructionWorker = 7,
            ShopClerk = 8,
            MaintenanceWorker = 9
        }

        [Header("👷 المهن والوظائف الحرة - Wise City Jobs")]
        public JobType currentJob = JobType.Unemployed;
        public int jobXPLevel = 1;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void SelectJob(JobType newJob)
        {
            currentJob = newJob;
            Debug.Log("[WiseCity Jobs] Selected Job: " + newJob);
        }

        public void CompleteJobTask(double payout)
        {
            if (currentJob == JobType.Unemployed) return;

            jobXPLevel += 10;
            if (HUDController.Instance != null)
            {
                HUDController.Instance.cash += payout;
                HUDController.Instance.playerXP += 5;
                HUDController.Instance.UpdatePlayerInfoUI();
            }

            Debug.Log(string.Format("[WiseCity Jobs] Task Complete! Earned ${0} cash | Job XP: {1}", payout, jobXPLevel));
        }
    }
}
