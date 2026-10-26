using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class LandmarksManager : MonoBehaviour
    {
        public static LandmarksManager Instance { get; private set; }

        [System.Serializable]
        public class AuctionLot
        {
            public int lotID;
            public string title;
            public double startingBid;
            public double highestBid;
            public string highestBidderAccountID;
            public float timeRemainingSeconds;
            public bool isClosed;
        }

        [Header("🏛️ المعالم والمزادات الأسبوعية - Wise City Landmarks & Auction")]
        public List<AuctionLot> activeAuctionLots = new List<AuctionLot>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
            InitializeAuctions();
        }

        void InitializeAuctions()
        {
            activeAuctionLots.Add(new AuctionLot { lotID = 101, title = "سيارة Bugatti Secret Rare", startingBid = 1000000.0, highestBid = 1000000.0, highestBidderAccountID = "System", timeRemainingSeconds = 3600f, isClosed = false });
            activeAuctionLots.Add(new AuctionLot { lotID = 102, title = "قصر الأكابر الفاخر مع مرآب خاص", startingBid = 5000000.0, highestBid = 5000000.0, highestBidderAccountID = "System", timeRemainingSeconds = 7200f, isClosed = false });
        }

        public void PlaceAuctionBid(int lotID, double bidAmount, string bidderAccountID)
        {
            AuctionLot lot = activeAuctionLots.Find(l => l.lotID == lotID);
            if (lot == null || lot.isClosed) return;

            if (bidAmount >= lot.highestBid + 50000.0) // Minimum increment
            {
                lot.highestBid = bidAmount;
                lot.highestBidderAccountID = bidderAccountID;
                if (lot.timeRemainingSeconds < 120f) lot.timeRemainingSeconds += 120f; // Anti-sniping extension

                Debug.Log(string.Format("[WiseCity Auction] New Highest Bid on Lot {0}: ${1} by {2}", lotID, bidAmount, bidderAccountID));
            }
        }
    }
}
