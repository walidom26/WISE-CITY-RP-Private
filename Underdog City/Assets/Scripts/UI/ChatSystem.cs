using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Photon.Pun;

namespace UnderdogCity
{
    public class ChatSystem : MonoBehaviourPunCallbacks
    {
        public static ChatSystem Instance { get; private set; }

        public enum ChatChannel
        {
            Proximity, // قريب
            Radio,     // راديو الفصائل
            Admin,     // شات الإدارة
            Phone      // رسائل الهاتف
        }

        [Header("1️⃣ عناصر واجهة الشات - وايز سيتي")]
        public InputField inputChat;
        public Text txtChatLog;
        public Button btnSend;
        public Button[] channelTabs; // 0: Proximity, 1: Radio, 2: Admin, 3: Phone
        public CanvasGroup chatCanvasGroup;

        [Header("إعدادات القنوات")]
        public ChatChannel currentChannel = ChatChannel.Proximity;
        public float proximityDistance = 25f;

        private struct ChatMessage
        {
            public string senderName;
            public int senderID;
            public string text;
            public ChatChannel channel;
            public Vector3 senderPosition;
        }

        private List<ChatMessage> messageHistory = new List<ChatMessage>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            if (btnSend != null) btnSend.onClick.AddListener(OnSendMessageClicked);
            if (inputChat != null) inputChat.placeholder.GetComponent<Text>().text = "Tap to open chat...";

            for (int i = 0; i < channelTabs.Length; i++)
            {
                int index = i;
                if (channelTabs[i] != null)
                {
                    channelTabs[i].onClick.AddListener(() => SwitchChannel((ChatChannel)index));
                }
            }

            AddLocalSystemMessage("مرحباً بك في مدينة WISE CITY RP. اكتب /help للحصول على التعليمات.");
        }

        public void SwitchChannel(ChatChannel channel)
        {
            currentChannel = channel;
            RefreshChatLogUI();
            Debug.Log("[WiseCity Chat] Switched channel to: " + currentChannel.ToString());
        }

        public void OnSendMessageClicked()
        {
            if (inputChat == null || string.IsNullOrEmpty(inputChat.text.Trim())) return;

            string messageText = inputChat.text.Trim();
            inputChat.text = "";

            int myID = PhotonNetwork.LocalPlayer != null ? PhotonNetwork.LocalPlayer.ActorNumber : 1001;
            string myName = PhotonNetwork.LocalPlayer != null ? PhotonNetwork.LocalPlayer.NickName : "المواطن";

            if (messageText.StartsWith("/"))
            {
                HandleCommand(messageText);
                return;
            }

            Vector3 myPos = transform.position;
            photonView.RPC("BroadcastChatMessageRpc", RpcTarget.All, myName, myID, messageText, (int)currentChannel, myPos);
        }

        [PunRPC]
        void BroadcastChatMessageRpc(string senderName, int senderID, string text, int channelInt, Vector3 senderPosition)
        {
            ChatChannel channel = (ChatChannel)channelInt;

            // التحقق من المسافة لقناة "قريب"
            if (channel == ChatChannel.Proximity)
            {
                float dist = Vector3.Distance(transform.position, senderPosition);
                if (dist > proximityDistance) return;
            }

            ChatMessage msg = new ChatMessage
            {
                senderName = senderName,
                senderID = senderID,
                text = text,
                channel = channel,
                senderPosition = senderPosition
            };

            messageHistory.Add(msg);
            RefreshChatLogUI();
        }

        public void AddLocalSystemMessage(string text)
        {
            ChatMessage msg = new ChatMessage
            {
                senderName = "النظام",
                senderID = 0,
                text = text,
                channel = ChatChannel.Proximity,
                senderPosition = Vector3.zero
            };

            messageHistory.Add(msg);
            RefreshChatLogUI();
        }

        private void RefreshChatLogUI()
        {
            if (txtChatLog == null) return;

            txtChatLog.text = "";
            foreach (var msg in messageHistory)
            {
                if (msg.channel == currentChannel || msg.senderID == 0)
                {
                    string colorPrefix = GetChannelColorHex(msg.channel);
                    txtChatLog.text += string.Format("<color={0}>[{1}] [{2}#{3}]: {4}</color>\n",
                        colorPrefix, GetChannelTag(msg.channel), msg.senderName, msg.senderID, msg.text);
                }
            }
        }

        private string GetChannelTag(ChatChannel channel)
        {
            switch (channel)
            {
                case ChatChannel.Proximity: return "قريب";
                case ChatChannel.Radio: return "راديو";
                case ChatChannel.Admin: return "إدارة";
                case ChatChannel.Phone: return "هاتف";
                default: return "عام";
            }
        }

        private string GetChannelColorHex(ChatChannel channel)
        {
            switch (channel)
            {
                case ChatChannel.Proximity: return "#FFFFFF";
                case ChatChannel.Radio: return "#00FFFF";
                case ChatChannel.Admin: return "#FF4444";
                case ChatChannel.Phone: return "#FFFF00";
                default: return "#FFFFFF";
            }
        }

        private void HandleCommand(string command)
        {
            if (command.Equals("/help", System.StringComparison.OrdinalIgnoreCase))
            {
                AddLocalSystemMessage("الأوامر المتاحة: /me [الفعل], /do [الوصف], /pay [المبلغ], /stats");
            }
            else if (command.StartsWith("/me ", System.StringComparison.OrdinalIgnoreCase))
            {
                string action = command.Substring(4);
                photonView.RPC("BroadcastChatMessageRpc", RpcTarget.All, PhotonNetwork.LocalPlayer.NickName, PhotonNetwork.LocalPlayer.ActorNumber, "* " + action + " *", (int)currentChannel, transform.position);
            }
            else
            {
                AddLocalSystemMessage("أمر غير معروف. اكتب /help للتعليمات.");
            }
        }
    }
}
