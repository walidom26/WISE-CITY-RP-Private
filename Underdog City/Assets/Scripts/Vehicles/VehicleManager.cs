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
            public int engineTuningLevel;
            public bool hasTurbo;
            public string customLicensePlate;
        }

        [Header("🏎️ أسطول السيارات المتقدم - Wise City Fleet")]
        public List<VehicleData> vehicleCatalog = new List<VehicleData>();
        public List<VehicleData> playerOwnedVehicles = new List<VehicleData>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);

            InitializeCatalog();
        }

        void InitializeCatalog()
        {
            vehicleCatalog.Add(new VehicleData { modelName = "Wise-Civic-Common", tier = VehicleTier.Common, basePrice = 25000.0, topSpeedKMH = 160f, paintColor = Color.white, engineTuningLevel = 0, hasTurbo = false, customLicensePlate = "WISE-101" });
            vehicleCatalog.Add(new VehicleData { modelName = "Wise-Mustang-Epic", tier = VehicleTier.Epic, basePrice = 120000.0, topSpeedKMH = 240f, paintColor = Color.red, engineTuningLevel = 1, hasTurbo = true, customLicensePlate = "WISE-777" });
            vehicleCatalog.Add(new VehicleData { modelName = "Wise-Hyper-Legendary", tier = VehicleTier.Legendary, basePrice = 850000.0, topSpeedKMH = 340f, paintColor = Color.yellow, engineTuningLevel = 3, hasTurbo = true, customLicensePlate = "WISE-999" });
            vehicleCatalog.Add(new VehicleData { modelName = "Wise-Secret-GTR", tier = VehicleTier.SecretRare, basePrice = 2500000.0, topSpeedKMH = 380f, paintColor = Color.black, engineTuningLevel = 4, hasTurbo = true, customLicensePlate = "MR-WALID" });
        }

        public void UpgradeEngine(VehicleData vehicle)
        {
            if (vehicle != null && vehicle.engineTuningLevel < 5)
            {
                vehicle.engineTuningLevel++;
                vehicle.topSpeedKMH += 15f;
                Debug.Log(string.Format("[WISE GARAGE] Upgraded engine to Level {0} | New Top Speed: {1} KM/H", vehicle.engineTuningLevel, vehicle.topSpeedKMH));
            }
        }

        public void ApplyCustomPaint(CarPhysics car, Color newColor)
        {
            if (car != null && car.bodyMeshRenderer != null)
            {
                car.bodyMeshRenderer.material.color = newColor;
                Debug.Log("[WiseCity Garage] Custom paint applied to " + car.name);
            }
        }

        public void RepairVehicle(VehicleController vehicle)
        {
            if (vehicle != null)
            {
                vehicle.vehicleHealth = 100f;
                Debug.Log("[WISE GARAGE] Vehicle repaired to 100% health.");
            }
        }
    }
}
