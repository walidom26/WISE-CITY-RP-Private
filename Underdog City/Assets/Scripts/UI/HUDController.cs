using UnityEngine;

namespace UnderdogCity
{
    public class HUDController : MonoBehaviour
    {
        public static HUDController Instance { get; private set; }

        public string currentDistrict = "وسط المدينة";
        public int playerID = 1001;
        public int playerXP = 15;
        public int currentXP = 15;
        public float health = 100f;
        public float armor = 100f;
        public double cash = 50000.0;
        public double bank = 250000.0;
        public float vehicleSpeedKMH = 0f;
        public float fuelLevel = 100f;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void UpdateHUD(float speed, float fuel)
        {
            vehicleSpeedKMH = speed;
            fuelLevel = fuel;
        }

        public void UpdatePlayerInfoUI()
        {
            currentXP = playerXP;
            Debug.Log(string.Format("[HUDController] Updated Player UI: Cash={0}, Bank={1}, XP={2}", cash, bank, playerXP));
        }
    }
}
