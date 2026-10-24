using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class ProximityVoiceSystem : MonoBehaviour
    {
        public static ProximityVoiceSystem Instance { get; private set; }

        [Header("8️⃣ الصوتيات ثلاثية الأبعاد والراديو - وايز سيتي")]
        public float voiceMaxDistance = 20f;
        public float whisperDistance = 5f;
        public float shoutDistance = 40f;

        public bool isTransmittingVoice = false;
        public int currentEncryptedRadioFrequency = 100; // ترددات الفصائل

        public GameObject hudSpeakingIndicator;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity 3D Voice] Proximity Voice & Encrypted Radio System initialized.");
        }

        public void SetVoiceTransmission(bool active)
        {
            isTransmittingVoice = active;
            if (hudSpeakingIndicator != null)
            {
                hudSpeakingIndicator.SetActive(isTransmittingVoice);
            }
            Debug.Log("[WiseCity 3D Voice] Transmission status: " + (active ? "TALKING" : "SILENT"));
        }

        public float CalculateSpatialVolume(Vector3 listenerPosition, Vector3 speakerPosition, float maxDist)
        {
            float distance = Vector3.Distance(listenerPosition, speakerPosition);
            if (distance > maxDist) return 0f;
            return Mathf.Clamp01(1f - (distance / maxDist));
        }

        public void TuneRadioFrequency(int frequency)
        {
            currentEncryptedRadioFrequency = frequency;
            Debug.Log("[WiseCity Encrypted Radio] Radio tuned to encrypted channel: " + frequency + " MHz");
        }
    }
}
