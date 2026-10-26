using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class VehicleManager : MonoBehaviour
    {
        public static VehicleManager Instance { get; private set; }

        public enum VehicleTier
        {
            Common,   // السيارات الشائعة الاقتصادية
            Epic,     // السيارات الملحمية الرياضية
            Legendary,// السيارات الأسطورية الفائقة
            SecretRare// السيارات السرية النادرة
        }

        [System.Serializable]
        public class VehicleData
        {
            public string modelName;
            public VehicleTier tier;
            public double basePrice;
            public float topSpeedKMH;
            public Color paintColor;
        }

        [Header("🏎️ أسطول السيارات المتقدم - Wise City Fleet")]
        public List<VehicleData> vehicleCatalog = new List<VehicleData>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);

            InitializeCatalog();
        }

        void InitializeCatalog()
        {
            vehicleCatalog.Add(new VehicleData { modelName = "Wise-Civic-Common", tier = VehicleTier.Common, basePrice = 25000.0, topSpeedKMH = 160f, paintColor = Color.white });
            vehicleCatalog.Add(new VehicleData { modelName = "Wise-Mustang-Epic", tier = VehicleTier.Epic, basePrice = 120000.0, topSpeedKMH = 240f, paintColor = Color.red });
            vehicleCatalog.Add(new VehicleData { modelName = "Wise-Hyper-Legendary", tier = VehicleTier.Legendary, basePrice = 850000.0, topSpeedKMH = 340f, paintColor = Color.yellow });
            vehicleCatalog.Add(new VehicleData { modelName = "Wise-Secret-GTR", tier = VehicleTier.SecretRare, basePrice = 2500000.0, topSpeedKMH = 380f, paintColor = Color.black });
        }

        public void ApplyCustomPaint(CarPhysics car, Color newColor)
        {
            if (car != null && car.bodyMeshRenderer != null)
            {
                car.bodyMeshRenderer.material.color = newColor;
                Debug.Log("[WiseCity Garage] Custom paint applied to " + car.name);
            }
        }
    }
}
