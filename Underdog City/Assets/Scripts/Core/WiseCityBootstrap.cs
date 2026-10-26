using UnityEngine;

namespace UnderdogCity
{
    public static class WiseCityBootstrap
    {
        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
        public static void OnGameStart()
        {
            Debug.Log("=========================================");
            Debug.Log("🌐 WISE CITY RP — COMPLETE AAA BOOTSTRAP INIT");
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

            // 2. Instantiate Directional Sun Light & Ambient Lighting
            Light sunLight = UnityEngine.Object.FindObjectOfType<Light>();
            if (sunLight == null)
            {
                GameObject lightObj = new GameObject("Directional Light (Sun)");
                Light lightComp = lightObj.AddComponent<Light>();
                lightComp.type = LightType.Directional;
                lightComp.intensity = 1.25f;
                lightComp.color = new Color(1.0f, 0.96f, 0.88f);
                lightObj.transform.rotation = Quaternion.Euler(50, -30, 0);
            }

            // 3. Initialize Asset Registry
            AssetRegistry.Initialize();

            // 4. Instantiate Core System Managers
            EnsureManager<ProceduralWorldGenerator>("WorldGenerator");
            EnsureManager<AccountManager>("AccountManager");
            EnsureManager<HUDManager>("HUDManager");
            EnsureManager<HUDController>("HUDController");
            EnsureManager<PlayerController>("PlayerController");
            EnsureManager<VehicleController>("VehicleController");
            EnsureManager<AdminAndAntiCheatManager>("AdminAndAntiCheatManager");
            EnsureManager<ServerAntiCheatValidator>("ServerAntiCheatValidator");
            EnsureManager<FinancialManager>("FinancialManager");
            EnsureManager<FactionManager>("FactionManager");
            EnsureManager<DispatchManager>("DispatchManager");
            EnsureManager<JudiciaryManager>("JudiciaryManager");
            EnsureManager<JobManager>("JobManager");
            EnsureManager<GangManager>("GangManager");
            EnsureManager<RealEstateManager>("RealEstateManager");
            EnsureManager<InventoryManager>("InventoryManager");
            EnsureManager<BusinessManager>("BusinessManager");
            EnsureManager<WiseProSmartphone>("WiseProSmartphone");
            EnsureManager<LandmarksManager>("LandmarksManager");
            EnsureManager<WeatherAndEnvironmentManager>("WeatherAndEnvironmentManager");
            EnsureManager<SeamlessInteriorManager>("SeamlessInteriorManager");
            EnsureManager<ProximityVoiceSystem>("ProximityVoiceSystem");
            EnsureManager<VehicleManager>("VehicleManager");

            Debug.Log("✅ Wise City RP Master Runtime Bootstrap initialized all 23 core game managers!");
        }

        private static T EnsureManager<T>(string name) where T : MonoBehaviour, new()
        {
            T manager = UnityEngine.Object.FindObjectOfType<T>();
            if (manager == null)
            {
                GameObject obj = new GameObject("[SYSTEM] " + name);
                manager = obj.AddComponent<T>();
                UnityEngine.Object.DontDestroyOnLoad(obj);
            }
            return manager;
        }
    }
}
