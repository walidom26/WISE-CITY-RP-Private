using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using Photon.Pun;

namespace UnderdogCity
{
    public class PlayerMechanics : MonoBehaviourPunCallbacks
    {
        public static PlayerMechanics Instance { get; private set; }

        public enum BodyLimb
        {
            Head,
            Torso,
            LeftArm,
            RightArm,
            LeftLeg,
            RightLeg
        }

        [System.Serializable]
        public class ForensicEvidence
        {
            public string evidenceID;
            public Vector3 location;
            public int suspectPlayerID;
            public string fingerprintHash;
            public string weaponCaliber;
        }

        [Header("8️⃣ الأنظمة المتقدمة، الجرائم، والأنيميشن - وايز سيتي")]
        public bool hasDMVDriverLicense = true;
        public bool isLegInjured = false;
        public float baseWalkSpeed = 10f;
        public float injuredSpeedMultiplier = 0.45f; // تقليل السرعة عند إصابة القدمين

        public bool isCarryingPlayer = false;
        public int carriedPlayerID = 0;

        public List<ForensicEvidence> crimeSceneEvidences = new List<ForensicEvidence>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity Mechanics] Advanced Player Mechanics, DMV License & Forensics initialized.");
        }

        public void TakeLimbDamage(BodyLimb limb, float damage)
        {
            if (limb == BodyLimb.LeftLeg || limb == BodyLimb.RightLeg)
            {
                isLegInjured = true;
                Debug.Log("[WiseCity Health] Leg injury sustained! Movement speed reduced.");
            }
        }

        public float GetAdjustedMovementSpeed()
        {
            return isLegInjured ? (baseWalkSpeed * injuredSpeedMultiplier) : baseWalkSpeed;
        }

        public void DropForensicEvidenceAtPosition(Vector3 position, int suspectID, string fingerprint)
        {
            ForensicEvidence ev = new ForensicEvidence
            {
                evidenceID = "EVID-" + UnityEngine.Random.Range(10000, 99999),
                location = position,
                suspectPlayerID = suspectID,
                fingerprintHash = fingerprint,
                weaponCaliber = "9mm"
            };
            crimeSceneEvidences.Add(ev);
            Debug.Log("[WiseCity Forensics] Bullet shell / Fingerprint evidence logged at scene.");
        }

        public void PlayEmote(string emoteTriggerName)
        {
            Animator anim = GetComponentInChildren<Animator>();
            if (anim != null)
            {
                anim.SetTrigger(emoteTriggerName);
                Debug.Log("[WiseCity Emotes] Playing animation emote: " + emoteTriggerName);
            }
        }

        public void ToggleCarryPlayer(int targetPlayerID)
        {
            isCarryingPlayer = !isCarryingPlayer;
            carriedPlayerID = isCarryingPlayer ? targetPlayerID : 0;
            Debug.Log(string.Format("[WiseCity Carry] {0} player ID: {1}", isCarryingPlayer ? "Carrying" : "Dropped", targetPlayerID));
        }

        public bool CheckDMVLicenseBeforeDriving()
        {
            if (!hasDMVDriverLicense)
            {
                Debug.LogWarning("[WiseCity DMV] Driving without a valid driver's license!");
                return false;
            }
            return true;
        }
    }
}
