using UnityEngine;

namespace UnderdogCity
{
    public static class WiseCityBootstrap
    {
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
        public static void OnGameStart()
        {
            Debug.Log("=========================================");
            Debug.Log("🌐 WISE CITY RP — RUNTIME BOOTSTRAP INIT");
            Debug.Log("=========================================");

            // 1. Ensure Main Camera exists
            if (Camera.main == null)
            {
                GameObject camObj = new GameObject("Main Camera");
                Camera cam = camObj.AddComponent<Camera>();
                cam.tag = "MainCamera";
                camObj.AddComponent<AudioListener>();
                camObj.transform.position = new Vector3(0, 15, -25);
                camObj.transform.rotation = Quaternion.Euler(25, 0, 0);
            }

            // 2. Instantiate Directional Sun Light
            Light sunLight = Object.FindObjectOfType<Light>();
            if (sunLight == null)
            {
                GameObject lightObj = new GameObject("Directional Light (Sun)");
                Light lightComp = lightObj.AddComponent<Light>();
                lightComp.type = LightType.Directional;
                lightComp.intensity = 1.2f;
                lightComp.color = new Color(1.0f, 0.95f, 0.85f);
                lightObj.transform.rotation = Quaternion.Euler(50, -30, 0);
            }

            // 3. Initialize Asset Registry
            AssetRegistry.Initialize();

            // 4. Instantiate Core System Managers
            EnsureManager<ProceduralWorldGenerator>("WorldGenerator");
            EnsureManager<AccountManager>("AccountManager");
            EnsureManager<HUDManager>("HUDManager");
            EnsureManager<PlayerController>("PlayerController");
            EnsureManager<VehicleController>("VehicleController");
            EnsureManager<AdminAndAntiCheatManager>("AdminAndAntiCheatManager");
            EnsureManager<FinancialManager>("FinancialManager");
            EnsureManager<FactionManager>("FactionManager");
            EnsureManager<JobManager>("JobManager");
            EnsureManager<GangManager>("GangManager");
            EnsureManager<RealEstateManager>("RealEstateManager");
            EnsureManager<WiseProSmartphone>("WiseProSmartphone");
            EnsureManager<LandmarksManager>("LandmarksManager");
            EnsureManager<WeatherAndEnvironmentManager>("WeatherAndEnvironmentManager");
            EnsureManager<SeamlessInteriorManager>("SeamlessInteriorManager");
            EnsureManager<ProximityVoiceSystem>("ProximityVoiceSystem");

            Debug.Log("✅ Wise City RP Runtime Bootstrap successfully initialized all core systems!");
        }

        private static T EnsureManager<T>(string name) where T : MonoBehaviour
        {
            T manager = Object.FindObjectOfType<T>();
            if (manager == null)
            {
                GameObject obj = new GameObject("[SYSTEM] " + name);
                manager = obj.AddComponent<T>();
                Object.DontDestroyOnLoad(obj);
            }
            return manager;
        }
    }
}
