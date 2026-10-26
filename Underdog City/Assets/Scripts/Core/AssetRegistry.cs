using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public static class AssetRegistry
    {
        private static Dictionary<string, Material> materialCache = new Dictionary<string, Material>();
        private static Dictionary<string, Mesh> meshCache = new Dictionary<string, Mesh>();

        public static void Initialize()
        {
            Debug.Log("[AssetRegistry] Initializing AAA PBR materials and procedural geometry registry...");

            // Standard PBR Surface Materials
            GetMaterial("AsphaltRoad", new Color(0.12f, 0.12f, 0.14f), 0.1f, 0.8f);
            GetMaterial("SidewalkConcrete", new Color(0.58f, 0.58f, 0.60f), 0.05f, 0.85f);
            GetMaterial("BuildingGlass", new Color(0.1f, 0.25f, 0.4f, 0.65f), 0.95f, 0.1f);
            GetMaterial("GoldLuxury", new Color(0.88f, 0.70f, 0.15f), 0.85f, 0.2f);
            GetMaterial("DarkMetal", new Color(0.18f, 0.18f, 0.20f), 0.7f, 0.3f);
            GetMaterial("ChromeFinish", new Color(0.95f, 0.95f, 0.98f), 0.98f, 0.05f);
            GetMaterial("MattePaintRed", new Color(0.75f, 0.12f, 0.12f), 0.1f, 0.9f);
            GetMaterial("SatinPaintBlack", new Color(0.08f, 0.08f, 0.09f), 0.3f, 0.7f);
            GetMaterial("MosqueMarbleWhite", new Color(0.95f, 0.95f, 0.92f), 0.2f, 0.3f);
            GetMaterial("DesertSandstone", new Color(0.82f, 0.68f, 0.48f), 0.05f, 0.95f);
            GetMaterial("NeonGoldEmissive", new Color(1.0f, 0.82f, 0.2f), 0.5f, 0.1f);
        }

        public static Material GetMaterial(string matName, Color defaultColor, float metallic = 0f, float smoothness = 0.5f)
        {
            if (materialCache.TryGetValue(matName, out Material mat))
            {
                return mat;
            }

            Shader shader = Shader.Find("Standard");
            if (shader == null) shader = Shader.Find("Diffuse");

            Material newMat = new Material(shader)
            {
                name = matName,
                color = defaultColor
            };

            if (newMat.HasProperty("_Metallic")) newMat.SetFloat("_Metallic", metallic);
            if (newMat.HasProperty("_Glossiness")) newMat.SetFloat("_Glossiness", smoothness);

            materialCache[matName] = newMat;
            return newMat;
        }

        public static Mesh GetPrimitiveMesh(PrimitiveType type)
        {
            string key = type.ToString();
            if (meshCache.TryGetValue(key, out Mesh mesh))
            {
                return mesh;
            }

            GameObject tempPrimitive = GameObject.CreatePrimitive(type);
            Mesh primitiveMesh = tempPrimitive.GetComponent<MeshFilter>().sharedMesh;
            Object.Destroy(tempPrimitive);

            meshCache[key] = primitiveMesh;
            return primitiveMesh;
        }
    }
}
