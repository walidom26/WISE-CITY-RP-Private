using UnityEngine;

namespace UnderdogCity
{
    [RequireComponent(typeof(Rigidbody))]
    public class CarPhysics : MonoBehaviour
    {
        [Header("إدخلات قيادة السيارة - وايز سيتي")]
        public float forwardInput;
        public float steerInput;

        protected Rigidbody Rigidbody;
        
        [Header("توازن وسياسة المركبة")]
        public Vector3 CenterOfMass = Vector3.zero;

        [Header("نظام الوقود")]
        public float fuelAmount = 100f;
        public float maxFuel = 100f;

        [Header("نظام الضرر والاصطدام")]
        public float vehicleHealth = 100f;

        [Header("خصائص الثبات والقوة")]
        public float Grip = 5f;
        public float MotorPower = 4500f;
        public MeshRenderer bodyMeshRenderer;

        void Awake()
        {
            Rigidbody = GetComponent<Rigidbody>();
            Rigidbody.centerOfMass = CenterOfMass;
        }

        void FixedUpdate()
        {
            if (Mathf.Abs(forwardInput) > 0.05f && fuelAmount > 0)
            {
                fuelAmount -= Time.fixedDeltaTime * 0.01f;
            }
        }
    }
}
