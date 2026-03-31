#pragma once

#include "SkyRenderer.h"
#include <stdint.h>

class GUIRenderer {
public:
  GUIRenderer();
  ~GUIRenderer();

  void render(int screenWidth, int screenHeight);

private:
  SimpleTexture m_iconsTex;
  SkyVertex *m_vertices;
};
