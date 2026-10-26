using UnityEngine;
using UnityEngine.UI;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class HUDManager : MonoBehaviour
    {
        public static HUDManager Instance { get; private set; }

        [Header("📱 عناصر واجهة المستخدم HUD Elements")]
        public string currentDistrictName = "وسط المدينة (Downtown)";
        public int playerID = 1001;
        public int playerXPLevel = 15;
        public float healthAmount = 100f;
        public float armorAmount = 100f;
        public double pocketCash = 50000.0;
        public double bankBalance = 250000.0;

        [Header("طبلون السرعة والوقود")]
        public float speedKMH = 0f;
        public float fuelLevel = 100f;

        [Header("شات وايز سيتي - Chat Channels")]
        public List<string> chatNearbyMessages = new List<string>();
        public List<string> chatRadioMessages = new List<string>();
        public List<string> chatAdminMessages = new List<string>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else UnityEngine.Object.Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[HUDManager] Initialized Wise City HUD layout & dynamic data binding.");
            AddChatMessage("قريب", "مرحباً بك في مدينة وايز سيتي RP!");
        }

        private void Update()
        {
            if (HUDController.Instance != null)
            {
                pocketCash = HUDController.Instance.cash;
                bankBalance = HUDController.Instance.bank;
                playerXPLevel = HUDController.Instance.playerXP;
                healthAmount = HUDController.Instance.health;
                armorAmount = HUDController.Instance.armor;
            }

            if (PlayerController.LocalPlayer != null)
            {
                Vector3 pos = PlayerController.LocalPlayer.transform.position;
                if (ProceduralWorldGenerator.Instance != null)
                {
                    currentDistrictName = ProceduralWorldGenerator.Instance.GetDistrictAtPosition(pos);
                }

                if (PlayerController.LocalPlayer.isDriving && PlayerController.LocalPlayer.currentVehicle != null)
                {
                    speedKMH = PlayerController.LocalPlayer.currentVehicle.GetSpeedKMH();
                    fuelLevel = PlayerController.LocalPlayer.currentVehicle.currentFuel;
                }
                else
                {
                    speedKMH = 0f;
                }
            }
        }

        public void AddChatMessage(string channel, string text)
        {
            string shapedText = ArabicTextShaper.ShapeText(text);
            string formatted = string.Format("[{0}] {1}", channel, shapedText);

            if (channel == "راديو") chatRadioMessages.Add(formatted);
            else if (channel == "إداري") chatAdminMessages.Add(formatted);
            else chatNearbyMessages.Add(formatted);

            Debug.Log("[Chat] " + formatted);
        }

        private void OnGUI()
        {
            GUI.backgroundColor = new Color(0.1f, 0.1f, 0.12f, 0.85f);

            // 1. Top-Left: Circular Minimap & District Name
            GUI.Box(new Rect(20, 20, 200, 100), "🗺️ " + ArabicTextShaper.ShapeText(currentDistrictName));
            GUI.Label(new Rect(30, 50, 180, 25), "📍 GPS: Active (Safe Zone)");

            // 2. Top-Right: Player Info & Money
            GUI.Box(new Rect(Screen.width - 240, 20, 220, 120), "👤 ID: " + playerID + " | Level " + playerXPLevel);
            GUI.Label(new Rect(Screen.width - 230, 45, 200, 20), "❤️ HP: " + (int)healthAmount + " | 🛡️ Armor: " + (int)armorAmount);
            GUI.Label(new Rect(Screen.width - 230, 68, 200, 20), "💵 Cash: $" + pocketCash.ToString("N0"));
            GUI.Label(new Rect(Screen.width - 230, 90, 200, 20), "💳 Bank: $" + bankBalance.ToString("N0"));

            // 3. Bottom-Left: Driving Controls Info
            if (PlayerController.LocalPlayer != null && PlayerController.LocalPlayer.isDriving)
            {
                GUI.Box(new Rect(20, Screen.height - 130, 200, 110), "🏎️ Driving Controls");
                GUI.Label(new Rect(30, Screen.height - 105, 180, 20), "W/S: Accelerate/Brake");
                GUI.Label(new Rect(30, Screen.height - 85, 180, 20), "A/D: Steer Left/Right");
                GUI.Label(new Rect(30, Screen.height - 65, 180, 20), "F / E: Exit Vehicle");
            }

            // 4. Bottom-Right: Speedometer & Fuel Gauge
            GUI.Box(new Rect(Screen.width - 220, Screen.height - 120, 200, 100), "⚡ WISE SPEEDOMETER");
            GUI.Label(new Rect(Screen.width - 210, Screen.height - 95, 180, 25), "🏎️ Speed: " + (int)speedKMH + " KM/H");
            GUI.Label(new Rect(Screen.width - 210, Screen.height - 70, 180, 25), "⛽ Fuel: " + (int)fuelLevel + "% [E----F]");

            // 5. Bottom-Center: Transparent Chat Box
            GUI.Box(new Rect(Screen.width / 2 - 200, Screen.height - 160, 400, 140), "💬 Chat Log");
            int msgY = (int)(Screen.height - 135);
            for (int i = Mathf.Max(0, chatNearbyMessages.Count - 4); i < chatNearbyMessages.Count; i++)
            {
                GUI.Label(new Rect(Screen.width / 2 - 190, msgY, 380, 22), chatNearbyMessages[i]);
                msgY += 22;
            }
        }
    }
}
