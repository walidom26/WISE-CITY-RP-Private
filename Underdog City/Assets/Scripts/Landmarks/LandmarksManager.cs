using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class LandmarksManager : MonoBehaviour
    {
        public static LandmarksManager Instance { get; private set; }

        public enum LandmarkType
        {
            TheTavern,             // الحانة والنادي الاجتماعي
            MunicipalTheater,      // المسرح البلدي
            CityStadium,           // الملعب الرياضي
            ClassicCarShowroom,    // معرض السيارات الكلاسيكية
            StandardCarShowroom,   // معرض السيارات العادية
            LuxuryCarShowroom,     // معرض السيارات الفاخرة
            AuctionHouse           // دار المزادات الكبرى
        }

        [System.Serializable]
        public class Landmark
        {
            public string landmarkName;
            public LandmarkType type;
            public Vector3 locationCoordinates;
            public string description;
        }

        [System.Serializable]
        public class AuctionItem
        {
            public string itemID;
            public string itemName; // اسم سيارة أو عقار نادر
            public double startingPrice;
            public double highestBid;
            public string highestBidderName;
            public int auctionDurationMinutes;
            public bool isAuctionActive;
        }

        [Header("3️⃣ المعالم الكبرى والمزادات - وايز سيتي")]
        public List<Landmark> cityLandmarks = new List<Landmark>();
        public List<AuctionItem> activeAuctions = new List<AuctionItem>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        private void Start()
        {
            InitializeDefaultLandmarks();
            Debug.Log("[WiseCity Landmarks] Landmarks and Entertainment Hubs initialized.");
        }

        private void InitializeDefaultLandmarks()
        {
            cityLandmarks.Add(new Landmark { landmarkName = "The Tavern", type = LandmarkType.TheTavern, locationCoordinates = new Vector3(100, 0, 50), description = "مكان فاخر للاسترخاء وعقد الصفقات السرية." });
            cityLandmarks.Add(new Landmark { landmarkName = "المسرح البلدي", type = LandmarkType.MunicipalTheater, locationCoordinates = new Vector3(-200, 0, 150), description = "منصة كبرى لإقامة الفعاليات والمؤتمرات الجماهيرية." });
            cityLandmarks.Add(new Landmark { landmarkName = "الملعب الرياضي", type = LandmarkType.CityStadium, locationCoordinates = new Vector3(300, 0, -400), description = "ملعب متكامل للتحديات الكبرى والبطولات بين اللاعبين." });
            cityLandmarks.Add(new Landmark { landmarkName = "معرض السيارات الكلاسيكية", type = LandmarkType.ClassicCarShowroom, locationCoordinates = new Vector3(-50, 0, -100), description = "سيارات قديمة ونادرة جداً." });
            cityLandmarks.Add(new Landmark { landmarkName = "معرض السيارات الحديثة", type = LandmarkType.StandardCarShowroom, locationCoordinates = new Vector3(0, 0, 0), description = "سيارات اقتصادية وعادية." });
            cityLandmarks.Add(new Landmark { landmarkName = "معرض السيارات الفخمة", type = LandmarkType.LuxuryCarShowroom, locationCoordinates = new Vector3(150, 0, 300), description = "سيارات خارقة وأسطورية." });
            cityLandmarks.Add(new Landmark { landmarkName = "دار المزادات الكبرى", type = LandmarkType.AuctionHouse, locationCoordinates = new Vector3(50, 0, 400), description = "قاعة علنية أسبوعية للمزايدة على أندر السيارات والعقارات." });
        }

        public void PlaceAuctionBid(string itemID, string bidderName, double bidAmount)
        {
            AuctionItem item = activeAuctions.Find(a => a.itemID == itemID && a.isAuctionActive);
            if (item != null)
            {
                if (bidAmount > item.highestBid)
                {
                    item.highestBid = bidAmount;
                    item.highestBidderName = bidderName;
                    Debug.Log(string.Format("[WiseCity Auction] New highest bid on {0}: ${1:N0} by {2}", item.itemName, bidAmount, bidderName));
                }
            }
        }
    }
}
