#include "DarkTheme.hpp"

void Themes::applyDarkTheme() {
    // Dark theme is the default, already set in Colors::init()
    Colors::applyTheme(Colors::Theme::DARK);
}

void Themes::applyLightTheme() {
    Colors::applyTheme(Colors::Theme::LIGHT);
}

void Themes::applyNeonTheme() {
    Colors::applyTheme(Colors::Theme::NEON);
}

void Themes::applyMatrixTheme() {
    Colors::applyTheme(Colors::Theme::MATRIX);
}