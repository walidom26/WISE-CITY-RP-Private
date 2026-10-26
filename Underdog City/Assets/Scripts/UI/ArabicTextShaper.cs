using System;
using System.Text;
using System.Collections.Generic;

namespace UnderdogCity
{
    public static class ArabicTextShaper
    {
        // Dictionary for Arabic character contextual forms (Isolated, Initial, Medial, Final)
        private struct ArabicGlyph
        {
            public char isolated;
            public char initial;
            public char medial;
            public char final;

            public ArabicGlyph(char iso, char init, char med, char fin)
            {
                isolated = iso;
                initial = init;
                medial = med;
                final = fin;
            }
        }

        private static Dictionary<char, ArabicGlyph> glyphMap = new Dictionary<char, ArabicGlyph>()
        {
            { 'ا', new ArabicGlyph('ﺍ', 'ﺍ', 'ﺎ', 'ﺎ') },
            { 'ب', new ArabicGlyph('ﺏ', 'ﺑ', 'ﺒ', 'ﺐ') },
            { 'ت', new ArabicGlyph('ﺕ', 'ﺗ', 'ﺘ', 'ﺖ') },
            { 'ث', new ArabicGlyph('ﺙ', 'ﺛ', 'ﺜ', 'ﺚ') },
            { 'ج', new ArabicGlyph('ﺝ', 'ﺟ', 'ﺠ', 'ﺞ') },
            { 'ح', new ArabicGlyph('ﺡ', 'ﺣ', 'ﺤ', 'ﺢ') },
            { 'خ', new ArabicGlyph('ﺥ', 'ﺧ', 'ﺨ', 'ﺺ') },
            { 'د', new ArabicGlyph('ﺩ', 'ﺩ', 'ﺪ', 'ﺪ') },
            { 'ذ', new ArabicGlyph('ﺫ', 'ﺫ', 'ﺬ', 'ﺬ') },
            { 'ر', new ArabicGlyph('ﺭ', 'ﺭ', 'ﺮ', 'ﺮ') },
            { 'ز', new ArabicGlyph('ﺯ', 'ﺯ', 'ﺰ', 'ﺰ') },
            { 'س', new ArabicGlyph('ﺱ', 'ﺳ', 'ﺴ', 'ﺲ') },
            { 'ش', new ArabicGlyph('ﺵ', 'ﺷ', 'ﺸ', 'ﺶ') },
            { 'ص', new ArabicGlyph('ﺹ', 'ﺻ', 'ﺼ', 'ﺺ') },
            { 'ض', new ArabicGlyph('ﺽ', 'ﺿ', 'ﻀ', 'ﺾ') },
            { 'ط', new ArabicGlyph('ﻁ', 'ﻃ', 'ﻄ', 'ﻂ') },
            { 'ظ', new ArabicGlyph('ﻅ', 'ﻇ', 'ﻈ', 'ﻆ') },
            { 'ع', new ArabicGlyph('ﻉ', 'ﻋ', 'ﻌ', 'ﻊ') },
            { 'غ', new ArabicGlyph('ﻍ', 'ﻏ', 'ﻐ', 'ﻎ') },
            { 'ف', new ArabicGlyph('ﻑ', 'ﻓ', 'ﻔ', 'ﻒ') },
            { 'ق', new ArabicGlyph('ﻕ', 'ﻗ', 'ﻘ', 'ﻖ') },
            { 'ك', new ArabicGlyph('ﻙ', 'ﻛ', 'ﻜ', 'ﻚ') },
            { 'ل', new ArabicGlyph('ﻝ', 'ﻟ', 'ﻠ', 'ﻞ') },
            { 'م', new ArabicGlyph('ﻡ', 'ﻣ', 'ﻤ', 'ﻢ') },
            { 'ن', new ArabicGlyph('ﻥ', 'ﻧ', 'ﻨ', 'ﻦ') },
            { 'ه', new ArabicGlyph('ﻩ', 'ﻫ', 'ﻬ', 'ﻪ') },
            { 'و', new ArabicGlyph('ﻭ', 'ﻭ', 'ﻮ', 'ﻮ') },
            { 'ي', new ArabicGlyph('ﻱ', 'ﻳ', 'ﻴ', 'ﻲ') },
            { 'ى', new ArabicGlyph('ﻯ', 'ﻯ', 'ﻰ', 'ﻰ') },
            { 'ة', new ArabicGlyph('ﺓ', 'ﺓ', 'ﺔ', 'ﺔ') },
            { 'ء', new ArabicGlyph('ﺀ', 'ﺀ', 'ﺀ', 'ﺀ') },
            { 'أ', new ArabicGlyph('ﺃ', 'ﺃ', 'ﺄ', 'ﺄ') },
            { 'إ', new ArabicGlyph('ﺇ', 'ﺇ', 'ﺈ', 'ﺈ') },
            { 'آ', new ArabicGlyph('ﺁ', 'ﺁ', 'ﺂ', 'ﺂ') }
        };

        public static string ShapeText(string input)
        {
            if (string.IsNullOrEmpty(input)) return input;

            bool hasArabic = false;
            foreach (char c in input)
            {
                if (c >= 0x0600 && c <= 0x06FF)
                {
                    hasArabic = true;
                    break;
                }
            }

            if (!hasArabic) return input;

            StringBuilder shapedBuilder = new StringBuilder();
            int length = input.Length;

            for (int i = 0; i < length; i++)
            {
                char current = input[i];

                if (glyphMap.TryGetValue(current, out ArabicGlyph glyph))
                {
                    bool prevConnects = (i > 0 && glyphMap.ContainsKey(input[i - 1]));
                    bool nextConnects = (i < length - 1 && glyphMap.ContainsKey(input[i + 1]));

                    if (prevConnects && nextConnects) shapedBuilder.Append(glyph.medial);
                    else if (prevConnects) shapedBuilder.Append(glyph.final);
                    else if (nextConnects) shapedBuilder.Append(glyph.initial);
                    else shapedBuilder.Append(glyph.isolated);
                }
                else
                {
                    shapedBuilder.Append(current);
                }
            }

            // Reverse for RTL presentation
            char[] arr = shapedBuilder.ToString().ToCharArray();
            Array.Reverse(arr);
            return new string(arr);
        }
    }
}
