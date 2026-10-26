using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class DispatchManager : MonoBehaviour
    {
        public static DispatchManager Instance { get; private set; }

        public enum DispatchType
        {
            Police911,
            EMS997,
            GuardSovereignCall
        }

        [System.Serializable]
        public class DispatchCall
        {
            public int callID;
            public DispatchType type;
            public string callerName;
            public Vector3 location;
            public string description;
            public bool isAccepted;
            public int assignedUnitID;
            public string timestamp;
        }

        [Header("🚨 غرفة الطوارئ والدوريات - Wise Emergency Dispatch")]
        public List<DispatchCall> activeCalls = new List<DispatchCall>();

        private int nextCallID = 501;

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void CreateEmergencyCall(DispatchType type, string callerName, Vector3 pos, string desc)
        {
            DispatchCall call = new DispatchCall
            {
                callID = nextCallID++,
                type = type,
                callerName = callerName,
                location = pos,
                description = desc,
                isAccepted = false,
                assignedUnitID = 0,
                timestamp = System.DateTime.UtcNow.ToString("T")
            };

            activeCalls.Add(call);
            Debug.Log(string.Format("[DISPATCH] New {0} Emergency Call #{1} from {2} at {3}: {4}", type, call.callID, callerName, pos, desc));
        }

        public void AcceptCall(int callID, int respondingUnitID)
        {
            DispatchCall call = activeCalls.Find(c => c.callID == callID);
            if (call != null && !call.isAccepted)
            {
                call.isAccepted = true;
                call.assignedUnitID = respondingUnitID;
                Debug.Log(string.Format("[DISPATCH] Call #{0} ACCEPTED by Responding Unit #{1}", callID, respondingUnitID));
            }
        }

        public void CloseCall(int callID)
        {
            activeCalls.RemoveAll(c => c.callID == callID);
            Debug.Log("[DISPATCH] Call #" + callID + " CLOSED.");
        }
    }
}
