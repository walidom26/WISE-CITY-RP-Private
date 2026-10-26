using UnityEngine;
using System.Security.Cryptography;
using System.Text;
using System.IO;

namespace UnderdogCity
{
    public class AccountManager : MonoBehaviour
    {
        public static AccountManager Instance { get; private set; }

        public const string OWNER_ACCOUNT_ID = "mr_walid_owner_id";

        [System.Serializable]
        public class UserAccount
        {
            public string accountID;
            public string username;
            public string passwordHash;
            public string salt;
            public Vector3 lastSavedPosition;
            public decimal cashBalance;
            public decimal bankBalance;
            public int levelXP;
        }

        public UserAccount currentAccount;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[AccountManager] Account system initialized.");
            RegisterOrLogin("walid_om", "WiseCity2025!");
        }

        public void RegisterOrLogin(string username, string password)
        {
            string salt = GenerateSalt();
            string hash = HashPassword(password, salt);

            currentAccount = new UserAccount
            {
                accountID = username == "walid_om" ? OWNER_ACCOUNT_ID : System.Guid.NewGuid().ToString(),
                username = username,
                passwordHash = hash,
                salt = salt,
                lastSavedPosition = new Vector3(0, 2f, 0),
                cashBalance = 50000m,
                bankBalance = 250000m,
                levelXP = 1
            };

            Debug.Log(string.Format("[AccountManager] ✅ User Logged In: {0} | ID: {1}", currentAccount.username, currentAccount.accountID));
        }

        public string HashPassword(string password, string salt)
        {
            using (SHA256 sha256 = SHA256.Create())
            {
                byte[] bytes = sha256.ComputeHash(Encoding.UTF8.GetBytes(password + salt));
                StringBuilder builder = new StringBuilder();
                foreach (byte b in bytes) builder.Append(b.ToString("x2"));
                return builder.ToString();
            }
        }

        private string GenerateSalt()
        {
            byte[] saltBytes = new byte[16];
            using (RNGCryptoServiceProvider rng = new RNGCryptoServiceProvider())
            {
                rng.GetBytes(saltBytes);
            }
            return System.Convert.ToBase64String(saltBytes);
        }

        public Vector3 GetSafeSpawnPosition()
        {
            if (currentAccount != null && currentAccount.lastSavedPosition != Vector3.zero)
            {
                return currentAccount.lastSavedPosition + Vector3.up * 1f;
            }
            return new Vector3(0, 2f, 0); // Fallback spawn point
        }
    }
}
