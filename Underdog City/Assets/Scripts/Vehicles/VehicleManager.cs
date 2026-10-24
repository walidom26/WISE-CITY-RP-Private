using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class VehicleManager : MonoBehaviour
    {
        public static VehicleManager Instance { get; private set; }

        public enum VehicleTier
        {
            Common,     // شائعة (اقتصادية)
            Epic,       // ملحمية (رياضية متوسطة)
            Legendary,  // أسطورية (خارقة)
            SecretRare  // سرية / حصرية (مزادات وعصابات)
        }

        public enum PaintFinish
        {
            Metallic, // ماتليقي
            Matte,    // مطفي
            Chrome    // كروم
        }

        [System.Serializable]
        public class VehicleCustomization
        {
            public Color primaryColor = Color.black;
            public PaintFinish paintType = PaintFinish.Metallic;
            public int engineTuningStage = 0; // 0 to 3
            public bool hasTurbo = false;
        }

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            Debug.Log("[WiseCity Vehicles] Vehicle Manager initialized with Tiered Vehicle System.");
        }

        public void RefuelVehicle(CarPhysics car, float amount)
        {
            if (car != null)
            {
                car.fuelAmount = Mathf.Clamp(car.fuelAmount + amount, 0f, car.maxFuel);
                Debug.Log(string.Format("[WiseCity Gas Station] Refueled vehicle. Current fuel: {0}/{1}", car.fuelAmount, car.maxFuel));
            }
        }

        public void TuneEngine(CarPhysics car, int stage, bool turbo)
        {
            if (car != null)
            {
                car.MotorPower += stage * 1000f;
                if (turbo) car.MotorPower += 1500f;
                Debug.Log(string.Format("[WiseCity Workshop] Engine tuned to Stage {0}. Motor Power: {1}", stage, car.MotorPower));
            }
        }

        public void ApplyCustomPaint(CarPhysics car, Color color, PaintFinish finish)
        {
            if (car != null && car.bodyMeshRenderer != null)
            {
                car.bodyMeshRenderer.material.color = color;
                switch (finish)
                {
                    case PaintFinish.Metallic:
                        car.bodyMeshRenderer.material.SetFloat("_Metallic", 0.9f);
                        car.bodyMeshRenderer.material.SetFloat("_Glossiness", 0.9f);
                        break;
                    case PaintFinish.Matte:
                        car.bodyMeshRenderer.material.SetFloat("_Metallic", 0.0f);
                        car.bodyMeshRenderer.material.SetFloat("_Glossiness", 0.1f);
                        break;
                    case PaintFinish.Chrome:
                        car.bodyMeshRenderer.material.SetFloat("_Metallic", 1.0f);
                        car.bodyMeshRenderer.material.SetFloat("_Glossiness", 1.0f);
                        break;
                }
                Debug.Log("[WiseCity Workshop] Paint applied successfully: " + finish.ToString());
            }
        }
    }
}
