# ☁️ دليل البناء السحابي المباشر لبناء نسخة الأندرويد (APK) عبر GitHub Actions

يوفر هذا المشروع نظام بناء سحابي مباشر ومتحقق منه بالكامل لاستخراج ملفات التطبيق (APK) للعبة **Wise City RP** تلقائياً عبر **GameCI** و **GitHub Actions** دون الحاجة لأي خطوات تفعيل يدوية أو ملفات `.alf`.

---

## 1️⃣ آلية البناء السحابي التلقائي المباشر (Direct Automated Cloud Build)

تم إعداد workflow في `.github/workflows/build.yml` للقيام بعملية التسجيل والبناء والتجميع بشكل آلي ومباشر.

### الأسرار المطلوبة في GitHub Repository Secrets:

قم بإضافة الأسرار التالية في إعدادات المستودع (`Settings -> Secrets and variables -> Actions`):

1. **`UNITY_LICENSE`**: نص ترخيص يونيتي (Personal License أو Professional License ULF/XML).
2. **`UNITY_EMAIL`** و **`UNITY_PASSWORD`**: بيانات حساب Unity للتسجيل والتفعيل الآلي مباشرة في سيرفر البناء.
3. **`UNITY_SERIAL`** *(اختياري)*: الرقم التسلسلي لترخيص Unity Plus / Pro / Enterprise.
4. **`UNITY_LICENSING_SERVER`** *(اختياري)*: عنوان سيرفر التراخيص الخاص بالشركات والمؤسسات.

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
