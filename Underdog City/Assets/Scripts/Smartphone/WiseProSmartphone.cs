using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Photon.Pun;

namespace UnderdogCity
{
    public class WiseProSmartphone : MonoBehaviourPunCallbacks
    {
        public static WiseProSmartphone Instance { get; private set; }

        public enum SmartphoneApp
        {
            Home,          // القائمة الرئيسية
            CentralBank,   // البنك المركزي
            MapGPS,        // الخريطة والـ GPS
            Marketplace,   // سوق البيع والشراء P2P
            ChirperSocial, // تطبيق Chirper
            VoiceAndSMS    // المكالمات والرسائل
        }

        [System.Serializable]
        public class ChirperPost
        {
            public string authorName;
            public string messageText;
            public string timestampString;
        }

        [Header("7️⃣ هاتف WisePro الذكي - وايز سيتي")]
        public GameObject phoneUIFrame;
        public SmartphoneApp currentApp = SmartphoneApp.Home;
        public List<ChirperPost> socialFeed = new List<ChirperPost>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity Smartphone] WisePro Smartphone OS initialized.");
            PostChirperMessage("WiseCityNews", "أهلاً بكم في تطبيق Chirper الرسمي لمدينة Wise City!");
        }

        public void TogglePhoneVisibility()
        {
            if (phoneUIFrame != null)
            {
                phoneUIFrame.SetActive(!phoneUIFrame.activeSelf);
                Debug.Log("[WiseCity Smartphone] Phone visibility toggled: " + phoneUIFrame.activeSelf);
            }
        }

        public void OpenApp(SmartphoneApp app)
        {
            currentApp = app;
            Debug.Log("[WiseCity Smartphone] Opened App: " + currentApp.ToString());
        }

        public void PostChirperMessage(string author, string message)
        {
            ChirperPost post = new ChirperPost
            {
                authorName = author,
                messageText = message,
                timestampString = System.DateTime.Now.ToString("HH:mm")
            };
            socialFeed.Insert(0, post);
            Debug.Log(string.Format("[WiseCity Chirper] @{0}: {1}", author, message));
        }

        public void MakePrivateCall(int targetPlayerID)
        {
            Debug.Log("[WiseCity Phone Call] Dialing player ID: " + targetPlayerID);
        }

        public void SendPrivateSMS(int targetPlayerID, string smsBody)
        {
            photonView.RPC("ReceivePrivateSMSRpc", RpcTarget.All, targetPlayerID, PhotonNetwork.LocalPlayer.NickName, smsBody);
        }

        [PunRPC]
        void ReceivePrivateSMSRpc(int targetPlayerID, string senderName, string body)
        {
            if (PhotonNetwork.LocalPlayer != null && PhotonNetwork.LocalPlayer.ActorNumber == targetPlayerID)
            {
                Debug.Log(string.Format("[WiseCity WisePro SMS] New message from {0}: {1}", senderName, body));
            }
        }
    }
}
