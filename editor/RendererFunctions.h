#pragma once
//Just tries to emulate the win32 style

//Fail
void DrawBox(SDL_Renderer* target, int x, int y, int sx, int sy) {
	DestR = { x, y, sx, sy };
	SDL_SetRenderDrawColor(target, 255, 255, 255, 255);
	SDL_RenderDrawRect(target, &DestR);

	DestR = { x, y, sx - 1, sy - 1 };
	SDL_SetRenderDrawColor(target, 160, 160, 160, 255);
	SDL_RenderDrawRect(target, &DestR);

	DestR = { x + 1, y + 1, sx - 2, sy - 2 };
	SDL_SetRenderDrawColor(target, 240, 240, 240, 255);
	SDL_RenderDrawRect(target, &DestR);
}

//Fail
void DrawFrameBox(SDL_Renderer* target, int x, int y, int sx, int sy) {
	DestR = { x, y, sx, sy };
	SDL_SetRenderDrawColor(target, 160, 160, 160, 255);
	SDL_RenderDrawRect(target, &DestR);

	DestR = { x+1, y+1, sx-1, sy-1 };
	SDL_SetRenderDrawColor(target, 105, 105, 105, 255);
	SDL_RenderDrawRect(target, &DestR);

	SDL_SetRenderDrawColor(target, 227, 227, 227, 255);
	DestR = { x + 1, y + sy - 2, sx - 2, 1 };
	SDL_RenderFillRect(target, &DestR);

	DestR = { x + sx - 2, y + 1, 1, sy - 2 };
	SDL_RenderFillRect(target, &DestR);

	SDL_SetRenderDrawColor(target, 255, 255, 255, 255);
	DestR = { x + sx - 1, y, 1, sy };
	SDL_RenderFillRect(target, &DestR);

	DestR = { x, y + sy - 1, sx, 1 };
	SDL_RenderFillRect(target, &DestR);
}

void DrawTextWindow(int renderPointX, int renderPointY, string text) {
	SDL_Texture* T_S = CurrentWindow->RequestTextureTransfer(UI_SURF);

	//Draw the text
	int origx = renderPointX;
	for (int i = 0; i < text.length(); i++) {
		uint_fast8_t caracter = uint_fast8_t(text[i]);
		if (caracter != 0x20) {
			int xS = 0; int yS = 0;
			if (caracter >= 0x41 && caracter <= 0x5A) { yS = 48; xS = (caracter - 0x41) * 6; }
			if (caracter >= 0x61 && caracter <= 0x7A) { yS = 58; xS = (caracter - 0x61) * 6; }
			if (caracter >= 0x30 && caracter <= 0x3A) { yS = 68; xS = (caracter - 0x30) * 6; }
			if ((caracter >= 0x21 && caracter <= 0x2F) || caracter == 0x3F) { yS = 78; xS = (caracter - 0x21) * 6; }
			if (caracter == 0x5C) { renderPointX = origx; renderPointY += 16; }

			if (yS > 0) {
				SrcR = { xS, yS, 6, 10 };
				DestR = { renderPointX, renderPointY, 6, 10 };
				SDL_RenderCopy(CurrentWindow->mRenderer, T_S, &SrcR, &DestR);
			}
		}
		renderPointX += 6;
	}
}

//Test
void DrawTextWindowA(int renderPointX, int renderPointY, string text) {
	//Draw the text
	int origx = renderPointX;
	for (int i = 0; i < text.length(); i++) {
		uint_fast8_t caracter = uint_fast8_t(text[i]);
		if (caracter != 0x20) {
			int xS = 0; int yS = 0;
			if (caracter >= 0x41 && caracter <= 0x5A) { yS = 48; xS = (caracter - 0x41) * 6; }
			if (caracter >= 0x61 && caracter <= 0x7A) { yS = 58; xS = (caracter - 0x61) * 6; }
			if (caracter >= 0x30 && caracter <= 0x3A) { yS = 68; xS = (caracter - 0x30) * 6; }
			if ((caracter >= 0x21 && caracter <= 0x2F) || caracter == 0x3F) { yS = 78; xS = (caracter - 0x21) * 6; }
			if (caracter == 0x5C) { renderPointX = origx; renderPointY += 16; }

			if (yS > 0) {
				SrcR = { xS, yS, 6, 10 };
				DestR = { renderPointX, renderPointY, 6, 10 };
				SDL_RenderCopy(ren, UI_TEX, &SrcR, &DestR);
			}
		}
		renderPointX += 6;
	}
}