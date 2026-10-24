using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class SeamlessInteriorManager : MonoBehaviour
    {
        public static SeamlessInteriorManager Instance { get; private set; }

        [System.Serializable]
        public class InteriorPortal
        {
            public string interiorName;
            public Transform exteriorEntrancePoint;
            public Transform interiorSpawnPoint;
            public Transform interiorExitPoint;
            public Transform exteriorExitSpawnPoint;
            public GameObject interiorParentGameObject; // المجسم الداخلي
        }

        [Header("2️⃣ إدارة المباني والتنقل الانسيابي (Seamless Interiors)")]
        public List<InteriorPortal> interiorPortals = new List<InteriorPortal>();
        public float transitionFadeTime = 0.25f; // انتقال سريع جداً بدون شاشات انتظار (Loading Screen)

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            // تفعيل جميع الأماكن الداخلية في الذاكرة لمنع أي شاشة تحميل
            foreach (var portal in interiorPortals)
            {
                if (portal.interiorParentGameObject != null)
                {
                    portal.interiorParentGameObject.SetActive(true);
                }
            }
            Debug.Log("[WiseCity Seamless] Seamless Interior Manager initialized with zero loading screens.");
        }

        public void EnterInteriorSeamless(string interiorName, Transform playerTransform)
        {
            InteriorPortal portal = interiorPortals.Find(p => p.interiorName.Equals(interiorName, System.StringComparison.OrdinalIgnoreCase));
            if (portal == null || playerTransform == null) return;

            StartCoroutine(SeamlessTransitionCoroutine(playerTransform, portal.interiorSpawnPoint.position, portal.interiorSpawnPoint.rotation));
            Debug.Log("[WiseCity Seamless] Player entered interior seamlessly: " + interiorName);
        }

        public void ExitInteriorSeamless(string interiorName, Transform playerTransform)
        {
            InteriorPortal portal = interiorPortals.Find(p => p.interiorName.Equals(interiorName, System.StringComparison.OrdinalIgnoreCase));
            if (portal == null || playerTransform == null) return;

            StartCoroutine(SeamlessTransitionCoroutine(playerTransform, portal.exteriorExitSpawnPoint.position, portal.exteriorExitSpawnPoint.rotation));
            Debug.Log("[WiseCity Seamless] Player exited interior seamlessly: " + interiorName);
        }

        private IEnumerator SeamlessTransitionCoroutine(Transform targetTransform, Vector3 destinationPos, Quaternion destinationRot)
        {
            // انتقال فوري وسلس للموقع دون استدعاء SceneManager.LoadScene()
            targetTransform.position = destinationPos;
            targetTransform.rotation = destinationRot;

            yield return null;
        }
    }
}
