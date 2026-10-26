using UnityEngine;
using System.Collections.Generic;

namespace UnderdogCity
{
    public class JudiciaryManager : MonoBehaviour
    {
        public static JudiciaryManager Instance { get; private set; }

        [System.Serializable]
        public class CourtCase
        {
            public int caseID;
            public string defendantName;
            public string charge;
            public string assignedLawyerName;
            public string judgeName;
            public double bailAmount;
            public int sentenceJailMinutes;
            public bool isResolved;
            public string verdict;
        }

        [Header("⚖️ المحاكم والقضاء - Wise Judiciary & Legal System")]
        public List<CourtCase> pendingCases = new List<CourtCase>();

        private void Awake()
        {
            if (Instance == null) Instance = this;
            else Destroy(gameObject);
        }

        public void FileCourtCase(string defendantName, string charge, double bail)
        {
            CourtCase c = new CourtCase
            {
                caseID = Random.Range(1000, 9999),
                defendantName = defendantName,
                charge = charge,
                assignedLawyerName = "محامي الدولة",
                judgeName = "القاضي العادل",
                bailAmount = bail,
                sentenceJailMinutes = 0,
                isResolved = false,
                verdict = "قيد النظر"
            };

            pendingCases.Add(c);
            Debug.Log(string.Format("[JUDICIARY] Filed Case #{0} against {1} for {2} | Bail: ${3}", c.caseID, defendantName, charge, bail));
        }

        public void IssueVerdict(int caseID, string verdict, int sentenceJailMinutes)
        {
            CourtCase c = pendingCases.Find(x => x.caseID == caseID);
            if (c != null)
            {
                c.verdict = verdict;
                c.sentenceJailMinutes = sentenceJailMinutes;
                c.isResolved = true;
                Debug.Log(string.Format("[JUDICIARY] Case #{0} Verdict: {1} | Jail Sentence: {2} mins", caseID, verdict, sentenceJailMinutes));
            }
        }
    }
}
