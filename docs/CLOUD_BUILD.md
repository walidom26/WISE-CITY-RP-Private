# ☁️ دليل البناء السحابي المباشر وبناء نسخة الأندرويد (APK) عبر GitHub Actions

يوفر هذا المشروع نظام بناء سحابي مباشر ومتحقق منه بالكامل لاستخراج ملفات التطبيق (APK) للعبة **Wise City RP** تلقائياً عبر **GameCI** و **GitHub Actions** باستخدام تسجيل الدخول التلقائي والمصادقة المباشرة بحساب يونيتي عبر `game-ci/unity-builder` دون الحاجة لأي مفاتيح أو أرقام تسلسلية (`UNITY_SERIAL`) أو ملفات ترخيص يدوية (`UNITY_LICENSE` / `.alf`).

---

## 1️⃣ آلية البناء السحابي المباشر بالتسجيل التلقائي

تم إعداد workflow في `.github/workflows/build.yml` للقيام بعملية المصادقة التلقائية بحساب يونيتي وبناء ملفات الـ APK مباشرة.

### الأسرار المطلوبة في GitHub Repository Secrets:

قم بإضافة الأسرار التالية فقط في إعدادات المستودع (`Settings -> Secrets and variables -> Actions`):

* **`UNITY_EMAIL`**: البريد الإلكتروني لحساب Unity الخاص بك.
* **`UNITY_PASSWORD`**: كلمة المرور لحساب Unity الخاص بك.

تقوم أداة `game-ci/unity-builder` باستخدام الأسرار الممررة عبر متغيرات البيئة (`env`) لتسجيل الدخول تلقائياً وتنفيذ ترخيص البناء قبل استخراج ملفات الـ APK.

---

## 2️⃣ كيفية تشغيل البناء المباشر وتحميل ملف الـ APK

1. اذهب إلى تبويب **Actions** في المستودع على GitHub.
2. اختر **"Build Unity Android APK"**.
3. اضغط على **Run workflow** لبدء عملية البناء المباشرة.
4. عند انتهاء البناء، ستجد ملف الـ APK جاهزاً للتحميل مباشرة تحت اسم **`WiseCityRP-Android-APK`** في قسم **Artifacts**.

---

## 3️⃣ مواصفات بيئة البناء

* **نسخة المحرك (Unity Version):** `2018.3.0f2`
* **مسار المشروع (Project Path):** `Underdog City`
* **منصة الاستهداف (Target Platform):** `Android` (`androidPackage`)
* **التوافقية البرمجية:** `net10.0` C# / Photon PUN Multiplayer Framework
