#include <M5Unified.h>
#include <SD.h>
#include <lgfx/v1/misc/DataWrapper.hpp>

// --- Variables de navigation ---
static int currentIndex = 1;
static const int maxImages = 7;
// Largeur de la photo fixée à 480px pour tes fichiers
static const int TEXT_X = 480, TEXT_Y = 0, TEXT_W = 800, TEXT_H = 720, PAD = 24;

// --- Tes variables d'affichage ---
String titre       = "";
String auteur      = "";
String annee       = "";
String description = "";

// --- Structure pour les données du catalogue ---
struct Oeuvre {
  String t;
  String aut;
  String ann;
  String desc;
};

Oeuvre catalogue[maxImages + 1] = {
  {"", "", "", ""},
  {"Le Cinema Louis-Lumiere", "Cite des Sciences", "1986", "Une salle immersive projetant des films scientifiques et des documentaires en haute definition."},
  {"La Bibliotheque", "Espace Etude", "2024", "Un lieu de savoir offrant des ressources illimitees sur les sciences et les technologies."},
  {"La Geode", "Adrien Fainsilber", "1985", "Ce dome miroir emblematique abrite un ecran hemispherique geant de 1000m2."},
  {"Cite des Metiers", "Beroepenpunt", "Espace Conseil", "Un centre de ressources pour s'orienter et construire son avenir professionnel."},
  {"Explora", "Multi-niveaux", "Permanent", "Le coeur de la Cite avec ses expositions sur l'espace, l'ocean et l'energie."},
  {"Cite des Enfants", "Espace Jeu", "Atelier 1", "Un lieu d'experimentation pour apprendre les sciences en s'amusant."},
  {"Le Grand Toboggan", "Espace Aventure", "Zone 7", "Une structure ludique permettant d'explorer la physique du mouvement."}
};

// ---- Wrapper File -> DataWrapper (Structure inchangée) ----
class FileDataWrapper : public lgfx::v1::DataWrapper {
public:
  explicit FileDataWrapper(fs::File& file) : _file(&file) {}
  int read(uint8_t* buf, uint32_t len) override { return _file ? _file->read(buf, len) : -1; }
  void skip(int32_t offset) override { if (!_file) return; _file->seek(_file->position() + offset); }
  bool seek(uint32_t offset) override { return _file ? _file->seek(offset) : false; }
  void close() override { if (_file && *_file) _file->close(); }
  int32_t tell() override { return _file ? (int32_t)_file->position() : -1; }
private:
  fs::File* _file = nullptr;
};

// Dessin des boutons de navigation
void drawUI() {
  M5.Display.fillTriangle(15, 360, 45, 330, 45, 390, TFT_WHITE);
  M5.Display.fillTriangle(1265, 360, 1235, 330, 1235, 390, TFT_WHITE);
  M5.Display.fillRoundRect(1170, 15, 100, 45, 8, TFT_DARKGRAY);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextDatum(middle_center);
  M5.Display.setTextSize(1);
  M5.Display.drawString("HOME", 1220, 38);
}

// Fonction pour encadrer la photo (Gardée mais non appelée)
void drawPhotoFrame() {
  uint16_t frameColor = TFT_WHITE;
  int thickness = 4;
  for (int i = 0; i < thickness; i++) {
    M5.Display.drawRect(i, i, TEXT_X - (2 * i), 720 - (2 * i), frameColor);
  }
}

static void drawWrappedText(const String& text, int x, int y, int w, int lineH) {
  int cursorY = y;
  int i = 0;
  String line = "";
  while (i < (int)text.length()) {
    int nextSpace = text.indexOf(' ', i);
    if (nextSpace == -1) nextSpace = text.length();
    String word = text.substring(i, nextSpace);
    String candidate = line + (line.length() > 0 ? " " : "") + word;
    if (M5.Display.textWidth(candidate) <= w) {
      line = candidate;
    } else {
      M5.Display.drawString(line, x, cursorY);
      line = word;
      cursorY += lineH;
    }
    i = nextSpace + 1;
    if (cursorY > (TEXT_Y + TEXT_H - lineH)) break;
  }
  M5.Display.drawString(line, x, cursorY);
}

static void drawScreen() {
  // Mise à jour de tes variables avant l'affichage
  titre = catalogue[currentIndex].t;
  auteur = catalogue[currentIndex].aut;
  annee = catalogue[currentIndex].ann;
  description = catalogue[currentIndex].desc;

  M5.Display.setFont(&fonts::FreeSansBold9pt7b);

  // Extension fixe en .png pour toutes les photos
  String dynamicPath = "/img/Zone " + String(currentIndex) + ".png";

  fs::File f = SD.open(dynamicPath, FILE_READ);
  if (!f) {
    M5.Display.fillScreen(TFT_BLACK); // FOND NOIR
    M5.Display.setTextColor(TFT_RED);
    M5.Display.drawString("Erreur SD: " + dynamicPath, 640, 360);
    return;
  }

  M5.Display.fillScreen(TFT_BLACK); // FOND NOIR
  FileDataWrapper dw(f);

  // On utilise drawPng pour tout
  M5.Display.drawPng(&dw, 0, 0);
  dw.close();

  // drawPhotoFrame(); // CADRE SUPPRIMÉ
  drawUI();

  M5.Display.setClipRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H);
  M5.Display.drawFastHLine(TEXT_X + PAD, 120, TEXT_W - (PAD*2), TFT_WHITE); // LIGNE BLANCHE

  int x = TEXT_X + PAD;
  int y = TEXT_Y + PAD;
  int w = TEXT_W - 2 * PAD;

  M5.Display.setTextColor(TFT_WHITE); // ECRITURE BLANCHE
  M5.Display.setTextDatum(top_left);
  M5.Display.setTextSize(1.5);
  M5.Display.drawString(titre, x, y);

  y += 100;
  M5.Display.setTextColor(TFT_WHITE); // ECRITURE BLANCHE
  M5.Display.setTextSize(1.0);
  M5.Display.drawString(auteur, x, y); y += 40;
  M5.Display.drawString(annee, x, y); y += 50;

  drawWrappedText(description, x, y, w, 28);
  M5.Display.clearClipRect();
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(255);

  if (!SD.begin()) {
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.drawString("SD ERROR", 640, 360);
    while (1) delay(100);
  }
  drawScreen();
}

void loop() {
  M5.update();
  if (M5.Touch.getCount() > 0) {
    auto detail = M5.Touch.getDetail();
    if (detail.wasClicked()) {
      if (detail.x > 1170 && detail.y < 70) {
        currentIndex = 1;
      }
      else if (detail.x > 640) {
        currentIndex++;
        if (currentIndex > maxImages) currentIndex = 1;
      }
      else {
        currentIndex--;
        if (currentIndex < 1) currentIndex = maxImages;
      }
      drawScreen();
    }
  }
  delay(10);
}