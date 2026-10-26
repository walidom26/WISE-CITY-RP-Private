using UnityEngine;

namespace UnderdogCity
{
    public class PlayerMechanics : MonoBehaviour
    {
        public static PlayerMechanics Instance { get; private set; }

        public enum InjuredLimb
        {
            None,
            LeftLeg,
            RightLeg,
            LeftArm,
            RightArm
        }

        [Header("🩸 الحالة الصحية والجرائم - Health & Forensics")]
        public InjuredLimb currentInjury = InjuredLimb.None;
        public bool hasValidDMVLicense = true;
        public bool isCarryingBody = false;

        private PlayerController playerController;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            playerController = GetComponent<PlayerController>();
        }

        public void ApplyLimbDamage(InjuredLimb limb)
        {
            currentInjury = limb;
            if (limb == InjuredLimb.LeftLeg || limb == InjuredLimb.RightLeg)
            {
                if (playerController != null) playerController.walkSpeed = 2.0f; // Slow down
            }
            Debug.Log("[WiseCity Health] Injury applied: " + limb);
        }

        public void DropForensicEvidence(Vector3 pos, string evidenceType)
        {
            Debug.Log(string.Format("[WiseCity Forensics] Dropped {0} evidence at {1}", evidenceType, pos));
        }
    }
}
