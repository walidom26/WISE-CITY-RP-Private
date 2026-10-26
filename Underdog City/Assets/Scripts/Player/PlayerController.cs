using UnityEngine;

namespace UnderdogCity
{
    [RequireComponent(typeof(CharacterController))]
    public class PlayerController : MonoBehaviour
    {
        public static PlayerController LocalPlayer { get; private set; }

        [Header("🎮 تحكم اللاعب والتنقل - Player Controller")]
        public float walkSpeed = 4.5f;
        public float runSpeed = 8.0f;
        public float jumpHeight = 1.8f;
        public float gravity = -15f;

        [Header("حالة اللاعب والركوب")]
        public bool isDriving = false;
        public VehicleController currentVehicle = null;

        private CharacterController characterController;
        private Vector3 velocity;
        private Transform cameraTransform;

        // Touch Joystick Controls
        public Vector2 touchJoystickInput;
        public Vector2 touchLookInput;

        private void Awake()
        {
            characterController = GetComponent<CharacterController>();
            LocalPlayer = this;
        }

        private void Start()
        {
            if (Camera.main != null)
            {
                cameraTransform = Camera.main.transform;
            }
        }

        private void Update()
        {
            if (!isDriving)
            {
                HandlePlayerMovement();
            }

            HandleVehicleInteractionCheck();
        }

        private void HandlePlayerMovement()
        {
            float horizontal = Input.GetAxis("Horizontal") + touchJoystickInput.x;
            float vertical = Input.GetAxis("Vertical") + touchJoystickInput.y;

            Vector3 moveDirection = new Vector3(horizontal, 0f, vertical).normalized;

            if (moveDirection.magnitude >= 0.1f)
            {
                float targetAngle = Mathf.Atan2(moveDirection.x, moveDirection.z) * Mathf.Rad2Deg + cameraTransform.eulerAngles.y;
                transform.rotation = Quaternion.Euler(0f, targetAngle, 0f);

                Vector3 moveDir = Quaternion.Euler(0f, targetAngle, 0f) * Vector3.forward;
                float speed = Input.GetKey(KeyCode.LeftShift) ? runSpeed : walkSpeed;

                characterController.Move(moveDir.normalized * speed * Time.deltaTime);
            }

            if (characterController.isGrounded)
            {
                if (velocity.y < 0) velocity.y = -2f;

                if (Input.GetButtonDown("Jump"))
                {
                    velocity.y = Mathf.Sqrt(jumpHeight * -2f * gravity);
                }
            }

            velocity.y += gravity * Time.deltaTime;
            characterController.Move(velocity * Time.deltaTime);

            if (cameraTransform != null)
            {
                Vector3 targetCamPos = transform.position + new Vector3(0, 3f, -5f);
                cameraTransform.position = Vector3.Lerp(cameraTransform.position, targetCamPos, Time.deltaTime * 5f);
                cameraTransform.LookAt(transform.position + Vector3.up * 1.5f);
            }
        }

        private void HandleVehicleInteractionCheck()
        {
            if (Input.GetKeyDown(KeyCode.F) || Input.GetKeyDown(KeyCode.E))
            {
                if (isDriving && currentVehicle != null)
                {
                    currentVehicle.ExitVehicle(this);
                }
                else
                {
                    Collider[] colliders = Physics.OverlapSphere(transform.position, 4f);
                    foreach (var col in colliders)
                    {
                        VehicleController vehicle = col.GetComponentInParent<VehicleController>();
                        if (vehicle != null && vehicle.canEnter)
                        {
                            vehicle.EnterVehicle(this);
                            break;
                        }
                    }
                }
            }
        }

        public void SetDrivingState(bool driving, VehicleController vehicle)
        {
            isDriving = driving;
            currentVehicle = vehicle;
            characterController.enabled = !driving;

            Renderer[] renderers = GetComponentsInChildren<Renderer>();
            foreach (var r in renderers) r.enabled = !driving;

            Debug.Log("[PlayerController] Driving state updated: " + driving);
        }
    }
}
