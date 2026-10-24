# ☁️ دليل البناء السحابي المباشر وبناء نسخة الأندرويد (APK) عبر GitHub Actions

يوفر هذا المشروع نظام بناء سحابي مباشر ومتحقق منه بالكامل لاستخراج ملفات التطبيق (APK) للعبة **Wise City RP** تلقائياً عبر **GameCI** و **GitHub Actions** باستخدام الترخيص الشخصي المجاني (Personal / Community License)، دون الحاجة لأي أرقام تسلسلية مدفوعة (Serials) أو ملفات تفعيل يدوية (`.alf`).

---

## 1️⃣ آلية البناء السحابي المباشر بالترخيص المجاني

تم إعداد workflow في `.github/workflows/build.yml` للبناء التلقائي عبر خوادم GitHub.

### الأسرار المطلوبة في GitHub Repository Secrets:

قم بإضافة الأسرار التالية في إعدادات المستودع (`Settings -> Secrets and variables -> Actions`):

1. **`UNITY_EMAIL`** و **`UNITY_PASSWORD`**: بيانات حساب Unity الخاص بك لاستخراج الترخيص المجاني وتفعيله سحابياً تلقائياً.
2. **`UNITY_LICENSE`** *(اختياري)*: نص ترخيص يونيتي المجاني الشخصي (Personal ULF).

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
