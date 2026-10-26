using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class ServerAntiCheatValidator : MonoBehaviour
    {
        public static ServerAntiCheatValidator Instance { get; private set; }

        [Header("🛡️ نظام الحماية المتقدم - Server-Authoritative Anti-Cheat")]
        public float maxPlayerMovementSpeed = 12.0f; // Max sprint speed threshold
        public float maxVehicleSpeedKMH = 400.0f;
        public float maxTeleportDistanceThreshold = 100.0f;

        private Dictionary<int, Vector3> lastPlayerPositions = new Dictionary<int, Vector3>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public bool ValidateMovement(int playerID, Vector3 newPosition, float deltaTime)
        {
            if (lastPlayerPositions.TryGetValue(playerID, out Vector3 lastPos))
            {
                float distance = Vector3.Distance(lastPos, newPosition);
                float speed = distance / Mathf.Max(deltaTime, 0.001f);

                if (distance > maxTeleportDistanceThreshold)
                {
                    Debug.LogWarning(string.Format("[ANTI-CHEAT ALERT] Player #{0} Teleport Jump Detected! Distance: {1}m", playerID, distance));
                    FlagViolation(playerID, "Illegal Teleportation Jump");
                    return false;
                }

                if (speed > maxPlayerMovementSpeed)
                {
                    Debug.LogWarning(string.Format("[ANTI-CHEAT ALERT] Player #{0} Speed Hack Detected! Speed: {1}m/s", playerID, speed));
                    FlagViolation(playerID, "Movement Speed Hack");
                    return false;
                }
            }

            lastPlayerPositions[playerID] = newPosition;
            return true;
        }

        public bool ValidateTransaction(string senderID, string receiverID, double amount)
        {
            if (amount <= 0 || double.IsNaN(amount) || double.IsInfinity(amount))
            {
                Debug.LogWarning(string.Format("[ANTI-CHEAT ALERT] Invalid Transaction Amount from {0}: {1}", senderID, amount));
                return false;
            }

            return true;
        }

        private void FlagViolation(int playerID, string reason)
        {
            if (AdminAndAntiCheatManager.Instance != null)
            {
                AdminAndAntiCheatManager.Instance.LogAuditAction("CHEAT_VIOLATION", string.Format("Player #{0} flagged for: {1}", playerID, reason), "FLAGGED");
            }
        }
    }
}
