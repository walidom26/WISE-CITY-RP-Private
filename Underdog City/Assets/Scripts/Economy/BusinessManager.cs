using System.Collections.Generic;

namespace UnderdogCity
{
    public class BusinessManager : UnityEngine.MonoBehaviour
    {
        public static BusinessManager Instance { get; private set; }

        [System.Serializable]
        public class CommercialShop
        {
            public int shopID;
            public string shopName;
            public string ownerAccountID;
            public double companyBankBalance;
            public double dailyRevenue;
            public List<string> employeesList = new List<string>();
        }

        [UnityEngine.Header("🏪 الشركات والمتاجر الخاصة - Wise Commercial Businesses")]
        public List<CommercialShop> registeredShops = new List<CommercialShop>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else UnityEngine.Object.Destroy(gameObject);
            InitializeShops();
        }

        void InitializeShops()
        {
            registeredShops.Add(new CommercialShop { shopID = 1, shopName = "معرض السيارات الفاخرة", ownerAccountID = AdminAndAntiCheatManager.OWNER_ACCOUNT_ID, companyBankBalance = 10000000.0, dailyRevenue = 250000.0 });
            registeredShops.Add(new CommercialShop { shopID = 2, shopName = "سوبرماركت وسط المدينة", ownerAccountID = "System", companyBankBalance = 500000.0, dailyRevenue = 15000.0 });
        }

        public void RegisterNewShop(string name, string ownerID, double initialCapital)
        {
            CommercialShop shop = new CommercialShop
            {
                shopID = UnityEngine.Random.Range(10, 99),
                shopName = name,
                ownerAccountID = ownerID,
                companyBankBalance = initialCapital,
                dailyRevenue = 0.0
            };

            registeredShops.Add(shop);
            UnityEngine.Debug.Log(string.Format("[BUSINESS] Registered New Company '{0}' for Owner {1} with ${2} capital", name, ownerID, initialCapital));
        }

        public void ProcessSale(int shopID, double saleAmount)
        {
            CommercialShop shop = registeredShops.Find(s => s.shopID == shopID);
            if (shop != null)
            {
                shop.companyBankBalance += saleAmount;
                shop.dailyRevenue += saleAmount;
                UnityEngine.Debug.Log(string.Format("[BUSINESS] Shop '{0}' processed sale: +${1}", shop.shopName, saleAmount));
            }
        }
    }
}
