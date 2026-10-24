using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Photon.Pun;

namespace UnderdogCity
{
    public class HUDController : MonoBehaviour
    {
        public static HUDController Instance { get; private set; }

        [Header("1️⃣ الزاوية العلوية اليسرى - الخريطة المصغرة (Minimap & GPS)")]
        public Image minimapMask;
        public RectTransform minimapContent;
        public Text txtAreaName;
        public Text txtGPSDistance;
        public GameObject gpsIndicator;

        [Header("1️⃣ الزاوية العلوية اليمنى - لوحة معلومات اللاعب")]
        public Text txtPlayerID;
        public Text txtLevelXP;
        public Image barHealth;
        public Image barArmor;
        public Text txtCashBalance;
        public Text txtBankBalance;
        public Button btnAddMoney;

        [Header("1️⃣ منتصف الشاشة السفلي - الشات الشفاف")]
        public GameObject chatPanel;

        [Header("1️⃣ الزاوية السفلية اليسرى - الميكروفون والتحكم")]
        public Button btnVoiceMic;
        public Image imgMicStatus;
        public Color micActiveColor = Color.green;
        public Color micInactiveColor = Color.gray;
        private bool isMicActive = false;

        [Header("1️⃣ الزاوية السفلية اليمنى - طبلون السرعة الرقمي والوقود")]
        public GameObject speedometerPanel;
        public Text txtSpeedKMH;
        public Image barFuelGauge;
        public Text txtOdometer;

        [Header("بيانات اللاعب الحالية")]
        public int playerID = 1001;
        public int playerLevel = 1;
        public int currentXP = 0;
        public float health = 100f;
        public float armor = 100f;
        public double cash = 5000;
        public double bank = 25000;

        [Header("بيانات المركبة الحالية")]
        public float currentSpeed = 0f;
        public float currentFuel = 100f; // 0 to 100
        public float totalDistanceKM = 0f;

        private void Awake()
        {
            if (Instance == null)
            {
                Instance = this;
            }
            else
            {
                Destroy(gameObject);
            }
        }

        private void Start()
        {
            UpdatePlayerInfoUI();
            UpdateAreaName("وسط المدينة - وايز سيتي");
            if (btnAddMoney != null) btnAddMoney.onClick.AddListener(OnAddMoneyClicked);
            if (btnVoiceMic != null) btnVoiceMic.onClick.AddListener(ToggleVoiceMic);

            Debug.Log("[WiseCity HUD] HUD Controller initialized according to master spec.");
        }

        private void Update()
        {
            UpdateSpeedometerUI();
        }

        public void UpdatePlayerInfoUI()
        {
            if (PhotonNetwork.LocalPlayer != null)
            {
                playerID = PhotonNetwork.LocalPlayer.ActorNumber > 0 ? PhotonNetwork.LocalPlayer.ActorNumber : playerID;
            }

            if (txtPlayerID != null) txtPlayerID.text = "ID: " + playerID;
            if (txtLevelXP != null) txtLevelXP.text = "Lvl " + playerLevel + " (" + currentXP + " XP)";
            if (barHealth != null) barHealth.fillAmount = Mathf.Clamp01(health / 100f);
            if (barArmor != null) barArmor.fillAmount = Mathf.Clamp01(armor / 100f);
            if (txtCashBalance != null) txtCashBalance.text = "$" + cash.ToString("N0");
            if (txtBankBalance != null) txtBankBalance.text = "$" + bank.ToString("N0");
        }

        public void UpdateAreaName(string areaName)
        {
            if (txtAreaName != null) txtAreaName.text = areaName;
        }

        public void SetGPSDestination(Vector3 targetPosition)
        {
            if (gpsIndicator != null) gpsIndicator.SetActive(true);
            if (txtGPSDistance != null) txtGPSDistance.text = "GPS: " + Vector3.Distance(transform.position, targetPosition).ToString("F0") + "m";
        }

        public void ClearGPS()
        {
            if (gpsIndicator != null) gpsIndicator.SetActive(false);
            if (txtGPSDistance != null) txtGPSDistance.text = "GPS: Off";
        }

        public void ToggleVoiceMic()
        {
            isMicActive = !isMicActive;
            if (imgMicStatus != null)
            {
                imgMicStatus.color = isMicActive ? micActiveColor : micInactiveColor;
            }
            Debug.Log("[WiseCity Voice] Microphone status toggled: " + (isMicActive ? "ACTIVE" : "MUTED"));
        }

        public void UpdateSpeedometerUI()
        {
            if (txtSpeedKMH != null) txtSpeedKMH.text = Mathf.RoundToInt(currentSpeed).ToString() + " KM/H";
            if (barFuelGauge != null) barFuelGauge.fillAmount = Mathf.Clamp01(currentFuel / 100f);
            if (txtOdometer != null) txtOdometer.text = totalDistanceKM.ToString("F1") + " KM";
        }

        public void ShowVehicleSpeedometer(bool show)
        {
            if (speedometerPanel != null) speedometerPanel.SetActive(show);
        }

        private void OnAddMoneyClicked()
        {
            Debug.Log("[WiseCity Bank] Add Money / ATM Modal opened for player: " + playerID);
        }
    }
}
