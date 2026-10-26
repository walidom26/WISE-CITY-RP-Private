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
            Debug.Log("[AssetRegistry] Initializing procedural asset materials & meshes...");
            GetMaterial("AsphaltRoad", new Color(0.15f, 0.15f, 0.16f), 0.1f, 0.8f);
            GetMaterial("SidewalkConcrete", new Color(0.55f, 0.55f, 0.56f), 0.0f, 0.9f);
            GetMaterial("BuildingGlass", new Color(0.1f, 0.2f, 0.3f, 0.6f), 0.9f, 0.1f);
            GetMaterial("GoldLuxury", new Color(0.85f, 0.65f, 0.12f), 0.8f, 0.2f);
            GetMaterial("DarkMetal", new Color(0.2f, 0.2f, 0.22f), 0.5f, 0.4f);
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
