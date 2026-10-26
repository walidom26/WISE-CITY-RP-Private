using UnityEngine;

namespace UnderdogCity
{
    public class ProximityVoiceSystem : MonoBehaviour
    {
        public static ProximityVoiceSystem Instance { get; private set; }

        [Header("🎙️ الصوت ثلاثي الأبعاد - 3D Proximity Voice")]
        public bool isMicrophoneActive = false;
        public float voiceMaxDistance = 25f;
        public int currentRadioFrequency = 1015; // 101.5 MHz encrypted radio

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void ToggleMicrophone()
        {
            isMicrophoneActive = !isMicrophoneActive;
            Debug.Log("[WiseVoice] Microphone: " + (isMicrophoneActive ? "ON" : "OFF"));
        }

        public void SetRadioFrequency(int freq)
        {
            currentRadioFrequency = freq;
            Debug.Log("[WiseVoice] Radio frequency tuned to: " + freq);
        }
    }
}
