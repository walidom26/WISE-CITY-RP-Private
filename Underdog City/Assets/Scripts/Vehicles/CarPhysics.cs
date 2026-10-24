using Photon.Pun;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace UnderdogCity
{
    [RequireComponent(typeof(Rigidbody))]
    public class CarPhysics : MonoBehaviourPun, IPunObservable {

        [Header("إدخالات قيادة السيارة - وايز سيتي")]
        [HideInInspector]
        public InputStr Input;
        public struct InputStr
        {
            public float Forward; // الحركة للأمام/الخلف
            public float Steer;   // التوجيه يميناً/يساراً
        }

        protected Rigidbody Rigidbody;
        
        [Header("توازن وسياسة المركبة")]
        public Vector3 CenterOfMass = Vector3.zero; // مركز ثقل السيارة

        [HideInInspector]
        public NetworkStr Network;
        public struct NetworkStr
        {
            public Vector3 Position;
            public Quaternion Rotation;
        }

        [Header("حالة المركبة")]
        [HideInInspector]
        public CarState State;

        [Header("تصنيف جودة السيارة والطلاء")]
        public VehicleManager.VehicleTier vehicleTier = VehicleManager.VehicleTier.Common;
        public MeshRenderer bodyMeshRenderer;

        [Header("نظام الوقود (Fuel System)")]
        public float fuelAmount = 100f;
        public float maxFuel = 100f;
        public float fuelConsumptionRate = 0.5f; // استهلاك الوقود لكل دقيقة قيادة

        [Header("نظام الضرر والاصطدام (Damage & Smoke)")]
        public float vehicleHealth = 100f;
        public ParticleSystem smokeEffect;
        public GameObject[] glassWindows;

        [Header("قائمة العجلات والإطارات")]
        public WheelInfo[] Wheels;

        [Header("خصائص المحرك والقيادة")]
        public float MotorPower = 5000f; // قوة المحرك
        public float SteerAngle = 35f;   // زاوية الانعطاف

        [Header("إعدادات الثبات وقوة الاحتكاك")]
        [Range(0, 1)]
        public float KeepGrip = 1f;
        public float Grip = 5f; // مستوى ثبات السيارة على الطريق
        
        // التهيئة عند التشغيل
        void Awake () {
            Rigidbody = GetComponent<Rigidbody>();
            Rigidbody.centerOfMass = CenterOfMass;
            OnValidate();
            
            Debug.Log("تم تفعيل نظام فيزياء السيارات بنجاح لسيرفر WISE-CITY-RP!");
        }

        void FixedUpdate()
        {
            // استهلاك الوقود عند الحركة
            if (State == CarState.OCCUPIED && Mathf.Abs(Input.Forward) > 0.05f)
            {
                if (fuelAmount > 0)
                {
                    fuelAmount -= fuelConsumptionRate * Time.fixedDeltaTime / 60f;
                }
                else
                {
                    fuelAmount = 0f;
                    Input.Forward = 0f; // توقف المحرك عند نفاد البنزين
                }
            }

            // تطبيق قوة المحرك والتوجيه ودوران العجلات
            for(int i = 0; i < Wheels.Length; i++)
            {
                if (Wheels[i].WheelCollider != null)
                {
                    if (Wheels[i].Motor)
                        Wheels[i].WheelCollider.motorTorque = (fuelAmount > 0 ? Input.Forward : 0) * MotorPower;
                    if (Wheels[i].Steer)
                        Wheels[i].WheelCollider.steerAngle = Input.Steer * SteerAngle;

                    Wheels[i].Rotation += Wheels[i].WheelCollider.rpm / 60 * 360 * Time.fixedDeltaTime;

                    if (Wheels[i].MeshRenderer != null && Wheels[i].MeshRenderer.parent != null)
                    {
                        Wheels[i].MeshRenderer.localRotation = Wheels[i].MeshRenderer.parent.localRotation * Quaternion.Euler(Wheels[i].Rotation, -Wheels[i].WheelCollider.steerAngle, 0);
                    }
                }
            }

            // إضافة قوة الثبات السفلية ومنع الانقلاب
            Rigidbody.AddForceAtPosition(transform.up * Rigidbody.velocity.magnitude * -0.1f * Grip, transform.position + transform.rotation * CenterOfMass);

            // تحريك ومزامنة موقع السيارة للاعبين الآخرين عبر الشبكة
            if (!photonView.IsMine)
            {
                Rigidbody.position = Vector3.MoveTowards(Rigidbody.position, Network.Position, Time.fixedDeltaTime);
                Rigidbody.rotation = Quaternion.RotateTowards(Rigidbody.rotation, Network.Rotation, Time.fixedDeltaTime * 90f);
            }
        }

        private void OnCollisionEnter(Collision collision)
        {
            float impactForce = collision.relativeVelocity.magnitude;
            if (impactForce > 5f)
            {
                vehicleHealth -= impactForce * 2f;
                if (vehicleHealth < 40f && smokeEffect != null && !smokeEffect.isPlaying)
                {
                    smokeEffect.Play();
                }

                if (vehicleHealth < 20f && glassWindows != null)
                {
                    foreach (var glass in glassWindows)
                    {
                        if (glass != null) glass.SetActive(false); // انكسار الزجاج
                    }
                }
            }
        }

        void OnDrawGizmos()
        {
            // رسم مركز ثقل السيارة في محرر Unity
            Gizmos.color = Color.red;
            Gizmos.DrawWireSphere(transform.position + CenterOfMass, .1f);
            Gizmos.DrawWireSphere(transform.position + CenterOfMass, .11f);
        }

        void OnValidate()
        {
            // ضبط احتكاك العجلات وثباتها تلقائياً
            if (Wheels == null) return;

            for (int i = 0; i < Wheels.Length; i++)
            {
                if (Wheels[i].WheelCollider == null) continue;

                var ffriction = Wheels[i].WheelCollider.forwardFriction;
                var sfriction = Wheels[i].WheelCollider.sidewaysFriction;

                ffriction.asymptoteValue = Wheels[i].WheelCollider.forwardFriction.extremumValue * KeepGrip * 0.998f + 0.002f;
            sfriction.extremumSlip = 0.5f;
                ffriction.extremumSlip = 1f;
                ffriction.asymptoteSlip = 2f;
                ffriction.stiffness = Grip;

                sfriction.extremumValue = 1f;
                sfriction.asymptoteValue = Wheels[i].WheelCollider.sidewaysFriction.extremumValue * KeepGrip * 0.998f + 0.002f;
                sfriction.extremumValue = 1f;
                sfriction.asymptoteSlip = 1f;
                sfriction.stiffness = Grip;

                Wheels[i].WheelCollider.forwardFriction = ffriction;
                Wheels[i].WheelCollider.sidewaysFriction = sfriction;
            }
        }

        // مزامنة حركة السيارة وسرعتها أونلاين عبر شبكة الفوتون (Photon)
        public void OnPhotonSerializeView(PhotonStream stream, PhotonMessageInfo info)
        {
            if (stream.IsWriting)
            {
                if (!photonView.IsMine)
                    return;
                stream.SendNext(Input.Forward);
                stream.SendNext(Input.Steer);
                stream.SendNext(State);
                stream.SendNext(fuelAmount);
                stream.SendNext(vehicleHealth);
                
                stream.SendNext(Rigidbody.position);
                stream.SendNext(Rigidbody.rotation);
                stream.SendNext(Rigidbody.velocity);
                stream.SendNext(Rigidbody.angularVelocity);
            }
            else
            {
                Input.Forward = (float)stream.ReceiveNext();
                Input.Steer = (float)stream.ReceiveNext();
                State = (CarState)stream.ReceiveNext();
                fuelAmount = (float)stream.ReceiveNext();
                vehicleHealth = (float)stream.ReceiveNext();

                Network.Position = (Vector3)stream.ReceiveNext();
                Network.Rotation = (Quaternion)stream.ReceiveNext();
                Rigidbody.velocity = (Vector3)stream.ReceiveNext();
                Rigidbody.angularVelocity = (Vector3)stream.ReceiveNext();

                float lag = Mathf.Abs((float)(PhotonNetwork.Time - info.timestamp));
                Network.Position += Rigidbody.velocity * lag;
            }
        }

        [System.Serializable]
        public struct WheelInfo
        {
            public WheelCollider WheelCollider; // كولايدر العجلة
            public Transform MeshRenderer;      // مجسم العجلة
            public bool Steer;                 // هل العجلة للتوجيه؟
            public bool Motor;                 // هل العجلة دفع/سحب؟
            [HideInInspector]
            public float Rotation;
        }

        [System.Serializable]
        public enum CarState
        {
            FREE = 0,     // السيارة فارغة
            OCCUPIED = 1  // السيارة مأهولة/بها سائق
        }
    }
}
