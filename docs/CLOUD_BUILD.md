# ☁️ دليل البناء السحابي التلقائي وبناء نسخة الأندرويد (APK) عبر GitHub Actions

يوفر هذا المشروع نظام بناء سحابي مباشر ومتحقق منه بالكامل لاستخراج ملفات التطبيق (APK) للعبة **Wise City RP** باستخدام **GameCI** و **GitHub Actions**.

---

## 1️⃣ آلية البناء السحابي المباشر (Direct Automated Cloud Build)

تم إعداد workflow في `.github/workflows/build.yml` للبناء التلقائي والمباشر دون الحاجة لخطوات يدوية عند إعداد أسرار GitHub Secrets.

### الأسرار المطلوبة في GitHub Repository Secrets:

لإتاحة البناء التلقائي، قم بإضافة الأسرار التالية في إعدادات مستودع GitHub الخاص بك (`Settings -> Secrets and variables -> Actions`):

1. **`UNITY_LICENSE`** (موصى به): نص ترخيص يونيتي (Personal License أو Professional License XML/ULF).
2. **`UNITY_EMAIL`** و **`UNITY_PASSWORD`**: بريد وكلمة مرور حساب Unity الخاص بك (في حال استخدام التفعيل المباشر بالحساب).
3. **`UNITY_SERIAL`** *(اختياري)*: الرقم التسلسلي لترخيص Unity Plus / Pro.
4. **`UNITY_LICENSING_SERVER`** *(اختياري)*: عنوان سيرفر التراخيص الخاص بالمؤسسات إن وجد.

---

## 2️⃣ طريقة البناء عبر ملف التفعيل اليدوي (`.alf`) - خيار اختياري

إذا كنت تفضل استخراج ملف الترخيص اليدوي بدل الأسرار المباشرة:

1. قم بتشغيل سير العمل **"Acquire Unity Activation File"** من تبويب `Actions`.
2. قم بتحميل الملف الناتج `unity-activation-file.alf`.
3. ارفعه إلى [license.unity3d.com](https://license.unity3d.com) للحصول على ملف `.ulf`.
4. انسخ محتوى الملف `.ulf` وضعه في سر `UNITY_LICENSE` في المستودع.

---

## 3️⃣ كيفية تشغيل البناء المباشر وتحميل ملف الـ APK

1. اذهب إلى تبويب **Actions** في المستودع على GitHub.
2. اختر **"Build Unity Android APK"**.
3. اضغط على **Run workflow** للبدء المباشر.
4. بعد اكتمال البناء، ستجد ملف الـ APK جاهزاً للتحميل مباشرة تحت اسم **`WiseCityRP-Android-APK`** في قسم **Artifacts**.

---

## 4️⃣ تفاصيل إعدادات المشروع في البناء السحابي

* **نسخة المحرك (Unity Version):** `2018.3.0f2`
* **مسار المشروع (Project Path):** `Underdog City`
* **منصة الاستهداف (Target Platform):** `Android` (`androidPackage`)
* **التوافقية:** `net10.0` C# / Photon PUN Multiplayer Framework
