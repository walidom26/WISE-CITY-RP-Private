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
            BusDriver = 1,
            HeavyTrucker = 2,
            MaintenanceWorker = 3
        }

        [Header("👷 المهن والوظائف الحرة - Wise City Jobs")]
        public JobType currentJob = JobType.Unemployed;
        public int jobXPLevel = 1;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
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
