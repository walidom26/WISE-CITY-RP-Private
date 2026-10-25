# ☁️ دليل البناء السحابي وبناء نسخة الأندرويد (APK) عبر GitHub Actions

يوفر هذا المشروع نظام بناء سحابي متكامل لاستخراج ملفات التطبيق (APK) للعبة **Wise City RP** تلقائياً عبر **GameCI** و **GitHub Actions** على بيئة `ubuntu-24.04`.

---

## 1️⃣ آلية البناء السحابي المباشر

تم إعداد workflow في `.github/workflows/build.yml` للبناء التلقائي عبر خوادم GitHub Actions مع الاعتماد على أداة `game-ci/unity-builder@v4`.

### الأسرار المطلوبة في GitHub Repository Secrets:

قم بإضافة الأسرار التالية في إعدادات المستودع (`Settings -> Secrets and variables -> Actions`):

1. **`UNITY_EMAIL`**: البريد الإلكتروني لحساب Unity الخاص بك.
2. **`UNITY_PASSWORD`**: كلمة المرور لحساب Unity الخاص بك.
3. **`UNITY_LICENSE`**: نص ترخيص يونيتي (Personal License أو Professional ULF/XML).

---

## 2️⃣ كيفية تشغيل البناء المباشر وتحميل ملف الـ APK

1. اذهب إلى تبويب **Actions** في المستودع على GitHub.
2. اختر **"Build Unity Android APK"**.
3. اضغط على **Run workflow** لبدء عملية البناء.
4. عند انتهاء البناء، ستجد ملف الـ APK جاهزاً للتحميل تحت اسم **`WiseCityRP-Android-APK`** في قسم **Artifacts**.

---

## 3️⃣ مواصفات بيئة البناء

* **بيئة التشغيل (Runner):** `ubuntu-24.04`
* **نسخة المحرك (Unity Version):** `2018.3.0f2`
* **مسار المشروع (Project Path):** `Underdog City`
* **منصة الاستهداف (Target Platform):** `Android` (`androidPackage`)
* **التوافقية البرمجية:** `net10.0` C# / Photon PUN Multiplayer Framework
