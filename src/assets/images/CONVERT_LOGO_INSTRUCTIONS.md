# Logo Konvertierung für LVGL

## Schritte:

1. Gehe zu: https://lvgl.io/tools/imageconverter
2. Lade dein Logo hoch: `Logo/Logo.png`
3. Einstellungen:
   - **Name**: `logo` (wichtig!)
   - **Color format**: `RGB565` (für 16-bit Display)
   - **Output format**: `C array`
   - **Binary format**: `LVGL C array`
4. Klicke auf "Convert"
5. Lade die generierte `logo.c` Datei herunter
6. Speichere sie als `src/assets/images/logo.c`
7. Lösche diese Anleitung

## Empfohlene Logo-Größe:
- **Breite**: 300-360 Pixel (Display ist 360x360)
- **Höhe**: 300-360 Pixel
- **Format**: PNG mit transparentem Hintergrund (optional)

Wenn dein Logo zu groß ist, wird es automatisch skaliert, aber die beste Qualität erhältst du mit der nativen Auflösung.

