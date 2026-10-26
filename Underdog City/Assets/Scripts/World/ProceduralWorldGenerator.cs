using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class ProceduralWorldGenerator : MonoBehaviour
    {
        public static ProceduralWorldGenerator Instance { get; private set; }

        [Header("🌍 12 أحياء ومناطق وايز سيتي - Wise City Districts")]
        public string[] DistrictNames = new string[]
        {
            "وسط المدينة (Downtown)",
            "الأحياء الشعبية (Popular Quarter)",
            "حي الأكابر (Upscale District)",
            "المنطقة التجارية (Commercial Zone)",
            "المنطقة السكنية (Residential Area)",
            "المنطقة الصناعية (Industrial Zone)",
            "الميناء والمستودعات (Port & Warehouses)",
            "المطار الدولي (International Airport)",
            "الطرق السريعة (Highways)",
            "المناطق الريفية (Rural Outskirts)",
            "الجبال والرمال (Mountain & Desert)",
            "القاعدة العسكرية (Military Base)"
        };

        private List<GameObject> spawnedWorldObjects = new List<GameObject>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            GenerateWiseCityWorld();
        }

        public void GenerateWiseCityWorld()
        {
            Debug.Log("[ProceduralWorldGenerator] Generating 80+ buildings, roads, terrain, streetlights & districts...");

            // 1. Create Terrain Ground
            GameObject ground = GameObject.CreatePrimitive(PrimitiveType.Plane);
            ground.name = "WiseCityGroundPlane";
            ground.transform.position = Vector3.zero;
            ground.transform.localScale = new Vector3(100, 1, 100); // 1000m x 1000m
            Renderer groundRend = ground.GetComponent<Renderer>();
            if (groundRend != null)
            {
                groundRend.material = AssetRegistry.GetMaterial("AsphaltRoad", new Color(0.18f, 0.2f, 0.18f));
            }
            spawnedWorldObjects.Add(ground);

            // 2. Generate Road Grids & Streetlights
            int gridDimension = 10;
            float spacing = 80f;

            for (int x = -gridDimension; x <= gridDimension; x++)
            {
                for (int z = -gridDimension; z <= gridDimension; z++)
                {
                    Vector3 pos = new Vector3(x * spacing, 0.05f, z * spacing);

                    // Generate Road Section
                    if (x % 2 == 0 || z % 2 == 0)
                    {
                        GameObject roadBlock = GameObject.CreatePrimitive(PrimitiveType.Cube);
                        roadBlock.name = string.Format("RoadBlock_{0}_{1}", x, z);
                        roadBlock.transform.position = pos;
                        roadBlock.transform.localScale = new Vector3(spacing * 0.9f, 0.1f, spacing * 0.9f);
                        Renderer rend = roadBlock.GetComponent<Renderer>();
                        if (rend != null) rend.material = AssetRegistry.GetMaterial("AsphaltRoad", new Color(0.12f, 0.12f, 0.14f));
                        spawnedWorldObjects.Add(roadBlock);

                        // Add Streetlight
                        if ((x + z) % 4 == 0)
                        {
                            GameObject lightPost = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
                            lightPost.name = "StreetLightPost";
                            lightPost.transform.position = pos + new Vector3(spacing * 0.4f, 4f, spacing * 0.4f);
                            lightPost.transform.localScale = new Vector3(0.3f, 4f, 0.3f);

                            GameObject lightHead = new GameObject("StreetLightPoint");
                            lightHead.transform.position = lightPost.transform.position + new Vector3(0, 4f, 0);
                            Light l = lightHead.AddComponent<Light>();
                            l.type = LightType.Point;
                            l.range = 25f;
                            l.intensity = 2f;
                            l.color = new Color(1f, 0.9f, 0.7f);

                            lightPost.transform.parent = roadBlock.transform;
                            lightHead.transform.parent = roadBlock.transform;
                        }
                    }
                    else
                    {
                        // 3. Generate 80+ Buildings Across Districts
                        int districtIndex = Mathf.Abs((x * 3 + z * 7)) % DistrictNames.Length;
                        GameObject building = GameObject.CreatePrimitive(PrimitiveType.Cube);
                        building.name = string.Format("Building_{0}_{1}_[{2}]", x, z, DistrictNames[districtIndex]);

                        float buildingHeight = Random.Range(12f, 60f);
                        building.transform.position = pos + new Vector3(0, buildingHeight / 2f, 0);
                        building.transform.localScale = new Vector3(35f, buildingHeight, 35f);

                        Renderer rend = building.GetComponent<Renderer>();
                        if (rend != null)
                        {
                            Color buildingColor = Color.HSVToRGB((districtIndex * 0.08f) % 1f, 0.4f, 0.8f);
                            rend.material = AssetRegistry.GetMaterial("BuildingFacade_" + districtIndex, buildingColor, 0.3f, 0.6f);
                        }
                        spawnedWorldObjects.Add(building);
                    }
                }
            }

            Debug.Log("[ProceduralWorldGenerator] ✅ Generated 100+ Road Sections and 80+ Buildings across 12 Wise City Districts!");
        }

        public string GetDistrictAtPosition(Vector3 pos)
        {
            int index = Mathf.Abs((int)(pos.x * 0.01f + pos.z * 0.03f)) % DistrictNames.Length;
            return DistrictNames[index];
        }
    }
}
