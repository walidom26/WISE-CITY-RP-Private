using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class ChatSystem : MonoBehaviour
    {
        public static ChatSystem Instance { get; private set; }

        public enum ChatChannel
        {
            Nearby = 0, // قريب
            Radio = 1,  // راديو الفصيل
            Admin = 2,  // إدارة
            Phone = 3   // هاتف
        }

        [Header("💬 نظام الشات وإدارة القنوات - وايز سيتي")]
        public ChatChannel currentChannel = ChatChannel.Nearby;
        public float nearbyChatDistance = 20f;
        public int maxMessageHistory = 50;

        [System.Serializable]
        public struct ChatMessage
        {
            public string senderName;
            public int senderID;
            public string messageText;
            public ChatChannel channel;
            public Vector3 senderPosition;
        }

        public List<ChatMessage> chatHistory = new List<ChatMessage>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void SendChatMessage(string text)
        {
            if (string.IsNullOrEmpty(text)) return;

            int myID = 1001;
            string myName = "المواطن";

            if (AccountManager.Instance != null && AccountManager.Instance.currentAccount != null)
            {
                myName = AccountManager.Instance.currentAccount.username;
            }

            Vector3 pos = Vector3.zero;
            if (PlayerController.LocalPlayer != null) pos = PlayerController.LocalPlayer.transform.position;

            BroadcastChatMessageRpc(myName, myID, text, (int)currentChannel, pos);
        }

        void BroadcastChatMessageRpc(string senderName, int senderID, string text, int channelInt, Vector3 senderPosition)
        {
            ChatChannel channel = (ChatChannel)channelInt;

            Vector3 myPos = Vector3.zero;
            if (PlayerController.LocalPlayer != null) myPos = PlayerController.LocalPlayer.transform.position;

            if (channel == ChatChannel.Nearby && Vector3.Distance(myPos, senderPosition) > nearbyChatDistance)
            {
                return;
            }

            ChatMessage msg = new ChatMessage
            {
                senderName = senderName,
                senderID = senderID,
                messageText = text,
                channel = channel,
                senderPosition = senderPosition
            };

            chatHistory.Add(msg);
            if (chatHistory.Count > maxMessageHistory) chatHistory.RemoveAt(0);

            if (HUDManager.Instance != null)
            {
                HUDManager.Instance.AddChatMessage(channel.ToString(), senderName + ": " + text);
            }
        }
    }
}
