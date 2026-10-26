using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class RealEstateManager : MonoBehaviour
    {
        public static RealEstateManager Instance { get; private set; }

        public enum PropertyType
        {
            EconomyApartment,
            MediumHouse,
            LuxuryVilla
        }

        [System.Serializable]
        public class RealEstateProperty
        {
            public int propertyID;
            public string propertyName;
            public PropertyType type;
            public double purchasePrice;
            public Vector3 location;
            public bool isOwned;
            public bool hasPrivateGarage;
        }

        [Header("🏡 نظام العقارات والمرائب - Wise City Real Estate")]
        public List<RealEstateProperty> availableProperties = new List<RealEstateProperty>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
            InitializePropertyList();
        }

        void InitializePropertyList()
        {
            availableProperties.Add(new RealEstateProperty { propertyID = 1, propertyName = "شقة الاقتصاد الشاطئية", type = PropertyType.EconomyApartment, purchasePrice = 150000.0, location = new Vector3(100, 5, 200), isOwned = false, hasPrivateGarage = false });
            availableProperties.Add(new RealEstateProperty { propertyID = 2, propertyName = "منزل وسط المدينة", type = PropertyType.MediumHouse, purchasePrice = 500000.0, location = new Vector3(-150, 5, 300), isOwned = false, hasPrivateGarage = true });
            availableProperties.Add(new RealEstateProperty { propertyID = 3, propertyName = "فيلا الأكابر الفاخرة", type = PropertyType.LuxuryVilla, purchasePrice = 2500000.0, location = new Vector3(500, 10, 500), isOwned = false, hasPrivateGarage = true });
        }

        public void PurchaseProperty(int propertyID)
        {
            RealEstateProperty prop = availableProperties.Find(p => p.propertyID == propertyID);
            if (prop == null || prop.isOwned || HUDController.Instance == null) return;

            if (HUDController.Instance.bank >= prop.purchasePrice)
            {
                HUDController.Instance.bank -= prop.purchasePrice;
                prop.isOwned = true;
                HUDController.Instance.UpdatePlayerInfoUI();
                Debug.Log("[WiseCity RealEstate] Purchased " + prop.propertyName + " for $" + prop.purchasePrice);
            }
        }
    }
}
