using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class RealEstateManager : MonoBehaviour
    {
        public static RealEstateManager Instance { get; private set; }

        public enum PropertyTier
        {
            EconomyApartment, // شقق اقتصادية
            MediumHouse,      // منازل متوسطة
            LuxuryVilla       // فلل وقصور فاخرة بمرائب خاصة
        }

        [System.Serializable]
        public class Property
        {
            public string propertyID;
            public string title;
            public PropertyTier tier;
            public double purchasePrice;
            public Vector3 doorCoordinates;
            public string ownerName;
            public bool isOwned;
            public bool hasPrivateGarage;
            public int garageCapacity;
        }

        [Header("7️⃣ العقارات والفلل والقصور - وايز سيتي")]
        public List<Property> propertiesCatalog = new List<Property>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            InitializeDefaultProperties();
            Debug.Log("[WiseCity Real Estate] Tiered Real Estate Manager initialized.");
        }

        private void InitializeDefaultProperties()
        {
            propertiesCatalog.Add(new Property { propertyID = "PROP-101", title = "شقة وسط المدينة الاقتصادية", tier = PropertyTier.EconomyApartment, purchasePrice = 45000, doorCoordinates = new Vector3(120, 0, -80), ownerName = "", isOwned = false, hasPrivateGarage = false, garageCapacity = 0 });
            propertiesCatalog.Add(new Property { propertyID = "PROP-202", title = "منزل حي الياسمين المتوسط", tier = PropertyTier.MediumHouse, purchasePrice = 180000, doorCoordinates = new Vector3(-250, 0, 310), ownerName = "", isOwned = false, hasPrivateGarage = true, garageCapacity = 2 });
            propertiesCatalog.Add(new Property { propertyID = "PROP-303", title = "قصر التلة الملكي والفيلا الفاخرة", tier = PropertyTier.LuxuryVilla, purchasePrice = 1200000, doorCoordinates = new Vector3(400, 0, 500), ownerName = "", isOwned = false, hasPrivateGarage = true, garageCapacity = 6 });
        }

        public bool BuyProperty(string propertyID, string buyerName)
        {
            Property prop = propertiesCatalog.Find(p => p.propertyID == propertyID && !p.isOwned);
            if (prop != null)
            {
                if (HUDController.Instance != null && HUDController.Instance.bank >= prop.purchasePrice)
                {
                    HUDController.Instance.bank -= prop.purchasePrice;
                    HUDController.Instance.UpdatePlayerInfoUI();

                    prop.isOwned = true;
                    prop.ownerName = buyerName;
                    Debug.Log(string.Format("[WiseCity Real Estate] Purchased {0} for ${1:N0} by {2}", prop.title, prop.purchasePrice, buyerName));
                    return true;
                }
            }
            return false;
        }
    }
}
