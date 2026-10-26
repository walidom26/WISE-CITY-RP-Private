using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class WiseProSmartphone : MonoBehaviour
    {
        public static WiseProSmartphone Instance { get; private set; }

        public enum PhoneApp
        {
            CentralBank,
            GPSMap,
            MarketP2P,
            Chirper,
            CallsSMS
        }

        [Header("📱 هاتف وايز برود الذكي - WisePro Smartphone")]
        public bool isPhoneOpen = false;
        public PhoneApp activeApp = PhoneApp.CentralBank;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void TogglePhoneUI()
        {
            isPhoneOpen = !isPhoneOpen;
            Debug.Log("[WisePro Phone] UI state: " + (isPhoneOpen ? "OPEN" : "CLOSED"));
        }

        public void SendPrivateSMS(int targetPlayerID, string smsBody)
        {
            ReceivePrivateSMSRpc(targetPlayerID, "المواطن", smsBody);
        }

        void ReceivePrivateSMSRpc(int targetPlayerID, string senderName, string smsBody)
        {
            Debug.Log(string.Format("[WisePro Phone] SMS to ID {0} from {1}: {2}", targetPlayerID, senderName, smsBody));
        }
    }
}
