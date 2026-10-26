using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class InventoryManager : MonoBehaviour
    {
        public static InventoryManager Instance { get; private set; }

        public enum ItemType
        {
            Consumable,
            Weapon,
            Ammo,
            Key,
            Tool,
            IllegalContraband
        }

        [System.Serializable]
        public class InventoryItem
        {
            public int itemID;
            public string itemName;
            public ItemType type;
            public int quantity;
            public double value;
        }

        [Header("🎒 الحقيبة والتجارة المباشرة P2P Trade")]
        public List<InventoryItem> playerInventory = new List<InventoryItem>();
        public int maxInventorySlots = 20;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
            InitializeDefaultItems();
        }

        void InitializeDefaultItems()
        {
            AddItem("هاتف WisePro", ItemType.Tool, 1, 1000.0);
            AddItem("مشروب طاقة", ItemType.Consumable, 3, 25.0);
            AddItem("مسدس 9mm", ItemType.Weapon, 1, 1500.0);
            AddItem("ذخيرة مسدس 9mm", ItemType.Ammo, 60, 100.0);
        }

        public bool AddItem(string name, ItemType type, int qty, double val)
        {
            if (playerInventory.Count >= maxInventorySlots) return false;

            InventoryItem existing = playerInventory.Find(i => i.itemName == name);
            if (existing != null)
            {
                existing.quantity += qty;
            }
            else
            {
                playerInventory.Add(new InventoryItem
                {
                    itemID = Random.Range(100, 999),
                    itemName = name,
                    type = type,
                    quantity = qty,
                    value = val
                });
            }

            Debug.Log(string.Format("[INVENTORY] Added {0} x{1} to inventory.", name, qty));
            return true;
        }

        public bool ExecuteSecureP2PTrade(InventoryItem item, double price, string buyerAccountID)
        {
            if (item == null || item.quantity <= 0 || price < 0) return false;

            Debug.Log(string.Format("[SECURE TRADE] Sold {0} to Buyer {1} for ${2}", item.itemName, buyerAccountID, price));
            item.quantity--;
            if (item.quantity <= 0) playerInventory.Remove(item);
            return true;
        }
    }
}
