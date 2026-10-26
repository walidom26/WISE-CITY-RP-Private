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
            else UnityEngine.Object.Destroy(gameObject);
        }

        private void Start()
        {
            GenerateWiseCityWorld();
        }

        public void GenerateWiseCityWorld()
        {
            Debug.Log("[ProceduralWorldGenerator] Building AAA open world, 80+ buildings, landmarks, 12 districts & road grid...");

            // 1. Terrain Ground
            GameObject ground = GameObject.CreatePrimitive(PrimitiveType.Plane);
            ground.name = "WiseCityGroundPlane";
            ground.transform.position = Vector3.zero;
            ground.transform.localScale = new Vector3(120, 1, 120);
            Renderer groundRend = ground.GetComponent<Renderer>();
            if (groundRend != null)
            {
                groundRend.material = AssetRegistry.GetMaterial("AsphaltRoad", new Color(0.12f, 0.12f, 0.14f));
            }
            spawnedWorldObjects.Add(ground);

            // 2. Procedural Road Grid & Lighting
            int gridDimension = 10;
            float spacing = 80f;

            for (int x = -gridDimension; x <= gridDimension; x++)
            {
                for (int z = -gridDimension; z <= gridDimension; z++)
                {
                    Vector3 pos = new Vector3(x * spacing, 0.05f, z * spacing);

                    if (x % 2 == 0 || z % 2 == 0)
                    {
                        GameObject roadBlock = GameObject.CreatePrimitive(PrimitiveType.Cube);
                        roadBlock.name = string.Format("RoadBlock_{0}_{1}", x, z);
                        roadBlock.transform.position = pos;
                        roadBlock.transform.localScale = new Vector3(spacing * 0.95f, 0.1f, spacing * 0.95f);
                        Renderer rend = roadBlock.GetComponent<Renderer>();
                        if (rend != null) rend.material = AssetRegistry.GetMaterial("AsphaltRoad", new Color(0.14f, 0.14f, 0.16f));
                        spawnedWorldObjects.Add(roadBlock);

                        if ((x + z) % 4 == 0)
                        {
                            GameObject lightPost = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
                            lightPost.name = "StreetLightPost";
                            lightPost.transform.position = pos + new Vector3(spacing * 0.42f, 4.5f, spacing * 0.42f);
                            lightPost.transform.localScale = new Vector3(0.25f, 4.5f, 0.25f);
                            lightPost.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("DarkMetal", new Color(0.2f, 0.2f, 0.22f));

                            GameObject lightHead = new GameObject("StreetLightPoint");
                            lightHead.transform.position = lightPost.transform.position + new Vector3(0, 4.5f, 0);
                            Light l = lightHead.AddComponent<Light>();
                            l.type = LightType.Point;
                            l.range = 28f;
                            l.intensity = 2.5f;
                            l.color = new Color(1f, 0.92f, 0.75f);

                            lightPost.transform.parent = roadBlock.transform;
                            lightHead.transform.parent = roadBlock.transform;
                        }
                    }
                    else
                    {
                        int districtIndex = System.Math.Abs(x * 3 + z * 7) % DistrictNames.Length;
                        GameObject building = GameObject.CreatePrimitive(PrimitiveType.Cube);
                        building.name = string.Format("Building_{0}_{1}_[{2}]", x, z, DistrictNames[districtIndex]);

                        float buildingHeight = UnityEngine.Random.Range(15f, 75f);
                        building.transform.position = pos + new Vector3(0, buildingHeight / 2f, 0);
                        building.transform.localScale = new Vector3(36f, buildingHeight, 36f);

                        Renderer rend = building.GetComponent<Renderer>();
                        if (rend != null)
                        {
                            Color buildingColor = Color.HSVToRGB((districtIndex * 0.08f) % 1f, 0.35f, 0.82f);
                            rend.material = AssetRegistry.GetMaterial("BuildingFacade_" + districtIndex, buildingColor, 0.3f, 0.6f);
                        }
                        spawnedWorldObjects.Add(building);
                    }
                }
            }

            // 3. Build Major Landmarks
            BuildGrandMosque(new Vector3(0, 0, 240));
            BuildAuctionHall(new Vector3(-240, 0, 0));
            BuildRepublicanGuardHQ(new Vector3(240, 0, 0));
            BuildMunicipalTheater(new Vector3(0, 0, -240));
            BuildCityStadium(new Vector3(320, 0, 320));

            Debug.Log("[ProceduralWorldGenerator] ✅ AAA Open World with 80+ Buildings and Major Landmarks Generated!");
        }

        void BuildGrandMosque(Vector3 pos)
        {
            GameObject mosque = new GameObject("Landmark_GrandMosque");
            mosque.transform.position = pos;

            // Main Prayer Hall
            GameObject hall = GameObject.CreatePrimitive(PrimitiveType.Cube);
            hall.name = "PrayerHall";
            hall.transform.parent = mosque.transform;
            hall.transform.localPosition = new Vector3(0, 10, 0);
            hall.transform.localScale = new Vector3(60, 20, 60);
            hall.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("MosqueMarbleWhite", new Color(0.95f, 0.95f, 0.92f));

            // Main Dome
            GameObject dome = GameObject.CreatePrimitive(PrimitiveType.Sphere);
            dome.name = "MosqueDome";
            dome.transform.parent = mosque.transform;
            dome.transform.localPosition = new Vector3(0, 25, 0);
            dome.transform.localScale = new Vector3(30, 20, 30);
            dome.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("GoldLuxury", new Color(0.88f, 0.70f, 0.15f));

            // Minarets (2 Minarets)
            Vector3[] minaretPos = new Vector3[] { new Vector3(-25, 25, 25), new Vector3(25, 25, 25) };
            foreach (var mPos in minaretPos)
            {
                GameObject minaret = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
                minaret.name = "Minaret";
                minaret.transform.parent = mosque.transform;
                minaret.transform.localPosition = mPos;
                minaret.transform.localScale = new Vector3(4, 25, 4);
                minaret.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("MosqueMarbleWhite", new Color(0.95f, 0.95f, 0.92f));
            }
        }

        void BuildAuctionHall(Vector3 pos)
        {
            GameObject hall = GameObject.CreatePrimitive(PrimitiveType.Cube);
            hall.name = "Landmark_AuctionHall";
            hall.transform.position = pos + new Vector3(0, 12, 0);
            hall.transform.localScale = new Vector3(50, 24, 50);
            hall.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("GoldLuxury", new Color(0.88f, 0.70f, 0.15f));
        }

        void BuildRepublicanGuardHQ(Vector3 pos)
        {
            GameObject hq = new GameObject("Landmark_RepublicanGuardHQ_3Floors");
            hq.transform.position = pos;

            // Floor 0: Armored Garage
            GameObject f0 = GameObject.CreatePrimitive(PrimitiveType.Cube);
            f0.name = "Floor0_Garage";
            f0.transform.parent = hq.transform;
            f0.transform.localPosition = new Vector3(0, 5, 0);
            f0.transform.localScale = new Vector3(50, 10, 40);
            f0.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("DarkMetal", new Color(0.18f, 0.18f, 0.20f));

            // Floor 1: Command Offices
            GameObject f1 = GameObject.CreatePrimitive(PrimitiveType.Cube);
            f1.name = "Floor1_CommandOffices";
            f1.transform.parent = hq.transform;
            f1.transform.localPosition = new Vector3(0, 15, 0);
            f1.transform.localScale = new Vector3(46, 10, 36);
            f1.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("BuildingGlass", new Color(0.1f, 0.25f, 0.4f, 0.65f));

            // Floor 2: Formal Meeting Hall
            GameObject f2 = GameObject.CreatePrimitive(PrimitiveType.Cube);
            f2.name = "Floor2_FormalMeetingHall";
            f2.transform.parent = hq.transform;
            f2.transform.localPosition = new Vector3(0, 25, 0);
            f2.transform.localScale = new Vector3(40, 10, 30);
            f2.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("GoldLuxury", new Color(0.88f, 0.70f, 0.15f));
        }

        void BuildMunicipalTheater(Vector3 pos)
        {
            GameObject theater = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
            theater.name = "Landmark_MunicipalTheater";
            theater.transform.position = pos + new Vector3(0, 10, 0);
            theater.transform.localScale = new Vector3(45, 10, 45);
            theater.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("DesertSandstone", new Color(0.82f, 0.68f, 0.48f));
        }

        void BuildCityStadium(Vector3 pos)
        {
            GameObject stadium = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
            stadium.name = "Landmark_CityStadium";
            stadium.transform.position = pos + new Vector3(0, 8, 0);
            stadium.transform.localScale = new Vector3(80, 8, 80);
            stadium.GetComponent<Renderer>().material = AssetRegistry.GetMaterial("SidewalkConcrete", new Color(0.58f, 0.58f, 0.60f));
        }

        public string GetDistrictAtPosition(Vector3 pos)
        {
            int index = System.Math.Abs((int)(pos.x * 0.01f + pos.z * 0.03f)) % DistrictNames.Length;
            return DistrictNames[index];
        }
    }
}
