using UnityEngine;

namespace UnderdogCity
{
    public class WeatherAndEnvironmentManager : MonoBehaviour
    {
        public static WeatherAndEnvironmentManager Instance { get; private set; }

        public enum WeatherState
        {
            Clear,
            Rainy,
            Foggy
        }

        [Header("🌦️ الطقس الديناميكي والبيئة - Wise City Weather")]
        public WeatherState currentWeather = WeatherState.Clear;
        public float dayNightTimeHour = 12f; // 0 to 24
        public float timeSpeed = 0.5f;

        [Header("تأثير الطقس على الإطارات - Tire Grip Effect")]
        public float wetGripMultiplier = 0.65f; // انزلاق واقعي أثناء المطر

        private Light sunDirectionalLight;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else UnityEngine.Object.Destroy(gameObject);
        }

        private void Start()
        {
            sunDirectionalLight = UnityEngine.Object.FindObjectOfType<Light>();
        }

        private void Update()
        {
            // Day/Night Cycle Progression
            dayNightTimeHour += Time.deltaTime * timeSpeed;
            if (dayNightTimeHour >= 24f) dayNightTimeHour = 0f;

            if (sunDirectionalLight != null)
            {
                float rotX = (dayNightTimeHour / 24f) * 360f - 90f;
                sunDirectionalLight.transform.rotation = Quaternion.Euler(rotX, 170f, 0f);
            }
        }

        public void SetWeather(WeatherState newWeather)
        {
            currentWeather = newWeather;
            float gripMult = (currentWeather == WeatherState.Rainy) ? wetGripMultiplier : 1.0f;

            CarPhysics[] cars = UnityEngine.Object.FindObjectsOfType<CarPhysics>();
            foreach (var car in cars)
            {
                if (car != null)
                {
                    car.Grip = 5f * gripMult;
                }
            }

            Debug.Log("[WiseCity Environment] Weather updated to: " + newWeather + " | Grip Multiplier: " + gripMult);
        }
    }
}
