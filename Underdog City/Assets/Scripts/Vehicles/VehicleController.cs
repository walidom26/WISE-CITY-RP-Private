using UnityEngine;

namespace UnderdogCity
{
    public class VehicleController : MonoBehaviour
    {
        public enum VehicleClass
        {
            Economy,
            Family,
            Sport,
            Luxury,
            SUV,
            Pickup,
            Bus,
            Truck,
            Motorcycle
        }

        [Header("🚗 تصنيف وخصائص المركبة - Wise City Vehicles")]
        public VehicleClass vehicleClass = VehicleClass.Sport;
        public string vehicleName = "Wise-Sport-GT";
        public bool canEnter = true;

        [Header("الوقود والسرعة - Fuel & Physics")]
        public float currentFuel = 100f;
        public float maxFuel = 100f;
        public float fuelConsumptionRate = 0.8f;
        public float motorPower = 4500f;
        public float maxSteerAngle = 35f;

        [Header("الصحة والضرر")]
        public float vehicleHealth = 100f;

        public PlayerController currentDriver = null;

        private Rigidbody rb;
        private WheelCollider[] wheels;

        private void Awake()
        {
            rb = GetComponent<Rigidbody>();
            if (rb == null) rb = gameObject.AddComponent<Rigidbody>();
            rb.mass = 1500f;

            wheels = GetComponentsInChildren<WheelCollider>();
            if (wheels.Length == 0)
            {
                CreateDefaultWheelColliders();
            }
        }

        private void CreateDefaultWheelColliders()
        {
            GameObject wheelsGroup = new GameObject("WheelsGroup");
            wheelsGroup.transform.parent = transform;
            wheelsGroup.transform.localPosition = Vector3.zero;

            Vector3[] wheelPositions = new Vector3[]
            {
                new Vector3(-0.9f, 0f, 1.4f),
                new Vector3(0.9f, 0f, 1.4f),
                new Vector3(-0.9f, 0f, -1.4f),
                new Vector3(0.9f, 0f, -1.4f)
            };

            wheels = new WheelCollider[4];
            for (int i = 0; i < 4; i++)
            {
                GameObject wObj = new GameObject("WheelCollider_" + i);
                wObj.transform.parent = wheelsGroup.transform;
                wObj.transform.localPosition = wheelPositions[i];
                wheels[i] = wObj.AddComponent<WheelCollider>();
            }
        }

        private void Update()
        {
            if (currentDriver != null)
            {
                HandleDrivingControls();
            }
        }

        private void HandleDrivingControls()
        {
            float motor = Input.GetAxis("Vertical") * motorPower;
            float steer = Input.GetAxis("Horizontal") * maxSteerAngle;

            if (currentFuel <= 0) motor = 0;
            else if (Mathf.Abs(motor) > 0.1f)
            {
                currentFuel -= fuelConsumptionRate * Time.deltaTime;
            }

            if (wheels != null && wheels.Length >= 4)
            {
                wheels[0].steerAngle = steer;
                wheels[1].steerAngle = steer;
                wheels[2].motorTorque = motor;
                wheels[3].motorTorque = motor;
            }
        }

        public void EnterVehicle(PlayerController player)
        {
            if (currentDriver != null) return;

            currentDriver = player;
            player.SetDrivingState(true, this);
            player.transform.position = transform.position;
            player.transform.parent = transform;

            Debug.Log("[VehicleController] Player entered vehicle: " + vehicleName);
        }

        public void ExitVehicle(PlayerController player)
        {
            if (currentDriver != player) return;

            currentDriver = null;
            player.transform.parent = null;
            player.transform.position = transform.position + transform.right * 2f;
            player.SetDrivingState(false, null);

            Debug.Log("[VehicleController] Player exited vehicle: " + vehicleName);
        }

        public float GetSpeedKMH()
        {
            return rb != null ? rb.velocity.magnitude * 3.6f : 0f;
        }

        public void Refuel(float amount)
        {
            currentFuel = Mathf.Min(currentFuel + amount, maxFuel);
            Debug.Log("[VehicleController] Refueled vehicle. Current fuel: " + currentFuel);
        }
    }
}
