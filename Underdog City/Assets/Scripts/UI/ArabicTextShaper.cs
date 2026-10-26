using System;
using System.Text;
using System.Collections.Generic;

namespace UnderdogCity
{
    public static class ArabicTextShaper
    {
        // Simple RTL Arabic Letter Reshaper for Unity UI
        public static string ShapeText(string input)
        {
            if (string.IsNullOrEmpty(input)) return input;

            // Basic check if input contains Arabic characters
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

            // Reverse string for Right-to-Left presentation
            char[] charArray = input.ToCharArray();
            Array.Reverse(charArray);
            return new string(charArray);
        }
    }
}
