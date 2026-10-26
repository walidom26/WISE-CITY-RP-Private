# ☁️ دليل البناء السحابي المباشر وبناء نسخة الأندرويد (APK) عبر GitHub Actions

يوفر هذا المشروع نظام بناء سحابي مباشر ومتحقق منه بالكامل لاستخراج ملفات التطبيق (APK) للعبة **Wise City RP** تلقائياً عبر **GameCI** و **GitHub Actions** على بيئة `ubuntu-24.04` ومحرك **Unity 2022.3.62f3**.

---

## 1️⃣ آلية البناء السحابي المباشر بالتسجيل الآلي

تم إعداد workflow في `.github/workflows/build.yml` للبناء التلقائي عبر خوادم GitHub Actions.

### الأسرار المطلوبة في GitHub Repository Secrets:

قم بإضافة الأسرار التالية في إعدادات المستودع (`Settings -> Secrets and variables -> Actions`):

* **`UNITY_EMAIL`**: البريد الإلكتروني لحساب Unity الخاص بك.
* **`UNITY_PASSWORD`**: كلمة المرور لحساب Unity الخاص بك.
* **`UNITY_LICENSE`**: نص ترخيص يونيتي (Personal License أو Professional ULF/XML).

تقوم أداة `game-ci/unity-builder@v4` باستخدام الأسرار الممررة عبر متغيرات البيئة (`env`) لبناء واستخراج ملفات الـ APK تلقائياً.

---

## 2️⃣ كيفية تشغيل البناء المباشر وتحميل ملف الـ APK

1. اذهب إلى تبويب **Actions** في المستودع على GitHub.
2. اختر **"Build Unity Android APK"**.
3. اضغط على **Run workflow** لبدء عملية البناء المباشرة.
4. عند انتهاء البناء، ستجد ملف الـ APK جاهزاً للتحميل مباشرة تحت اسم **`WiseCityRP-Android-APK`** في قسم **Artifacts**.

---

## 3️⃣ مواصفات بيئة البناء

* **نسخة المحرك (Unity Version):** `2022.3.62f3`
* **مسار المشروع (Project Path):** `Underdog City`
* **منصة الاستهداف (Target Platform):** `Android` (`androidPackage`)
* **بيئة التشغيل (Runner):** `ubuntu-24.04`
* **التوافقية البرمجية:** `net10.0` C# / Mirror Multiplayer Framework
