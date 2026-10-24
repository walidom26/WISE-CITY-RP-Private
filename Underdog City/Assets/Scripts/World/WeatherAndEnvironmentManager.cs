using UnityEngine;
using System.Collections;

namespace UnderdogCity
{
    public class WeatherAndEnvironmentManager : MonoBehaviour
    {
        public static WeatherAndEnvironmentManager Instance { get; private set; }

        public enum WeatherState
        {
            Clear,  // صحو
            Rain,   // أمطار
            Fog     // ضباب
        }

        [Header("2️⃣ الطقس والدورة الزمنية - وايز سيتي")]
        public WeatherState currentWeather = WeatherState.Clear;
        public Light mainDirectionalLight;
        public ParticleSystem rainParticleSystem;
        public ParticleSystem fogParticleSystem;

        [Header("إعدادات الليل والنهار")]
        public float dayCycleDurationSeconds = 1200f; // 20 دقيقة لدورة كاملة
        [Range(0f, 24f)]
        public float timeOfDayHours = 12f;
        public Light[] cityStreetLights;

        [Header("تأثير الطقس على احتكاك المركبات (Tire Grip)")]
        public float wetGripMultiplier = 0.55f; // تقليل الثبات بنسبة 45% في المطر
        public float dryGripMultiplier = 1.0f;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            SetWeather(currentWeather);
            Debug.Log("[WiseCity Weather] Weather & Environment Manager started.");
        }

        private void Update()
        {
            // تحديث دورة الوقت (الليل والنهار)
            timeOfDayHours += (Time.deltaTime / dayCycleDurationSeconds) * 24f;
            if (timeOfDayHours >= 24f) timeOfDayHours -= 24f;

            UpdateLightingAndStreetLights();
        }

        public void SetWeather(WeatherState newWeather)
        {
            currentWeather = newWeather;

            if (rainParticleSystem != null)
            {
                if (currentWeather == WeatherState.Rain) rainParticleSystem.Play();
                else rainParticleSystem.Stop();
            }

            if (fogParticleSystem != null)
            {
                if (currentWeather == WeatherState.Fog) fogParticleSystem.Play();
                else fogParticleSystem.Stop();
            }

            // تعديل ثبات كافّة السيارات في الخريطة بناءً على حالة الطقس
            ApplyTireGripToAllVehicles();

            Debug.Log("[WiseCity Weather] Weather changed to: " + currentWeather.ToString());
        }

        public float GetCurrentGripMultiplier()
        {
            return (currentWeather == WeatherState.Rain) ? wetGripMultiplier : dryGripMultiplier;
        }

        private void ApplyTireGripToAllVehicles()
        {
            CarPhysics[] cars = FindObjectsOfType<CarPhysics>();
            float mult = GetCurrentGripMultiplier();

            foreach (var car in cars)
            {
                if (car != null)
                {
                    car.Grip = 5f * mult;
                }
            }
        }

        private void UpdateLightingAndStreetLights()
        {
            if (mainDirectionalLight != null)
            {
                float sunAngle = (timeOfDayHours / 24f) * 360f - 90f;
                mainDirectionalLight.transform.rotation = Quaternion.Euler(sunAngle, 170f, 0f);

                bool isNight = (timeOfDayHours < 6f || timeOfDayHours > 18f);
                mainDirectionalLight.intensity = isNight ? 0.1f : 1.2f;

                if (cityStreetLights != null)
                {
                    foreach (var light in cityStreetLights)
                    {
                        if (light != null) light.enabled = isNight;
                    }
                }
            }
        }
    }
}
