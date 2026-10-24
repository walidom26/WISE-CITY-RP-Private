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
            Unemployed,             // عاطل / حر
            BusDriver,              // سائق الحافلات
            HeavyTruckDriver,       // سائق الشاحنات الثقيلة
            MaintenanceAndConstruction // عمال الصيانة والبناء
        }

        [System.Serializable]
        public class JobRoute
        {
            public string routeName;
            public Vector3[] waypoints;
            public double baseReward;
            public int requiredJobXP;
        }

        [Header("6️⃣ المهن الحرة ونظام المستويات - وايز سيتي")]
        public JobType currentJob = JobType.Unemployed;
        public int jobXP = 0;
        public int jobLevel = 1;
        public List<JobRoute> activeRoutes = new List<JobRoute>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity Jobs] Free Jobs & Cargo Transportation System initialized.");
        }

        public void StartJob(JobType newJob)
        {
            currentJob = newJob;
            Debug.Log("[WiseCity Jobs] Started job: " + currentJob.ToString());
        }

        public void CompleteJobWaypoint(double payout, int xpEarned)
        {
            jobXP += xpEarned;
            if (jobXP >= jobLevel * 500)
            {
                jobLevel++;
                Debug.Log("[WiseCity Jobs] Job Level Up! New Level: " + jobLevel);
            }

            if (HUDController.Instance != null)
            {
                HUDController.Instance.cash += payout;
                HUDController.Instance.currentXP += xpEarned;
                HUDController.Instance.UpdatePlayerInfoUI();
            }

            Debug.Log(string.Format("[WiseCity Jobs] Route Waypoint Completed! Earned: ${0:N0} and {1} XP", payout, xpEarned));
        }

        public void QuitJob()
        {
            currentJob = JobType.Unemployed;
            Debug.Log("[WiseCity Jobs] Quit current job.");
        }
    }
}
